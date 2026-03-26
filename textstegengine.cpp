#include "textstegengine.h"
#include <QRegularExpression>

TextStegEngine::TextStegEngine(QObject *parent)
    : QObject(parent)
{
}

TextStegEngine::~TextStegEngine()
{
}

QString TextStegEngine::hideInText(const QString &coverText, const QByteArray &secretData)
{
    m_lastError.clear();

    if (coverText.isEmpty()) {
        m_lastError = "Cover text is empty";
        return QString();
    }

    if (secretData.isEmpty()) {
        m_lastError = "Secret data is empty";
        return QString();
    }

    // Prepare data with 4-byte size header
    QByteArray fullData = prepareDataWithHeader(secretData);

    // Check capacity
    int capacity = calculateCapacity(coverText);
    if (fullData.size() > capacity) {
        m_lastError = QString("Secret data too large: %1 bytes, capacity: %2 bytes")
                         .arg(fullData.size()).arg(capacity);
        return QString();
    }

    // Convert secret to binary string
    QString binary = bytesToBinary(fullData);

    // Insert zero-width characters after each visible character
    QString result;
    int binaryPos = 0;

    for (int i = 0; i < coverText.length(); i++) {
        // Add the visible character
        result += coverText[i];

        // Add zero-width characters for data (one per visible char)
        if (binaryPos < binary.length()) {
            // Add one zero-width char for each bit
            for (int b = 0; b < 1 && binaryPos < binary.length(); b++) {
                if (binary[binaryPos] == '0') {
                    result += ZW_SPACE;      // 0
                } else {
                    result += ZW_NON_JOINER; // 1
                }
                binaryPos++;
            }
        }
    }

    // If we still have data left, append at the end
    while (binaryPos < binary.length()) {
        if (binary[binaryPos] == '0') {
            result += ZW_SPACE;
        } else {
            result += ZW_NON_JOINER;
        }
        binaryPos++;
    }

    return result;
}

QByteArray TextStegEngine::extractFromText(const QString &stegoText)
{
    m_lastError.clear();

    if (stegoText.isEmpty()) {
        m_lastError = "Stego text is empty";
        return QByteArray();
    }

    // Extract binary string from zero-width characters
    QString binary;
    for (const QChar &c : stegoText) {
        if (c == ZW_SPACE) {
            binary += '0';
        } else if (c == ZW_NON_JOINER) {
            binary += '1';
        }
        // Ignore ZW_JOINER and all other characters
    }


    if (binary.isEmpty()) {
        m_lastError = "No hidden data found";
        return QByteArray();
    }

    // Convert binary to bytes
    QByteArray fullData = binaryToBytes(binary);

    // Extract header to get actual data size
    quint32 dataSize;
    if (!extractHeader(fullData, dataSize)) {
        m_lastError = "Invalid header or no hidden data";
        return QByteArray();
    }


    // Make sure we have enough data
    if (fullData.size() < HEADER_SIZE + dataSize) {
        m_lastError = "Data truncated";
        return fullData.mid(HEADER_SIZE);
    }

    return fullData.mid(HEADER_SIZE, dataSize);
}

bool TextStegEngine::containsHiddenData(const QString &text)
{
    for (const QChar &c : text) {
        if (c == ZW_SPACE || c == ZW_NON_JOINER || c == ZW_JOINER) {
            return true;
        }
    }
    return false;
}

int TextStegEngine::calculateCapacity(const QString &coverText) const
{
    // Each visible character can hide 1 bit
    // Plus we can append at the end
    // So total bits = coverText.length() + extra space
    int totalBits = coverText.length() * 1 + 100; // 100 extra at end
    
    // Convert to bytes, subtract header
    int totalBytes = totalBits / 8;
    if (totalBytes <= HEADER_SIZE) return 0;
    
    return totalBytes - HEADER_SIZE;
}

int TextStegEngine::getHiddenDataSize(const QString &stegoText) const
{
    int bitCount = 0;
    for (const QChar &c : stegoText) {
        if (c == ZW_SPACE || c == ZW_NON_JOINER) {
            bitCount++;
        }
    }
    return bitCount / 8;
}

QString TextStegEngine::bytesToBinary(const QByteArray &data) const
{
    QString binary;
    for (char byte : data) {
        for (int i = 7; i >= 0; i--) {
            binary += ((byte >> i) & 1) ? '1' : '0';
        }
    }
    return binary;
}

QByteArray TextStegEngine::binaryToBytes(const QString &binary) const
{
    QByteArray data;
    int len = binary.length();

    for (int i = 0; i + 8 <= len; i += 8) {
        char byte = 0;
        for (int j = 0; j < 8; j++) {
            if (binary[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        data.append(byte);
    }

    return data;
}

QByteArray TextStegEngine::prepareDataWithHeader(const QByteArray &secretData) const
{
    QByteArray fullData(HEADER_SIZE + secretData.size(), 0);
    quint32 dataSize = secretData.size();

    // Store size in big-endian
    fullData[0] = (dataSize >> 24) & 0xFF;
    fullData[1] = (dataSize >> 16) & 0xFF;
    fullData[2] = (dataSize >> 8) & 0xFF;
    fullData[3] = dataSize & 0xFF;

    // Copy secret data
    memcpy(fullData.data() + HEADER_SIZE, secretData.constData(), secretData.size());

    return fullData;
}

bool TextStegEngine::extractHeader(const QByteArray &data, quint32 &dataSize) const
{
    if (data.size() < HEADER_SIZE) {
        return false;
    }

    dataSize = (static_cast<quint32>(static_cast<unsigned char>(data[0])) << 24) |
               (static_cast<quint32>(static_cast<unsigned char>(data[1])) << 16) |
               (static_cast<quint32>(static_cast<unsigned char>(data[2])) << 8) |
                static_cast<quint32>(static_cast<unsigned char>(data[3]));

    // Sanity check - data size shouldn't be huge
    if (dataSize > 1024 * 1024) { // Max 1MB for sanity
        return false;
    }

    return true;
}