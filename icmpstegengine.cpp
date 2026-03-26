#include "icmpstegengine.h"
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include<QSocketNotifier>
#include<QDir>
#include"constants.h"
#include<QHostInfo>

ICMPStegEngine::ICMPStegEngine(QObject *parent)
    : QObject(parent)
    , m_sockfd(-1)
    , m_notifier(nullptr)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setInterval(500);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ICMPStegEngine::checkTimeouts);
}

ICMPStegEngine::~ICMPStegEngine()
{
    stopListening();
}

bool ICMPStegEngine::initRawSocket()
{
    // Create raw ICMP socket
    m_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (m_sockfd < 0) {
        emit error("Need root privileges. Run with sudo or: setcap cap_net_raw+ep ./ermis");
        return false;
    }

    // Set non-blocking for Qt integration
    fcntl(m_sockfd, F_SETFL, O_NONBLOCK);

    return true;
}

bool ICMPStegEngine::startListening()
{
    if (m_sockfd < 0 && !initRawSocket()) {
        return false;
    }

    m_notifier = new QSocketNotifier(m_sockfd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, 
            this, &ICMPStegEngine::handleIncomingPacket);

    m_timeoutTimer->start();
    return true;
}

void ICMPStegEngine::stopListening()
{
    /*
    if (m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }
    if (m_sockfd >= 0) {
        close(m_sockfd);
        m_sockfd = -1;
    }
    m_timeoutTimer->stop();
    */

    if (m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }
    if (m_sockfd >= 0) {
        close(m_sockfd);
        m_sockfd = -1;
    }

    // Clear all pending sessions
    m_sendSessions.clear();
    m_receiveSessions.clear();

    m_currentSessionId = 0;
    m_cancelled = false;

    m_timeoutTimer->stop();
}



void ICMPStegEngine::sendNextPacket(const QString& targetIp, quint32 sessionId, int seq)
{
    auto& session = m_sendSessions[sessionId];
    
    // Extract chunk
    int start = seq * MAX_PAYLOAD;
    int end = qMin(start + MAX_PAYLOAD, session.data.size());
    QByteArray chunk = session.data.mid(start, end - start);

    // Create header
    PacketHeader header;
    header.magic = 0x45524D49;  // "ERMI"
    header.sessionId = sessionId;
    header.sequence = seq;
    header.total = session.totalPackets;
    header.dataSize = chunk.size();
    header.checksum = qChecksum(chunk.constData(), chunk.size());




    // Create packet
    QByteArray packet = createICMPPacket(header, chunk);

    // Send
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, targetIp.toLatin1().constData(), &dest.sin_addr);

    sendto(m_sockfd, packet.constData(), packet.size(), 0,
           (struct sockaddr*)&dest, sizeof(dest));

    // Track send time
    session.sentTime[seq].start();
    
    emit sendProgress(seq, session.totalPackets);
}

void ICMPStegEngine::handleIncomingPacket()
{
    struct sockaddr_in src;
    socklen_t srcLen = sizeof(src);
    char buffer[4096];

    int len = recvfrom(m_sockfd, buffer, sizeof(buffer), 0,
                       (struct sockaddr*)&src, &srcLen);
    
    if (len < (int)(sizeof(struct iphdr) + sizeof(struct icmphdr))) {
        return; // Too small
    }

    // Parse packet
    PacketHeader header;
    QByteArray payload;
    QString sourceIp = inet_ntoa(src.sin_addr);

    // ===== APPLY IP FILTER =====
    if (Constants::ipFilterEnabled) {
        bool allowed = true;

        if (Constants::ipFilterMode == 0) { // Block mode
            for (const QString& entry : Constants::ipFilterEntries) {
                if (entry == sourceIp) {
                    allowed = false;
                    break;
                }
            }
        } else { // Allow mode
            allowed = false;
            for (const QString& entry : Constants::ipFilterEntries) {
                if (entry == sourceIp) {
                    allowed = true;
                    break;
                }
            }
        }

        if (!allowed) {
            return;
        }
    }
    // ===== END FILTER =====


    if (!parseICMPPacket(QByteArray(buffer, len), header, payload, sourceIp)) {
        return; // Not our packet or invalid
    }

    // Handle ACK/NACK packets (special small packets)
    if (payload.size() == 0) {
        // This is an ACK/NACK control packet
        handleControlPacket(header, sourceIp);
        return;
    }

    // Data packet received
    handleDataPacket(header, payload, sourceIp);
}

void ICMPStegEngine::handleDataPacket(const PacketHeader& header,
                                      const QByteArray& payload,
                                      const QString& sourceIp)
{
    //quint32 sessionId = header.magic;  // Actually magic, but we need session ID
    // For simplicity, we'll use a hash of source IP + magic as session ID
    //sessionId = qHash(sourceIp + QString::number(header.magic));
    quint32 sessionId = header.sessionId;
    // Create or get receive session

    if (!m_receiveSessions.contains(sessionId)) {
        ReceiveSession session;
        session.totalPackets = header.total;
        session.completionTimer = new QTimer(this);
        session.completionTimer->setSingleShot(true);
        session.completionTimer->setInterval(5000);
        connect(session.completionTimer, &QTimer::timeout, [this, sessionId]() {
            // Timeout - assume transfer complete
            m_receiveSessions.remove(sessionId);
        });
        m_receiveSessions[sessionId] = session;
    }

    auto& session = m_receiveSessions[sessionId];
    
    // Verify checksum
    quint16 computedChecksum = qChecksum(payload.constData(), payload.size());




    if (computedChecksum != header.checksum) {
        // Request retransmission
        requestRetransmit(sourceIp, sessionId, header.sequence);
        return;
    }

    // Store chunk
    session.chunks[header.sequence] = payload;
    emit receiveProgress(header.sequence, header.total);

    // Send ACK
    sendACK(sourceIp, sessionId, header.sequence);
    // Reset completion timer
    session.completionTimer->start();

    // Check if complete
    if (session.chunks.size() == header.total) {
        assembleFile(sessionId, sourceIp);
    }
}




void ICMPStegEngine::checkTimeouts()
{

    if (m_cancelled) {
        m_cancelled = false;
        return;
    }

    // Check for unacknowledged packets and retransmit
    for (auto it = m_sendSessions.begin(); it != m_sendSessions.end(); ) {
        bool allAcked = true;
        
        for (int i = 0; i < it->totalPackets; i++) {
            if (!it->acked[i]) {
                allAcked = false;
                
                if (it->sentTime.contains(i) && 
                    it->sentTime[i].elapsed() > TIMEOUT_MS) {
                    
                    if (it->retryCount[i] < MAX_RETRIES) {
                        it->retryCount[i]++;
                        // Retransmit
                        //sendNextPacket("", it.key(), i);  // Need target IP stored
                        sendNextPacket(it->targetIp, it.key(), i);
                    } else {
                        emit error(QString("Packet %1 failed after %2 retries")
                                  .arg(i).arg(MAX_RETRIES));
                    }
                }
            }
        }
        
        if (allAcked) {
            // Transfer complete
            emit sendProgress(it->totalPackets, it->totalPackets);
            it = m_sendSessions.erase(it);
        } else {
            ++it;
        }
    }
}


void ICMPStegEngine::handleControlPacket(const PacketHeader& header, const QString& sourceIp)
{
    // Control packets have empty payload and special magic numbers
    switch (header.magic) {
        case 0x41434B00: {  // "ACK" magic
            // ACK received - mark packet as acknowledged
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                auto& session = m_sendSessions[sessionId];
                session.acked[header.sequence] = true;

                // Slide window forward if possible
                checkAndSlideWindow(sessionId);
            }
            break;
        }

        case 0x4E41434B: {  // "NACK" magic
            // NACK received - packet was lost/corrupted
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                auto& session = m_sendSessions[sessionId];

                if (session.retryCount[header.sequence] < MAX_RETRIES) {
                    session.retryCount[header.sequence]++;

                    // Immediate retransmission
                    sendNextPacket(sourceIp, sessionId, header.sequence);
                } else {
                    emit error(QString("Packet %1 failed after NACK").arg(header.sequence));
                }
            }
            break;
        }

        case 0x52545800: {  // "RTX" magic - Retransmit request
            // Receiver explicitly requests retransmission
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                auto& session = m_sendSessions[sessionId];

                if (session.retryCount[header.sequence] < MAX_RETRIES) {
                    session.retryCount[header.sequence]++;
                    sendNextPacket(sourceIp, sessionId, header.sequence);
                }
            }
            break;
        }

        case 0x444F4E45: {  // "DONE" magic - Transfer complete
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                m_sendSessions.remove(sessionId);
                emit sendProgress(header.total, header.total);  // 100% complete
            }
            break;
        }

        default:
            break;
    }
}



// Helper to slide window forward
void ICMPStegEngine::checkAndSlideWindow(quint32 sessionId)
{
    auto& session = m_sendSessions[sessionId];

    // Find the next unacked packet
    int nextUnacked = -1;
    for (int i = 0; i < session.totalPackets; i++) {
        if (!session.acked[i]) {
            nextUnacked = i;
            break;
        }
    }

    if (nextUnacked == -1) {
        // All packets acknowledged!
        sendDonePacket(sessionId);
        return;
    }

    // Slide window and send more packets if needed
    int windowEnd = qMin(nextUnacked + WINDOW_SIZE, session.totalPackets);
    for (int i = nextUnacked; i < windowEnd; i++) {
        if (!session.acked[i] && !session.sentTime.contains(i)) {
            // This packet hasn't been sent yet
            sendNextPacket(session.targetIp, sessionId, i);
        }
    }
}



void ICMPStegEngine::sendNACK(const QString& target, quint32 sessionid, quint16 sequence)
{
    PacketHeader header;
    header.magic = 0x4E41434B;  // "NACK"
    header.sequence = sequence;
    header.sessionId = sessionid;
    header.total = 0;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray packet = createICMPPacket(header, QByteArray());

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, target.toLatin1().constData(), &dest.sin_addr);

    sendto(m_sockfd, packet.constData(), packet.size(), 0,
           (struct sockaddr*)&dest, sizeof(dest));
}

void ICMPStegEngine::sendDonePacket(quint32 sessionId)
{
    if (!m_sendSessions.contains(sessionId)) {
        return;
    }

    QString targetIp = getTargetIpForSession(sessionId);
    if (targetIp.isEmpty()) {
        return;
    }

    PacketHeader header;
    header.magic = 0x444F4E45;  // "DONE" in hex
    header.sequence = m_sendSessions[sessionId].totalPackets - 1;  // Last packet sequence
    header.total = m_sendSessions[sessionId].totalPackets;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray packet = createICMPPacket(header, QByteArray());

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    if (inet_pton(AF_INET, targetIp.toLatin1().constData(), &dest.sin_addr) <= 0) {
        return;
    }

    ssize_t sent = sendto(m_sockfd, packet.constData(), packet.size(), 0,
                          (struct sockaddr*)&dest, sizeof(dest));

}

QByteArray ICMPStegEngine::createICMPPacket(const PacketHeader& header, const QByteArray& payload)
{
    // Calculate total packet size
    int totalSize = sizeof(struct icmphdr) + sizeof(PacketHeader) + payload.size();
    QByteArray packet;
    packet.resize(totalSize);

    // ICMP header
    struct icmphdr *icmp = reinterpret_cast<struct icmphdr*>(packet.data());
    icmp->type = 8;                    // Echo Request
    icmp->code = 0;                     // Always 0 for echo
    icmp->un.echo.id = htons(getpid() & 0xFFFF);  // Identifier (process ID)
    icmp->un.echo.sequence = htons(m_seq++);       // Sequence number
    icmp->checksum = 0;                 // Will calculate after filling packet

    // Our protocol header (after ICMP header)
    char* protoHeader = packet.data() + sizeof(struct icmphdr);
    memcpy(protoHeader, &header, sizeof(PacketHeader));

    // Payload (after our header)
    if (!payload.isEmpty()) {
        char* dataPtr = protoHeader + sizeof(PacketHeader);
        memcpy(dataPtr, payload.constData(), payload.size());
    }

    // Calculate ICMP checksum (must be done last)
    // Checksum covers the entire ICMP packet (header + our header + payload)
    icmp->checksum = calculateChecksum(packet);

    return packet;
}

quint16 ICMPStegEngine::calculateChecksum(const QByteArray& data)
{
    quint32 sum = 0;
    const quint16* words = reinterpret_cast<const quint16*>(data.constData());
    int wordCount = data.size() / 2;

    // Sum all 16-bit words
    for (int i = 0; i < wordCount; i++) {
        sum += ntohs(words[i]);
    }

    // If odd number of bytes, add the last byte
    if (data.size() % 2) {
        sum += static_cast<quint8>(data[data.size() - 1]) << 8;
    }

    // Fold 32-bit sum to 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum & 0xFFFF;
}

// ===== GET TARGET IP FOR SESSION =====
QString ICMPStegEngine::getTargetIpForSession(quint32 sessionId)
{
    if (m_sendSessions.contains(sessionId)) {
        return m_sendSessions[sessionId].targetIp;
    }
    return QString();
}

// ===== PARSE ICMP PACKET =====
bool ICMPStegEngine::parseICMPPacket(const QByteArray& rawPacket,
                                      PacketHeader& header,
                                      QByteArray& payload,
                                      QString& sourceIp)
{
    if (rawPacket.size() < (int)(sizeof(struct iphdr) + sizeof(struct icmphdr))) {
        return false;
    }

    const struct iphdr* ip = reinterpret_cast<const struct iphdr*>(rawPacket.constData());
    int ipHeaderLen = ip->ihl * 4;

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip->saddr), ipStr, INET_ADDRSTRLEN);
    sourceIp = QString(ipStr);

    int icmpOffset = ipHeaderLen;
    const struct icmphdr* icmp = reinterpret_cast<const struct icmphdr*>(
        rawPacket.constData() + icmpOffset
    );

    if (icmp->type != 0 && icmp->type != 8) {
        return false;
    }

    int protoOffset = icmpOffset + sizeof(struct icmphdr);
    if (rawPacket.size() < protoOffset + (int)sizeof(PacketHeader)) {
        return false;
    }

    memcpy(&header, rawPacket.constData() + protoOffset, sizeof(PacketHeader));

    if (header.magic != 0x45524D49 && header.magic != 0x41434B00 &&
        header.magic != 0x4E41434B && header.magic != 0x52545800 &&
        header.magic != 0x444F4E45) {
        return false;
    }

    int payloadOffset = protoOffset + sizeof(PacketHeader);
    if (rawPacket.size() > payloadOffset) {
        payload = rawPacket.mid(payloadOffset);
    }

    return true;
}

// ===== REQUEST RETRANSMIT =====
void ICMPStegEngine::requestRetransmit(const QString& target, quint32 sessionid, quint16 sequence)
{
    PacketHeader header;
    header.magic = 0x52545800;  // "RTX"
    header.sequence = sequence;
    header.sessionId = sessionid;
    header.total = 0;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray packet = createICMPPacket(header, QByteArray());

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, target.toLatin1().constData(), &dest.sin_addr);

    sendto(m_sockfd, packet.constData(), packet.size(), 0,
           (struct sockaddr*)&dest, sizeof(dest));

}

// ===== SEND NEXT WINDOW =====
void ICMPStegEngine::sendNextWindow()
{
    for (auto it = m_sendSessions.begin(); it != m_sendSessions.end(); ++it) {
        checkAndSlideWindow(it.key());
    }
}

void ICMPStegEngine::setEncryptionKey(const QString& passphrase)
{
    m_encryptionPassphrase = passphrase;
    m_encryptionEnabled = true;
}

void ICMPStegEngine::clearEncryption()
{
    m_encryptionEnabled = false;
    m_encryptionPassphrase.clear();
}

void ICMPStegEngine::sendACK(const QString& target, quint32 sessionId, quint16 sequence)
{
    PacketHeader header;
    header.magic = 0x41434B00;  // "ACK"
    header.sessionId = sessionId;
    header.sequence = sequence;
    header.total = 0;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray packet = createICMPPacket(header, QByteArray());

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, target.toLatin1().constData(), &dest.sin_addr);

    sendto(m_sockfd, packet.constData(), packet.size(), 0,
           (struct sockaddr*)&dest, sizeof(dest));
}

//////////////////////////////////////



void ICMPStegEngine::assembleFile(quint32 sessionId, const QString& sourceIp)
{
    auto& session = m_receiveSessions[sessionId];

    QByteArray complete;

    for (int i = 0; i < session.totalPackets; i++) {
        if (!session.chunks.contains(i)) {
            return;
        }
        complete.append(session.chunks[i]);
    }


    // ===== DECRYPT =====
    if (complete.startsWith("ENCR")) {

     if (!m_encryptionEnabled) {
         emit dataReceived(complete, sourceIp);
         m_receiveSessions.remove(sessionId);
         return;
     }

     QByteArray decrypted = m_crypto.decryptData(complete.mid(4), m_encryptionPassphrase);

     if (decrypted.isEmpty()) {
         emit dataReceived(complete, sourceIp);
         m_receiveSessions.remove(sessionId);
         return;
     }

     // Replace complete with decrypted data
     complete = decrypted;
     // ← NO RETURN HERE - CONTINUE TO TYPE HANDLING
 }

    // ===== TYPE HANDLING =====
    if (complete.startsWith("FILE")) {

        QByteArray filePayload = complete.mid(4);

        QDataStream ds(filePayload);
        ds.setVersion(QDataStream::Qt_6_0);

        QString filename;
        qint64 fileSize;
        QByteArray hash;

        ds >> filename >> fileSize >> hash;

        if (ds.status() != QDataStream::Ok) {
            m_receiveSessions.remove(sessionId);
            return;
        }

        QByteArray fileData = filePayload.mid(ds.device()->pos(), fileSize);
        QByteArray computedHash = QCryptographicHash::hash(fileData, QCryptographicHash::Sha256);

        if (computedHash != hash) {
            m_receiveSessions.remove(sessionId);
            return;
        }

        QString savePath = Constants::ermistmpPath + "/" +
            QString("ermis_%1_%2")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"))
            .arg(filename);

        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileData);
            file.close();

            emit fileReceived(savePath, sourceIp);
        }

    } else if (complete.startsWith("TEXT")) {

        QByteArray text = complete.mid(4);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            QString savePath = Constants::ermistmpPath + "/" +
                QString("text_%1.txt").arg(timestamp);

            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(text);
                file.close();
                emit fileReceived(savePath, sourceIp);  // ← Call fileReceived instead
            } else {
                emit dataReceived(text, sourceIp);     // ← Fallback to dataReceived
            }

    } else {
        emit dataReceived(complete, sourceIp);
    }

    m_receiveSessions.remove(sessionId);
}


bool ICMPStegEngine::sendData(const QString& targetIp, const QByteArray& data)
{
    if (m_sockfd < 0 && !initRawSocket()) {
        return false;
    }

    QByteArray dataToSend = data;  // ← Start with original data

    // ===== APPLY ENCRYPTION ONCE =====
    if (m_encryptionEnabled && !dataToSend.startsWith("ENCR")) {
        QByteArray encrypted = m_crypto.encryptData(dataToSend, m_encryptionPassphrase);
        if (encrypted.isEmpty()) {
            emit error("Encryption failed");
            return false;
        }
        dataToSend = "ENCR" + encrypted;
    }

    // ===== SESSION SETUP =====
    quint32 sessionId = QRandomGenerator::global()->generate();
    int totalPackets = (dataToSend.size() + MAX_PAYLOAD - 1) / MAX_PAYLOAD;

    SendSession session;
    m_currentSessionId = sessionId;
    m_cancelled = false;
    session.data = dataToSend;
    session.totalPackets = totalPackets;
    session.targetIp = targetIp;
    session.timeoutTimer = new QTimer(this);

    for (int i = 0; i < totalPackets; i++) {
        session.acked[i] = false;
        session.retryCount[i] = 0;
    }

    m_sendSessions[sessionId] = session;

    // ===== SEND INITIAL WINDOW =====
    for (int i = 0; i < qMin(WINDOW_SIZE, totalPackets); i++) {
        sendNextPacket(targetIp, sessionId, i);
    }

    return true;
}



/*
bool ICMPStegEngine::sendFile(const QString& targetIp, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Cannot open file: " + filePath);
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QByteArray manifest;
    QDataStream ms(&manifest, QIODevice::WriteOnly);
    ms.setVersion(QDataStream::Qt_6_0);

    ms << QFileInfo(filePath).fileName();
    ms << fileData.size();
    ms << QCryptographicHash::hash(fileData, QCryptographicHash::Sha256);

    // FINAL FORMAT: FILE + manifest + data
    QByteArray payload = "FILE" + manifest + fileData;

    return sendData(resolveDomainToIp(targetIp), payload);
}
*/
bool ICMPStegEngine::sendFile(const QString& targetIpOrDomain, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Cannot open file: " + filePath);
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QByteArray manifest;
    QDataStream ms(&manifest, QIODevice::WriteOnly);
    ms.setVersion(QDataStream::Qt_6_0);

    ms << QFileInfo(filePath).fileName();
    ms << fileData.size();
    ms << QCryptographicHash::hash(fileData, QCryptographicHash::Sha256);

    // FINAL FORMAT: FILE + manifest + data
    QByteArray payload = "FILE" + manifest + fileData;

    QString ip = resolveDomainToIp(targetIpOrDomain);
    if (ip.isEmpty()) {
        emit error("Cannot resolve target: " + targetIpOrDomain);
        return false;
    }

    return sendData(ip, payload);
}


QString ICMPStegEngine::resolveDomainToIp(const QString& target)
{
    // Check if it's already a valid IP
    QHostAddress ipAddr;
    if (ipAddr.setAddress(target)) {
        return target; // already an IP
    }

    // Resolve domain
    QHostInfo info = QHostInfo::fromName(target);
    if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
        return {}; // empty string = failed to resolve
    }

    return info.addresses().first().toString();
}

void ICMPStegEngine::cancelCurrentTransfer()
{
    m_cancelled = true;
    if (m_currentSessionId != 0 && m_sendSessions.contains(m_currentSessionId)) {
        m_sendSessions.remove(m_currentSessionId);
        m_currentSessionId = 0;
    }
}

void ICMPStegEngine::forceReset()
{
    // Stop listening and clean up
    stopListening();

    // Clear all sessions
    m_sendSessions.clear();
    m_receiveSessions.clear();

    // Reset session tracking
    m_currentSessionId = 0;
    m_cancelled = false;

    // Close and recreate raw socket if needed
    if (m_sockfd >= 0) {
        close(m_sockfd);
        m_sockfd = -1;
    }

    if (m_notifier) {
        delete m_notifier;
        m_notifier = nullptr;
    }

    // Reinitialize socket for next use
    initRawSocket();

}
