#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

#include <QMediaPlayer>
#include <QAudioOutput>

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    // Public methods (no Q_INVOKABLE needed)
    void playFile(const QString &filePath);
    void play();
    void pause();
    void stop();
    void setVolume(int percent);
    void setPosition(qint64 milliseconds);

    // Getters
    qint64 duration() const;
    qint64 position() const;
    int volume() const;
    bool isPlaying() const;
    QString currentFile() const { return m_currentFile; }
    //QMediaPlayer::PlaybackState getPlaybackState() const { return m_player->playbackState(); }
signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void volumeChanged(int volume);
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void errorOccurred(const QString &errorString);
    void mediaLoaded(const QString &fileName);

private slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QString m_currentFile;
    bool m_userSeeking;
};
#endif // AUDIOPLAYER_H
