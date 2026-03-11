#ifndef AUDIOSTEGENGINE_H
#define AUDIOSTEGENGINE_H

#include <QObject>

#include <QByteArray>
#include <QAudioFormat>
#include <QFile>
#include <QDebug>
#include <openssl/evp.h>
#include <openssl/sha.h>

class AudioStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit AudioStegEngine(QObject *parent = nullptr);
    ~AudioStegEngine();

    bool embedDataInAudio(const QByteArray &audioData,
                          QByteArray &stegoAudio,
                          const QByteArray &secretData,
                          int sampleRate = 44100,
                          int channels = 2,
                          int bitsPerSample = 16,
                          int lsbCount = 2);


    QByteArray extractDataFromAudio(const QByteArray &stegoAudio,
                                     int sampleRate = 44100,
                                     int channels = 2,
                                     int bitsPerSample = 16,
                                     int lsbCount = 2) const;

    // ===== CAPACITY CALCULATION =====


    int calculateAudioCapacity(int totalSamples, int channels, int bitsPerSample, int lsbCount = 2) const;


    int getSampleCountFromData(const QByteArray &audioData, int bitsPerSample = 16) const;

    // ===== METADATA & ANALYSIS =====

    struct AudioMetadata {
        int sampleRate = 0;
        int channels = 0;
        int bitsPerSample = 0;
        int durationSeconds = 0;
        qint64 sampleCount = 0;
        qint64 dataSize = 0;
        int capacityBytes = 0;      // With default LSB settings
        bool isValid = false;
    };


    AudioMetadata analyzeAudioFile(const QString &filePath);


    AudioMetadata analyzeAudioData(const QByteArray &audioData,
                                   int sampleRate,
                                   int channels,
                                   int bitsPerSample) const;

    // ===== AUDIO MARKERS (like PRT but for audio) =====


    bool hasAudioMarkers(const QByteArray &audioData) const;


    void addAudioMarkers(QByteArray &audioData);

    // ===== UTILITY =====


    bool isValidAudio(const QByteArray &audioData, int bitsPerSample = 16) const;

private:
    // ===== PRIVATE HELPERS (like StegEngine) =====


    QByteArray extractAudioHeader(const QByteArray &audioData,
                                  int channels,
                                  int bitsPerSample,
                                  int lsbCount);


    unsigned char extractByteFromAudio(const QByteArray &audioData,
                                       int startBitPosition,
                                       int channels,
                                       int bitsPerSample,
                                       int lsbCount);


    void embedByteIntoAudio(QVector<qint16> &samples,
                            int startBitPosition,
                            unsigned char byte,
                            int channels,
                            int lsbCount);


    int getBitFromSamples(const QVector<qint16> &samples, int bitPosition, int lsbCount) const;


    void setBitInSamples(QVector<qint16> &samples, int bitPosition, int bit, int lsbCount) const;

    // ===== MARKER CONSTANTS (like PRT) =====
    static constexpr char AUDIO_MARKER[] = "ATR";  // Audio Transcendental Resonance
    static constexpr char MARKER_VERSION = 0x01;
    static constexpr int MARKER_FREQUENCY = 440;   // A440 tone for marker detection
    QVector<qint16> bytesToSamples(const QByteArray &audioData) const;
    QByteArray samplesToBytes(const QVector<qint16> &samples) const;

};

#endif // AUDIOSTEGENGINE_H
