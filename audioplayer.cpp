#include "audioplayer.h"

#include <QDebug>
#include <QUrl>
#include <QFileInfo>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_userSeeking(false)
{
    m_player->setAudioOutput(m_audioOutput);

    // Connect internal signals
    connect(m_player, &QMediaPlayer::positionChanged,
            this, &AudioPlayer::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &AudioPlayer::onDurationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &AudioPlayer::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &AudioPlayer::onPlaybackStateChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &AudioPlayer::onErrorOccurred);

    setVolume(70);
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

void AudioPlayer::playFile(const QString &filePath)
{
    if (filePath.isEmpty()) return;

    if (filePath == m_currentFile) {
        // Toggle play/pause for same file
        if (isPlaying()) {
            pause();
        } else {
            play();
        }
        return;
    }

    stop();
    m_currentFile = filePath;
    m_player->setSource(QUrl::fromLocalFile(filePath));
    emit mediaLoaded(QFileInfo(filePath).fileName());
    play(); // Auto-play
}

void AudioPlayer::play()
{
    m_player->play();
}

void AudioPlayer::pause()
{
    m_player->pause();
}

void AudioPlayer::stop()
{
    m_player->stop();
    m_currentFile.clear();
    emit playbackStopped();
}

void AudioPlayer::setPosition(qint64 milliseconds)
{
    m_userSeeking = true;
    m_player->setPosition(milliseconds);
    m_userSeeking = false;
    emit positionChanged(milliseconds);
}

void AudioPlayer::setVolume(int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    m_audioOutput->setVolume(percent / 100.0);
    emit volumeChanged(percent);
}

qint64 AudioPlayer::duration() const
{
    return m_player->duration();
}

qint64 AudioPlayer::position() const
{
    return m_player->position();
}

int AudioPlayer::volume() const
{
    return static_cast<int>(m_audioOutput->volume() * 100);
}

bool AudioPlayer::isPlaying() const
{
    return m_player->playbackState() == QMediaPlayer::PlayingState;
}

void AudioPlayer::onPositionChanged(qint64 position)
{
    if (!m_userSeeking) {
        emit positionChanged(position);
    }
}

void AudioPlayer::onDurationChanged(qint64 duration)
{
    emit durationChanged(duration);
}

void AudioPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        stop();
    }
}

void AudioPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
        case QMediaPlayer::PlayingState:
            emit playbackStarted();
            break;
        case QMediaPlayer::PausedState:
            emit playbackPaused();
            break;
        case QMediaPlayer::StoppedState:
            emit playbackStopped();
            break;
    }
}

void AudioPlayer::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error);
    emit errorOccurred(errorString);
}
