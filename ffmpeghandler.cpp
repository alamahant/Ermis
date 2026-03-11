#include "ffmpeghandler.h"


#include <QFileInfo>
#include <QRegularExpression>
#include <QCoreApplication>

// ===== CONSTRUCTOR / DESTRUCTOR =====

FFmpegHandler::FFmpegHandler(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_tempFile(nullptr)
{
    m_ffmpegPath = findFFmpeg();
        m_ffprobePath = findFFprobe();  // Just call a separate finder


       if (m_ffmpegPath.isEmpty()) {
       }
}

FFmpegHandler::~FFmpegHandler()
{
    cleanup();
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
        delete m_process;
    }
}

// ===== CONVERSION METHODS =====

bool FFmpegHandler::convertToWav(const QString &inputFile,
                                  QByteArray &outputWav,
                                  int sampleRate,
                                  int channels)
{
    if (!QFile::exists(inputFile)) {
        emit errorOccurred("Input file does not exist: " + inputFile);
        return false;
    }

    QString tempFile = createTempFile(".wav");
    QStringList args;

    args << "-y"
         <<   "-i" << inputFile
         << "-acodec" << "pcm_s16le"  // 16-bit PCM
         << "-f" << "wav";             // WAV format

    if (sampleRate > 0) {
        args << "-ar" << QString::number(sampleRate);
    }

    if (channels > 0) {
        args << "-ac" << QString::number(channels);
    }

    args << tempFile;


    if (!runFFmpeg(args)) {
        return false;
    }

    // Read the converted file
    QFile file(tempFile);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Could not read temporary WAV file");
        return false;
    }

    outputWav = file.readAll();
    file.close();

    // Clean up temp file
    QFile::remove(tempFile);

    return true;
}

bool FFmpegHandler::convertToMp3(const QByteArray &wavData,
                                  const QString &outputFile,
                                  int quality)
{
    // Save WAV data to temp file
    QString tempWav = createTempFile(".wav");
    QFile wavFile(tempWav);
    if (!wavFile.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Could not create temporary WAV file");
        return false;
    }
    wavFile.write(wavData);
    wavFile.close();

    // Convert to MP3
    QStringList args;
    args << "-i" << tempWav
         << "-codec:a" << "libmp3lame"
         << "-qscale:a" << QString::number(quality)  // 0-9, 2 is good
         << "-y" << outputFile;  // Overwrite output


    bool success = runFFmpeg(args);

    // Clean up
    QFile::remove(tempWav);

    if (success) {
    }

    return success;
}

bool FFmpegHandler::convertToFormat(const QByteArray &wavData,
                                     const QString &outputFile,
                                     const QString &format)
{
    QString tempWav = createTempFile(".wav");
    QFile wavFile(tempWav);
    if (!wavFile.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Could not create temporary WAV file");
        return false;
    }
    wavFile.write(wavData);
    wavFile.close();

    QStringList args;
    args << "-i" << tempWav
         << "-codec:a" << format
         << "-y" << outputFile;


    bool success = runFFmpeg(args);
    QFile::remove(tempWav);

    return success;
}

// ===== METADATA EXTRACTION =====

FFmpegHandler::AudioInfo FFmpegHandler::getAudioInfo(const QString &filePath)
{
    AudioInfo info;
    info.filePath = filePath;
    info.fileSize = QFileInfo(filePath).size();

    if (!QFile::exists(filePath)) {
        info.error = "File does not exist";
        return info;
    }

    QStringList args;
    args << "-v" << "quiet"
         << "-print_format" << "json"
         << "-show_format"
         << "-show_streams"
         << filePath;

    QProcess ffprobe;
    ffprobe.start(m_ffprobePath, args);
    ffprobe.waitForFinished(5000);

    if (ffprobe.exitCode() != 0) {
        info.error = ffprobe.readAllStandardError();
        return info;
    }

    QString output = ffprobe.readAllStandardOutput();
    parseFFprobeOutput(output, info);
    info.isValid = true;

    return info;
}




FFmpegHandler::AudioInfo FFmpegHandler::getAudioInfoFromData(const QByteArray &audioData, const QString &formatHint)
{
    // Save to temp file first
    QString tempFile = createTempFile("." + formatHint);
    QFile file(tempFile);
    if (!file.open(QIODevice::WriteOnly)) {
        AudioInfo info;
        info.error = "Could not create temp file";
        return info;
    }
    file.write(audioData);
    file.close();

    AudioInfo info = getAudioInfo(tempFile);
    QFile::remove(tempFile);

    return info;
}



// ===== UTILITY =====

bool FFmpegHandler::isAvailable() const
{
    return !m_ffmpegPath.isEmpty() && QFile::exists(m_ffmpegPath);
}

QString FFmpegHandler::version() const
{
    if (!isAvailable()) return "ffmpeg not found";

    QProcess process;
    process.start(m_ffmpegPath, QStringList() << "-version");
    process.waitForFinished(2000);
    return QString(process.readAllStandardOutput()).split('\n').first();
}

void FFmpegHandler::cleanup()
{
    if (m_tempFile) {
        if (m_tempFile->isOpen()) {
            m_tempFile->close();
        }
        delete m_tempFile;
        m_tempFile = nullptr;
    }
}

// ===== PRIVATE METHODS =====

bool FFmpegHandler::runFFmpeg(const QStringList &arguments, QByteArray *outputData)
{
    if (!isAvailable()) {
        emit errorOccurred("ffmpeg not found. Please install ffmpeg.");
        return false;
    }


    // Check if input file exists (arguments index of "-i" + 1)
    int iIndex = arguments.indexOf("-i");
    if (iIndex >= 0 && iIndex + 1 < arguments.size()) {
        QString inputFile = arguments[iIndex + 1];
    }

    // Check output file path
    QString outputFile = arguments.last();

    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
        delete m_process;
    }

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    QString fullOutput;
    connect(m_process, &QProcess::readyReadStandardOutput, [this, &fullOutput]() {
        QString output = m_process->readAllStandardOutput();
        fullOutput += output;
        emit ffmpegOutput(output);
    });

    m_process->start(m_ffmpegPath, arguments);

    if (!m_process->waitForStarted(5000)) {
        emit errorOccurred("FFmpeg failed to start");
        return false;
    }

    m_process->waitForFinished(-1);

    bool success = (m_process->exitCode() == 0);

    QString errorOutput = m_process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
    }


    if (!success) {
        emit errorOccurred("ffmpeg error: " + errorOutput);
    }

    if (outputData && success) {
        *outputData = m_process->readAllStandardOutput();
    }

    m_process->deleteLater();
    m_process = nullptr;

    return success;
}

QString FFmpegHandler::findFFmpeg() const
{
    // Common locations
    QStringList paths;
    paths << "ffmpeg"  // in PATH
          << "/usr/bin/ffmpeg"
          << "/usr/local/bin/ffmpeg"
          << QCoreApplication::applicationDirPath() + "/ffmpeg";

#ifdef Q_OS_WIN
    paths << "ffmpeg.exe"
          << "C:\\ffmpeg\\bin\\ffmpeg.exe"
          << "C:\\Program Files\\ffmpeg\\bin\\ffmpeg.exe";
#endif

    for (const QString &path : paths) {
        QFileInfo info(path);
        if (info.exists() && info.isExecutable()) {
            return path;
        }
    }

    return QString();
}

QString FFmpegHandler::createTempFile(const QString &suffix)
{
    cleanup();
    m_tempFile = new QTemporaryFile(QDir::temp().absoluteFilePath("ffmpeg_XXXXXX" + suffix));
    if(!m_tempFile->open()) {
        qWarning() << "Failed to open temporary file";
        return QString();
    }
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    return fileName;
}

void FFmpegHandler::parseFFprobeOutput(const QString &output, AudioInfo &info)
{
    // Simple parsing - in reality you'd use JSON
    QRegularExpression sampleRateRegex("\"sample_rate\"\\s*:\\s*\"(\\d+)\"");
    QRegularExpression channelsRegex("\"channels\"\\s*:\\s*(\\d+)");
    QRegularExpression durationRegex("\"duration\"\\s*:\\s*\"([\\d\\.]+)\"");
    QRegularExpression bitrateRegex("\"bit_rate\"\\s*:\\s*\"(\\d+)\"");
    QRegularExpression codecRegex("\"codec_name\"\\s*:\\s*\"([^\"]+)\"");

    QRegularExpressionMatch match;

    match = sampleRateRegex.match(output);
    if (match.hasMatch()) {
        info.sampleRate = match.captured(1).toInt();
    }

    match = channelsRegex.match(output);
    if (match.hasMatch()) {
        info.channels = match.captured(1).toInt();
    }

    match = durationRegex.match(output);
    if (match.hasMatch()) {
        info.durationMs = match.captured(1).toDouble() * 1000;
        info.durationSeconds = match.captured(1).toDouble();
    }

    match = bitrateRegex.match(output);
    if (match.hasMatch()) {
        info.bitrate = match.captured(1).toInt();
    }

    match = codecRegex.match(output);
    if (match.hasMatch()) {
        info.format = match.captured(1);
    }

    // Default to 16-bit for PCM
    info.bitsPerSample = 16;
}

QString FFmpegHandler::findFFprobe() const
{
    QStringList paths;
    paths << "ffprobe"
          << "/usr/bin/ffprobe"
          << "/usr/local/bin/ffprobe"
          << QCoreApplication::applicationDirPath() + "/ffprobe";

#ifdef Q_OS_WIN
    paths << "ffprobe.exe"
          << "C:\\ffmpeg\\bin\\ffprobe.exe"
          << "C:\\Program Files\\ffmpeg\\bin\\ffprobe.exe";
#endif

    for (const QString &path : paths) {
        QFileInfo info(path);
        if (info.exists() && info.isExecutable()) {
            return path;
        }
    }
    return QString();
}
