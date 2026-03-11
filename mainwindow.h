#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QRandomGenerator>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include"passphrasedialog.h"
#include"stegengine.h"
//#include<QStandardPaths>
#include<QCheckBox>
#include<QCamera>
#include<QVideoFrame>
#include <QVideoSink>
#include<QDragEnterEvent>
#include<QCloseEvent>
#include<QDropEvent>
#include<QByteArray>
#include<QEvent>
#include"audioplayer.h"
#include<QSlider>
#include"audiostegengine.h"
#include"ffmpeghandler.h"
#include<QMenu>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File operations
    void openCarrierImage();
    void saveModifiedImage();

    // Steganography operations
    //void hideData();

    // UI state changes
    void updateCapacityStatus();
    void toggleInputMethod();
    void selectDataFile();

private:
    // UI setup methods
    void setupUi();
    void createMenus();
    void createConnections();

    // UI components
    QTabWidget *tabWidget;

    // Hide tab components
    QLabel *originalImageLabel;
    QLabel *modifiedImageLabel;
    QPushButton *openImageButton;
    QPushButton *saveImageButton;
    QRadioButton *textInputRadio;
    QRadioButton *fileInputRadio;
    QPlainTextEdit *textInput;
    QLineEdit *filePathInput;
    QPushButton *browseFileButton;
    QPushButton *hideButton;
    QLabel *capacityLabel;
    QCheckBox *encryptionCheckBox;

    // Extract tab components
    QLabel *stegoImageLabel;
    QPushButton *openStegoImageButton;
    QPushButton *extractButton;
    QPlainTextEdit *extractedTextOutput;
    QPushButton *saveExtractedFileButton;
    QPushButton *resetButton;

    // Data members
    QImage originalImage;
    QImage modifiedImage;
    QImage stegoImage;
    bool isTextInput;

    // Encryption-related members
    QString rememberedPassphrase;
    bool hasRememberedPassphrase;

private:
    Ui::MainWindow *ui;
    StegEngine *engine = nullptr;
    QCheckBox *prtCheckBox = nullptr;
    bool isPRTMode = false;
    QPushButton *cameraButton;
    QPushButton *clipboardButton;

    QCamera *camera = nullptr;
    void setupCamera();
    void stopCamera();
    QVideoSink *videoSink = nullptr;
private slots:
    void onCameraButtonClicked();
    void processCameraFrame(const QVideoFrame &frame);
    void resetAll();
    //drag drop
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    void processDroppedImage(const QString& filePath);
    QWidget *hideTab = nullptr;      // For carrier images (no markers)
    QWidget *extractTab = nullptr;   // For stego images (with markers)
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    // Zoom support
    QImage originalStegoImage;      // Original loaded stego image
    QImage originalCarrierImage;    // Original loaded carrier image
    QImage originalModifiedImage;   // Original modified image
    double stegoZoom = 1.0;         // Zoom level for stego image
    double carrierZoom = 1.0;       // Zoom level for carrier image
    double modifiedZoom = 1.0;      // Zoom level for modified image

private:
    struct ImagePan {
        bool panning = false;
        QPoint panStartPosition;
        QPoint currentOffset;
    };

    ImagePan stegoPan, carrierPan, modifiedPan;
    void updatePannedImage(QLabel *label, ImagePan &pan);

private:
    QByteArray m_currentAudioData;
    QByteArray m_modifiedAudioData;
    int m_currentSampleRate;
    int m_currentChannels;
    int m_currentBitsPerSample;
    void updateUIForMode();
    AudioPlayer *audioPlayer = nullptr;
    QCheckBox *audioStegCheckBox;
    QWidget* createAudioPlayerBar();
    QWidget *m_audioToolbar;
    QPushButton *m_audioPlayButton;
    QPushButton *m_audioPauseButton;
    QPushButton *m_audioStopButton;
    QSlider *m_audioVolumeSlider;
    QLabel *m_audioVolumeLabel;
    QSlider *m_audioProgressSlider;
    QLabel *m_audioTimeLabel;
    AudioStegEngine *audioEngine = nullptr;
    QVBoxLayout *mainLayout;
    QPushButton *m_openMusicButton;

private slots:
    void openAudioCarrier();
    void openAudioStegoFile();
    void saveModifiedAudio();
    void hideDataInAudio();
    void extractDataFromAudio();
    void updateAudioCapacity();
    void toggleAudioInputMethod();
    void selectAudioDataFile();
    void processDroppedAudio(const QString &filePath);
    void openAudioCarrierFromDrop(const QString &filePath);
    void onMediaLoaded(const QString &fileName);
    void onPlaybackStarted();
    void onPlaybackPaused();
    void onPlaybackStopped();
    void updateTimeLabel(qint64 position, qint64 duration);  // Change to 2 parameters
    void onAudioVolumeChanged(int volume);
    void onAudioPositionChanged(qint64 position);
    void onAudioDurationChanged(qint64 duration);
    void onAudioStegToggled(bool checked);

private:

    QByteArray m_stegoAudioData;
    QByteArray m_extractedData;
    QString m_currentAudioPath;
    QString m_stegoAudioPath;
    int m_currentDuration;
    FFmpegHandler* ffmpeg = nullptr;
    bool isAudioMode = false;
    QString currentCarrierPath;
    QString currentModifiedPath;
    QString currentToExtractPath;
    QString formatTime(qint64 milliseconds);
    bool isAudioBarVisible = false;
    QAction *toggleAudioBarAction;
    QMenu* createImageContextMenu(QLabel* label, const QImage& image);

    Qt::AspectRatioMode m_originalAspectMode = Qt::KeepAspectRatio;
    Qt::AspectRatioMode m_modifiedAspectMode = Qt::KeepAspectRatio;
    Qt::AspectRatioMode m_stegoAspectMode = Qt::KeepAspectRatio;
};

#endif // MAINWINDOW_H

