#ifndef TEXTSTEGENGINE_H
#define TEXTSTEGENGINE_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QDebug>

class TextStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit TextStegEngine(QObject *parent = nullptr);
    ~TextStegEngine();

    // Core steganography functions
    QString hideInText(const QString &coverText, const QByteArray &secretData);
    QByteArray extractFromText(const QString &stegoText);

    // Utility functions
    bool containsHiddenData(const QString &text);
    int calculateCapacity(const QString &coverText) const;
    int getHiddenDataSize(const QString &stegoText) const;

    // Error handling
    QString lastError() const { return m_lastError; }

private:
    // Unicode zero-width characters
    const QChar ZW_SPACE = QChar(0x200B);      // Zero Width Space - binary 0
    const QChar ZW_NON_JOINER = QChar(0x200C); // Zero Width Non-Joiner - binary 1
    const QChar ZW_JOINER = QChar(0x200D);     // Zero Width Joiner - marker/delimiter

    // Header constants
    static constexpr int HEADER_SIZE = 4;      // 4-byte header for data size
    static constexpr char TEXT_MARKER[] = "TEXT"; // Optional marker

    // Conversion helpers
    QString bytesToBinary(const QByteArray &data) const;
    QByteArray binaryToBytes(const QString &binary) const;

    // Header management
    QByteArray prepareDataWithHeader(const QByteArray &secretData) const;
    bool extractHeader(const QByteArray &data, quint32 &dataSize) const;

    // Error tracking
    QString m_lastError;
};

#endif // TEXTSTEGENGINE_H