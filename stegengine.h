#ifndef STEGENGINE_H
#define STEGENGINE_H


#include <QObject>
#include <QImage>
#include <QByteArray>
#include <QString>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include<QPoint>
#include<QColor>
#include<QPainter>

class StegEngine : public QObject
{
    Q_OBJECT

public:
    explicit StegEngine(QObject *parent = nullptr);
    ~StegEngine();

    // Steganography methods
    bool embedDataInImage(const QImage &originalImage, QImage &modifiedImage, const QByteArray &data);
    QByteArray extractDataFromImage(const QImage &stegoImage);
    int calculateImageCapacity(const QImage &image);

    // Encryption methods
    QByteArray encryptData(const QByteArray &data, const QString &passphrase);
    QByteArray decryptData(const QByteArray &encryptedData, const QString &passphrase);

    // Utility methods
    bool isValidImage(const QImage &image) const;

private:
    // Helper methods for steganography
    QByteArray extractHeader(const QImage &image);
    bool embedHeader(QImage &image, quint32 dataSize);
    unsigned char extractByteFromImage(const QImage &image, int startBitPosition);

// pretty QR
public:
    // PRT Mode Methods (no encryption, just embedding/extracting)
    bool embedPRTData(const QImage &originalImage, QImage &prtImage, const QByteArray &data);
    QByteArray extractPRTData(const QImage &prtImage);

    // PRT Marker Management
    void addPRTMarkers(QImage &image);

private:
    // PRT Helper Methods
    QColor calculateDotColor(const QImage &image, const QPoint &corner) const;
    void drawPRTDot(QPainter &painter, const QPoint &center, int size, const QColor &color);

    //dot detection
public:
    bool hasPRTMarkers(const QImage &image) const;

private:

    int calculateOptimalDotSize(const QImage& image) const;

};



#endif // STEGENGINE_H
