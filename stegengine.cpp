#include "stegengine.h"

#include <QDebug>
#include <QRandomGenerator>
#include<QMessageBox>
#include"constants.h"

StegEngine::StegEngine(QObject *parent) : QObject(parent)
{
}

StegEngine::~StegEngine()
{
}

bool StegEngine::embedDataInImage(const QImage &originalImage, QImage &modifiedImage, const QByteArray &data)
{
    if (originalImage.isNull() || data.isEmpty()) {
        return false;
    }


    // Create a copy of the original image to modify
    modifiedImage = originalImage.copy();

    // Prepare header: 4 bytes for data size
    QByteArray header(4, 0);
    quint32 dataSize = data.size();
    header[0] = (dataSize >> 24) & 0xFF;
    header[1] = (dataSize >> 16) & 0xFF;
    header[2] = (dataSize >> 8) & 0xFF;
    header[3] = dataSize & 0xFF;


    // Combine header and data
    QByteArray fullData = header + data;

    // Check if the image can hold the data
    int capacity = calculateImageCapacity(originalImage);

    if (fullData.size() > capacity) {
        return false;
    }

    // Embed data using LSB (Least Significant Bit) method
    for (int byteIndex = 0; byteIndex < fullData.size(); byteIndex++) {
        unsigned char currentByte = static_cast<unsigned char>(fullData[byteIndex]);

        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            // Calculate bit position in the overall sequence
            int bitPosition = byteIndex * 8 + bitIndex;

            // Calculate pixel position
            int pixelIndex = bitPosition / 3;
            int channelIndex = bitPosition % 3;

            int x = pixelIndex % modifiedImage.width();
            int y = pixelIndex / modifiedImage.width();

            // Get the bit to embed
            int bit = (currentByte >> (7 - bitIndex)) & 1;

            QColor pixel = modifiedImage.pixelColor(x, y);

            // Embed the bit in the appropriate channel
            if (channelIndex == 0) {
                // Red channel
                int r = (pixel.red() & 0xFE) | bit;
                pixel.setRed(r);
            } else if (channelIndex == 1) {
                // Green channel
                int g = (pixel.green() & 0xFE) | bit;
                pixel.setGreen(g);
            } else { // channelIndex == 2
                // Blue channel
                int b = (pixel.blue() & 0xFE) | bit;
                pixel.setBlue(b);
            }

            modifiedImage.setPixelColor(x, y, pixel);
        }
    }

    return true;
}

QByteArray StegEngine::extractDataFromImage(const QImage &stegoImage)
{
    if (stegoImage.isNull()) {
        return QByteArray();
    }


    // Extract header
    QByteArray header = extractHeader(stegoImage);
    if (header.size() != 4) {
        return QByteArray();
    }

    // Parse data size from header
    quint32 dataSize = (static_cast<quint32>(static_cast<unsigned char>(header[0])) << 24) |
                       (static_cast<quint32>(static_cast<unsigned char>(header[1])) << 16) |
                       (static_cast<quint32>(static_cast<unsigned char>(header[2])) << 8) |
                       static_cast<quint32>(static_cast<unsigned char>(header[3]));


    // Sanity check for data size
    int maxCapacity = calculateImageCapacity(stegoImage) - 4; // Subtract header size

    if (dataSize == 0) {
        return QByteArray();
    }

    if (dataSize > maxCapacity) {
        return QByteArray();
    }

    // Now extract the actual data
    QByteArray extractedData(dataSize, 0);

    // Start extracting data after the header
    int startBitPosition = 4 * 8; // Skip header bits

    for (int byteIndex = 0; byteIndex < dataSize; byteIndex++) {
        extractedData[byteIndex] = extractByteFromImage(stegoImage, startBitPosition + byteIndex * 8);
    }

    return extractedData;
}

int StegEngine::calculateImageCapacity(const QImage &image)
{
    if (image.isNull()) {
        return 0;
    }

    // Each pixel can store 3 bits (one in each RGB channel)
    // We need 4 bytes (32 bits) for the header
    int totalBits = image.width() * image.height() * 3;
    int dataBits = totalBits - 32; // Subtract header size

    // Convert bits to bytes (integer division)
    return dataBits / 8;
}


bool StegEngine::isValidImage(const QImage &image) const
{
    return !image.isNull() && image.width() > 0 && image.height() > 0;
}

// Private helper methods

QByteArray StegEngine::extractHeader(const QImage &image)
{
    QByteArray header(4, 0);

    // Extract header bits
    for (int byteIndex = 0; byteIndex < 4; byteIndex++) {
        header[byteIndex] = extractByteFromImage(image, byteIndex * 8);
    }


    return header;
}

unsigned char StegEngine::extractByteFromImage(const QImage &image, int startBitPosition)
{
    unsigned char byte = 0;

    for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
        // Calculate bit position in the overall sequence
        int bitPosition = startBitPosition + bitIndex;

        // Calculate pixel position
        int pixelIndex = bitPosition / 3;
        int channelIndex = bitPosition % 3;

        int x = pixelIndex % image.width();
        int y = pixelIndex / image.width();

        QColor pixel = image.pixelColor(x, y);

        // Get the LSB from the appropriate channel
        int bit = 0;
        if (channelIndex == 0) {
            bit = pixel.red() & 1;
        } else if (channelIndex == 1) {
            bit = pixel.green() & 1;
        } else { // channelIndex == 2
            bit = pixel.blue() & 1;
        }

        // Set the bit in the byte
        byte |= (bit << (7 - bitIndex));
    }

    return byte;
}




QByteArray StegEngine::encryptData(const QByteArray &data, const QString &passphrase)
{
    if (data.isEmpty() || passphrase.isEmpty()) {
        return QByteArray();
    }

    // Generate a random salt (16 bytes)
    QByteArray salt;
    salt.resize(16);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(salt.data()), salt.size() / 4);

    // Generate key and IV from passphrase and salt using PBKDF2
    QByteArray key(32, 0); // 256 bits
    QByteArray iv(16, 0);  // 128 bits

    // Using OpenSSL for PBKDF2
    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.toUtf8().constData(),
        passphrase.toUtf8().length(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.length(),
        10000, // iterations
        key.length(),
        reinterpret_cast<unsigned char*>(key.data())
        );

    // Generate IV (can also use PBKDF2 with different salt)
    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.toUtf8().constData(),
        passphrase.toUtf8().length(),
        reinterpret_cast<const unsigned char*>(salt.constData()) + 8, // Use part of salt
        8,
        5000, // fewer iterations for IV
        iv.length(),
        reinterpret_cast<unsigned char*>(iv.data())
        );

    // Set up encryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                       reinterpret_cast<const unsigned char*>(key.constData()),
                       reinterpret_cast<const unsigned char*>(iv.constData()));

    // Encrypt the data
    QByteArray encryptedData;
    encryptedData.resize(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outlen1 = 0, outlen2 = 0;

    EVP_EncryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(encryptedData.data()),
                      &outlen1,
                      reinterpret_cast<const unsigned char*>(data.constData()),
                      data.length());

    EVP_EncryptFinal_ex(ctx,
                        reinterpret_cast<unsigned char*>(encryptedData.data()) + outlen1,
                        &outlen2);

    EVP_CIPHER_CTX_free(ctx);

    encryptedData.resize(outlen1 + outlen2);

    // Format: [salt][iv][encrypted data]
    return salt + iv + encryptedData;
}

QByteArray StegEngine::decryptData(const QByteArray &encryptedData, const QString &passphrase)
{
    if (encryptedData.size() <= 32 || passphrase.isEmpty()) { // 16 (salt) + 16 (IV)
        return QByteArray();
    }

    // Extract salt and IV
    QByteArray salt = encryptedData.left(16);
    QByteArray iv = encryptedData.mid(16, 16);
    QByteArray ciphertext = encryptedData.mid(32);

    // Generate key from passphrase and salt
    QByteArray key(32, 0); // 256 bits

    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.toUtf8().constData(),
        passphrase.toUtf8().length(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.length(),
        10000, // iterations
        key.length(),
        reinterpret_cast<unsigned char*>(key.data())
        );

    // Set up decryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                       reinterpret_cast<const unsigned char*>(key.constData()),
                       reinterpret_cast<const unsigned char*>(iv.constData()));

    // Decrypt the data
    QByteArray decryptedData;
    decryptedData.resize(ciphertext.size());
    int outlen1 = 0, outlen2 = 0;

    EVP_DecryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(decryptedData.data()),
                      &outlen1,
                      reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                      ciphertext.length());

    // Check if final decryption was successful
    int result = EVP_DecryptFinal_ex(ctx,
                                     reinterpret_cast<unsigned char*>(decryptedData.data()) + outlen1,
                                     &outlen2);

    EVP_CIPHER_CTX_free(ctx);

    // If decryption failed (wrong passphrase), return empty array
    if (result != 1) {
        return QByteArray();
    }

    decryptedData.resize(outlen1 + outlen2);
    return decryptedData;
}

//pretty QR
bool StegEngine::embedPRTData(const QImage &originalImage, QImage &prtImage, const QByteArray &data)
{
    if (originalImage.isNull() || data.isEmpty()) {
        return false;
    }

    //

    //add a header also
    QByteArray markedData;
        markedData.append("PRT");  // 3-byte header
        markedData.append(char(0x01)); // Version 1
        markedData.append(data);   // Actual payload
    bool embedSuccess = embedDataInImage(originalImage, prtImage, markedData);

    if (!embedSuccess) {
        return false;
    }

    // Step 2: Add PRT markers to the stego image
    addPRTMarkers(prtImage);

    return true;
}



QByteArray StegEngine::extractPRTData(const QImage &prtImage)
{

    if (prtImage.isNull()) {
        return QByteArray();
    }

    // STEP 1: Try extraction with visual marker detection
    if (hasPRTMarkers(prtImage) && !Constants::isCameraOn) {
        QByteArray data = extractDataFromImage(prtImage);

        if (!data.isEmpty()) {
            if (data.size() >= 4 &&
                data.startsWith("PRT") &&
                data[3] == 0x01) {
                QByteArray payload = data.mid(4);
                return payload;
            }
            //return data; // Return whatever we extracted with markers
        }
    }

    // STEP 2: No markers found, try header-based detection
    QByteArray extractedData = extractDataFromImage(prtImage);

    if (extractedData.isEmpty()) {
        return QByteArray();
    }

    // STEP 3: Check for PRT header
    if (extractedData.size() >= 4 &&
        extractedData.startsWith("PRT") &&
        extractedData[3] == 0x01) {
        QByteArray payload = extractedData.mid(4);
        return payload;
    }

    // STEP 4: Neither approach worked
    return QByteArray();
}

void StegEngine::addPRTMarkers(QImage &image)
{
    if (image.isNull()) return;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int dotSize = calculateOptimalDotSize(image);

    // ONLY CENTER DOT
    QPoint centerPoint(image.width() / 2, image.height() / 2);
    QColor centerColor = calculateDotColor(image, centerPoint);
    drawPRTDot(painter, centerPoint, dotSize, centerColor);

}


// Smart Color Calculation
QColor StegEngine::calculateDotColor(const QImage &image, const QPoint &center) const
{
    // Sample a small region around the corner
    int sampleSize = 30;
    QRect sampleRect(center.x() - sampleSize/2, center.y() - sampleSize/2,
                     sampleSize, sampleSize);

    // Clamp to image bounds
    sampleRect = sampleRect.intersected(QRect(0, 0, image.width(), image.height()));

    if (sampleRect.isEmpty()) {
        return QColor(0, 0, 0, 180); // Fallback: dark semi-transparent
    }

    // Calculate average color of the sample region
    long long r = 0, g = 0, b = 0;
    int pixelCount = 0;

    for (int y = sampleRect.top(); y <= sampleRect.bottom(); ++y) {
        for (int x = sampleRect.left(); x <= sampleRect.right(); ++x) {
            QColor pixel = image.pixelColor(x, y);
            r += pixel.red();
            g += pixel.green();
            b += pixel.blue();
            pixelCount++;
        }
    }

    if (pixelCount == 0) {
        return QColor(0, 0, 0, 180);
    }

    QColor averageColor(r / pixelCount, g / pixelCount, b / pixelCount, 255);

    // Choose contrasting color based on brightness
    qreal brightness = (averageColor.redF() * 0.299 +
                       averageColor.greenF() * 0.587 +
                       averageColor.blueF() * 0.114);

    if (brightness > 0.5) {
        return QColor(0, 0, 0, 180); // Dark dot for light backgrounds
    } else {
        return QColor(255, 255, 255, 180); // Light dot for dark backgrounds
    }
}

// Draw a PRT Dot (Semi-transparent circle)
void StegEngine::drawPRTDot(QPainter &painter, const QPoint &center, int size, const QColor &color)
{
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, size/2, size/2);
}

// dot detection

bool StegEngine::hasPRTMarkers(const QImage &image) const
{
    if (image.isNull()) return false;

    // ONLY CHECK CENTER
    QPoint centerPoint(image.width() / 2, image.height() / 2);
    QColor centerColor = image.pixelColor(centerPoint);
    qreal centerBrightness = centerColor.redF() * 0.299 + centerColor.greenF() * 0.587 + centerColor.blueF() * 0.114;

    // Check if center has a contrasting marker
    bool hasCenterMarker = (centerBrightness < 0.3 || centerBrightness > 0.7);


    return hasCenterMarker;
}

int StegEngine::calculateOptimalDotSize(const QImage& image) const
{
    if (image.isNull()) return 20; // Fallback

    int minDimension = qMin(image.width(), image.height());

    // Relative sizing with intelligent bounds
    int baseSize = 0;

    if (minDimension <= 640) {
        baseSize = minDimension * 0.03; // 3% for small images
    } else if (minDimension <= 1920) {
        baseSize = minDimension * 0.02; // 2% for HD images
    } else if (minDimension <= 3840) {
        baseSize = minDimension * 0.015; // 1.5% for 4K
    } else {
        baseSize = minDimension * 0.01; // 1% for huge images
    }

    // Ensure reasonable bounds
    baseSize = qMax(baseSize, 8);   // Never smaller than 8px
    baseSize = qMin(baseSize, 120); // Never larger than 120px


    return baseSize;
}

