#ifndef ERTP_STRUCTS_H
#define ERTP_STRUCTS_H

#include <QByteArray>
#include <QMap>
#include <QTimer>
#include <QElapsedTimer>

struct PacketHeader {
    quint32 magic;      // "ERMI"
    quint32 sessionId;
    quint16 sequence;
    quint16 total;
    quint32 checksum;
    quint16 dataSize;
    char padding[2];
};

struct SendSession {
    QByteArray data;
    int totalPackets;
    QString targetIp;
    quint16 targetPort;
    QMap<int, bool> acked;
    QMap<int, QElapsedTimer> sentTime;
    QMap<int, int> retryCount;
    QTimer* timeoutTimer;
};

struct ReceiveSession {
    QByteArray filename;
    int totalPackets;
    QMap<int, QByteArray> chunks;
    QTimer* completionTimer;
    //bool completed = false;
};

#endif

