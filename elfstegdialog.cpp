#include "elfstegdialog.h"
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
#include <QTabBar>
#include<QSettings>

ELFStegDialog::ELFStegDialog(QWidget *parent)
    : QDialog(parent)
    , m_engine(new ELFStegEngine(this))
    , m_encryptionEnabled(false)
{
    setWindowTitle("ELF Binary Steganography - Hide Secret Data in Executables");
    setMinimumSize(850, 700);
    setupUI();

    connect(m_engine, &ELFStegEngine::progressUpdated,
            this, &ELFStegDialog::onProgressUpdated);
    connect(m_engine, &ELFStegEngine::errorOccurred,
            this, &ELFStegDialog::onEngineError);
    connect(m_engine, &ELFStegEngine::embedSuccess,
            this, &ELFStegDialog::onEmbedSuccess);

    m_encryptCheckBox->setChecked(false);
}

ELFStegDialog::~ELFStegDialog()
{
}

void ELFStegDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &ELFStegDialog::onTabChanged);
    setupCornerWidget();

    setupHideTab();
    setupExtractTab();

    m_tabWidget->addTab(m_hideTab, "Hide Data");
    m_tabWidget->addTab(m_extractTab, "Extract Data");

    mainLayout->addWidget(m_tabWidget);
}

void ELFStegDialog::setupHideTab()
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
    m_secretTextEdit->setPlaceholderText("Enter text to hide in ELF binary...\n\nSupports letters, numbers, spaces, and punctuation.");
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

    // Carrier ELF Binary Section
    QGroupBox *carrierGroup = new QGroupBox("Carrier ELF Binary", m_hideTab);
    QHBoxLayout *carrierLayout = new QHBoxLayout(carrierGroup);
    m_carrierBinaryEdit = new QLineEdit(carrierGroup);
    m_carrierBinaryEdit->setPlaceholderText("Select ELF binary file to hide data in...");
    m_carrierBinaryEdit->setReadOnly(true);
    m_browseCarrierBinaryBtn = new QPushButton("Browse...", carrierGroup);
    connect(m_browseCarrierBinaryBtn, &QPushButton::clicked,
            this, &ELFStegDialog::onSelectCarrierBinary);
    carrierLayout->addWidget(m_carrierBinaryEdit);
    carrierLayout->addWidget(m_browseCarrierBinaryBtn);
    layout->addWidget(carrierGroup);

    // Output Binary Section
    QGroupBox *outputGroup = new QGroupBox("Output Binary", m_hideTab);
    QHBoxLayout *outputLayout = new QHBoxLayout(outputGroup);
    m_outputBinaryEdit = new QLineEdit(outputGroup);
    m_outputBinaryEdit->setPlaceholderText("Where to save the stego binary...");
    m_outputBinaryEdit->setReadOnly(true);
    m_browseOutputBinaryBtn = new QPushButton("Browse...", outputGroup);
    connect(m_browseOutputBinaryBtn, &QPushButton::clicked,
            this, &ELFStegDialog::onSelectOutputBinary);
    outputLayout->addWidget(m_outputBinaryEdit);
    outputLayout->addWidget(m_browseOutputBinaryBtn);
    layout->addWidget(outputGroup);

    // Options Section
    QGroupBox *optionsGroup = new QGroupBox("Options", m_hideTab);
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsGroup);

    m_encryptCheckBox = new QCheckBox("Encrypt data before hiding", optionsGroup);
    connect(m_encryptCheckBox, &QCheckBox::clicked,
            this, &ELFStegDialog::onEncryptToggled);

    m_clearAllBtn = new QPushButton("Clear All", optionsGroup);
    connect(m_clearAllBtn, &QPushButton::clicked,
            this, &ELFStegDialog::onClearAll);

    optionsLayout->addWidget(m_encryptCheckBox);
    optionsLayout->addStretch();
    optionsLayout->addWidget(m_clearAllBtn);
    layout->addWidget(optionsGroup);

    // Capacity Info
    QHBoxLayout *capacityLayout = new QHBoxLayout();
    m_capacityLabel = new QLabel("ELF Capacity: Not selected", m_hideTab);
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
            this, &ELFStegDialog::onEmbedClicked);
    actionLayout->addWidget(m_embedBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);

    layout->addStretch();
}

void ELFStegDialog::setupExtractTab()
{
    m_extractTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_extractTab);

    // Stego Binary Selection
    QGroupBox *inputGroup = new QGroupBox("Stego Binary", m_extractTab);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputGroup);
    m_stegoBinaryEdit = new QLineEdit(inputGroup);
    m_stegoBinaryEdit->setPlaceholderText("Select ELF binary containing hidden data...");
    m_stegoBinaryEdit->setReadOnly(true);
    m_browseStegoBinaryBtn = new QPushButton("Browse...", inputGroup);
    connect(m_browseStegoBinaryBtn, &QPushButton::clicked,
            this, &ELFStegDialog::onSelectStegoBinary);
    inputLayout->addWidget(m_stegoBinaryEdit);
    inputLayout->addWidget(m_browseStegoBinaryBtn);
    layout->addWidget(inputGroup);

    // Extract Button
    QHBoxLayout *extractBtnLayout = new QHBoxLayout();
    extractBtnLayout->addStretch();
    m_extractBtn = new QPushButton("Extract Data", m_extractTab);
    m_extractBtn->setMinimumWidth(150);
    connect(m_extractBtn, &QPushButton::clicked,
            this, &ELFStegDialog::onExtractClicked);
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
            this, &ELFStegDialog::onCopyExtracted);
    connect(m_saveExtractedBtn, &QPushButton::clicked,
            this, &ELFStegDialog::onSaveExtracted);
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

void ELFStegDialog::setupCornerWidget()
{
    QPushButton *infoButton = new QPushButton("ⓘ", this);
    infoButton->setFixedSize(16, 16);
    infoButton->setToolTip(
        "<html><body style='white-space: nowrap;'>"
        "<b>⚙️ ELF Binary Steganography</b><br>"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━<br><br>"

        "<b>🔒 Hide Tab:</b><br>"
        "• Enter text or select a file to hide<br>"
        "• Choose a carrier ELF binary<br>"
        "• Specify output location<br>"
        "• Optional: Encrypt data with passphrase<br>"
        "• Click 'Embed Data' to hide secrets<br><br>"

        "<b>🔓 Extract Tab:</b><br>"
        "• Select a stego binary<br>"
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
        "• Limited by ELF section size limits<br>"
        "• Capacity shown when binary selected<br>"
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

void ELFStegDialog::onTabChanged(int index)
{
    Q_UNUSED(index)
    m_statusLabel->setText("Ready");
    m_progressBar->setVisible(false);
}

/*
void ELFStegDialog::onSelectCarrierBinary()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");

    QString fileName = QFileDialog::getOpenFileName(this,
        "Select Carrier ELF Binary", Constants::appDirPath, "ELF Binaries (*)");

    if (!fileName.isEmpty()) {
        m_carrierBinaryEdit->setText(fileName);
        m_currentCarrierBaseName = QFileInfo(fileName).baseName();
        QString suggestedOutput = Constants::fusedImagesPath + "/" + timestamp + "_" + m_currentCarrierBaseName + "_stego";
        m_outputBinaryEdit->setText(suggestedOutput);
        updateCapacityDisplay();
    }
}
*/

void ELFStegDialog::onSelectCarrierBinary()
{
    // Check if we should show the warning
    QSettings settings;
    bool skipWarning = settings.value("elf_stego_skip_binary_warning", false).toBool();

    if (!skipWarning) {
        QMessageBox warningBox(this);
        warningBox.setWindowTitle("ELF Binary Selection");
        warningBox.setIcon(QMessageBox::Information);
        warningBox.setText(
            QString(
                "<b>Important Information About ELF Binaries</b><br><br>"
                "The binary you select must be:<br><br>"
                "• A valid ELF executable (e.g., /bin/ls, /usr/sbin/httpd, or a native Linux game)<br>"
                "• Located in <b>%1</b> or anywhere in your home directory (~)<br>"
                "• Owned by your user (you may need to run <b>chown</b> if copied from system directories)<br><br>"
                "<b>⚠️ AppImages are NOT recommended</b> — they contain a nested filesystem and may not work reliably.<br><br>"
                "If you're copying a binary from /bin/ or /usr/bin/, you may need to run:<br>"
                "<code>sudo cp /bin/ls ~/ &amp;&amp; sudo chown $USER:$USER ~/ls</code><br><br>"
                "The binary will remain fully functional after embedding data."
            ).arg(Constants::elfPath)
        );

        // Add "Do not show again" checkbox
        QCheckBox *doNotShowAgain = new QCheckBox("Do not show this message again");
        warningBox.setCheckBox(doNotShowAgain);

        warningBox.setStandardButtons(QMessageBox::Ok);
        warningBox.exec();

        if (doNotShowAgain->isChecked()) {
            settings.setValue("elf_stego_skip_binary_warning", true);
        }
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");

    QString fileName = QFileDialog::getOpenFileName(this,
        "Select Carrier ELF Binary",
        Constants::elfPath,  // Start in the elfPath directory
        "ELF Binaries (*)");

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    if (!isValidELFBinary(fileName)) {
        return;  // Not a valid ELF
    }

    m_carrierBinaryEdit->setText(fileName);
    m_currentCarrierBaseName = QFileInfo(fileName).baseName();
    QString suggestedOutput = Constants::fusedImagesPath + "/" + timestamp + "_" + m_currentCarrierBaseName + "_stego";
    m_outputBinaryEdit->setText(suggestedOutput);
    updateCapacityDisplay();

}

void ELFStegDialog::onSelectOutputBinary()
{
    QString defaultPath = m_outputBinaryEdit->text();
    if (defaultPath.isEmpty()) {
        defaultPath = Constants::fusedImagesPath + "/stego_binary";
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Stego Binary As", defaultPath, "All Files (*)");

    if (!fileName.isEmpty()) {
        m_outputBinaryEdit->setText(fileName);
    }
}

void ELFStegDialog::onSelectStegoBinary()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Select Stego Binary", Constants::fusedImagesPath, "All Files (*)");

    if (!fileName.isEmpty()) {
        m_stegoBinaryEdit->setText(fileName);
    }
}

void ELFStegDialog::onEncryptToggled(bool checked)
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

void ELFStegDialog::onEmbedClicked()
{
    // Validate inputs
    if (m_carrierBinaryEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a carrier ELF binary.");
        return;
    }

    if (m_outputBinaryEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify an output binary path.");
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
    qint64 capacity = m_engine->calculateELFCapacity(m_carrierBinaryEdit->text());
    if (packet.size() > capacity) {
        QMessageBox warningBox(this);
        warningBox.setWindowTitle("Capacity Warning");
        warningBox.setIcon(QMessageBox::Warning);
        warningBox.setText(
            QString("The data you're trying to hide (%1) exceeds the recommended capacity (%2) for this binary.\n\n"
                    "While it may still work, the resulting binary will be significantly larger than normal, "
                    "which could make it suspicious.\n\n"
                    "Do you want to continue anyway?")
                .arg(formatSize(packet.size()))
                .arg(formatSize(capacity))
        );

        QPushButton *continueButton = warningBox.addButton("Continue Anyway", QMessageBox::AcceptRole);
        QPushButton *cancelButton = warningBox.addButton("Cancel", QMessageBox::RejectRole);
        warningBox.setDefaultButton(cancelButton);

        warningBox.exec();

        if (warningBox.clickedButton() != continueButton) {
            return;  // User cancelled
        }
        // If continueButton was clicked, proceed with embedding
    }

    // Embed
    m_embedBtn->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 100);
    m_statusLabel->setText("Embedding data...");

    bool success = m_engine->embedDataInELF(
        m_carrierBinaryEdit->text(),
        m_outputBinaryEdit->text(),
        packet,
        ".debug_types"  // Use the stealth section name
    );

    if (!success) {
        onEngineError(m_engine->lastError());
    }
}


void ELFStegDialog::onExtractClicked()
{
    if (m_stegoBinaryEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a stego binary.");
        return;
    }

    m_extractBtn->setEnabled(false);
    m_statusLabel->setText("Extracting data...");
    m_extractedDataEdit->clear();

    QByteArray packet = m_engine->extractDataFromELF(m_stegoBinaryEdit->text(), ".debug_types");

    if (packet.isEmpty()) {
        m_extractBtn->setEnabled(true);
        m_statusLabel->setText("Ready");
        if (!m_engine->lastError().isEmpty()) {
            QMessageBox::warning(this, "Error", m_engine->lastError());
        } else {
            QMessageBox::warning(this, "Error", "No hidden data found in this binary.");
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
        QString suggestedName = originalFilename.isEmpty() ? "extracted.bin" : originalFilename;
        QString savePath = QFileDialog::getSaveFileName(this, "Save Extracted File",
            Constants::extractedImagesPath + "/" + suggestedName, "All Files (*.*)");

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

        /*
    } else {
        displayExtractedData(actualData);
        m_statusLabel->setText("Data extracted successfully!");
        QMessageBox::information(this, "Success", QString("Data extracted successfully! (%1)").arg(formatSize(actualData.size())));
    }
    */

    } else {
        // Text data
        if (!originalFilename.isEmpty()) {
            // It was a text file → save it with the original name
            QString savePath = QFileDialog::getSaveFileName(this, "Save Extracted File",
                Constants::extractedImagesPath + "/" + originalFilename, "All Files (*.*)");

            if (!savePath.isEmpty()) {
                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(actualData);
                    file.close();
                    m_extractedDataEdit->setPlainText(
                        QString("✅ File extracted successfully!\n\n"
                                "File: %1\n"
                                "Size: %2\n"
                                "Saved to: %3")
                            .arg(originalFilename)
                            .arg(formatSize(actualData.size()))
                            .arg(savePath)
                    );
                    m_statusLabel->setText("File saved successfully!");
                    displayExtractedData(actualData);

                    QMessageBox::information(this, "Success",
                        "File extracted and saved to:\n" + savePath);
                } else {
                    QMessageBox::warning(this, "Error", "Could not save file.");
                    m_statusLabel->setText("Save failed");
                }
            } else {
                m_statusLabel->setText("Save cancelled");
            }
        } else {
            // Plain text (typed in UI) → display in the view
            displayExtractedData(actualData);
            m_statusLabel->setText("Data extracted successfully!");
            QMessageBox::information(this, "Success",
                QString("Data extracted successfully! (%1)").arg(formatSize(actualData.size())));
        }
    }

    m_extractBtn->setEnabled(true);
}


/*
void ELFStegDialog::onExtractClicked()
{
    if (m_stegoBinaryEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a stego binary.");
        return;
    }

    m_extractBtn->setEnabled(false);
    m_statusLabel->setText("Extracting data...");
    m_extractedDataEdit->clear();

    QByteArray packet = m_engine->extractDataFromELF(m_stegoBinaryEdit->text(), ".debug_types");

    if (packet.isEmpty()) {
        m_extractBtn->setEnabled(true);
        m_statusLabel->setText("Ready");
        if (!m_engine->lastError().isEmpty()) {
            QMessageBox::warning(this, "Error", m_engine->lastError());
        } else {
            QMessageBox::warning(this, "Error", "No hidden data found in this binary.");
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
        QString suggestedName = originalFilename.isEmpty() ? "extracted.bin" : originalFilename;
        QString savePath = QFileDialog::getSaveFileName(this, "Save Extracted File",
            Constants::extractedImagesPath + "/" + suggestedName, "All Files (*.*)");

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
        displayExtractedData(actualData);
        m_statusLabel->setText("Data extracted successfully!");
        QMessageBox::information(this, "Success", QString("Data extracted successfully! (%1)").arg(formatSize(actualData.size())));
    }

    m_extractBtn->setEnabled(true);
}
*/


void ELFStegDialog::onProgressUpdated(int current, int total)
{
    if (total > 0) {
        m_progressBar->setValue((current * 100) / total);
    }
}

void ELFStegDialog::onEngineError(const QString &error)
{
    m_embedBtn->setEnabled(true);
    m_extractBtn->setEnabled(true);
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Error: " + error);
    QMessageBox::warning(this, "Engine Error", error);
}

void ELFStegDialog::onEmbedSuccess()
{
    m_embedBtn->setEnabled(true);
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Data embedded successfully!");

    QMessageBox::information(this, "Success",
        QString("Data successfully hidden in ELF binary!\n\nOutput: %1")
            .arg(m_outputBinaryEdit->text()));
}

void ELFStegDialog::onCopyExtracted()
{
    if (m_lastExtractedData.isEmpty()) {
        QMessageBox::warning(this, "Error", "No data to copy.");
        return;
    }

    QString text = QString::fromUtf8(m_lastExtractedData);
    if (text.contains(QRegularExpression("^[\\x20-\\x7E\\n\\r\\t\\x80-\\xFF]+$"))) {
        QApplication::clipboard()->setText(text);
    } else {
        QMessageBox::information(this, "Info",
            "Binary data cannot be copied to clipboard. Please use 'Save to File'.");
        return;
    }

    QMessageBox::information(this, "Success", "Data copied to clipboard.");
}

void ELFStegDialog::onSaveExtracted()
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

void ELFStegDialog::onClearAll()
{
    m_secretTextEdit->clear();
    m_secretFileEdit->clear();
    m_carrierBinaryEdit->clear();
    m_outputBinaryEdit->clear();
    m_extractedDataEdit->clear();
    m_stegoBinaryEdit->clear();

    m_encryptCheckBox->blockSignals(true);
    m_encryptCheckBox->setChecked(false);
    m_encryptCheckBox->blockSignals(false);
    m_encryptionEnabled = false;
    m_encryptionPassphrase.clear();

    m_lastExtractedData.clear();

    m_statusLabel->setText("Ready");
    m_progressBar->setVisible(false);
    m_capacityLabel->setText("ELF Capacity: Not selected");
    m_currentCarrierBaseName.clear();
    m_tabWidget->setCurrentIndex(0);
}

void ELFStegDialog::updateCapacityDisplay()
{
    QString carrierPath = m_carrierBinaryEdit->text();
    if (!carrierPath.isEmpty() && QFile::exists(carrierPath)) {
        qint64 capacity = m_engine->calculateELFCapacity(carrierPath);
        m_capacityLabel->setText(QString("ELF Capacity: %1").arg(formatSize(capacity)));
    } else {
        m_capacityLabel->setText("ELF Capacity: Not selected");
    }
}

QString ELFStegDialog::formatSize(qint64 bytes) const
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

QByteArray ELFStegDialog::getDataToEmbed()
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

void ELFStegDialog::displayExtractedData(const QByteArray &data)
{
    QString text = QString::fromUtf8(data);

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

bool ELFStegDialog::isBinaryData(const QByteArray &data) const
{
    if (data.isEmpty()) return false;
    int checkSize = qMin(data.size(), 1000);
    for (int i = 0; i < checkSize; ++i) {
        if (static_cast<unsigned char>(data[i]) == 0) return true;
    }
    return false;
}

QPushButton *ELFStegDialog::clearAllBtn() const
{
    return m_clearAllBtn;
}

void ELFStegDialog::onSelectSecretFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Select File to Hide",
        Constants::elfPath,
        QString());

    if (!fileName.isEmpty()) {
        m_secretFileEdit->setText(fileName);
        updateCapacityDisplay();
    }
}


bool ELFStegDialog::isValidELFBinary(const QString &filePath, bool showWarning)
{
    if (filePath.isEmpty()) {
        if (showWarning) {
            QMessageBox::warning(this, "Invalid File", "No file selected.");
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (showWarning) {
            QMessageBox::warning(this, "Invalid File",
                QString("Cannot open file:\n%1").arg(filePath));
        }
        return false;
    }

    QByteArray magic = file.read(4);
    file.close();

    bool isValid = (magic.size() == 4 &&
                    static_cast<unsigned char>(magic[0]) == 0x7F &&
                    magic[1] == 'E' &&
                    magic[2] == 'L' &&
                    magic[3] == 'F');

    if (!isValid && showWarning) {
        QMessageBox::warning(this, "Invalid ELF Binary",
            "The selected file is not a valid ELF binary.\n\n"
            "Please select an executable file (e.g., /bin/ls, /usr/bin/bash).\n"
            "AppImages, scripts, and other file types are NOT supported.");
    }

    return isValid;
}
