#include "audiostegengine.h"


#include <QRandomGenerator>
#include <QAudioFormat>
#include <cmath>


AudioStegEngine::AudioStegEngine(QObject *parent)
    : QObject(parent)
{
}

AudioStegEngine::~AudioStegEngine()
{
}


bool AudioStegEngine::embedDataInAudio(const QByteArray &audioData,
                                       QByteArray &stegoAudio,
                                       const QByteArray &secretData,
                                       int sampleRate,
                                       int channels,
                                       int bitsPerSample,
                                       int lsbCount)
{
    if (audioData.isEmpty() || secretData.isEmpty()) return false;

    int dataPos = audioData.indexOf("data");
    if (dataPos == -1) return false;
    dataPos += 8; // skip "data" + size field

    QByteArray wavHeader = audioData.left(dataPos);
    QByteArray samplesData = audioData.mid(dataPos);
    QVector<qint16> samples = bytesToSamples(samplesData);

    int maxBytes = (samples.size() * lsbCount) / 8 - 4;
    if (secretData.size() + 4 > maxBytes) return false;

    // 4-byte header
    QByteArray fullData(4 + secretData.size(), 0);
    quint32 dataSize = secretData.size();
    fullData[0] = (dataSize >> 24) & 0xFF;
    fullData[1] = (dataSize >> 16) & 0xFF;
    fullData[2] = (dataSize >> 8) & 0xFF;
    fullData[3] = dataSize & 0xFF;
    memcpy(fullData.data() + 4, secretData.constData(), secretData.size());

    // Embed each byte
    for (int byteIndex = 0; byteIndex < fullData.size(); byteIndex++) {
        unsigned char currentByte = static_cast<unsigned char>(fullData[byteIndex]);
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            int bit = (currentByte >> bitIndex) & 1;
            int globalBitPos = (byteIndex * 8 + bitIndex) * lsbCount; // multiply by lsbCount
            for (int l = 0; l < lsbCount; l++) {
                setBitInSamples(samples, globalBitPos + l, bit, lsbCount);
            }
        }
    }

    stegoAudio = wavHeader + samplesToBytes(samples);
    return true;
}


QByteArray AudioStegEngine::extractDataFromAudio(const QByteArray &stegoAudio,
                                                  int sampleRate,
                                                  int channels,
                                                  int bitsPerSample,
                                                  int lsbCount) const
{
    if (stegoAudio.isEmpty()) return QByteArray();

    int dataPos = stegoAudio.indexOf("data");
    if (dataPos == -1) return QByteArray();
    dataPos += 8;

    QByteArray samplesData = stegoAudio.mid(dataPos);
    QVector<qint16> samples = bytesToSamples(samplesData);

    // Extract 4-byte header
    QByteArray dataHeader(4, 0);
    for (int byteIndex = 0; byteIndex < 4; byteIndex++) {
        unsigned char byte = 0;
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            int globalBitPos = (byteIndex * 8 + bitIndex) * lsbCount; // multiply by lsbCount
            int sum = 0;
            for (int l = 0; l < lsbCount; l++) sum += getBitFromSamples(samples, globalBitPos + l, lsbCount);
            byte |= ((sum > lsbCount / 2) ? 1 : 0) << bitIndex;
        }
        dataHeader[byteIndex] = byte;
    }

    quint32 dataSize = (static_cast<quint32>(static_cast<unsigned char>(dataHeader[0])) << 24) |
                       (static_cast<quint32>(static_cast<unsigned char>(dataHeader[1])) << 16) |
                       (static_cast<quint32>(static_cast<unsigned char>(dataHeader[2])) << 8) |
                        static_cast<quint32>(static_cast<unsigned char>(dataHeader[3]));

    int maxCapacity = (samples.size() * lsbCount) / 8 - 4;
    if (dataSize == 0 || dataSize > maxCapacity) return QByteArray();

    QByteArray extractedData(dataSize, 0);
    for (int byteIndex = 0; byteIndex < dataSize; byteIndex++) {
        unsigned char byte = 0;
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            int globalBitPos = ((4 + byteIndex) * 8 + bitIndex) * lsbCount; // multiply by lsbCount
            int sum = 0;
            for (int l = 0; l < lsbCount; l++) sum += getBitFromSamples(samples, globalBitPos + l, lsbCount);
            byte |= ((sum > lsbCount / 2) ? 1 : 0) << bitIndex;
        }
        extractedData[byteIndex] = byte;
    }

    return extractedData;
}

// ===== CAPACITY CALCULATION =====

int AudioStegEngine::calculateAudioCapacity(int totalSamples, int channels, int bitsPerSample, int lsbCount) const
{
    // Each sample provides bitsPerSample bits, but we only use lsbCount bits per sample
    // Total bits available = totalSamples * channels * lsbCount
    // Subtract 32 bits (4 bytes) for header
    qint64 totalBits = static_cast<qint64>(totalSamples) * channels * lsbCount;
    qint64 dataBits = totalBits - 32;  // Subtract header

    if (dataBits <= 0) return 0;

    return static_cast<int>(dataBits / 8);  // Convert to bytes
}

int AudioStegEngine::getSampleCountFromData(const QByteArray &audioData, int bitsPerSample) const
{
    if (audioData.isEmpty()) return 0;
    int bytesPerSample = bitsPerSample / 8;
    return audioData.size() / bytesPerSample;
}

// ===== METADATA & ANALYSIS =====

AudioStegEngine::AudioMetadata AudioStegEngine::analyzeAudioFile(const QString &filePath)
{
    AudioMetadata meta;
    meta.isValid = false;

    QFile file(filePath);
    if (!file.exists()) {
        return meta;
    }

    return meta;
}

AudioStegEngine::AudioMetadata AudioStegEngine::analyzeAudioData(const QByteArray &audioData,
                                                                  int sampleRate,
                                                                  int channels,
                                                                  int bitsPerSample) const
{
    AudioMetadata meta;
    meta.isValid = !audioData.isEmpty() && sampleRate > 0 && channels > 0;

    if (!meta.isValid) return meta;

    meta.sampleRate = sampleRate;
    meta.channels = channels;
    meta.bitsPerSample = bitsPerSample;
    meta.dataSize = audioData.size();
    meta.sampleCount = getSampleCountFromData(audioData, bitsPerSample);
    meta.durationSeconds = meta.sampleCount / sampleRate / channels;
    meta.capacityBytes = calculateAudioCapacity(meta.sampleCount, channels, bitsPerSample, 2);


    return meta;
}

// ===== AUDIO MARKERS (like PRT) =====

bool AudioStegEngine::hasAudioMarkers(const QByteArray &audioData) const
{
    if (audioData.size() < 1024) return false;  // Too small

    // Convert to samples
    QVector<qint16> samples = bytesToSamples(audioData);

    // Look for a 440 Hz tone pattern (simplified detection)
    // In reality, you'd do FFT to check frequency content
    // This is a placeholder for the "ATR" concept

    // For now, try extracting and checking header
    QByteArray extracted = extractDataFromAudio(audioData, 44100, 2, 16, 1);
    if (extracted.size() >= 4 &&
        extracted.startsWith(AUDIO_MARKER) &&
        extracted[3] == MARKER_VERSION) {
        return true;
    }

    return false;
}

void AudioStegEngine::addAudioMarkers(QByteArray &audioData)
{
    if (audioData.isEmpty()) return;

    // Convert to samples
    QVector<qint16> samples = bytesToSamples(audioData);

    // Add a subtle 440 Hz tone at the beginning (for detection)
    // This is a placeholder - you'd generate a sine wave here

    QByteArray markerData;
    markerData.append(AUDIO_MARKER);
    markerData.append(MARKER_VERSION);

    // Convert back
    audioData = samplesToBytes(samples);

}

// ===== UTILITY FUNCTIONS =====

bool AudioStegEngine::isValidAudio(const QByteArray &audioData, int bitsPerSample) const
{
    if (audioData.isEmpty()) return false;
    int bytesPerSample = bitsPerSample / 8;
    return (audioData.size() % bytesPerSample) == 0;
}

// ===== PRIVATE HELPERS =====

QByteArray AudioStegEngine::extractAudioHeader(const QByteArray &audioData,
                                                int channels,
                                                int bitsPerSample,
                                                int lsbCount)
{
    QByteArray header(4, 0);
    // Implementation similar to extractDataFromAudio but only for first 4 bytes
    return header;
}

unsigned char AudioStegEngine::extractByteFromAudio(const QByteArray &audioData,
                                                     int startBitPosition,
                                                     int channels,
                                                     int bitsPerSample,
                                                     int lsbCount)
{
    unsigned char byte = 0;
    // Implementation here (extract one byte)
    return byte;
}

void AudioStegEngine::embedByteIntoAudio(QVector<qint16> &samples,
                                          int startBitPosition,
                                          unsigned char byte,
                                          int channels,
                                          int lsbCount)
{
    // Implementation here (embed one byte)
}


int AudioStegEngine::getBitFromSamples(const QVector<qint16> &samples, int globalBitPos, int lsbCount) const
{
    if (lsbCount <= 0) return 0;

    // Which sample and which LSB inside that sample
    int sampleIndex = globalBitPos / lsbCount;
    int bitOffset = globalBitPos % lsbCount;

    if (sampleIndex >= samples.size()) return 0;

    return (samples[sampleIndex] >> bitOffset) & 1;
}

// Set the bit at a specific global bit position in samples
void AudioStegEngine::setBitInSamples(QVector<qint16> &samples, int globalBitPos, int bit, int lsbCount) const
{
    if (lsbCount <= 0) return;

    int sampleIndex = globalBitPos / lsbCount;
    int bitOffset = globalBitPos % lsbCount;

    if (sampleIndex >= samples.size()) return;

    qint16 &sample = samples[sampleIndex];
    sample = (sample & ~(1 << bitOffset)) | ((bit & 1) << bitOffset);
}

QVector<qint16> AudioStegEngine::bytesToSamples(const QByteArray &audioData) const
{
    int sampleCount = audioData.size() / sizeof(qint16);

    QVector<qint16> samples(sampleCount);

    memcpy(samples.data(), audioData.constData(), sampleCount * sizeof(qint16));

    return samples;
}

QByteArray AudioStegEngine::samplesToBytes(const QVector<qint16> &samples) const
{
    QByteArray bytes(samples.size() * sizeof(qint16), 0);

    memcpy(bytes.data(), samples.constData(), bytes.size());

    return bytes;
}
