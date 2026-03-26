#ifndef UDPSTEGENGINE_H
#define UDPSTEGENGINE_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QTimer>
#include <QHostAddress>
#include <QUdpSocket>
#include <QElapsedTimer>
#include "stegengine.h"
#include"ertp_structs.h"
#include<QSet>


class UDPStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit UDPStegEngine(QObject *parent = nullptr);
    ~UDPStegEngine();

    bool sendData(const QString& targetIp, const QByteArray& data, quint16 port = 5353);
    bool sendFile(const QString& targetIp, const QString& filePath, quint16 port = 5353);
    bool startListening(quint16 port = 5353);
    void stopListening();
    void setEncryptionKey(const QString& passphrase);
    void clearEncryption();
    bool isEncryptionEnabled() const { return m_encryptionEnabled; }

signals:
    void dataReceived(const QByteArray& data, const QString& sourceIp);
    void fileReceived(const QString& filePath, const QString& sourceIp);
    void sendProgress(int packet, int total);
    void receiveProgress(int packet, int total);
    void error(const QString& message);

private slots:
    void onReadyRead();
    void checkTimeouts();
    void sendNextWindow();
    void sendNextPacket(const QString& targetIp, quint16 targetPort, quint32 sessionId, int seq);
    void sendDonePacket(quint32 sessionId);

private:
    bool initSocket();
    void handleDataPacket(const PacketHeader& header, const QByteArray& payload, const QString& sourceIp);
    void handleControlPacket(const PacketHeader& header, const QString& sourceIp);
    void assembleFile(quint32 sessionId, const QString& sourceIp);
    void sendACK(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence);
    void sendNACK(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence);
    void requestRetransmit(const QString& target, quint16 targetPort, quint32 sessionId, quint16 sequence);
    void checkAndSlideWindow(quint32 sessionId);
    QString getTargetIpForSession(quint32 sessionId);
    quint16 getTargetPortForSession(quint32 sessionId);
    QString resolveDomainToIp(const QString& target);

    QUdpSocket* m_socket;
    quint16 m_listenPort = 5353;
    bool m_listening = false;

    QMap<quint32, SendSession> m_sendSessions;
    QMap<quint32, ReceiveSession> m_receiveSessions;
    QTimer* m_timeoutTimer;

    StegEngine m_crypto;
    QString m_encryptionPassphrase;
    bool m_encryptionEnabled = false;

    static constexpr int MAX_PAYLOAD = 1400;
    static constexpr int MAX_RETRIES = 3;
    static constexpr int TIMEOUT_MS = 1000;
    static constexpr int WINDOW_SIZE = 32;

    quint16 m_seq = 0;

    quint32 m_currentSessionId = 0;  // Track current sending session
    bool m_cancelled = false;
public:
    void cancelCurrentTransfer();
    QSet<quint32> m_completedSessions;  // Track completed sessions
    void forceReset();
};

#endif
