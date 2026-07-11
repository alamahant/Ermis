#ifndef ELFSTEGDIALOG_H
#define ELFSTEGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QLabel>
#include <QByteArray>

#include "elfstegengine.h"
#include "stegengine.h"

class ELFStegDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ELFStegDialog(QWidget *parent = nullptr);
    ~ELFStegDialog();

    QPushButton *clearAllBtn() const;

private slots:
    void onTabChanged(int index);
    void onSelectCarrierBinary();
    void onSelectOutputBinary();
    void onSelectStegoBinary();
    void onSelectSecretFile();
    void onEncryptToggled(bool checked);
    void onEmbedClicked();
    void onExtractClicked();
    void onCopyExtracted();
    void onSaveExtracted();
    void onClearAll();
    void onProgressUpdated(int current, int total);
    void onEngineError(const QString &error);
    void onEmbedSuccess();
    bool isValidELFBinary(const QString &filePath, bool showWarning = true);

private:
    void setupUI();
    void setupHideTab();
    void setupExtractTab();
    void setupCornerWidget();
    void updateCapacityDisplay();
    QString formatSize(qint64 bytes) const;
    QByteArray getDataToEmbed();
    void displayExtractedData(const QByteArray &data);
    bool isBinaryData(const QByteArray &data) const;

    // UI Components
    QTabWidget *m_tabWidget;
    QWidget *m_hideTab;
    QWidget *m_extractTab;

    // Hide Tab
    QLineEdit *m_secretFileEdit;
    QPushButton *m_browseSecretFileBtn;
    QPlainTextEdit *m_secretTextEdit;
    QLineEdit *m_carrierBinaryEdit;
    QPushButton *m_browseCarrierBinaryBtn;
    QLineEdit *m_outputBinaryEdit;
    QPushButton *m_browseOutputBinaryBtn;
    QCheckBox *m_encryptCheckBox;
    QPushButton *m_clearAllBtn;
    QPushButton *m_embedBtn;
    QLabel *m_capacityLabel;

    // Extract Tab
    QLineEdit *m_stegoBinaryEdit;
    QPushButton *m_browseStegoBinaryBtn;
    QPushButton *m_extractBtn;
    QPushButton *m_copyExtractedBtn;
    QPushButton *m_saveExtractedBtn;
    QPlainTextEdit *m_extractedDataEdit;

    // Common
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    // Engine & Crypto
    ELFStegEngine *m_engine;
    StegEngine m_crypto;
    bool m_encryptionEnabled;
    QString m_encryptionPassphrase;
    QByteArray m_lastExtractedData;
    QString m_currentCarrierBaseName;

    // Constants
    static constexpr const char* SECTION_NAME = ".debug_types";
};

#endif // ELFSTEGDIALOG_H
