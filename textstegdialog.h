#ifndef TEXTSTEGDIALOG_H
#define TEXTSTEGDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QComboBox>
#include "textstegengine.h"
#include "passphrasedialog.h"

class TextStegDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextStegDialog(QWidget *parent = nullptr);
    ~TextStegDialog();
public slots:
    void onClearAll();

private slots:
    void onHideClicked();
    void onExtractClicked();
    void onLoadCoverFile();
    void onLoadSecretFile();
    void onSaveStegoFile();
    void onCopyToClipboard();
    void onTabChanged(int index);
    void onTextInputToggled(bool checked);
    void onFileInputToggled(bool checked);
    void onBrowseSecretFile();
    void onLoadStegoFile();
    void onSaveExtractedFile();
    void updateCapacity();

private:
    void setupUI();
    void setupHideTab();
    void setupExtractTab();

    // UI Elements
    QTabWidget *m_tabWidget = nullptr;

    // Hide Tab
    QWidget *m_hideTab = nullptr;
    QPlainTextEdit *m_coverTextEdit;
    QPushButton *m_loadCoverBtn;

    QRadioButton *m_textInputRadio;
    QRadioButton *m_fileInputRadio;
    QPlainTextEdit *m_secretTextEdit;
    QLineEdit *m_secretFilePath;
    QPushButton *m_browseSecretBtn;

    QCheckBox *m_encryptCheckBox;
    QPushButton *m_hideBtn;
    QPushButton *m_clearBtn;
    QLabel *m_capacityLabel;

    QPlainTextEdit *m_stegoTextEdit;
    QPushButton *m_copyBtn;
    QPushButton *m_saveStegoBtn;

    // Extract Tab
    QWidget *m_extractTab = nullptr;
    QPlainTextEdit *m_stegoInputEdit;
    QLineEdit *m_stegoFilePath;
    QPushButton *m_loadStegoBtn;
    QPushButton *m_extractBtn;

    QPlainTextEdit *m_extractedTextEdit;
    QLineEdit *m_extractedFilePath;
    QPushButton *m_saveExtractedBtn;

    // Engine
    TextStegEngine *m_engine;

    // State
    QByteArray m_currentSecretData;
    QByteArray m_extractedData;
    QString m_currentStegoText;
    bool m_isTextInput;
    void setupCornerWidget();
};

#endif // TEXTSTEGDIALOG_H
