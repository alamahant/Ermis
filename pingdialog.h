#ifndef PINGDIALOG_H
#define PINGDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QListWidget>
#include "icmpstegengine.h"
#include<QComboBox>
#include<QRadioButton>
#include<QCheckBox>
#include"ipfilter.h"
#include<QSettings>
#include"udpstegengine.h"
#include<QSpinBox>
#include"dnsstegengine.h"
#include"httpstegengine.h"

class PingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PingDialog(QWidget *parent = nullptr);

private slots:
    // Send tab
    void onSendText();
    void onSendFile();
    void onBrowseFile();
    
    // Receive tab
    void onStartListening();
    void onStopListening();
    void onClearLog();
    void onSaveReceivedFile();
    
    // Engine signals
    void onDataReceived(const QByteArray& data, const QString& source);
    void onFileReceived(const QString& path, const QString& source);
    void onSendProgress(int packet, int total);
    void onReceiveProgress(int packet, int total);
    void onError(const QString& message);
    void updateSendButtonState();
    void onClearAll();
    void onBlockedIpSettings();
private:
    void setupUI();
    void updateStatus(const QString& status);
    void addToLog(const QString& msg, bool isError = false);
    void updateButtonStates();

    // Engine
    ICMPStegEngine* m_engine;
    UDPStegEngine* udpEngine;
    DNSStegEngine* dnsEngine;
    HTTPStegEngine* httpEngine;
    // UI Elements
    QTabWidget* m_tabWidget;
    
    // Send Tab
    QWidget* m_sendTab;
    QLineEdit* m_targetIpEdit;
    QComboBox* m_targetHistoryCombo;
    QPlainTextEdit* m_textInput;
    QLineEdit* m_filePathEdit;
    QPushButton* m_browseBtn;
    QRadioButton* m_textModeRadio;
    QRadioButton* m_fileModeRadio;
    QPushButton* m_sendBtn;
    QProgressBar* m_sendProgress;
    QLabel* m_sendStatusLabel;
    
    // Receive Tab
    QWidget* m_receiveTab;
    QPushButton* m_listenBtn;
    QPushButton* m_stopBtn;
    QListWidget* m_receivedFilesList;
    QPlainTextEdit* m_logDisplay;
    QPushButton* m_clearLogBtn;
    QPushButton* m_saveSelectedBtn;
    QLabel* m_listenStatusLabel;
    
    // Constants
    static constexpr int MAX_LOG_ENTRIES = 1000;
    static constexpr int TextRole = Qt::UserRole + 3;
    static constexpr int PathRole = Qt::UserRole;
    static constexpr int IpRole = Qt::UserRole + 1;
    static constexpr int FilenameRole = Qt::UserRole + 2;


    QByteArray m_lastReceivedData;   // Stores last received text
    QString m_lastReceivedSource;
    QPushButton* m_blockedIpBtn;
    QPushButton* m_clearAllBtn;
    QProgressBar* m_receiveProgress;
    QString m_rememberedPassphrase;
      bool m_hasRememberedPassphrase = false;
    QCheckBox* m_encryptCheckBox;
private slots:
        void onEncryptToggled(bool checked);
        void onCancelSend();
        void onProtocolChanged(int index);
        void onReceiverProtocolChanged(int index);

private:
    void populateListWidgetFromTmp();
    IpFilter* ipFilter = nullptr;
    QPlainTextEdit* m_receivedTextEdit;
    QComboBox* m_protocolCombo;
    QSpinBox* m_portSpinBox;
    QComboBox* m_receiverProtocolCombo;
    QSpinBox* m_receiverPortSpinBox;
    QPushButton* m_cancelSendBtn;
    bool m_cancelling = false;
};

#endif
