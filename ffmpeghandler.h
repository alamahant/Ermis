#ifndef FFMPEGHANDLER_H
#define FFMPEGHANDLER_H

#include <QObject>

#include <QProcess>
#include <QByteArray>
#include <QString>
#include <QDir>
#include <QTemporaryFile>
#include <QDebug>

class FFmpegHandler : public QObject
{
    Q_OBJECT

public:
    explicit FFmpegHandler(QObject *parent = nullptr);
    ~FFmpegHandler();

    // ===== CONVERSION METHODS =====

    /**
     * Convert any audio file to WAV format (16-bit PCM)
     * @param inputFile Path to input audio file
     * @param outputWav Output: raw PCM data
     * @param sampleRate Desired sample rate (0 = keep original)
     * @param channels Desired channels (0 = keep original)
     * @return true if successful
     */
    bool convertToWav(const QString &inputFile,
                      QByteArray &outputWav,
                      int sampleRate = 44100,
                      int channels = 2);

    /**
     * Convert WAV data to MP3
     * @param wavData Raw PCM data
     * @param outputFile Path for output MP3
     * @param quality MP3 quality (0-9, 0=best, 9=worst)
     * @return true if successful
     */
    bool convertToMp3(const QByteArray &wavData,
                      const QString &outputFile,
                      int quality = 2);

    /**
     * Convert WAV data to any format
     * @param wavData Raw PCM data
     * @param outputFile Path for output file
     * @param format Format codec (e.g., "libmp3lame", "aac", "flac")
     * @return true if successful
     */
    bool convertToFormat(const QByteArray &wavData,
                         const QString &outputFile,
                         const QString &format);

    // ===== METADATA EXTRACTION =====

    struct AudioInfo {
        QString filePath;
        QString format;
        int sampleRate = 0;
        int channels = 0;
        int bitsPerSample = 0;
        qint64 durationMs = 0;
        qint64 durationSeconds = 0;
        qint64 bitrate = 0;
        qint64 fileSize = 0;
        bool isValid = false;
        QString error;
    };

    /**
     * Get detailed information about an audio file
     */
    AudioInfo getAudioInfo(const QString &filePath);

    /**
     * Get audio info from memory data
     */
    AudioInfo getAudioInfoFromData(const QByteArray &audioData, const QString &formatHint = "wav");

    // ===== UTILITY =====

    /**
     * Check if ffmpeg is available
     */
    bool isAvailable() const;

    /**
     * Get ffmpeg version string
     */
    QString version() const;

    /**
     * Clear temporary files
     */
    void cleanup();

signals:
    void progressUpdated(int percent);
    void errorOccurred(const QString &error);
    void ffmpegOutput(const QString &line);  // for debugging

private:
    bool runFFmpeg(const QStringList &arguments, QByteArray *outputData = nullptr);
    QString findFFmpeg() const;
    QString findFFprobe() const;

    QString createTempFile(const QString &suffix = ".wav");
    void parseFFprobeOutput(const QString &output, AudioInfo &info);

    QProcess *m_process;
    QTemporaryFile *m_tempFile;
    QString m_ffmpegPath;
    QString m_ffprobePath;
};

#endif // FFMPEGHANDLER_H
