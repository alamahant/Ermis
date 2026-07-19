#include "mainwindow.h"

#include <QApplication>
#include<QCoreApplication>
//#include<QStandardPaths>
#include<QDir>
#include"constants.h"
#include<QStyleFactory>


int main(int argc, char *argv[])
{

    QCoreApplication::setApplicationName("Ermis");
    QCoreApplication::setOrganizationName("Alamahant");
    QCoreApplication::setApplicationVersion("1.1.3");
    QDir().mkpath(Constants::appDirPath);
    QDir().mkpath(Constants::imagesPath);
    QDir().mkpath(Constants::fusedImagesPath);
    QDir().mkpath(Constants::extractedImagesPath);
    QDir().mkpath(Constants::receivedICMPPath);
    QDir().mkpath(Constants::ermistmpPath);
    QDir().mkpath(Constants::sslPath);
    QDir().mkpath(Constants::elfPath);

    //QDir().mkpath(Constants::picturesPath);
    //QDir().mkpath(Constants::musicPath);

    QApplication a(argc, argv);

#ifndef FLATPAK_BUILD

    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, Qt::white);
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    a.setPalette(lightPalette);
#endif

    a.setWindowIcon(QIcon(":/io.github.alamahant.Ermis.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
