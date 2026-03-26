#include "videostegengine.h"
#include <QFile>
#include <QImage>
#include <QDebug>
#include <QBuffer>
#include <QtConcurrent/QtConcurrent>

VideoStegEngine::VideoStegEngine(QObject *parent)
    : QObject(parent)
    , m_ffmpeg(new FFmpegHandler(this))
    , m_imageEngine(new StegEngine(this))
{
}

VideoStegEngine::~VideoStegEngine()
{
}

// ===== CORE STEGANOGRAPHY FUNCTIONS =====

bool VideoStegEngine::embedDataInVideo(const QString &inputVideoPath,
                                       const QString &outputVideoPath,
                                       const QByteArray &secretData,
                                       int lsbCount,
                                       bool useAudio,
                                       int framesToUse)
{
    if (secretData.isEmpty()) return false;
    if (!QFile::exists(inputVideoPath)) return false;

    // Analyze video to get metadata
    VideoMetadata meta = analyzeVideo(inputVideoPath);
    if (!meta.isValid) return false;

    // Check capacity
    qint64 capacity = calculateVideoCapacity(inputVideoPath, lsbCount, useAudio);
    if (secretData.size() > capacity) {
        return false;
    }

    // Prepare data with 4-byte header (same as audio engine)
    QByteArray fullData = prepareDataWithHeader(secretData);

    // Extract frames using FFmpeg
    QList<QImage> frames;
    QByteArray audioData;

    // First, convert VideoMetadata to FFmpegHandler::VideoInfo
    FFmpegHandler::VideoInfo ffmpegInfo = VideoStegEngine::toFFmpegVideoInfo(meta);

    // use the converted info in demuxVideo
    if (!m_ffmpeg->demuxVideo(inputVideoPath, frames, audioData, ffmpegInfo)) {
        return false;
    }


    QList<QImage> stegoFrames = frames;

    if (framesToUse == -1) framesToUse = frames.size();

    if (!useAudio) {
        // Hide only in video frames
        if (!embedInFrames(frames, stegoFrames, fullData, lsbCount, 0, framesToUse)) {
            return false;
        }
    } else {
        // Distribute between frames and audio
        QByteArray videoPortion, audioPortion;
        if (!distributeData(fullData, videoPortion, audioPortion, meta, 
                           lsbCount, BothBalanced)) {
            return false;
        }

        // Hide in frames
        if (!videoPortion.isEmpty()) {
            if (!embedInFrames(frames, stegoFrames, videoPortion, lsbCount, 
                              0, framesToUse)) {
                return false;
            }
        }

        // Hide in audio track
        if (!audioPortion.isEmpty()) {
            QByteArray stegoAudio;
            if (!embedInAudioTrack(audioData, stegoAudio, audioPortion,
                                   meta.audioSampleRate, meta.audioChannels,
                                   meta.audioBitsPerSample, lsbCount)) {
                return false;
            }
            audioData = stegoAudio;
        }
    }

    // Remux video with modified frames and audio

    // Then use the converted info in remuxVideo
    return m_ffmpeg->remuxVideo(outputVideoPath, stegoFrames, audioData, ffmpegInfo);
}

QByteArray VideoStegEngine::extractDataFromVideo(const QString &stegoVideoPath,
                                                 int lsbCount,
                                                 bool useAudio,
                                                 int framesToScan)
{
    if (!QFile::exists(stegoVideoPath)) return QByteArray();

    // Analyze video
    VideoMetadata meta = analyzeVideo(stegoVideoPath);
    if (!meta.isValid) return QByteArray();

    // Demux video
    QList<QImage> frames;
    QByteArray audioData;

    FFmpegHandler::VideoInfo ffmpegInfo = VideoStegEngine::toFFmpegVideoInfo(meta);

    if (!m_ffmpeg->demuxVideo(stegoVideoPath, frames, audioData, ffmpegInfo)) {
        return QByteArray();
    }

    QByteArray extractedFromFrames;
    QByteArray extractedFromAudio;

    // Extract from frames
    if (!frames.isEmpty()) {
        if (framesToScan == -1) framesToScan = frames.size();
        extractedFromFrames = extractFromFrames(frames, lsbCount, 0, framesToScan);
    }

    // Extract from audio if present and requested
    if (useAudio && !audioData.isEmpty()) {
        extractedFromAudio = extractFromAudioTrack(audioData,
                                                   meta.audioSampleRate,
                                                   meta.audioChannels,
                                                   meta.audioBitsPerSample,
                                                   lsbCount);
    }

    // Combine based on which one has valid header
    quint32 dataSize;
    if (extractHeader(extractedFromFrames, dataSize) && 
        dataSize > 0 && dataSize <= extractedFromFrames.size() - 4) {
        return extractedFromFrames.mid(4, dataSize);
    } else if (extractHeader(extractedFromAudio, dataSize) && 
               dataSize > 0 && dataSize <= extractedFromAudio.size() - 4) {
        return extractedFromAudio.mid(4, dataSize);
    }

    return QByteArray();
}

// ===== FRAME-BASED EMBEDDING (REUSES STEGENGINE) =====

bool VideoStegEngine::embedInFrames(const QList<QImage> &frames,
                                    QList<QImage> &stegoFrames,
                                    const QByteArray &data,
                                    int lsbCount,
                                    int startFrame,
                                    int framesToUse)
{
    if (frames.isEmpty() || data.isEmpty()) return false;

    int bytesPerFrame = (frames[0].width() * frames[0].height() * 3 * lsbCount) / 8;
    int framesNeeded = (data.size() + bytesPerFrame - 1) / bytesPerFrame;

    if (startFrame + framesNeeded > frames.size()) {
        return false;
    }

    int dataPos = 0;
    for (int i = 0; i < framesNeeded && i < framesToUse; i++) {
        int frameIndex = startFrame + i;
        int bytesForThisFrame = qMin(bytesPerFrame, data.size() - dataPos);
        QByteArray frameData = data.mid(dataPos, bytesForThisFrame);

        // Use StegEngine to embed in this frame
        QImage modifiedFrame;
        if (!m_imageEngine->embedDataInImage(frames[frameIndex], 
                                             modifiedFrame, 
                                             frameData)) {
            return false;
        }

        stegoFrames[frameIndex] = modifiedFrame;
        dataPos += bytesForThisFrame;
    }

    return true;
}

QByteArray VideoStegEngine::extractFromFrames(const QList<QImage> &frames,
                                              int lsbCount,
                                              int startFrame,
                                              int framesToScan)
{
    if (frames.isEmpty()) return QByteArray();

    QByteArray allExtracted;
    int bytesPerFrame = (frames[0].width() * frames[0].height() * 3 * lsbCount) / 8;

    // Extract from each frame until we find the header
    for (int i = 0; i < framesToScan && i < frames.size(); i++) {
        int frameIndex = startFrame + i;
        QByteArray frameData = m_imageEngine->extractDataFromImage(frames[frameIndex]);
        allExtracted.append(frameData);

        // Check if we have a complete header
        if (allExtracted.size() >= 4) {
            quint32 dataSize;
            if (extractHeader(allExtracted, dataSize) && 
                dataSize > 0 && 
                dataSize <= allExtracted.size() - 4) {
                return allExtracted;
            }
        }
    }

    return allExtracted;
}

// ===== AUDIO TRACK EMBEDDING (REUSES AUDIOSTEGENGINE) =====

bool VideoStegEngine::embedInAudioTrack(const QByteArray &audioData,
                                        QByteArray &stegoAudio,
                                        const QByteArray &data,
                                        int sampleRate,
                                        int channels,
                                        int bitsPerSample,
                                        int lsbCount)
{
    // This would call your existing AudioStegEngine
    // For now, placeholder - you'd integrate with AudioStegEngine
    Q_UNUSED(audioData)
    Q_UNUSED(stegoAudio)
    Q_UNUSED(data)
    Q_UNUSED(sampleRate)
    Q_UNUSED(channels)
    Q_UNUSED(bitsPerSample)
    Q_UNUSED(lsbCount)
    
    // TODO: Integrate with your AudioStegEngine
    return false;
}

QByteArray VideoStegEngine::extractFromAudioTrack(const QByteArray &stegoAudio,
                                                  int sampleRate,
                                                  int channels,
                                                  int bitsPerSample,
                                                  int lsbCount)
{
    // This would call your existing AudioStegEngine
    Q_UNUSED(stegoAudio)
    Q_UNUSED(sampleRate)
    Q_UNUSED(channels)
    Q_UNUSED(bitsPerSample)
    Q_UNUSED(lsbCount)
    
    // TODO: Integrate with your AudioStegEngine
    return QByteArray();
}

// ===== HEADER MANAGEMENT (SAME AS AUDIO) =====

QByteArray VideoStegEngine::prepareDataWithHeader(const QByteArray &secretData) const
{
    QByteArray fullData(4 + secretData.size(), 0);
    quint32 dataSize = secretData.size();
    fullData[0] = (dataSize >> 24) & 0xFF;
    fullData[1] = (dataSize >> 16) & 0xFF;
    fullData[2] = (dataSize >> 8) & 0xFF;
    fullData[3] = dataSize & 0xFF;
    memcpy(fullData.data() + 4, secretData.constData(), secretData.size());
    return fullData;
}

bool VideoStegEngine::extractHeader(const QByteArray &data, quint32 &dataSize) const
{
    if (data.size() < 4) return false;

    dataSize = (static_cast<quint32>(static_cast<unsigned char>(data[0])) << 24) |
               (static_cast<quint32>(static_cast<unsigned char>(data[1])) << 16) |
               (static_cast<quint32>(static_cast<unsigned char>(data[2])) << 8) |
                static_cast<quint32>(static_cast<unsigned char>(data[3]));

    return true;
}

// ===== CAPACITY CALCULATION =====

qint64 VideoStegEngine::calculateVideoCapacity(const QString &videoPath,
                                               int lsbCount,
                                               bool useAudio) const
{
    VideoMetadata meta = analyzeVideo(videoPath);
    if (!meta.isValid) return 0;

    qint64 capacity = calculateFramesCapacity(meta.frameCount, meta.width, 
                                              meta.height, lsbCount);

    if (useAudio && meta.audioDataSize > 0) {
        capacity += calculateAudioTrackCapacity(meta, lsbCount);
    }

    return capacity;
}

int VideoStegEngine::calculateFramesCapacity(int frameCount, int width, int height,
                                             int lsbCount) const
{
    if (frameCount <= 0 || width <= 0 || height <= 0) return 0;

    // Total pixels * 3 channels (RGB) * lsbCount bits per channel
    qint64 totalBits = static_cast<qint64>(frameCount) * width * height * 3 * lsbCount;
    qint64 dataBits = totalBits - 32;  // Subtract 4-byte header

    return (dataBits > 0) ? static_cast<int>(dataBits / 8) : 0;
}

int VideoStegEngine::calculateAudioTrackCapacity(const VideoMetadata &meta,
                                                 int lsbCount) const
{
    if (meta.audioDataSize <= 0) return 0;

    int bytesPerSample = meta.audioBitsPerSample / 8;
    int sampleCount = meta.audioDataSize / bytesPerSample;

    qint64 totalBits = static_cast<qint64>(sampleCount) * meta.audioChannels * lsbCount;
    qint64 dataBits = totalBits - 32;  // Subtract 4-byte header

    return (dataBits > 0) ? static_cast<int>(dataBits / 8) : 0;
}

// ===== DATA DISTRIBUTION =====

bool VideoStegEngine::distributeData(const QByteArray &fullData,
                                     QByteArray &videoPortion,
                                     QByteArray &audioPortion,
                                     const VideoMetadata &meta,
                                     int lsbCount,
                                     DistributionStrategy strategy)
{
    qint64 videoCapacity = calculateFramesCapacity(meta.frameCount, meta.width,
                                                   meta.height, lsbCount);
    qint64 audioCapacity = calculateAudioTrackCapacity(meta, lsbCount);

    switch (strategy) {
        case FramesOnly:
            videoPortion = fullData;
            audioPortion.clear();
            return videoPortion.size() <= videoCapacity;

        case AudioOnly:
            videoPortion.clear();
            audioPortion = fullData;
            return audioPortion.size() <= audioCapacity;

        case BothBalanced: {
            int totalCapacity = videoCapacity + audioCapacity;
            if (fullData.size() > totalCapacity) return false;

            // Try to put as much as possible in frames first
            int videoBytes = qMin(fullData.size(), (int)videoCapacity);
            videoPortion = fullData.left(videoBytes);
            audioPortion = fullData.mid(videoBytes);
            return true;
        }

        default:
            return false;
    }

    return false;
}

// ===== METADATA & ANALYSIS =====

VideoStegEngine::VideoMetadata VideoStegEngine::analyzeVideo(const QString &videoPath) const
{
    VideoMetadata meta;
    meta.isValid = false;

    // Use FFmpegHandler to get video info
    auto ffmpegInfo = m_ffmpeg->getVideoInfo(videoPath);
    if (!ffmpegInfo.isValid) return meta;

    meta.isValid = true;
    meta.width = ffmpegInfo.width;
    meta.height = ffmpegInfo.height;
    meta.frameCount = ffmpegInfo.frameCount;
    meta.fps = ffmpegInfo.fps;
    meta.totalPixels = meta.width * meta.height * meta.frameCount;
    meta.durationSeconds = meta.frameCount / meta.fps;
    meta.codec = ffmpegInfo.codec;

    // Get audio info if present
    auto audioInfo = m_ffmpeg->getAudioInfo(videoPath);
    if (audioInfo.isValid) {
        meta.audioSampleRate = audioInfo.sampleRate;
        meta.audioChannels = audioInfo.channels;
        meta.audioBitsPerSample = audioInfo.bitsPerSample;
        meta.audioDataSize = audioInfo.dataSize;
    }

    // Calculate capacity
    meta.capacityBytes = calculateVideoCapacity(videoPath, 2, true);

    return meta;
}

VideoStegEngine::VideoMetadata VideoStegEngine::analyzeVideoData(const QByteArray &videoData,
                                                                 int width, int height,
                                                                 int frameCount, double fps) const
{
    VideoMetadata meta;
    meta.isValid = !videoData.isEmpty() && width > 0 && height > 0 && frameCount > 0;

    if (!meta.isValid) return meta;

    meta.width = width;
    meta.height = height;
    meta.frameCount = frameCount;
    meta.fps = fps;
    meta.totalPixels = width * height * frameCount;
    meta.durationSeconds = frameCount / fps;


    // Rough capacity estimate (assuming uncompressed)
    meta.capacityBytes = calculateFramesCapacity(frameCount, width, height, 2);

    return meta;
}

// ===== VALIDATION =====

bool VideoStegEngine::isValidVideo(const QString &videoPath) const
{
    VideoMetadata meta = analyzeVideo(videoPath);
    return meta.isValid;
}

// ===== MARKER FUNCTIONS (optional) =====

// You could add marker detection similar to AudioStegEngine's hasAudioMarkers
// but for video files

FFmpegHandler::VideoInfo VideoStegEngine::toFFmpegVideoInfo(const VideoStegEngine::VideoMetadata &meta)
{
    FFmpegHandler::VideoInfo info;
    info.isValid = meta.isValid;
    info.width = meta.width;
    info.height = meta.height;
    info.frameCount = meta.frameCount;
    info.fps = meta.fps;
    info.durationSeconds = meta.durationSeconds;
    info.codec = meta.codec;
    info.pixelFormat = meta.pixelFormat;  // ✅ Set from metadata
    info.bitrate = 0;  // You might want to add bitrate to VideoMetadata later

    // Map audio fields
    info.audioSampleRate = meta.audioSampleRate;
    info.audioChannels = meta.audioChannels;
    info.audioBitsPerSample = meta.audioBitsPerSample;
    info.audioDataSize = meta.audioDataSize;

    return info;
}
