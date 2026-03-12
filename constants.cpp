#include "constants.h"

namespace Constants {

//const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Ermis";
const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

const QString imagesPath = appDirPath + "/carriers";
const QString fusedImagesPath = appDirPath + "/fused";
const QString extractedImagesPath = appDirPath + "/extracted";
const QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
const QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
bool isCameraOn = false;

}
