#ifndef DNSSTEGENGINE_H
#define DNSSTEGENGINE_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QMap>
#include <QElapsedTimer>
#include <QCryptographicHash>
#include "stegengine.h"
#include"ertp_structs.h"
#include<QSet>

class DNSStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit DNSStegEngine(QObject *parent = nullptr);
    ~DNSStegEngine();

    bool startListening(quint16 udpport);
    void stopListening();

    bool sendData(const QString& targetIp, const QByteArray& data, quint16 dnsPort);
    bool sendFile(const QString& targetIpOrDomain, const QString& filePath, quint16 dnsPort);

    void setEncryptionKey(const QString& passphrase);
    void clearEncryption();
    void cancelCurrentTransfer();
    void forceReset();
signals:
    void dataReceived(const QByteArray& data, const QString& source);
    void fileReceived(const QString& path, const QString& source);
    void sendProgress(int packet, int total);
    void receiveProgress(int packet, int total);
    void error(const QString& message);

private slots:
    void onDnsQuery();
    void checkTimeouts();
    void sendNextWindow();

private:
    // Socket management
    QUdpSocket* m_dnsSocket;
    quint16 m_listenDnsPort;
    bool m_listening;

    // Session management
    QMap<quint32, SendSession> m_sendSessions;
    QMap<quint32, ReceiveSession> m_receiveSessions;
    QSet<quint32> m_completedSessions;
    QTimer* m_timeoutTimer;

    // Current transfer tracking
    quint32 m_currentSessionId = 0;
    bool m_cancelled = false;

    // Encryption
    StegEngine m_crypto;
    QString m_encryptionPassphrase;
    bool m_encryptionEnabled = false;

    // Helper methods
    bool initSocket();
    void sendNextPacket(const QString& targetIp, quint16 targetPort, quint32 sessionId, int seq);
    void handleDataPacket(const PacketHeader& header, const QByteArray& payload, const QString& sourceIp);
    void handleControlPacket(const PacketHeader& header, const QString& sourceIp);
    void assembleFile(quint32 sessionId, const QString& sourceIp);
    void checkAndSlideWindow(quint32 sessionId);

    // DNS packet creation/parsing
    QByteArray createDNSResponse(const PacketHeader& header, const QByteArray& payload);
    bool parseDNS(const QByteArray& packet, PacketHeader& header, QByteArray& payload, QString& sourceIp);

    // Control packets
    void sendACK(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence);
    void sendNACK(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence);
    void requestRetransmit(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence);
    void sendDonePacket(quint32 sessionId);

    QString getTargetIpForSession(quint32 sessionId);
    quint16 getTargetPortForSession(quint32 sessionId);
    QString resolveDomainToIp(const QString& target);

    static constexpr int MAX_PAYLOAD = 1400;
    static constexpr int WINDOW_SIZE = 64;
    static constexpr int MAX_RETRIES = 5;
    static constexpr int TIMEOUT_MS = 2000;
};

#endif // DNSSTEGENGINE_H
