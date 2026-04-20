#ifndef PDFSTEGDIALOG_H
#define PDFSTEGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QJsonArray>
#include "pdfstegengine.h"
#include "stegengine.h"

class PDFStegDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PDFStegDialog(QWidget *parent = nullptr);
    ~PDFStegDialog();
public slots:
    void onClearAll();

private slots:
    void onTabChanged(int index);
    void onEmbedClicked();
    void onExtractClicked();
    void onSelectCarrierPDF();
    void onSelectOutputPDF();
    void onSelectStegoPDF();
    void onEncryptToggled(bool checked);
    void onCopyExtracted();
    void onSaveExtracted();
    void onProgressUpdated(int current, int total);
    void onEngineError(const QString &error);
    void onEmbedSuccess();

private:
    void setupUI();
    void setupHideTab();
    void setupExtractTab();
    void updateCapacityDisplay();
    QString formatSize(qint64 bytes) const;
    QByteArray getDataToEmbed();
    void displayExtractedData(const QByteArray &data);

    // UI Components
    QTabWidget *m_tabWidget;
    QWidget *m_hideTab;
    QWidget *m_extractTab;

    // Hide Tab Widgets
    QPlainTextEdit *m_secretTextEdit;
    QLineEdit *m_secretFileEdit;
    QPushButton *m_browseSecretFileBtn;
    QLineEdit *m_carrierPDFEdit;
    QPushButton *m_browseCarrierPDFBtn;
    QLineEdit *m_outputPDFEdit;
    QPushButton *m_browseOutputPDFBtn;
    QCheckBox *m_encryptCheckBox;
    QLabel *m_capacityLabel;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QPushButton *m_embedBtn;
    QPushButton *m_clearAllBtn;

    // Extract Tab Widgets
    QLineEdit *m_stegoPDFEdit;
    QPushButton *m_browseStegoPDFBtn;
    QCheckBox *m_decryptCheckBox;
    QPushButton *m_extractBtn;
    QPlainTextEdit *m_extractedDataEdit;
    QPushButton *m_copyExtractedBtn;
    QPushButton *m_saveExtractedBtn;

    // Core Components
    PDFStegEngine *m_engine;
    StegEngine m_crypto;
    
    // State
    bool m_encryptionEnabled;
    QString m_encryptionPassphrase;
    QByteArray m_lastExtractedData;

private:
    bool isBinaryData(const QByteArray &data) const;
    QString autoSaveExtractedFile(const QByteArray &data, const QString &originalFilename);
    void setupCornerWidget();
    QString m_currentCarrierBaseName;
};

#endif // PDFSTEGDIALOG_H
