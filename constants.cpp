#include "constants.h"

namespace Constants {


#ifndef FLATPAK_BUILD
const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Ermis";
#else
const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif


const QString imagesPath = appDirPath + "/carriers";
const QString fusedImagesPath = appDirPath + "/fused";
const QString extractedImagesPath = appDirPath + "/extracted";
const QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
const QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
const QString receivedICMPPath = appDirPath + "/Network";
const QString ermistmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/Ermis";
const QString sslPath = appDirPath + "/.ssl";

bool isCameraOn = false;

// IP Filter
bool ipFilterEnabled = false;
int ipFilterMode = 0;           // 0 = Block mode (default)
QStringList ipFilterEntries;

Protocol currentProtocol = ICMP;
quint16 bindPort = 6363;

Protocol currentReceiverProtocol = ICMP;
quint16 bindReceiverPort = 6363;

}
