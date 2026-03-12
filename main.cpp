#include "mainwindow.h"

#include <QApplication>
#include<QCoreApplication>
//#include<QStandardPaths>
#include<QDir>
#include"constants.h"


int main(int argc, char *argv[])
{

    QCoreApplication::setApplicationName("Ermis");
    QCoreApplication::setOrganizationName("Alamahant");
    QCoreApplication::setApplicationVersion("1.0.0");
    QDir().mkpath(Constants::appDirPath);
    QDir().mkpath(Constants::imagesPath);
    QDir().mkpath(Constants::fusedImagesPath);
    QDir().mkpath(Constants::extractedImagesPath);
    //QDir().mkpath(Constants::picturesPath);
    //QDir().mkpath(Constants::musicPath);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/io.github.alamahant.Ermis.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
