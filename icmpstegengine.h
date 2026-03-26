#ifndef ICMPSTEGENGINE_H
#define ICMPSTEGENGINE_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QTimer>
#include <QHostAddress>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include<QElapsedTimer>
#include"stegengine.h"
#include"ertp_structs.h"





    class ICMPStegEngine : public QObject
    {
        Q_OBJECT

    public:
        explicit ICMPStegEngine(QObject *parent = nullptr);
        ~ICMPStegEngine();

        // Public API
        bool sendData(const QString& targetIp, const QByteArray& data);
        bool sendFile(const QString& targetIp, const QString& filePath);
        bool startListening();
        void stopListening();

    signals:
        void dataReceived(const QByteArray& data, const QString& sourceIp);
        void fileReceived(const QString& filePath, const QString& sourceIp);
        void sendProgress(int packet, int total);
        void receiveProgress(int packet, int total);
        void error(const QString& message);

    private slots:
        void handleIncomingPacket();
        void checkTimeouts();
        void sendNextWindow();
        void sendNextPacket(const QString& targetIp, quint32 sessionId, int seq);
        void handleDataPacket(const PacketHeader& header, const QByteArray& payload, const QString& sourceIp);
        void assembleFile(quint32 sessionId, const QString& sourceIp);
        void sendDonePacket(quint32 sessionId);

    private:
        // Core methods
        bool initRawSocket();
        quint16 calculateChecksum(const QByteArray& data);
        QByteArray createICMPPacket(const PacketHeader& header, const QByteArray& payload);
        bool parseICMPPacket(const QByteArray& rawPacket, PacketHeader& header,
                             QByteArray& payload, QString& sourceIp);


        // Session management methods - ADD THESE
        void handleControlPacket(const PacketHeader& header, const QString& sourceIp);
        QString getTargetIpForSession(quint32 sessionId);
        void checkAndSlideWindow(quint32 sessionId);

        // Socket
        int m_sockfd;
        QSocketNotifier* m_notifier;

        // Sending state
        QMap<quint32, SendSession> m_sendSessions;
        QTimer* m_timeoutTimer;

        // Receiving state
        QMap<quint32, ReceiveSession> m_receiveSessions;

        // Constants
        static constexpr int MAX_PAYLOAD = 1400;
        static constexpr int MAX_RETRIES = 3;
        static constexpr int TIMEOUT_MS = 1000;
        static constexpr int WINDOW_SIZE = 32;

        quint16 m_seq;

        void sendACK(const QString& target, quint32 sessionId, quint16 sequence);
        void sendNACK(const QString& target, quint32 sessionId, quint16 sequence);
        void requestRetransmit(const QString& target, quint32 sessionId, quint16 sequence);

    public:
        void setEncryptionKey(const QString& passphrase);
        void clearEncryption();
        bool isEncryptionEnabled() const { return m_encryptionEnabled; }

    private:
        StegEngine m_crypto;
        QString m_encryptionPassphrase;
        bool m_encryptionEnabled = false;
        QString resolveDomainToIp(const QString& target);

    public:
        void cancelCurrentTransfer();
        void forceReset();

    private:
        quint32 m_currentSessionId = 0;
        bool m_cancelled = false;
    };

#endif
