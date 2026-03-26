#include "dnsstegengine.h"
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QDir>
#include "constants.h"
#include <QCoreApplication>

// DNS packet constants
#define DNS_HEADER_SIZE 12

DNSStegEngine::DNSStegEngine(QObject *parent)
    : QObject(parent)
    , m_dnsSocket(nullptr)
    , m_listening(false)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setInterval(500);
    connect(m_timeoutTimer, &QTimer::timeout, this, &DNSStegEngine::checkTimeouts);
}

DNSStegEngine::~DNSStegEngine()
{
    stopListening();
}

bool DNSStegEngine::initSocket()
{
    if (!m_dnsSocket) {
        m_dnsSocket = new QUdpSocket(this);
    }
    return true;
}

bool DNSStegEngine::startListening(quint16 port)
{
    if (!initSocket()) return false;

    m_listenDnsPort = port;

    if (!m_dnsSocket->bind(QHostAddress::AnyIPv4, m_listenDnsPort)) {
        emit error("Failed to bind DNS port " + QString::number(port));
        return false;
    }
    connect(m_dnsSocket, &QUdpSocket::readyRead, this, &DNSStegEngine::onDnsQuery);

    m_listening = true;
    m_timeoutTimer->start();
    return true;
}

void DNSStegEngine::stopListening()
{
    /*
    if (m_dnsSocket) {
        m_dnsSocket->close();
        disconnect(m_dnsSocket, &QUdpSocket::readyRead, this, &DNSStegEngine::onDnsQuery);
    }
    m_listening = false;
    m_timeoutTimer->stop();
    */

    if (m_dnsSocket) {
        // Only flush if socket is in valid state
        if (m_dnsSocket->state() == QUdpSocket::BoundState) {
            while (m_dnsSocket->hasPendingDatagrams()) {
                m_dnsSocket->readDatagram(nullptr, 0);
            }
        }
        m_dnsSocket->close();
        disconnect(m_dnsSocket, &QUdpSocket::readyRead, this, &DNSStegEngine::onDnsQuery);
    }

    m_sendSessions.clear();
    m_receiveSessions.clear();
    m_completedSessions.clear();

    m_currentSessionId = 0;
    m_cancelled = false;

    m_listening = false;
    m_timeoutTimer->stop();
}

void DNSStegEngine::onDnsQuery()
{
    while (m_dnsSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_dnsSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_dnsSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        //
        QString sourceIp = sender.toString();

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
                continue; // Skip this packet (both queries and responses)
            }
        }
        // ===== END FILTER =====
        //


        PacketHeader header;
        QByteArray payload;
        //QString sourceIp = sender.toString();

        if (!parseDNS(datagram, header, payload, sourceIp)) {
            continue;
        }

        if (payload.isEmpty()) {
            handleControlPacket(header, sourceIp);
        } else {
            handleDataPacket(header, payload, sourceIp);
        }
    }
}



void DNSStegEngine::sendNextPacket(const QString& targetIp, quint16 targetPort, quint32 sessionId, int seq)
{
    auto& session = m_sendSessions[sessionId];

    int start = seq * MAX_PAYLOAD;
    int end = qMin(start + MAX_PAYLOAD, session.data.size());
    QByteArray chunk = session.data.mid(start, end - start);

    PacketHeader header;
    header.magic = 0x45524D49;
    header.sessionId = sessionId;
    header.sequence = seq;
    header.total = session.totalPackets;
    header.dataSize = chunk.size();
    header.checksum = qChecksum(chunk);

    QByteArray dnsResponse = createDNSResponse(header, chunk);
    m_dnsSocket->writeDatagram(dnsResponse, QHostAddress(targetIp), targetPort);

    session.sentTime[seq].start();
    emit sendProgress(seq, session.totalPackets);
}

void DNSStegEngine::handleDataPacket(const PacketHeader& header, const QByteArray& payload, const QString& sourceIp)
{
    quint32 sessionId = header.sessionId;

    if (m_completedSessions.contains(sessionId)) {
        sendACK(sourceIp, m_listenDnsPort, sessionId, header.sequence);
        return;
    }

    if (!m_receiveSessions.contains(sessionId)) {
        ReceiveSession session;
        session.totalPackets = header.total;
        session.completionTimer = new QTimer(this);
        session.completionTimer->setSingleShot(true);
        session.completionTimer->setInterval(5000);
        connect(session.completionTimer, &QTimer::timeout, [this, sessionId]() {
            m_receiveSessions.remove(sessionId);
        });
        m_receiveSessions[sessionId] = session;
    }

    auto& session = m_receiveSessions[sessionId];

    quint16 computedChecksum = qChecksum(payload);
    if (computedChecksum != header.checksum) {
        requestRetransmit(sourceIp, m_listenDnsPort, sessionId, header.sequence);
        return;
    }

    session.chunks[header.sequence] = payload;
    emit receiveProgress(header.sequence, header.total);

    sendACK(sourceIp, m_listenDnsPort, sessionId, header.sequence);
    session.completionTimer->start();

    if (session.chunks.size() == header.total) {
        m_completedSessions.insert(sessionId);
        assembleFile(sessionId, sourceIp);
    }
}

void DNSStegEngine::assembleFile(quint32 sessionId, const QString& sourceIp)
{
    if (!m_receiveSessions.contains(sessionId)) return;

    auto& session = m_receiveSessions[sessionId];

    QByteArray complete;

    for (int i = 0; i < session.totalPackets; i++) {
        if (!session.chunks.contains(i)) {
            return;
        }
        complete.append(session.chunks[i]);
    }

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
        complete = decrypted;
    }

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
    }
    else if (complete.startsWith("TEXT")) {
        QByteArray text = complete.mid(4);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString savePath = Constants::ermistmpPath + "/" +
            QString("text_%1.txt").arg(timestamp);

        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(text);
            file.close();
            emit fileReceived(savePath, sourceIp);
        } else {
            emit dataReceived(text, sourceIp);
        }
    }
    else {
        emit dataReceived(complete, sourceIp);
    }

    m_receiveSessions.remove(sessionId);

    QTimer::singleShot(10000, [this, sessionId]() {
        m_completedSessions.remove(sessionId);
    });
}

bool DNSStegEngine::sendData(const QString& targetIp, const QByteArray& data, quint16 dnsPort)
{
    if (!initSocket()) return false;

    QByteArray dataToSend = data;

    if (m_encryptionEnabled && !dataToSend.startsWith("ENCR")) {
        QByteArray encrypted = m_crypto.encryptData(dataToSend, m_encryptionPassphrase);
        if (encrypted.isEmpty()) {
            emit error("Encryption failed");
            return false;
        }
        dataToSend = "ENCR" + encrypted;
    }

    quint32 sessionId = QRandomGenerator::global()->generate();
    int totalPackets = (dataToSend.size() + MAX_PAYLOAD - 1) / MAX_PAYLOAD;

    m_currentSessionId = sessionId;
    m_cancelled = false;

    SendSession session;
    session.data = dataToSend;
    session.totalPackets = totalPackets;
    session.targetIp = targetIp;
    session.targetPort = dnsPort;
    session.timeoutTimer = new QTimer(this);

    for (int i = 0; i < totalPackets; i++) {
        session.acked[i] = false;
        session.retryCount[i] = 0;
    }

    m_sendSessions[sessionId] = session;

    for (int i = 0; i < qMin(WINDOW_SIZE, totalPackets); i++) {
        sendNextPacket(targetIp, dnsPort, sessionId, i);
    }

    return true;
}

bool DNSStegEngine::sendFile(const QString& targetIpOrDomain, const QString& filePath, quint16 dnsPort)
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

    QByteArray payload = "FILE" + manifest + fileData;

    QString ip = resolveDomainToIp(targetIpOrDomain);
    if (ip.isEmpty()) {
        emit error("Cannot resolve target: " + targetIpOrDomain);
        return false;
    }

    return sendData(ip, payload, dnsPort);
}

QString DNSStegEngine::resolveDomainToIp(const QString& target)
{
    QHostAddress ipAddr;
    if (ipAddr.setAddress(target)) {
        return target;
    }

    QHostInfo info = QHostInfo::fromName(target);
    if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
        return {};
    }

    return info.addresses().first().toString();
}

void DNSStegEngine::sendACK(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence)
{
    PacketHeader header;
    header.magic = 0x41434B00;
    header.sessionId = sessionId;
    header.sequence = sequence;
    header.total = 0;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray dnsResponse = createDNSResponse(header, QByteArray());
    m_dnsSocket->writeDatagram(dnsResponse, QHostAddress(target), targetPort);
}

void DNSStegEngine::sendNACK(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence)
{
    PacketHeader header;
    header.magic = 0x4E41434B;
    header.sessionId = sessionId;
    header.sequence = sequence;
    header.total = 0;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray dnsResponse = createDNSResponse(header, QByteArray());
    m_dnsSocket->writeDatagram(dnsResponse, QHostAddress(target), targetPort);
}

void DNSStegEngine::requestRetransmit(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence)
{
    PacketHeader header;
    header.magic = 0x52545800;
    header.sessionId = sessionId;
    header.sequence = sequence;
    header.total = 0;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray dnsResponse = createDNSResponse(header, QByteArray());
    m_dnsSocket->writeDatagram(dnsResponse, QHostAddress(target), targetPort);
}

void DNSStegEngine::sendDonePacket(quint32 sessionId)
{
    if (!m_sendSessions.contains(sessionId)) return;

    QString targetIp = getTargetIpForSession(sessionId);
    quint16 targetPort = getTargetPortForSession(sessionId);
    if (targetIp.isEmpty()) return;

    PacketHeader header;
    header.magic = 0x444F4E45;
    header.sessionId = sessionId;
    header.sequence = m_sendSessions[sessionId].totalPackets - 1;
    header.total = m_sendSessions[sessionId].totalPackets;
    header.dataSize = 0;
    header.checksum = 0;

    QByteArray dnsResponse = createDNSResponse(header, QByteArray());
    m_dnsSocket->writeDatagram(dnsResponse, QHostAddress(targetIp), targetPort);
}

void DNSStegEngine::checkTimeouts()
{
    if (m_cancelled) {
        m_cancelled = false;
        return;
    }

    for (auto it = m_sendSessions.begin(); it != m_sendSessions.end(); ) {
        bool allAcked = true;

        for (int i = 0; i < it->totalPackets; i++) {
            if (!it->acked[i]) {
                allAcked = false;
                if (it->sentTime.contains(i) && it->sentTime[i].elapsed() > TIMEOUT_MS) {
                    if (it->retryCount[i] < MAX_RETRIES) {
                        it->retryCount[i]++;
                        sendNextPacket(it->targetIp, it->targetPort, it.key(), i);
                    } else {
                        emit error(QString("Packet %1 failed after %2 retries").arg(i).arg(MAX_RETRIES));
                    }
                }
            }
        }

        if (allAcked) {
            ++it;
        } else {
            ++it;
        }
    }
}

void DNSStegEngine::sendNextWindow()
{
    for (auto it = m_sendSessions.begin(); it != m_sendSessions.end(); ++it) {
        checkAndSlideWindow(it.key());
    }
}

void DNSStegEngine::checkAndSlideWindow(quint32 sessionId)
{
    if (m_cancelled) return;

    auto& session = m_sendSessions[sessionId];

    int nextUnacked = -1;
    for (int i = 0; i < session.totalPackets; i++) {
        if (!session.acked[i]) {
            nextUnacked = i;
            break;
        }
    }

    if (nextUnacked == -1) {
        sendDonePacket(sessionId);
        emit sendProgress(session.totalPackets, session.totalPackets);
        return;
    }

    int windowEnd = qMin(nextUnacked + WINDOW_SIZE, session.totalPackets);
    for (int i = nextUnacked; i < windowEnd; i++) {
        if (!session.acked[i] && !session.sentTime.contains(i)) {
            sendNextPacket(session.targetIp, session.targetPort, sessionId, i);
        }
    }
}

void DNSStegEngine::handleControlPacket(const PacketHeader& header, const QString& sourceIp)
{
    switch (header.magic) {
        case 0x41434B00: {
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                auto& session = m_sendSessions[sessionId];
                session.acked[header.sequence] = true;
                checkAndSlideWindow(sessionId);
            }
            break;
        }
        case 0x4E41434B: {
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                auto& session = m_sendSessions[sessionId];
                if (session.retryCount[header.sequence] < MAX_RETRIES) {
                    session.retryCount[header.sequence]++;
                    sendNextPacket(session.targetIp, session.targetPort, sessionId, header.sequence);
                }
            }
            break;
        }
        case 0x52545800: {
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                auto& session = m_sendSessions[sessionId];
                if (session.retryCount[header.sequence] < MAX_RETRIES) {
                    session.retryCount[header.sequence]++;
                    sendNextPacket(session.targetIp, session.targetPort, sessionId, header.sequence);
                }
            }
            break;
        }
        case 0x444F4E45: {
            quint32 sessionId = header.sessionId;
            if (m_sendSessions.contains(sessionId)) {
                emit sendProgress(m_sendSessions[sessionId].totalPackets,
                                  m_sendSessions[sessionId].totalPackets);
                m_sendSessions.remove(sessionId);
            }
            break;
        }
        default:
            break;
    }
}

QString DNSStegEngine::getTargetIpForSession(quint32 sessionId)
{
    if (m_sendSessions.contains(sessionId)) {
        return m_sendSessions[sessionId].targetIp;
    }
    return QString();
}

quint16 DNSStegEngine::getTargetPortForSession(quint32 sessionId)
{
    if (m_sendSessions.contains(sessionId)) {
        return m_sendSessions[sessionId].targetPort;
    }
    return 0;
}

void DNSStegEngine::setEncryptionKey(const QString& passphrase)
{
    m_encryptionPassphrase = passphrase;
    m_encryptionEnabled = true;
}

void DNSStegEngine::clearEncryption()
{
    m_encryptionEnabled = false;
    m_encryptionPassphrase.clear();
}

void DNSStegEngine::cancelCurrentTransfer()
{
    m_cancelled = true;
    if (m_currentSessionId != 0 && m_sendSessions.contains(m_currentSessionId)) {
        m_sendSessions.remove(m_currentSessionId);
        m_currentSessionId = 0;
    }
}

///////////////////////////
/*
QByteArray DNSStegEngine::createDNSResponse(const PacketHeader& header, const QByteArray& payload)
{
    QByteArray response;

    // DNS Header (12 bytes) - USE BIG ENDIAN
    quint16 transactionId = qToBigEndian(header.sequence);
    response.append((char*)&transactionId, 2);

    // Flags: QR=1 (response)
    quint16 flags = qToBigEndian(static_cast<quint16>(0x8000));
    response.append((char*)&flags, 2);

    // QDCOUNT = 1
    quint16 qdcount = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&qdcount, 2);

    // ANCOUNT = 1
    quint16 ancount = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&ancount, 2);

    // NSCOUNT = 0
    quint16 nscount = 0;
    response.append((char*)&nscount, 2);

    // ARCOUNT = 0
    quint16 arcount = 0;
    response.append((char*)&arcount, 2);

    // Question Section
    QByteArray qname;
    qname.append(3, 'g');
    qname.append(3, 'e');
    qname.append(3, 't');
    qname.append(1, 0);
    response.append(qname);

    // QTYPE: TXT - BIG ENDIAN
    quint16 qtype = qToBigEndian(static_cast<quint16>(0x0010));
    response.append((char*)&qtype, 2);

    // QCLASS: IN - BIG ENDIAN
    quint16 qclass = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&qclass, 2);

    // Answer Section - TXT Record
    quint16 namePtr = qToBigEndian(static_cast<quint16>(0xC00C));
    response.append((char*)&namePtr, 2);

    // Type: TXT - BIG ENDIAN
    quint16 type = qToBigEndian(static_cast<quint16>(0x0010));
    response.append((char*)&type, 2);

    // Class: IN - BIG ENDIAN
    quint16 class_ = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&class_, 2);

    // TTL - BIG ENDIAN
    quint32 ttl = qToBigEndian(static_cast<quint32>(3600));
    response.append((char*)&ttl, 4);

    // Build data: PacketHeader + payload (these stay in host byte order)
    QByteArray data;
    data.append((char*)&header, sizeof(PacketHeader));
    data.append(payload);

    // RDATA length - BIG ENDIAN
    quint16 rdlength = qToBigEndian(static_cast<quint16>(data.size()));
    response.append((char*)&rdlength, 2);

    // RDATA (raw bytes, no conversion)
    response.append(data);

    return response;
}
*/


bool DNSStegEngine::parseDNS(const QByteArray& packet, PacketHeader& header,
                                   QByteArray& payload, QString& sourceIp)
{

    if (packet.size() < DNS_HEADER_SIZE) {
        return false;
    }

    // Read transaction ID as big-endian
    quint16 transactionId;
    memcpy(&transactionId, packet.constData(), 2);
    transactionId = qFromBigEndian(transactionId);

    int pos = DNS_HEADER_SIZE;

    // Skip QNAME
    int qnameStart = pos;
    while (pos < packet.size() && packet[pos] != 0) {
        int labelLen = packet[pos];
        pos += labelLen + 1;
        if (pos >= packet.size()) {
            return false;
        }
    }
    pos++; // Skip the terminating 0

    if (pos + 4 > packet.size()) {
        return false;
    }

    // Read QTYPE and QCLASS
    quint16 qtype;
    memcpy(&qtype, packet.constData() + pos, 2);
    qtype = qFromBigEndian(qtype);
    pos += 2;

    quint16 qclass;
    memcpy(&qclass, packet.constData() + pos, 2);
    qclass = qFromBigEndian(qclass);
    pos += 2;


    // Check if this is a query (no answer section)
    if (pos >= packet.size()) {
        header.magic = 0x41434B00;
        header.sessionId = transactionId;
        header.sequence = 0;
        header.total = 0;
        header.dataSize = 0;
        header.checksum = 0;
        payload.clear();
        return true;
    }


    // Parse answer section for TXT record
    if (pos + 2 > packet.size()) {
        return false;
    }
    pos += 2; // Skip Name pointer

    if (pos + 2 > packet.size()) {
        return false;
    }
    quint16 type;
    memcpy(&type, packet.constData() + pos, 2);
    type = qFromBigEndian(type);
    pos += 2;

    if (type != 0x0010) {
        return false;
    }

    pos += 2; // Skip Class
    pos += 4; // Skip TTL

    if (pos + 2 > packet.size()) {
        return false;
    }
    quint16 rdlength;
    memcpy(&rdlength, packet.constData() + pos, 2);
    rdlength = qFromBigEndian(rdlength);
    pos += 2;

    if (pos + rdlength > packet.size()) {
        return false;
    }

    QByteArray data = packet.mid(pos, rdlength);

    if (data.size() < (int)sizeof(PacketHeader)) {
        return false;
    }

    memcpy(&header, data.constData(), sizeof(PacketHeader));
    payload = data.mid(sizeof(PacketHeader));


    return true;
}

QByteArray DNSStegEngine::createDNSResponse(const PacketHeader& header, const QByteArray& payload)
{
    QByteArray response;

    // DNS Header - ALL BIG ENDIAN
    quint16 transactionId = qToBigEndian(header.sequence);
    response.append((char*)&transactionId, 2);

    quint16 flags = qToBigEndian(static_cast<quint16>(0x8000));
    response.append((char*)&flags, 2);

    quint16 qdcount = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&qdcount, 2);

    quint16 ancount = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&ancount, 2);

    quint16 nscount = qToBigEndian(static_cast<quint16>(0x0000));
    response.append((char*)&nscount, 2);

    quint16 arcount = qToBigEndian(static_cast<quint16>(0x0000));
    response.append((char*)&arcount, 2);

    // Question Section - QNAME "a"
    response.append(static_cast<char>(0x01));  // length 1
    response.append('a');
    response.append(static_cast<char>(0x00));  // terminator

    // QTYPE: TXT (16)
    quint16 qtype = qToBigEndian(static_cast<quint16>(0x0010));
    response.append((char*)&qtype, 2);

    // QCLASS: IN (1)
    quint16 qclass = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&qclass, 2);

    // Answer Section - Name pointer (offset 12)
    quint16 namePtr = qToBigEndian(static_cast<quint16>(0xC00C));
    response.append((char*)&namePtr, 2);

    // Type: TXT
    quint16 type = qToBigEndian(static_cast<quint16>(0x0010));
    response.append((char*)&type, 2);

    // Class: IN
    quint16 class_ = qToBigEndian(static_cast<quint16>(0x0001));
    response.append((char*)&class_, 2);

    // TTL
    quint32 ttl = qToBigEndian(static_cast<quint32>(3600));
    response.append((char*)&ttl, 4);

    // Data: PacketHeader + payload
    QByteArray data;
    data.append((char*)&header, sizeof(PacketHeader));
    data.append(payload);

    // RDATA length
    quint16 rdlength = qToBigEndian(static_cast<quint16>(data.size()));
    response.append((char*)&rdlength, 2);

    // RDATA
    response.append(data);


    return response;
}


void DNSStegEngine::forceReset()
{
    stopListening();

    m_sendSessions.clear();
    m_receiveSessions.clear();
    m_completedSessions.clear();

    m_currentSessionId = 0;
    m_cancelled = false;

    if (m_dnsSocket) {
        // Only flush if socket is in valid state
        if (m_dnsSocket->state() == QUdpSocket::BoundState) {
            while (m_dnsSocket->hasPendingDatagrams()) {
                m_dnsSocket->readDatagram(nullptr, 0);
            }
        }
        m_dnsSocket->close();
        delete m_dnsSocket;
        m_dnsSocket = nullptr;
    }

    initSocket();

}
