#ifndef CONSTANTS_H
#define CONSTANTS_H

#include<QStandardPaths>
#include<QString>

namespace Constants {

    extern const QString appDirPath;
    extern const QString imagesPath;
    extern const QString picturesPath;
    extern const QString musicPath;
    extern const QString fusedImagesPath;
    extern const QString extractedImagesPath;
    extern const QString receivedICMPPath;
    extern const QString ermistmpPath;
    extern const QString sslPath;

    extern bool isCameraOn;

    extern bool ipFilterEnabled;
    extern int ipFilterMode;        // 0 = Block mode, 1 = Allow mode
    extern QStringList ipFilterEntries;

    enum Protocol {
        ICMP,
        UDP,
        DNS,
        HTTP
    };

    extern Protocol currentProtocol;
    extern quint16 bindPort;

    extern Protocol currentReceiverProtocol;
    extern quint16 bindReceiverPort;
}

#endif // CONSTANTS_H
