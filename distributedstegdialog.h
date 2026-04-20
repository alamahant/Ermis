#ifndef DISTRIBUTEDSTEGDIALOG_H
#define DISTRIBUTEDSTEGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include<QJsonArray>
#include"distributedstegengine.h"
#include"stegengine.h"


class DistributedStegDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DistributedStegDialog(QWidget *parent = nullptr);
    ~DistributedStegDialog();
    void resetForNewOperation();
private slots:
    void onBuildMapClicked();
    void onReconstructClicked();
    void onCopyMap();
    void onSaveMap();
    void onLoadMap();
    void onCopyMessage();
    void onSaveMessage();
    void onTabChanged(int index);
    void onProgressUpdated(int current, int total);
    void onMapReady(const QJsonArray &map);
    void onMessageReconstructed(const QString &message);
    void onEngineError(const QString &error);

private:
    void setupUI();
    void setupHideTab();
    void setupExtractTab();

    // UI Components
    QTabWidget *m_tabWidget;
    QWidget *m_hideTab;
    QWidget *m_extractTab;

    // Hide Tab Widgets
    QPlainTextEdit *m_secretMessageEdit;
    QComboBox *m_languageCombo;
    QPushButton *m_buildMapBtn;
    QPlainTextEdit *m_mapDisplayEdit;
    QPushButton *m_copyMapBtn;
    QPushButton *m_saveMapBtn;
    QLabel *m_progressLabel;

    // Extract Tab Widgets
    QPushButton *m_loadMapBtn;
    QPlainTextEdit *m_mapInputEdit;
    QPushButton *m_reconstructBtn;
    QPlainTextEdit *m_recoveredMessageEdit;
    QPushButton *m_copyMessageBtn;
    QPushButton *m_saveMessageBtn;

    // Engine
    DistributedStegEngine *m_engine;

    // State
    QJsonArray m_currentMap;
    QString m_currentMessage;
    void validateSecretMessage();
    QCheckBox* m_encryptCheckBox = nullptr;

    QPushButton* m_clearAllBtn;
    void onEncryptToggled(bool checked);
    StegEngine m_crypto;
    QString m_encryptionPassphrase;
    bool m_encryptionEnabled = false;
    void setupCornerWidget();
};

#endif // DISTRIBUTEDSTEGDIALOG_H
