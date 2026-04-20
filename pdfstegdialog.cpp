#include "pdfstegdialog.h"
#include "passphrasedialog.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QProgressBar>
#include <QRegularExpression>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include<QTabBar>
#include<QDateTime>

PDFStegDialog::PDFStegDialog(QWidget *parent)
    : QDialog(parent)
    , m_engine(new PDFStegEngine(this))
    , m_encryptionEnabled(false)
{
    setWindowTitle("PDF Steganography - Hide Secret Data in PDF");
    setMinimumSize(850, 700);
    setupUI();

    connect(m_engine, &PDFStegEngine::progressUpdated,
            this, &PDFStegDialog::onProgressUpdated);
    connect(m_engine, &PDFStegEngine::errorOccurred,
            this, &PDFStegDialog::onEngineError);
    connect(m_engine, &PDFStegEngine::embedSuccess,
            this, &PDFStegDialog::onEmbedSuccess);


    m_encryptCheckBox->setChecked(false);
    m_decryptCheckBox->setChecked(false);
}

PDFStegDialog::~PDFStegDialog()
{
}

void PDFStegDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);
    connect(m_tabWidget, &QTabWidget::currentChanged, 
            this, &PDFStegDialog::onTabChanged);
    setupCornerWidget();

    setupHideTab();
    setupExtractTab();

    m_tabWidget->addTab(m_hideTab, "Hide Data");
    m_tabWidget->addTab(m_extractTab, "Extract Data");

    mainLayout->addWidget(m_tabWidget);
}

void PDFStegDialog::setupHideTab()
{
    m_hideTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_hideTab);

    // Secret Data Section
    QGroupBox *secretGroup = new QGroupBox("Secret Data to Hide", m_hideTab);
    QVBoxLayout *secretLayout = new QVBoxLayout(secretGroup);

    QTabWidget *inputTypeTabs = new QTabWidget(secretGroup);
    
    // Text input tab
    QWidget *textTab = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textTab);
    m_secretTextEdit = new QPlainTextEdit(textTab);
    m_secretTextEdit->setPlaceholderText("Enter text to hide in PDF...\n\nSupports letters, numbers, spaces, and punctuation.");
    m_secretTextEdit->setMinimumHeight(150);

    textLayout->addWidget(m_secretTextEdit);
    inputTypeTabs->addTab(textTab, "Text");

    // File input tab
    QWidget *fileTab = new QWidget();
    QVBoxLayout *fileLayout = new QVBoxLayout(fileTab);
    QLabel *fileLabel = new QLabel("Select file to hide (any file type):", fileTab);
    QHBoxLayout *fileSelectLayout = new QHBoxLayout();
    m_secretFileEdit = new QLineEdit(fileTab);
    m_secretFileEdit->setPlaceholderText("Path to file...");
    m_secretFileEdit->setReadOnly(true);
    m_browseSecretFileBtn = new QPushButton("Browse...", fileTab);
    connect(m_browseSecretFileBtn, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this,
                    "Select File to Hide",
                    Constants::appDirPath,
                    QString());
        if (!fileName.isEmpty()) {
            m_secretFileEdit->setText(fileName);
            updateCapacityDisplay();
        }
    });
    fileSelectLayout->addWidget(m_secretFileEdit);
    fileSelectLayout->addWidget(m_browseSecretFileBtn);
    fileLayout->addWidget(fileLabel);
    fileLayout->addLayout(fileSelectLayout);
    fileLayout->addStretch();
    inputTypeTabs->addTab(fileTab, "File");

    secretLayout->addWidget(inputTypeTabs);
    layout->addWidget(secretGroup);

    // Carrier PDF Section
    QGroupBox *carrierGroup = new QGroupBox("Carrier PDF", m_hideTab);
    QHBoxLayout *carrierLayout = new QHBoxLayout(carrierGroup);
    m_carrierPDFEdit = new QLineEdit(carrierGroup);
    m_carrierPDFEdit->setPlaceholderText("Select PDF file to hide data in...");
    m_carrierPDFEdit->setReadOnly(true);
    m_browseCarrierPDFBtn = new QPushButton("Browse...", carrierGroup);
    connect(m_browseCarrierPDFBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onSelectCarrierPDF);
    carrierLayout->addWidget(m_carrierPDFEdit);
    carrierLayout->addWidget(m_browseCarrierPDFBtn);
    layout->addWidget(carrierGroup);

    // Output PDF Section
    QGroupBox *outputGroup = new QGroupBox("Output PDF", m_hideTab);
    QHBoxLayout *outputLayout = new QHBoxLayout(outputGroup);
    m_outputPDFEdit = new QLineEdit(outputGroup);
    m_outputPDFEdit->setPlaceholderText("Where to save the stego PDF...");
    m_outputPDFEdit->setReadOnly(true);
    m_browseOutputPDFBtn = new QPushButton("Browse...", outputGroup);
    connect(m_browseOutputPDFBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onSelectOutputPDF);
    outputLayout->addWidget(m_outputPDFEdit);
    outputLayout->addWidget(m_browseOutputPDFBtn);
    layout->addWidget(outputGroup);

    // Options Section
    QGroupBox *optionsGroup = new QGroupBox("Options", m_hideTab);
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsGroup);
    
    m_encryptCheckBox = new QCheckBox("Encrypt data before hiding", optionsGroup);
    connect(m_encryptCheckBox, &QCheckBox::clicked, 
            this, &PDFStegDialog::onEncryptToggled);
    
    m_clearAllBtn = new QPushButton("Clear All", optionsGroup);
    connect(m_clearAllBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onClearAll);
    


    optionsLayout->addWidget(m_encryptCheckBox);
    optionsLayout->addStretch();
    optionsLayout->addWidget(m_clearAllBtn);
    layout->addWidget(optionsGroup);
    //optionsGroup->setVisible(false);

    // Capacity Info
    QHBoxLayout *capacityLayout = new QHBoxLayout();
    m_capacityLabel = new QLabel("PDF Capacity: Not selected", m_hideTab);
    capacityLayout->addWidget(m_capacityLabel);
    capacityLayout->addStretch();
    layout->addLayout(capacityLayout);

    // Progress Section
    m_progressBar = new QProgressBar(m_hideTab);
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);
    
    m_statusLabel = new QLabel("Ready", m_hideTab);
    layout->addWidget(m_statusLabel);

    // Action Button
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->addStretch();
    m_embedBtn = new QPushButton("Embed Data", m_hideTab);
    m_embedBtn->setMinimumWidth(150);
    connect(m_embedBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onEmbedClicked);
    actionLayout->addWidget(m_embedBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    layout->addStretch();
}

void PDFStegDialog::setupExtractTab()
{
    m_extractTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_extractTab);

    // Stego PDF Selection
    QGroupBox *inputGroup = new QGroupBox("Stego PDF", m_extractTab);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroup);
    m_stegoPDFEdit = new QLineEdit(inputGroup);
    m_stegoPDFEdit->setPlaceholderText("Select PDF containing hidden data...");
    m_stegoPDFEdit->setReadOnly(true);
    m_browseStegoPDFBtn = new QPushButton("Browse...", inputGroup);
    connect(m_browseStegoPDFBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onSelectStegoPDF);
    inputLayout->addWidget(m_stegoPDFEdit);
    inputLayout->addWidget(m_browseStegoPDFBtn);
    layout->addWidget(inputGroup);

    // Options Section
    QGroupBox *optionsGroup = new QGroupBox("Options", m_extractTab);
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsGroup);
    m_decryptCheckBox = new QCheckBox("Data is encrypted", optionsGroup);
    optionsLayout->addWidget(m_decryptCheckBox);
    optionsLayout->addStretch();
    layout->addWidget(optionsGroup);
    optionsGroup->setVisible(false);
    // Extract Button
    QHBoxLayout *extractBtnLayout = new QHBoxLayout();
    extractBtnLayout->addStretch();
    m_extractBtn = new QPushButton("Extract Data", m_extractTab);
    m_extractBtn->setMinimumWidth(150);
    connect(m_extractBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onExtractClicked);
    extractBtnLayout->addWidget(m_extractBtn);
    extractBtnLayout->addStretch();
    layout->addLayout(extractBtnLayout);

    // Extracted Data Display
    QGroupBox *outputGroup = new QGroupBox("Extracted Data", m_extractTab);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);

    QHBoxLayout *outputHeaderLayout = new QHBoxLayout();
    outputHeaderLayout->addStretch();
    m_copyExtractedBtn = new QPushButton("Copy to Clipboard", outputGroup);
    m_saveExtractedBtn = new QPushButton("Save to File...", outputGroup);
    connect(m_copyExtractedBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onCopyExtracted);
    connect(m_saveExtractedBtn, &QPushButton::clicked, 
            this, &PDFStegDialog::onSaveExtracted);
    outputHeaderLayout->addWidget(m_copyExtractedBtn);
    outputHeaderLayout->addWidget(m_saveExtractedBtn);

    m_extractedDataEdit = new QPlainTextEdit(outputGroup);
    m_extractedDataEdit->setReadOnly(true);
    m_extractedDataEdit->setPlaceholderText("Extracted data will appear here...");
    m_extractedDataEdit->setMinimumHeight(250);

    outputLayout->addLayout(outputHeaderLayout);
    outputLayout->addWidget(m_extractedDataEdit);
    layout->addWidget(outputGroup);

    layout->addStretch();
}

void PDFStegDialog::onTabChanged(int index)
{
    Q_UNUSED(index)
    // Clear status when switching tabs
    m_statusLabel->setText("Ready");
    m_progressBar->setVisible(false);
}

void PDFStegDialog::onSelectCarrierPDF()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");

    QString fileName = QFileDialog::getOpenFileName(this, 
        "Select Carrier PDF", Constants::appDirPath, "PDF Files (*.pdf)");
    
    if (!fileName.isEmpty()) {
        m_carrierPDFEdit->setText(fileName);
        
        // Auto-suggest output path
        QFileInfo info(fileName);
        //QString suggestedOutput = info.absolutePath() + "/" +
        //info.baseName() + "_stego.pdf";
        //m_outputPDFEdit->setText(suggestedOutput);
        m_currentCarrierBaseName = info.baseName();  // Store in member var
        QString suggestedOutput = Constants::fusedImagesPath + "/" + timestamp + "_" + m_currentCarrierBaseName + "_stego.pdf";
        m_outputPDFEdit->setText(suggestedOutput);

        updateCapacityDisplay();
    }
}


void PDFStegDialog::onSelectOutputPDF()
{
    QString defaultPath = m_outputPDFEdit->text();
    if (defaultPath.isEmpty()) {
        defaultPath = Constants::fusedImagesPath + "/stego.pdf";
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Stego PDF As", defaultPath, "PDF Files (*.pdf)");

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
            fileName += ".pdf";
        }
        m_outputPDFEdit->setText(fileName);
    }
}

void PDFStegDialog::onSelectStegoPDF()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        "Select Stego PDF", Constants::fusedImagesPath, "PDF Files (*.pdf)");
    
    if (!fileName.isEmpty()) {
        m_stegoPDFEdit->setText(fileName);
    }
}

void PDFStegDialog::onEncryptToggled(bool checked)
{
    if (checked) {
        PassphraseDialog dialog(true, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_encryptionPassphrase = dialog.getPassphrase();
            m_encryptionEnabled = true;
        } else {
            m_encryptCheckBox->blockSignals(true);
            m_encryptCheckBox->setChecked(false);
            m_encryptCheckBox->blockSignals(false);
            m_encryptionEnabled = false;
        }
    } else {
        m_encryptionEnabled = false;
        m_encryptionPassphrase.clear();
    }
}

void PDFStegDialog::onEmbedClicked()
{
    // Validate inputs
    if (m_carrierPDFEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a carrier PDF.");
        return;
    }

    if (m_outputPDFEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify an output PDF path.");
        return;
    }

    QByteArray packet = getDataToEmbed();
    if (packet.isEmpty()) {
        QMessageBox::warning(this, "Error", "No data to embed.");
        return;
    }

    // Encrypt the whole packet if enabled
    if (m_encryptionEnabled && !m_encryptionPassphrase.isEmpty()) {
        packet = m_crypto.encryptData(packet, m_encryptionPassphrase);
        if (packet.isEmpty()) {
            QMessageBox::warning(this, "Error", "Encryption failed.");
            return;
        }
        packet.prepend("ENCR");
    }

    // Check capacity
    qint64 capacity = m_engine->calculatePDFCapacity(m_carrierPDFEdit->text());
    if (packet.size() > capacity) {
        QMessageBox::critical(this, "Capacity Error",
            QString("Data too large!\n\nPDF Capacity: %1\nData Size: %2\n\n"
                    "Please use a larger PDF or less data.")
                .arg(formatSize(capacity))
                .arg(formatSize(packet.size())));
        return;
    }

    // Embed
    m_embedBtn->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 100);
    m_statusLabel->setText("Embedding data...");

    bool success = m_engine->embedDataInPDF(
        m_carrierPDFEdit->text(),
        m_outputPDFEdit->text(),
        packet
    );

    if (!success) {
        onEngineError(m_engine->lastError());
    }
}




void PDFStegDialog::onExtractClicked()
{
    if (m_stegoPDFEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a stego PDF.");
        return;
    }

    m_extractBtn->setEnabled(false);
    m_statusLabel->setText("Extracting data...");
    m_extractedDataEdit->clear();

    QByteArray packet = m_engine->extractDataFromPDF(m_stegoPDFEdit->text());

    if (packet.isEmpty()) {
        m_extractBtn->setEnabled(true);
        m_statusLabel->setText("Ready");
        if (!m_engine->lastError().isEmpty()) {
            QMessageBox::warning(this, "Error", m_engine->lastError());
        } else {
            QMessageBox::warning(this, "Error", "No hidden data found in this PDF.");
        }
        return;
    }

    // Check for ENCR flag
    if (packet.startsWith("ENCR")) {
        PassphraseDialog dialog(false, this);
        if (dialog.exec() == QDialog::Accepted) {
            QByteArray decrypted = m_crypto.decryptData(packet.mid(4), dialog.getPassphrase());
            if (!decrypted.isEmpty()) {
                packet = decrypted;
            } else {
                QMessageBox::warning(this, "Error", "Decryption failed. Wrong passphrase?");
                m_extractBtn->setEnabled(true);
                m_statusLabel->setText("Ready");
                return;
            }
        } else {
            m_extractBtn->setEnabled(true);
            m_statusLabel->setText("Ready");
            return;
        }
    }

    // Parse FNAM flag
    QString originalFilename;
    QByteArray actualData;

    if (packet.startsWith("FNAM") && packet.size() >= 5) {
        int nameLen = static_cast<unsigned char>(packet[4]);
        if (packet.size() >= 5 + nameLen) {
            originalFilename = QString::fromUtf8(packet.mid(5, nameLen));
            actualData = packet.mid(5 + nameLen);
        } else {
            actualData = packet;
        }
    } else {
        actualData = packet;
    }

    m_lastExtractedData = actualData;

    // Check if binary or text
    if (isBinaryData(actualData)) {
        // Binary - open save file dialog
        QString suggestedName = originalFilename.isEmpty() ? "extracted.bin" : originalFilename;
        QString savePath = QFileDialog::getSaveFileName(this, "Save Extracted File", Constants::extractedImagesPath + "/" + suggestedName, "All Files (*.*)");

        if (!savePath.isEmpty()) {
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(actualData);
                file.close();
                m_extractedDataEdit->setPlainText("[Binary Data - " + formatSize(actualData.size()) + "]\n\nSaved to:\n" + savePath);
                m_statusLabel->setText("File saved successfully!");
                QMessageBox::information(this, "Success", "Binary file saved to:\n" + savePath);
            } else {
                QMessageBox::warning(this, "Error", "Could not save file.");
                m_statusLabel->setText("Save failed");
            }
        } else {
            m_statusLabel->setText("Save cancelled");
        }
    } else {
        // Text - display in edit
        displayExtractedData(actualData);
        m_statusLabel->setText("Data extracted successfully!");
        QMessageBox::information(this, "Success", QString("Data extracted successfully! (%1)").arg(formatSize(actualData.size())));
    }

    m_extractBtn->setEnabled(true);
}


void PDFStegDialog::onProgressUpdated(int current, int total)
{
    if (total > 0) {
        m_progressBar->setValue((current * 100) / total);
    }
}

void PDFStegDialog::onEngineError(const QString &error)
{
    m_embedBtn->setEnabled(true);
    m_extractBtn->setEnabled(true);
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Error: " + error);
    QMessageBox::warning(this, "Engine Error", error);
}

void PDFStegDialog::onEmbedSuccess()
{
    m_embedBtn->setEnabled(true);
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Data embedded successfully!");
    
    QMessageBox::information(this, "Success", 
        QString("Data successfully hidden in PDF!\n\nOutput: %1")
            .arg(m_outputPDFEdit->text()));
}


void PDFStegDialog::onCopyExtracted()
{
    if (m_lastExtractedData.isEmpty()) {
        QMessageBox::warning(this, "Error", "No data to copy.");
        return;
    }
    
    // Try to copy as text if it looks like text
    QString text = QString::fromUtf8(m_lastExtractedData);
    if (text.contains(QRegularExpression("^[\\x20-\\x7E\\n\\r\\t\\x80-\\xFF]+$"))) {
        QApplication::clipboard()->setText(text);
    } else {
        // Binary data - can't copy to clipboard easily
        QMessageBox::information(this, "Info", 
            "Binary data cannot be copied to clipboard. Please use 'Save to File'.");
        return;
    }
    
    QMessageBox::information(this, "Success", "Data copied to clipboard.");
}

void PDFStegDialog::onSaveExtracted()
{
    if (m_lastExtractedData.isEmpty()) {
        QMessageBox::warning(this, "Error", "No data to save.");
        return;
    }
    
    QString defaultPath = Constants::extractedImagesPath + "/extracted_data";

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Extracted Data", defaultPath, "All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }
    
    file.write(m_lastExtractedData);
    file.close();
    
    QMessageBox::information(this, "Success", "Data saved to file.");
}

void PDFStegDialog::onClearAll()
{
    m_secretTextEdit->clear();
    m_secretFileEdit->clear();
    m_carrierPDFEdit->clear();
    m_outputPDFEdit->clear();
    m_extractedDataEdit->clear();
    m_stegoPDFEdit->clear();
    
    m_encryptCheckBox->blockSignals(true);
    m_encryptCheckBox->setChecked(false);
    m_encryptCheckBox->blockSignals(false);
    m_encryptionEnabled = false;
    m_encryptionPassphrase.clear();
    
    m_decryptCheckBox->setChecked(false);
    m_lastExtractedData.clear();
    
    m_statusLabel->setText("Ready");
    m_progressBar->setVisible(false);
    m_capacityLabel->setText("PDF Capacity: Not selected");
    m_currentCarrierBaseName.clear();
    m_tabWidget->setCurrentIndex(0);
}


void PDFStegDialog::updateCapacityDisplay()
{
    QString carrierPath = m_carrierPDFEdit->text();
    if (!carrierPath.isEmpty() && QFile::exists(carrierPath)) {
        qint64 capacity = m_engine->calculatePDFCapacity(carrierPath);
        m_capacityLabel->setText(QString("PDF Capacity: %1").arg(formatSize(capacity)));
    } else {
        m_capacityLabel->setText("PDF Capacity: Not selected");
    }
}

QString PDFStegDialog::formatSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1.%2 GB").arg(bytes / GB).arg((bytes % GB) * 10 / GB);
    } else if (bytes >= MB) {
        return QString("%1.%2 MB").arg(bytes / MB).arg((bytes % MB) * 10 / MB);
    } else if (bytes >= KB) {
        return QString("%1.%2 KB").arg(bytes / KB).arg((bytes % KB) * 10 / KB);
    } else {
        return QString("%1 bytes").arg(bytes);
    }
}


QByteArray PDFStegDialog::getDataToEmbed()
{
    QTabWidget *inputTabs = m_hideTab->findChild<QTabWidget*>();

    QByteArray rawData;
    QString filename;
    bool isFile = false;

    if (inputTabs && inputTabs->currentIndex() == 0) {
        // Text tab
        QString text = m_secretTextEdit->toPlainText();
        if (text.isEmpty()) {
            return QByteArray();
        }
        rawData = text.toUtf8();
    } else {
        // File tab
        QString filePath = m_secretFileEdit->text();
        if (filePath.isEmpty()) {
            return QByteArray();
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error",
                QString("Could not read file: %1").arg(filePath));
            return QByteArray();
        }

        rawData = file.readAll();
        file.close();
        isFile = true;
        filename = QFileInfo(filePath).fileName();
    }

    // Build packet with FNAM flag if it's a file
    QByteArray packet;
    if (isFile && !filename.isEmpty()) {
        packet.append("FNAM");
        QByteArray nameBytes = filename.toUtf8();
        packet.append(static_cast<char>(nameBytes.size()));
        packet.append(nameBytes);
    }
    packet.append(rawData);

    return packet;
}

void PDFStegDialog::displayExtractedData(const QByteArray &data)
{
    // Try to display as text if it looks like text
    QString text = QString::fromUtf8(data);
    
    // Check if it's mostly printable ASCII or UTF-8
    int printableCount = 0;
    for (int i = 0; i < text.size() && i < 1000; ++i) {
        if (text[i].isPrint() || text[i] == '\n' || text[i] == '\r' || text[i] == '\t') {
            printableCount++;
        }
    }
    
    if (printableCount > qMin(1000, text.size()) * 0.9) {
        m_extractedDataEdit->setPlainText(text);
    } else {
        m_extractedDataEdit->setPlainText("[Binary Data - " + formatSize(data.size()) + 
            "]\n\nUse 'Save to File' button to save this data.");
    }
}




bool PDFStegDialog::isBinaryData(const QByteArray &data) const
{
    if (data.isEmpty()) return false;
    int checkSize = qMin(data.size(), 1000);
    for (int i = 0; i < checkSize; ++i) {
        if (static_cast<unsigned char>(data[i]) == 0) return true;
    }
    return false;
}

QString PDFStegDialog::autoSaveExtractedFile(const QByteArray &data, const QString &originalFilename)
{
    QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (saveDir.isEmpty()) {
        saveDir = QDir::homePath();
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString filename;
    if (!originalFilename.isEmpty()) {
        filename = timestamp + "_ermis_" + originalFilename;
    } else {
        filename = timestamp + "_ermis_extracted.bin";
    }

    QString fullPath = saveDir + "/" + filename;

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return "Failed to save";
    }

    file.write(data);
    file.close();

    return fullPath;
}


void PDFStegDialog::setupCornerWidget()
{

    QPushButton *infoButton = new QPushButton("ⓘ", this);
    infoButton->setFixedSize(16, 16);
    infoButton->setToolTip(
        "<html><body style='white-space: nowrap;'>"
        "<b>📄 PDF Steganography</b><br>"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━<br><br>"

        "<b>🔒 Hide Tab:</b><br>"
        "• Enter text or select a file to hide<br>"
        "• Choose a carrier PDF<br>"
        "• Specify output location<br>"
        "• Optional: Encrypt data with passphrase<br>"
        "• Click 'Embed Data' to hide secrets<br><br>"

        "<b>🔓 Extract Tab:</b><br>"
        "• Select a stego PDF<br>"
        "• Click 'Extract Data'<br>"
        "• If encrypted, enter passphrase<br>"
        "• Text displays in viewer, binary files prompt to save<br><br>"

        "<b>📁 File Support:</b><br>"
        "• Any file type can be hidden<br>"
        "• Original filename preserved<br>"
        "• Binary files auto-prompt to save<br><br>"

        "<b>🔐 Encryption:</b><br>"
        "• AES-256-CBC encryption<br>"
        "• Passphrase protected<br>"
        "• Filename also encrypted when enabled<br><br>"

        "<b>📊 Capacity:</b><br>"
        "• Max ~50% of carrier PDF size<br>"
        "• Capacity shown when PDF selected<br>"
        "</body></html>"
    );
    infoButton->setObjectName("infoButton");

    if (Constants::isDarkTheme) {
        // Dark theme - exactly as you had it (working)
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
        // Light theme - same dimensions, light-appropriate colors
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
