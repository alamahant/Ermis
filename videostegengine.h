#ifndef VIDEOSTEGENGINE_H
#define VIDEOSTEGENGINE_H

#include <QObject>
#include <QByteArray>
#include <QImage>
#include <QVector>
#include "ffmpeghandler.h"
#include "stegengine.h"

class VideoStegEngine : public QObject
{
    Q_OBJECT

public:
    struct VideoMetadata {
        bool isValid = false;
        int width = 0;
        int height = 0;
        int frameCount = 0;
        double fps = 0.0;
        int totalPixels = 0;
        int audioSampleRate = 0;
        int audioChannels = 0;
        int audioBitsPerSample = 0;
        qint64 audioDataSize = 0;
        double durationSeconds = 0.0;
        qint64 capacityBytes = 0;
        QString codec;
        QString pixelFormat;
    };

    explicit VideoStegEngine(QObject *parent = nullptr);
    ~VideoStegEngine();

    // Core steganography functions
    bool embedDataInVideo(const QString &inputVideoPath,
                          const QString &outputVideoPath,
                          const QByteArray &secretData,
                          int lsbCount = 2,
                          bool useAudio = false,
                          int framesToUse = -1);  // -1 = use all frames

    QByteArray extractDataFromVideo(const QString &stegoVideoPath,
                                    int lsbCount = 2,
                                    bool useAudio = false,
                                    int framesToScan = -1);  // -1 = scan all

    // Capacity calculation
    qint64 calculateVideoCapacity(const QString &videoPath,
                                  int lsbCount = 2,
                                  bool useAudio = false) const;

    // Metadata & analysis
    VideoMetadata analyzeVideo(const QString &videoPath) const;
    VideoMetadata analyzeVideoData(const QByteArray &videoData,
                                   int width, int height,
                                   int frameCount, double fps) const;

    // Validation
    bool isValidVideo(const QString &videoPath) const;

    // Markers (similar to AUDIO_MARKER in AudioStegEngine)
    static constexpr char VIDEO_MARKER[] = "VIDE";
    static constexpr char MARKER_VERSION = 0x01;

private:
    FFmpegHandler *m_ffmpeg;
    StegEngine *m_imageEngine;  // Reuse image LSB for frames

    // Frame-based embedding (reuses StegEngine's image LSB)
    bool embedInFrames(const QList<QImage> &frames,
                       QList<QImage> &stegoFrames,
                       const QByteArray &data,
                       int lsbCount,
                       int startFrame,
                       int framesToUse);

    QByteArray extractFromFrames(const QList<QImage> &frames,
                                 int lsbCount,
                                 int startFrame,
                                 int framesToScan);

    // Audio track embedding (reuses AudioStegEngine)
    bool embedInAudioTrack(const QByteArray &audioData,
                           QByteArray &stegoAudio,
                           const QByteArray &data,
                           int sampleRate,
                           int channels,
                           int bitsPerSample,
                           int lsbCount);

    QByteArray extractFromAudioTrack(const QByteArray &stegoAudio,
                                     int sampleRate,
                                     int channels,
                                     int bitsPerSample,
                                     int lsbCount);

    // Header management (4-byte size header, same as audio)
    QByteArray prepareDataWithHeader(const QByteArray &secretData) const;
    bool extractHeader(const QByteArray &data, quint32 &dataSize) const;

    // Capacity helpers
    int calculateFramesCapacity(int frameCount, int width, int height,
                                int lsbCount) const;
    int calculateAudioTrackCapacity(const VideoMetadata &meta,
                                    int lsbCount) const;

    // Distribution strategies
    enum DistributionStrategy {
        FramesOnly,
        AudioOnly,
        BothBalanced,
        BothPriorityVideo,
        BothPriorityAudio
    };

    bool distributeData(const QByteArray &fullData,
                        QByteArray &videoPortion,
                        QByteArray &audioPortion,
                        const VideoMetadata &meta,
                        int lsbCount,
                        DistributionStrategy strategy);
public:
    static FFmpegHandler::VideoInfo toFFmpegVideoInfo(const VideoMetadata &meta);

};

#endif // VIDEOSTEGENGINE_H
