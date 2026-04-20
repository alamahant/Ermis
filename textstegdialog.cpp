#include "textstegdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDateTime>
#include <QDebug>
#include<QFileInfo>
#include"constants.h"

TextStegDialog::TextStegDialog(QWidget *parent)
    : QDialog(parent)
    , m_engine(new TextStegEngine(this))
    , m_isTextInput(true)
{




    setWindowTitle("Text Steganography");
    setMinimumSize(800, 700);
    setupUI();
}

TextStegDialog::~TextStegDialog()
{
}
/*
void TextStegDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Tab widget
    m_tabWidget = new QTabWidget(this);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &TextStegDialog::onTabChanged);

    // Setup tabs
    setupHideTab();
    setupExtractTab();

    mainLayout->addWidget(m_tabWidget);
}
*/

void TextStegDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);

    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &TextStegDialog::onTabChanged);

     setupCornerWidget();

    setupHideTab();

    setupExtractTab();


    m_tabWidget->addTab(m_hideTab, "Hide Data");
    m_tabWidget->addTab(m_extractTab, "Extract Data");


    mainLayout->addWidget(m_tabWidget);
}

void TextStegDialog::setupHideTab()
{

    m_hideTab = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(m_hideTab);

    // Cover Text Section
    QGroupBox *coverGroup = new QGroupBox("Cover Text", m_hideTab);

    QVBoxLayout *coverLayout = new QVBoxLayout(coverGroup);

    QHBoxLayout *coverHeaderLayout = new QHBoxLayout();

    coverHeaderLayout->addWidget(new QLabel("Enter or load cover text:"));

    coverHeaderLayout->addStretch();

    m_loadCoverBtn = new QPushButton("Load File", coverGroup);

    connect(m_loadCoverBtn, &QPushButton::clicked, this, &TextStegDialog::onLoadCoverFile);

    coverHeaderLayout->addWidget(m_loadCoverBtn);

    m_coverTextEdit = new QPlainTextEdit(coverGroup);

    m_coverTextEdit->setPlaceholderText("Paste your normal looking text here...");
    m_coverTextEdit->setMinimumHeight(100);

    coverLayout->addLayout(coverHeaderLayout);

    coverLayout->addWidget(m_coverTextEdit);

    layout->addWidget(coverGroup);

    // Secret Data Section
    QGroupBox *secretGroup = new QGroupBox("Secret Data", m_hideTab);

    QVBoxLayout *secretLayout = new QVBoxLayout(secretGroup);

    // Radio buttons for input type
    QHBoxLayout *radioLayout = new QHBoxLayout();

    m_textInputRadio = new QRadioButton("Text Input", secretGroup);

    m_fileInputRadio = new QRadioButton("File Input", secretGroup);

    m_textInputRadio->setChecked(true);

    radioLayout->addWidget(m_textInputRadio);

    radioLayout->addWidget(m_fileInputRadio);

    radioLayout->addStretch();

    connect(m_textInputRadio, &QRadioButton::toggled, this, &TextStegDialog::onTextInputToggled);

    connect(m_fileInputRadio, &QRadioButton::toggled, this, &TextStegDialog::onFileInputToggled);

    // Secret text input
    m_secretTextEdit = new QPlainTextEdit(secretGroup);

    m_secretTextEdit->setPlaceholderText("Type your secret message here...");
    m_secretTextEdit->setMinimumHeight(80);

    // Secret file input
    QHBoxLayout *fileLayout = new QHBoxLayout();

    m_secretFilePath = new QLineEdit(secretGroup);

    m_secretFilePath->setPlaceholderText("Select a file to hide...");
    m_secretFilePath->setEnabled(false);

    m_browseSecretBtn = new QPushButton("Browse...", secretGroup);

    m_browseSecretBtn->setEnabled(false);

    connect(m_browseSecretBtn, &QPushButton::clicked, this, &TextStegDialog::onBrowseSecretFile);

    fileLayout->addWidget(m_secretFilePath);

    fileLayout->addWidget(m_browseSecretBtn);

    secretLayout->addLayout(radioLayout);

    secretLayout->addWidget(m_secretTextEdit);

    secretLayout->addLayout(fileLayout);

    layout->addWidget(secretGroup);

    // Options Section
    QHBoxLayout *optionsLayout = new QHBoxLayout();

    m_encryptCheckBox = new QCheckBox("Encrypt with passphrase", m_hideTab);
    m_encryptCheckBox->setVisible(false);
    optionsLayout->addWidget(m_encryptCheckBox);

    optionsLayout->addStretch();

    layout->addLayout(optionsLayout);

    // Capacity Label
    m_capacityLabel = new QLabel("Capacity: 0/0 bytes (0%)", m_hideTab);

    layout->addWidget(m_capacityLabel);

    // Action Buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_hideBtn = new QPushButton("Hide Data", m_hideTab);

    m_clearBtn = new QPushButton("Clear", m_hideTab);

    connect(m_hideBtn, &QPushButton::clicked, this, &TextStegDialog::onHideClicked);

    connect(m_clearBtn, &QPushButton::clicked, this, &TextStegDialog::onClearAll);

    actionLayout->addStretch();

    actionLayout->addWidget(m_hideBtn);

    actionLayout->addWidget(m_clearBtn);

    layout->addLayout(actionLayout);

    // Stego Output Section
    QGroupBox *outputGroup = new QGroupBox("Stego Text Output", m_hideTab);

    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);

    QHBoxLayout *outputHeaderLayout = new QHBoxLayout();

    outputHeaderLayout->addStretch();

    m_copyBtn = new QPushButton("Copy to Clipboard", outputGroup);

    m_saveStegoBtn = new QPushButton("Save As...", outputGroup);

    connect(m_copyBtn, &QPushButton::clicked, this, &TextStegDialog::onCopyToClipboard);

    connect(m_saveStegoBtn, &QPushButton::clicked, this, &TextStegDialog::onSaveStegoFile);

    outputHeaderLayout->addWidget(m_copyBtn);

    outputHeaderLayout->addWidget(m_saveStegoBtn);

    m_stegoTextEdit = new QPlainTextEdit(outputGroup);

    m_stegoTextEdit->setPlaceholderText("Text with hidden data will appear here...");
    m_stegoTextEdit->setReadOnly(true);
    m_stegoTextEdit->setMinimumHeight(100);

    outputLayout->addLayout(outputHeaderLayout);

    outputLayout->addWidget(m_stegoTextEdit);

    layout->addWidget(outputGroup);

    // Connect text changes for capacity
    connect(m_coverTextEdit, &QPlainTextEdit::textChanged, this, &TextStegDialog::updateCapacity);

    connect(m_secretTextEdit, &QPlainTextEdit::textChanged, this, &TextStegDialog::updateCapacity);

    connect(m_secretFilePath, &QLineEdit::textChanged, this, &TextStegDialog::updateCapacity);

    if (!m_tabWidget) {
            m_tabWidget = new QTabWidget(this);
    }

    //m_tabWidget->addTab(m_hideTab, "Hide Data");


}



void TextStegDialog::setupExtractTab()
{

    m_extractTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_extractTab);

    // Stego Input Section
    QGroupBox *inputGroup = new QGroupBox("Stego Text Input", m_extractTab);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);

    QHBoxLayout *inputHeaderLayout = new QHBoxLayout();
    inputHeaderLayout->addWidget(new QLabel("Enter or load stego text:"));
    inputHeaderLayout->addStretch();
    m_loadStegoBtn = new QPushButton("Load File", inputGroup);
    connect(m_loadStegoBtn, &QPushButton::clicked, this, &TextStegDialog::onLoadStegoFile);
    inputHeaderLayout->addWidget(m_loadStegoBtn);

    m_stegoInputEdit = new QPlainTextEdit(inputGroup);
    m_stegoInputEdit->setPlaceholderText("Paste text with hidden data here...");
    m_stegoInputEdit->setMinimumHeight(150);

    inputLayout->addLayout(inputHeaderLayout);
    inputLayout->addWidget(m_stegoInputEdit);
    layout->addWidget(inputGroup);

    // Extract Button
    QHBoxLayout *extractLayout = new QHBoxLayout();
    extractLayout->addStretch();
    m_extractBtn = new QPushButton("Extract Data", m_extractTab);
    connect(m_extractBtn, &QPushButton::clicked, this, &TextStegDialog::onExtractClicked);
    extractLayout->addWidget(m_extractBtn);
    layout->addLayout(extractLayout);

    // Extracted Output Section
    QGroupBox *outputGroup = new QGroupBox("Extracted Data", m_extractTab);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);

    QHBoxLayout *outputHeaderLayout = new QHBoxLayout();
    outputHeaderLayout->addWidget(new QLabel("Extracted text:"));
    outputHeaderLayout->addStretch();

    m_saveExtractedBtn = new QPushButton("Save As...", outputGroup);
    connect(m_saveExtractedBtn, &QPushButton::clicked, this, &TextStegDialog::onSaveExtractedFile);
    outputHeaderLayout->addWidget(m_saveExtractedBtn);

    m_extractedTextEdit = new QPlainTextEdit(outputGroup);
    m_extractedTextEdit->setPlaceholderText("Extracted data will appear here...");
    m_extractedTextEdit->setReadOnly(true);
    m_extractedTextEdit->setMinimumHeight(150);

    outputLayout->addLayout(outputHeaderLayout);
    outputLayout->addWidget(m_extractedTextEdit);

    layout->addWidget(outputGroup);

    //m_tabWidget->addTab(m_extractTab, "Extract Data");
}




void TextStegDialog::onLoadCoverFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Cover Text File",
        Constants::appDirPath, "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file");
        return;
    }

    QTextStream stream(&file);
    m_coverTextEdit->setPlainText(stream.readAll());
    file.close();
}

void TextStegDialog::onLoadSecretFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Secret File",
        Constants::appDirPath, "All Files (*)");
    if (fileName.isEmpty()) return;

    m_secretFilePath->setText(fileName);
    updateCapacity();
}

void TextStegDialog::onSaveStegoFile()
{
    if (m_currentStegoText.isEmpty()) {
        QMessageBox::warning(this, "Error", "No stego text to save");
        return;
    }


    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString defaultFileName = QString("stego_text_%1.txt").arg(timestamp);
        QString defaultPath = Constants::fusedImagesPath + "/" + defaultFileName;

        QString fileName = QFileDialog::getSaveFileName(this, "Save Stego Text",
            defaultPath, "Text Files (*.txt);;All Files (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file");
        return;
    }

    QTextStream stream(&file);
    stream << m_currentStegoText;
    file.close();

    QMessageBox::information(this, "Success", "Stego text saved");
}

void TextStegDialog::onCopyToClipboard()
{
    if (m_currentStegoText.isEmpty()) {
        QMessageBox::warning(this, "Error", "No stego text to copy");
        return;
    }

    QApplication::clipboard()->setText(m_currentStegoText);
    //statusBar()->showMessage("Copied to clipboard");
}

void TextStegDialog::onClearAll()
{
    m_coverTextEdit->clear();
    m_secretTextEdit->clear();
    m_secretFilePath->clear();
    m_stegoTextEdit->clear();
    m_currentStegoText.clear();
    m_currentSecretData.clear();
    m_stegoInputEdit->clear();
    updateCapacity();
}

void TextStegDialog::onTabChanged(int index)
{
    // Clear extract output when switching to hide tab
    if (index == 0) {
        m_extractedTextEdit->clear();
    }
}

void TextStegDialog::onTextInputToggled(bool checked)
{
    m_isTextInput = checked;
    m_secretTextEdit->setEnabled(checked);
    m_secretFilePath->setEnabled(!checked);
    m_browseSecretBtn->setEnabled(!checked);
    updateCapacity();
}

void TextStegDialog::onFileInputToggled(bool checked)
{
    onTextInputToggled(!checked);
}

/*
void TextStegDialog::onBrowseSecretFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select File to Hide",
        Constants::appDirPath, "All Files (*)");
    if (!fileName.isEmpty()) {
        m_secretFilePath->setText(fileName);
        updateCapacity();
    }
}
*/

void TextStegDialog::onBrowseSecretFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select File to Hide",
        Constants::appDirPath, "All Files (*)");

    if (!fileName.isEmpty()) {
        // Quick peek at the file to check if it's text
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error", "Could not open file for preview");
            return;
        }

        // Read first 512 bytes to check
        QByteArray sample = file.read(512);
        file.close();

        bool isTextFile = true;
        int nullCount = 0;

        for (int i = 0; i < sample.size(); i++) {
            unsigned char c = static_cast<unsigned char>(sample[i]);

            // Count null bytes (strong indicator of binary)
            if (c == 0) {
                nullCount++;
                if (nullCount > 5) { // Too many nulls = binary
                    isTextFile = false;
                    break;
                }
            }

            // Check for non-printable characters (except common whitespace)
            if (c < 32 && c != '\n' && c != '\r' && c != '\t' && c != 0) {
                isTextFile = false;
                break;
            }

            // Check for high ASCII that might indicate binary
            if (c > 127) {
                // Could be UTF-8, so we need to be careful
                // For simplicity, allow if it's part of UTF-8 sequence
                if (c >= 192 && c <= 223) { // 2-byte UTF-8 start
                    i++; // Skip next byte
                } else if (c >= 224 && c <= 239) { // 3-byte UTF-8 start
                    i += 2; // Skip next two bytes
                } else if (c >= 240 && c <= 247) { // 4-byte UTF-8 start
                    i += 3; // Skip next three bytes
                } else {
                    isTextFile = false;
                    break;
                }
            }
        }

        if (!isTextFile && sample.size() > 0) {
            int response = QMessageBox::question(this, "Non-Text File",
                "This doesn't appear to be a text file. Text steganography works best with text files.\n"
                "Hiding binary files may not work well and could corrupt the data.\n\n"
                "Do you still want to select this file?",
                QMessageBox::Yes | QMessageBox::No);

            if (response == QMessageBox::No) {
                return;
            }
        }

        m_secretFilePath->setText(fileName);
        updateCapacity();
    }
}

void TextStegDialog::onLoadStegoFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Stego Text File",
        Constants::fusedImagesPath, "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file");
        return;
    }

    QTextStream stream(&file);
    m_stegoInputEdit->setPlainText(stream.readAll());
    file.close();
}





/////////////////////


void TextStegDialog::onHideClicked()
{
    QString coverText = m_coverTextEdit->toPlainText();
    if (coverText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter cover text");
        return;
    }

    QByteArray secretData;

    if (m_isTextInput) {
        QString secret = m_secretTextEdit->toPlainText();
        if (secret.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter secret text");
            return;
        }
        secretData = secret.toUtf8();
    } else {
        QString filePath = m_secretFilePath->text();
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please select a file");
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error", "Could not open file");
            return;
        }

        // Just read the file data - NO filename header
        secretData = file.readAll();
        file.close();
    }

    // Handle encryption
    if (m_encryptCheckBox->isChecked()) {
        PassphraseDialog dialog(true, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString passphrase = dialog.getPassphrase();
            QMessageBox::information(this, "Info", "Encryption will be implemented soon");
        } else {
            return;
        }
    }

    // Hide the data
    QString stegoText = m_engine->hideInText(coverText, secretData);

    if (stegoText.isEmpty()) {
        QMessageBox::warning(this, "Error", m_engine->lastError());
        return;
    }

    m_currentStegoText = stegoText;
    m_stegoTextEdit->setPlainText(stegoText);
}

void TextStegDialog::updateCapacity()
{
    QString coverText = m_coverTextEdit->toPlainText();
    int capacity = m_engine->calculateCapacity(coverText);

    int dataSize = 0;
    if (m_isTextInput) {
        dataSize = m_secretTextEdit->toPlainText().toUtf8().size();
    } else {
        QFile file(m_secretFilePath->text());
        if (file.exists()) {
            dataSize = file.size();  // Just file size, no header
        }
    }

    double percentage = (capacity > 0) ? (dataSize * 100.0 / capacity) : 0;
    m_capacityLabel->setText(QString("Capacity: %1/%2 bytes (%3%)")
        .arg(dataSize).arg(capacity).arg(percentage, 0, 'f', 1));

    m_hideBtn->setEnabled(capacity > 0 && dataSize > 0 && dataSize <= capacity);
}

void TextStegDialog::onExtractClicked()
{
    QString stegoText = m_stegoInputEdit->toPlainText();
    if (stegoText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter stego text");
        return;
    }

    QByteArray extracted = m_engine->extractFromText(stegoText);

    if (extracted.isEmpty()) {
        QMessageBox::warning(this, "Error", m_engine->lastError());
        return;
    }

    m_extractedData = extracted;

    // Try to parse as plain text
    QString text = QString::fromUtf8(extracted);
    bool isText = true;
    for (const QChar &c : text) {
        if (!c.isPrint() && !c.isSpace() && c != '\n' && c != '\r' && c != '\t') {
            isText = false;
            break;
        }
    }

    // Check for null bytes
    if (extracted.contains('\0')) {
        isText = false;
    }

    if (isText) {
        m_extractedTextEdit->setPlainText(text);
        QMessageBox::information(this, "Success",
            QString("Text extracted successfully (%1 bytes)").arg(extracted.size()));
    } else {
        m_extractedTextEdit->setPlainText(QString("Binary data: %1 bytes\n\n"
                                                  "This appears to be binary data. "
                                                  "Click 'Save As...' to save it to a file.")
            .arg(extracted.size()));
        QMessageBox::information(this, "Success",
            QString("Binary data extracted (%1 bytes)").arg(extracted.size()));
    }
}

void TextStegDialog::onSaveExtractedFile()
{
    if (m_extractedData.isEmpty()) {
        QMessageBox::warning(this, "Error", "No extracted data to save");
        return;
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString defaultFileName = QString("extracted_text_%1.txt").arg(timestamp);
    QString defaultPath = Constants::extractedImagesPath + "/" + defaultFileName;

    QString fileName = QFileDialog::getSaveFileName(this, "Save Extracted Data",
        defaultPath, "All Files (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file");
        return;
    }

    // Just write the extracted data - no header to strip
    file.write(m_extractedData);
    file.close();

    QMessageBox::information(this, "Success",
        QString("Extracted data saved to:\n%1").arg(fileName));
}


void TextStegDialog::setupCornerWidget()
{
    QPushButton *infoButton = new QPushButton("ⓘ", this);
    infoButton->setFixedSize(16, 16);
    infoButton->setToolTip(
        "<html><body style='white-space: nowrap;'>"
        "<b>📝 Text Steganography</b><br>"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━<br><br>"

        "<b>🔒 Hide Data:</b><br>"
        "• Enter or load cover text<br>"
        "• Provide secret text or select file<br>"
        "• Click 'Hide Data' to embed<br>"
        "• Save or copy resulting stego text<br><br>"

        "<b>🔓 Extract Data:</b><br>"
        "• Paste or load stego text<br>"
        "• Click 'Extract Data'<br>"
        "• Text displays in viewer<br>"
        "• Binary files prompt to save<br><br>"

        "<b>📁 File Support:</b><br>"
        "• Text files work best<br>"
        "• Binary files may cause issues<br>"
        "• Warning shown for non-text files<br><br>"

        "<b>📊 Capacity:</b><br>"
        "• Based on zero-width character count<br>"
        "• Shown in status bar<br>"
        "• Updates as text changes<br><br>"

        "<b>💡 Notes:</b><br>"
        "• Uses zero-width characters<br>"
        "• Stego text looks identical to cover<br>"
        "• Copy-paste preserves hidden data<br>"
        "</body></html>"
    );
    infoButton->setObjectName("infoButton");

    if (Constants::isDarkTheme) {
        infoButton->setStyleSheet(
            "QPushButton#infoButton { "
            "background-color: rgba(13, 71, 161, 80); "
            "border-radius: 16px; "
            "font-size: 18px; "
            "font-weight: bold; "
            "color: rgba(255, 255, 255, 80); "
            "border: none; "
            "padding: 0px; "
            "} "
            "QPushButton#infoButton:hover { "
            "background-color: rgba(21, 101, 192, 255); "
            "color: rgba(255, 255, 255, 255); "
            "}"
        );
    } else {
        infoButton->setStyleSheet(
            "QPushButton#infoButton { "
            "background-color: rgba(70, 90, 110, 60); "
            "border-radius: 16px; "
            "font-size: 18px; "
            "font-weight: bold; "
            "color: rgba(40, 50, 70, 120); "
            "border: none; "
            "padding: 0px; "
            "} "
            "QPushButton#infoButton:hover { "
            "background-color: rgba(21, 101, 192, 200); "
            "color: rgba(255, 255, 255, 255); "
            "}"
        );
    }

    m_tabWidget->setCornerWidget(infoButton, Qt::TopRightCorner);
}
