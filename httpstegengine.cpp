// httpsstegengine.cpp
#include "httpstegengine.h"
#include<QRandomGenerator>
#include"constants.h"

HTTPStegEngine::HTTPStegEngine(QObject *parent) : QObject(parent){


            certPath = Constants::sslPath + "/stegserver.crt";
            keyPath  = Constants::sslPath + "/stegserver.key";
            ensureSelfSignedCert(certPath, keyPath, "mystegserver");
}

HTTPStegEngine::~HTTPStegEngine() {

    stopListening();
    cleanupSendSocket();
}

void HTTPStegEngine::assembleFile(quint32 sessionId, const QString& sourceIp)
{
    auto& session = m_receiveSessions[sessionId];
    QByteArray complete;
    for (int i = 0; i < session.totalPackets; i++) {
        if (!session.chunks.contains(i)) return;
        complete.append(session.chunks[i]);
    }
    complete = QByteArray::fromBase64(complete);

    if (complete.startsWith("FILE")) {
        QByteArray filePayload = complete.mid(4);
        QDataStream ds(filePayload);
        ds.setVersion(QDataStream::Qt_6_0);

        QString filename;
        qint64 fileSize;
        QByteArray hash;
        ds >> filename >> fileSize >> hash;
        if (ds.status() != QDataStream::Ok) return;
        QByteArray fileData = filePayload.mid(ds.device()->pos(), fileSize);
        QByteArray computedHash = QCryptographicHash::hash(fileData, QCryptographicHash::Sha256);
        if (computedHash != hash) return;

        QString savePath = QDir::tempPath() + QString("/steg_%1_%2")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")).arg(filename);
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileData); file.close();
            emit fileReceived(savePath, sourceIp);
        }

    } else if (complete.startsWith("TEXT")) {
        QByteArray text = complete.mid(4);
        QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString savePath = QDir::tempPath() + QString("/steg_text_%1.txt").arg(ts);
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(text); file.close();
            emit fileReceived(savePath, sourceIp);
        } else {
            emit dataReceived(text, sourceIp);
        }
    } else {
        emit dataReceived(complete, sourceIp);
    }
    m_receiveSessions.remove(sessionId);
}

// ------------ Resolve host or IP ---------------
QString HTTPStegEngine::resolveDomainToIp(const QString& target)
{
    QHostAddress ipAddr;
    if (ipAddr.setAddress(target)) return target;
    QHostInfo info = QHostInfo::fromName(target);
    if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) return {};
    return info.addresses().first().toString();
}

// ------------ File Send Logic --------------------
bool HTTPStegEngine::sendFile(const QString& targetIpOrDomain, const QString& filePath, quint16 port)
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
    return sendData(ip, payload, port);
}

bool HTTPStegEngine::sendData(const QString& targetIp, const QByteArray& data, quint16 port)
{
    if (m_sending) {
        emit error("Transfer already in progress");
        return false;
    }
    m_currentSession.sessionId = QByteArray::number(QRandomGenerator::global()->generate());
    m_currentSession.data = data;
    m_currentSession.targetIp = targetIp;
    m_currentSession.targetPort = port;
    m_currentSession.totalPackets = 0;
    m_currentSession.currentPacket = 0;
    m_sending = true;
    m_cancelled = false;

    QByteArray encoded = m_currentSession.data.toBase64();
    m_currentSession.totalPackets = (encoded.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
    sendNextChunk();
    return true;
}

void HTTPStegEngine::cleanupSendSocket()
{
    if (m_currentSession.socket) {
        disconnect(m_currentSession.socket, nullptr, this, nullptr);
        m_currentSession.socket->disconnectFromHost();
        m_currentSession.socket->deleteLater();
        m_currentSession.socket = nullptr;
    }
    if (m_currentSession.timeoutTimer) {
        m_currentSession.timeoutTimer->stop();
        delete m_currentSession.timeoutTimer;
        m_currentSession.timeoutTimer = nullptr;
    }
    m_currentSession.readBuffer.clear();
}

void HTTPStegEngine::forceReset()
{
    stopListening();
    cleanupSendSocket();

    m_currentSession.data.clear();
    m_currentSession.sessionId.clear();
    m_currentSession.targetIp.clear();
    m_currentSession.targetPort = 0;
    m_currentSession.totalPackets = 0;
    m_currentSession.currentPacket = 0;

    m_sending = false;
    m_cancelled = false;
}

bool HTTPStegEngine::startListeningInternal(quint16 port, const QString &certPath, const QString &keyPath) {
    stopListening();
    m_server = new QSslServer(this);
    m_listenPort = port;

    if (!m_server->listen(QHostAddress::AnyIPv4, m_listenPort)) {
        emit error("Failed to listen on port " + QString::number(port));
        qWarning() << "[HTTPStegEngine] listen() failed on port" << m_listenPort;
        return false;
    }
    connect(m_server, &QTcpServer::newConnection, this, &HTTPStegEngine::onNewConnection);
    m_listening = true;
    this->certPath = certPath;
    this->keyPath = keyPath;
    return true;
}

void HTTPStegEngine::ensureSelfSignedCert(const QString &certPath, const QString &keyPath, const QString &commonName) {
    QFile cert(certPath);
    QFile key(keyPath);
    if (cert.exists() && key.exists()) {
        return;
    }
    QStringList args;
    args << "req" << "-x509"
         << "-newkey" << "rsa:2048"
         << "-keyout" << keyPath
         << "-out" << certPath
         << "-days" << "365"
         << "-nodes"
         << "-subj" << QString("/CN=%1").arg(commonName);
    QProcess proc;
    proc.start("openssl", args);
    if (!proc.waitForFinished(10000)) {
        qWarning() << "[HTTPStegEngine] Timed out running OpenSSL!";
        return;
    }
    QByteArray err = proc.readAllStandardError();
    if (proc.exitCode() == 0) {
    } else {
        qWarning() << "[HTTPStegEngine] OpenSSL failed:" << err;
    }
}

void HTTPStegEngine::stopListening() {
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    qDeleteAll(m_incomingBuffers.keys());
    m_incomingBuffers.clear();
    m_chunks.clear();
    m_totalChunks.clear();
    m_listening = false;
}


void HTTPStegEngine::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QSslSocket* sslSocket = qobject_cast<QSslSocket*>(m_server->nextPendingConnection());
        Q_ASSERT(sslSocket);



        sslSocket->setLocalCertificate(certPath);
        sslSocket->setPrivateKey(keyPath);
        sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);

        connect(sslSocket, &QSslSocket::encrypted, this, &HTTPStegEngine::onPeerEncrypted);



        connect(sslSocket, &QSslSocket::sslErrors, this, [sslSocket](const QList<QSslError>& errors){
            for (const QSslError &e : errors)
                qWarning() << "[HTTPStegEngine][SERVER][SSL ERROR]" << e.errorString();
        });
        connect(sslSocket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::errorOccurred),
                this, [sslSocket](QAbstractSocket::SocketError e){
            qWarning() << "[HTTPStegEngine][SERVER][Socket error]" << e << sslSocket->errorString();
        });
        connect(sslSocket, &QSslSocket::stateChanged, this, [sslSocket](QAbstractSocket::SocketState s){
        });

        sslSocket->startServerEncryption();
    }
}



void HTTPStegEngine::onPeerEncrypted() {
    QSslSocket* client = qobject_cast<QSslSocket*>(sender());
    if (!client) return;
    m_incomingBuffers[client] = QByteArray();
    m_chunks[client] = QMap<int, QByteArray>();
    m_totalChunks[client] = 0;
    connect(client, &QSslSocket::readyRead, this, &HTTPStegEngine::onRequestReceived);
    connect(client, &QSslSocket::disconnected, [this, client]() {
        m_incomingBuffers.remove(client);
        m_chunks.remove(client);
        m_totalChunks.remove(client);
        client->deleteLater();
    });
}

// --- Receiving data/chunks ---
void HTTPStegEngine::onRequestReceived() {
    QSslSocket* client = qobject_cast<QSslSocket*>(sender());
    if (!client) return;
    m_incomingBuffers[client].append(client->readAll());

    int headerEnd = m_incomingBuffers[client].indexOf("\r\n\r\n");
    if (headerEnd == -1) {
        return;
    }

    QByteArray headers = m_incomingBuffers[client].left(headerEnd);

    auto getHeaderValue = [&headers](const QByteArray& headerName) -> QByteArray {
        int pos = headers.indexOf(headerName);
        if (pos == -1) return QByteArray();
        pos += headerName.length();
        int end = headers.indexOf("\r\n", pos);
        if (end == -1) end = headers.length();
        return headers.mid(pos, end - pos).trimmed();
    };

    QByteArray sessionIdBytes = getHeaderValue("X-Session-Id: ");
    QByteArray chunkData = getHeaderValue("X-Data: ");
    QByteArray seqStr = getHeaderValue("X-Seq: ");
    QByteArray totalStr = getHeaderValue("X-Total: ");

    if (sessionIdBytes.isEmpty() || chunkData.isEmpty() || seqStr.isEmpty() || totalStr.isEmpty()) {
        qWarning() << "[HTTPStegEngine] Missing required HTTP headers";
        return;
    }
    int seq = seqStr.toInt();
    int total = totalStr.toInt();
    quint32 sessionId = qHash(sessionIdBytes);
    m_incomingBuffers[client].remove(0, headerEnd + 4);

    if (m_completedSessions.contains(sessionId)) {
        QByteArray response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nX-Ack: " + QByteArray::number(seq) + "\r\n\r\n";
        client->write(response);
        client->flush();
        return;
    }

    if (!m_receiveSessions.contains(sessionId)) {
        HTTPReceiveSession session;
        session.totalPackets = total;
        session.completionTimer = new QTimer(this);
        session.completionTimer->setSingleShot(true);
        session.completionTimer->setInterval(10000);
        connect(session.completionTimer, &QTimer::timeout, [this, sessionId]() {
            qWarning() << "[HTTPStegEngine] Session timed out:" << sessionId;
            m_receiveSessions.remove(sessionId);
        });
        m_receiveSessions[sessionId] = session;
    }

    auto& session = m_receiveSessions[sessionId];
    session.chunks[seq] = chunkData;

    QByteArray response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nX-Ack: " + QByteArray::number(seq) + "\r\n\r\n";
    client->write(response);
    client->flush();

    session.completionTimer->stop();
    session.completionTimer->start();
    emit receiveProgress(session.chunks.size(), total);

    if (session.chunks.size() == total) {
        session.completionTimer->stop();
        m_completedSessions.insert(sessionId);
        assembleFile(sessionId, client->peerAddress().toString());
        QTimer::singleShot(10000, [this, sessionId]() { m_completedSessions.remove(sessionId); });
    }
}

// --- Outbound (Client) ---
void HTTPStegEngine::sendNextChunk() {
    if (m_cancelled || m_currentSession.currentPacket >= m_currentSession.totalPackets) {
        if (!m_cancelled) emit sendProgress(m_currentSession.totalPackets, m_currentSession.totalPackets);
        m_sending = false;
        cleanupSendSocket();
        return;
    }

    int currentPacket = m_currentSession.currentPacket;
    QByteArray encoded = m_currentSession.data.toBase64();
    int start = currentPacket * CHUNK_SIZE;
    QByteArray chunk = encoded.mid(start, CHUNK_SIZE);

    cleanupSendSocket();
    QSslSocket* ssl = new QSslSocket(this);
    m_currentSession.socket = ssl;
    m_currentSession.readBuffer.clear();

    ssl->setPeerVerifyMode(QSslSocket::VerifyNone);

    // CLIENT SIDE SSL DEBUG
    connect(ssl, &QSslSocket::sslErrors, this, [ssl](const QList<QSslError>& errors){
        for (const QSslError &e : errors)
            qWarning() << "[HTTPStegEngine][CLIENT][SSL ERROR]" << e.errorString();
    });
    connect(ssl, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::errorOccurred),
            this, [ssl](QAbstractSocket::SocketError e){
        qWarning() << "[HTTPStegEngine][CLIENT][Socket error]" << e << ssl->errorString();
    });
    connect(ssl, &QSslSocket::stateChanged, this, [ssl](QAbstractSocket::SocketState s){
    });

    connect(ssl, &QSslSocket::encrypted, this, &HTTPStegEngine::onSendSocketEncrypted);
    connect(ssl, &QSslSocket::readyRead, this, &HTTPStegEngine::onSendSocketReadyRead);
    connect(ssl, &QSslSocket::disconnected, this, &HTTPStegEngine::onSendSocketDisconnected);

    m_currentSession.timeoutTimer = new QTimer(this);
    m_currentSession.timeoutTimer->setSingleShot(true);
    m_currentSession.timeoutTimer->setInterval(5000);
    connect(m_currentSession.timeoutTimer, &QTimer::timeout, [this]() {
        emit error("Socket timeout");
        m_sending = false;
        m_cancelled = true;
        cleanupSendSocket();
    });

    m_currentSession.currentPacket = currentPacket;
    m_currentSession.timeoutTimer->start();
    ssl->connectToHost(m_currentSession.targetIp, m_currentSession.targetPort);
    connect(ssl, &QSslSocket::connected, ssl, [ssl](){
        ssl->startClientEncryption();
    });
}

void HTTPStegEngine::onSendSocketEncrypted() {
    QSslSocket* ssl = qobject_cast<QSslSocket*>(m_currentSession.socket);
    if (ssl)
    onSendSocketConnected();
}

void HTTPStegEngine::onSendSocketConnected() {
    int currentPacket = m_currentSession.currentPacket;
    QByteArray encoded = m_currentSession.data.toBase64();
    int start = currentPacket * CHUNK_SIZE;
    QByteArray chunk = encoded.mid(start, CHUNK_SIZE);

    QByteArray request = "GET / HTTP/1.1\r\n";
    request += "Host: " + m_currentSession.targetIp.toUtf8() + ":" + QByteArray::number(m_currentSession.targetPort) + "\r\n";
    request += "User-Agent: Mozilla/5.0\r\n";
    request += "Connection: close\r\n";
    request += "X-Session-Id: " + m_currentSession.sessionId + "\r\n";
    request += "X-Data: " + chunk + "\r\n";
    request += "X-Seq: " + QByteArray::number(currentPacket) + "\r\n";
    request += "X-Total: " + QByteArray::number(m_currentSession.totalPackets) + "\r\n";
    request += "\r\n";

    QSslSocket* ssl = qobject_cast<QSslSocket*>(m_currentSession.socket);
    ssl->write(request);
    ssl->flush();
}

void HTTPStegEngine::onSendSocketReadyRead() {
    QSslSocket* ssl = qobject_cast<QSslSocket*>(m_currentSession.socket);
    if (!ssl) return;
    m_currentSession.readBuffer.append(ssl->readAll());
    int headerEnd = m_currentSession.readBuffer.indexOf("\r\n\r\n");
    if (headerEnd == -1) return;

    QByteArray headers = m_currentSession.readBuffer.left(headerEnd);

    auto getHeaderValue = [&headers](const QByteArray& headerName) -> QByteArray {
        int pos = headers.indexOf(headerName); if (pos == -1) return QByteArray();
        pos += headerName.length();
        int end = headers.indexOf("\r\n", pos);
        if (end == -1) end = headers.length();
        return headers.mid(pos, end - pos).trimmed();
    };

    QByteArray ackStr = getHeaderValue("X-Ack: ");
    int currentPacket = m_currentSession.currentPacket;

    m_currentSession.timeoutTimer->stop();
    cleanupSendSocket();
    m_currentSession.currentPacket++;
    emit sendProgress(m_currentSession.currentPacket, m_currentSession.totalPackets);
    sendNextChunk();
}

void HTTPStegEngine::onSendSocketDisconnected() {
    if (m_currentSession.timeoutTimer) m_currentSession.timeoutTimer->stop();
}

void HTTPStegEngine::onSendSocketError(QAbstractSocket::SocketError socketError) {
    qWarning() << "[HTTPStegEngine][CLIENT] Socket error:" << socketError;
    emit error(QString("Socket error: %1").arg(socketError));
    m_sending = false;
    m_cancelled = true;
    if (m_currentSession.timeoutTimer) m_currentSession.timeoutTimer->stop();
    cleanupSendSocket();
}

