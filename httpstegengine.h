// httpsstegengine.h
#pragma once

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDataStream>
#include <QHostInfo>
#include <QCryptographicHash>
#include <QSet>
#include <QMap>
#include <QProcess>
#include <QDebug>


class QSslServer : public QTcpServer {
public:
    explicit QSslServer(QObject *parent = nullptr) : QTcpServer(parent) {}

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QSslSocket *sslSocket = new QSslSocket(this);
        sslSocket->setSocketDescriptor(socketDescriptor);
        addPendingConnection(sslSocket);
    }
};


class HTTPStegEngine : public QObject {
    Q_OBJECT
public:
    explicit HTTPStegEngine(QObject *parent = nullptr);

    ~HTTPStegEngine();

    // ---- Legacy App Methods: signature unchanged ----
    bool startListening(quint16 port)                                { return startListeningInternal(port, certPath, keyPath); }
    void stopListening();
    void forceReset();
    bool sendFile(const QString& targetIpOrDomain, const QString& filePath, quint16 port);
    bool sendData(const QString& targetIpOrDomain, const QByteArray& data, quint16 port);

    void setEncryptionKey(const QString&)                            {/* TLS only now. No-op. */}
    void clearEncryption()                                           {/* TLS only now. No-op. */}
    void cancelCurrentTransfer()                                     { m_cancelled = true; m_sending = false; }

    // ---- Signals ----
signals:
    void error(const QString&);
    void dataReceived(QByteArray, QString);
    void fileReceived(QString, QString);
    void sendProgress(int, int);
    void receiveProgress(int, int);

private slots:
    void onNewConnection();
    void onPeerEncrypted();
    void onRequestReceived();
    void onSendSocketEncrypted();
    void onSendSocketReadyRead();
    void onSendSocketDisconnected();
    void onSendSocketError(QAbstractSocket::SocketError);
    void onSendSocketConnected();

private:
    // ---- Fields ----
    QTcpServer* m_server = nullptr;
    quint16 m_listenPort = 0;
    bool m_listening = false;

    QString certPath, keyPath;

    QMap<QSslSocket*, QByteArray> m_incomingBuffers;
    QMap<QSslSocket*, QMap<int, QByteArray>> m_chunks;
    QMap<QSslSocket*, int> m_totalChunks;

    struct HTTPReceiveSession {
        int totalPackets = 0;
        QTimer* completionTimer = nullptr;
        QMap<int, QByteArray> chunks;
    };
    QMap<quint32, HTTPReceiveSession> m_receiveSessions;
    QSet<quint32> m_completedSessions;

    struct HTTPSendSession {
        QByteArray sessionId;
        QByteArray data;
        QString targetIp;
        quint16 targetPort = 0;
        int totalPackets = 0;
        int currentPacket = 0;
        QSslSocket* socket = nullptr;
        QTimer* timeoutTimer = nullptr;
        QByteArray readBuffer;
    } m_currentSession;

    bool m_sending = false;
    bool m_cancelled = false;
    static constexpr int CHUNK_SIZE = 64000;

    void cleanupSendSocket();
    void sendNextChunk();
    void assembleFile(quint32 sessionId, const QString& sourceIp);
    QString resolveDomainToIp(const QString&);
    bool startListeningInternal(quint16 port, const QString& certPath, const QString& keyPath);

    // ---- Self-signed Cert Helper ----
    void ensureSelfSignedCert(const QString &certPath, const QString &keyPath, const QString &commonName);

    QSet<QTcpSocket*> pendingRawSockets;



};
