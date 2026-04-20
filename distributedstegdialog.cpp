#include "distributedstegdialog.h"
#include "distributedstegengine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>
#include<QJsonObject>
#include<QUrlQuery>
#include<QNetworkReply>
#include"passphrasedialog.h"
#include<QRegularExpression>
#include"constants.h"

DistributedStegDialog::DistributedStegDialog(QWidget *parent)
    : QDialog(parent)
    , m_engine(new DistributedStegEngine(this))
    , m_encryptionEnabled(false)
{
    setWindowTitle("Distributed Steganography - Wikipedia Pointer Map");
    setMinimumSize(900, 750);
    setupUI();

    connect(m_engine, &DistributedStegEngine::progressUpdated,
            this, &DistributedStegDialog::onProgressUpdated);
    connect(m_engine, &DistributedStegEngine::mapReady,
            this, &DistributedStegDialog::onMapReady);
    connect(m_engine, &DistributedStegEngine::messageReconstructed,
            this, &DistributedStegDialog::onMessageReconstructed);
    connect(m_engine, &DistributedStegEngine::errorOccurred,
            this, &DistributedStegDialog::onEngineError);

    m_encryptCheckBox->setChecked(false);
}

DistributedStegDialog::~DistributedStegDialog()
{
}

void DistributedStegDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &DistributedStegDialog::onTabChanged);
    setupCornerWidget();
    setupHideTab();
    setupExtractTab();

    m_tabWidget->addTab(m_hideTab, "Build Pointer Map");
    m_tabWidget->addTab(m_extractTab, "Reconstruct Message");

    mainLayout->addWidget(m_tabWidget);
}

void DistributedStegDialog::setupHideTab()
{
    m_hideTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_hideTab);

    // Secret Message Section
    QGroupBox *secretGroup = new QGroupBox("Secret Message", m_hideTab);
    QVBoxLayout *secretLayout = new QVBoxLayout(secretGroup);

    secretLayout->addWidget(new QLabel("Enter the secret message to hide:"));

    m_secretMessageEdit = new QPlainTextEdit(secretGroup);
    m_secretMessageEdit->setPlaceholderText("Type your secret message here...\n\nExample: meet mayor at midnight");
    m_secretMessageEdit->setMinimumHeight(120);

    connect(m_secretMessageEdit, &QPlainTextEdit::textChanged, this, &DistributedStegDialog::validateSecretMessage);

    secretLayout->addWidget(m_secretMessageEdit);

    layout->addWidget(secretGroup);

    // Options Section
    QGroupBox *optionsGroup = new QGroupBox("Options", m_hideTab);
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsGroup);

    optionsLayout->addWidget(new QLabel("Wikipedia Language:"));
    m_languageCombo = new QComboBox(optionsGroup);
    m_languageCombo->addItem("English", "en");
    m_languageCombo->addItem("German", "de");
    m_languageCombo->addItem("French", "fr");
    m_languageCombo->addItem("Spanish", "es");
    m_languageCombo->addItem("Italian", "it");
    m_languageCombo->addItem("Portuguese", "pt");
    m_languageCombo->addItem("Dutch", "nl");
    m_languageCombo->addItem("Russian", "ru");
    m_languageCombo->addItem("Japanese", "ja");
    m_languageCombo->addItem("Chinese", "zh");


    for (int i = 1; i < m_languageCombo->count(); ++i) {
        m_languageCombo->setItemData(i, QVariant(0), Qt::UserRole-1);
        m_languageCombo->setItemData(i,
            "Currently only English is supported. Other languages will be available in future updates.",
            Qt::ToolTipRole);
    }

    // Set English as default
    m_languageCombo->setCurrentIndex(0);


    m_encryptCheckBox = new QCheckBox("Encrypt JSON Pointer Map", optionsGroup);
    connect(m_encryptCheckBox, &QCheckBox::clicked, this, [this](bool checked){
            onEncryptToggled(checked);
    });
    m_clearAllBtn = new QPushButton("Clear All", optionsGroup);
    connect(m_clearAllBtn, &QPushButton::clicked, this, &DistributedStegDialog::resetForNewOperation);

    optionsLayout->addWidget(m_languageCombo);
    optionsLayout->addStretch();

    //optionsLayout->addSpacing(10);
    optionsLayout->addWidget(m_clearAllBtn);
    optionsLayout->addStretch();

    optionsLayout->addWidget(m_encryptCheckBox);
    //optionsLayout->addStretch();

    layout->addWidget(optionsGroup);

    // Progress Section
    QHBoxLayout *progressLayout = new QHBoxLayout();
    m_progressLabel = new QLabel("Ready", m_hideTab);
    progressLayout->addWidget(m_progressLabel);
    progressLayout->addStretch();
    layout->addLayout(progressLayout);

    // Action Buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_buildMapBtn = new QPushButton("Build Pointer Map", m_hideTab);
    connect(m_buildMapBtn, &QPushButton::clicked, this, &DistributedStegDialog::onBuildMapClicked);
    actionLayout->addStretch();
    actionLayout->addWidget(m_buildMapBtn);
    layout->addLayout(actionLayout);

    // Pointer Map Output Section
    QGroupBox *mapGroup = new QGroupBox("Generated Pointer Map", m_hideTab);
    QVBoxLayout *mapLayout = new QVBoxLayout(mapGroup);

    QHBoxLayout *mapHeaderLayout = new QHBoxLayout();
    mapHeaderLayout->addStretch();
    m_copyMapBtn = new QPushButton("Copy to Clipboard", mapGroup);
    m_saveMapBtn = new QPushButton("Save to File...", mapGroup);
    connect(m_copyMapBtn, &QPushButton::clicked, this, &DistributedStegDialog::onCopyMap);
    connect(m_saveMapBtn, &QPushButton::clicked, this, &DistributedStegDialog::onSaveMap);
    mapHeaderLayout->addWidget(m_copyMapBtn);
    mapHeaderLayout->addWidget(m_saveMapBtn);

    m_mapDisplayEdit = new QPlainTextEdit(mapGroup);
    m_mapDisplayEdit->setReadOnly(true);
    m_mapDisplayEdit->setPlaceholderText("JSON pointer map will appear here...");
    m_mapDisplayEdit->setMinimumHeight(200);

    mapLayout->addLayout(mapHeaderLayout);
    mapLayout->addWidget(m_mapDisplayEdit);

    layout->addWidget(mapGroup);
}

void DistributedStegDialog::setupExtractTab()
{
    m_extractTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_extractTab);

    // Pointer Map Input Section
    QGroupBox *inputGroup = new QGroupBox("Pointer Map Input", m_extractTab);
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);

    QHBoxLayout *inputHeaderLayout = new QHBoxLayout();
    inputHeaderLayout->addWidget(new QLabel("Load pointer map JSON:"));
    inputHeaderLayout->addStretch();
    m_loadMapBtn = new QPushButton("Load File", inputGroup);
    connect(m_loadMapBtn, &QPushButton::clicked, this, &DistributedStegDialog::onLoadMap);
    inputHeaderLayout->addWidget(m_loadMapBtn);

    m_mapInputEdit = new QPlainTextEdit(inputGroup);
    m_mapInputEdit->setPlaceholderText("Paste pointer map JSON here or load from file...");
    m_mapInputEdit->setMinimumHeight(150);

    inputLayout->addLayout(inputHeaderLayout);
    inputLayout->addWidget(m_mapInputEdit);

    layout->addWidget(inputGroup);

    // Reconstruct Button
    QHBoxLayout *reconstructLayout = new QHBoxLayout();
    reconstructLayout->addStretch();
    m_reconstructBtn = new QPushButton("Reconstruct Message", m_extractTab);
    connect(m_reconstructBtn, &QPushButton::clicked, this, &DistributedStegDialog::onReconstructClicked);
    reconstructLayout->addWidget(m_reconstructBtn);
    layout->addLayout(reconstructLayout);

    // Recovered Message Output Section
    QGroupBox *outputGroup = new QGroupBox("Recovered Message", m_extractTab);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);

    QHBoxLayout *outputHeaderLayout = new QHBoxLayout();
    outputHeaderLayout->addStretch();
    m_copyMessageBtn = new QPushButton("Copy to Clipboard", outputGroup);
    m_saveMessageBtn = new QPushButton("Save to File...", outputGroup);
    connect(m_copyMessageBtn, &QPushButton::clicked, this, &DistributedStegDialog::onCopyMessage);
    connect(m_saveMessageBtn, &QPushButton::clicked, this, &DistributedStegDialog::onSaveMessage);
    outputHeaderLayout->addWidget(m_copyMessageBtn);
    outputHeaderLayout->addWidget(m_saveMessageBtn);

    m_recoveredMessageEdit = new QPlainTextEdit(outputGroup);
    m_recoveredMessageEdit->setReadOnly(true);
    m_recoveredMessageEdit->setPlaceholderText("Recovered secret message will appear here...");
    m_recoveredMessageEdit->setMinimumHeight(150);

    outputLayout->addLayout(outputHeaderLayout);
    outputLayout->addWidget(m_recoveredMessageEdit);

    layout->addWidget(outputGroup);
}


void DistributedStegDialog::onBuildMapClicked()
{
    QString secretMessage = m_secretMessageEdit->toPlainText().trimmed();
    if (secretMessage.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a secret message.");
        return;
    }

    m_engine->reset();

    QString language = m_languageCombo->currentData().toString();
    m_mapDisplayEdit->clear();
    m_buildMapBtn->setEnabled(false);
    m_buildMapBtn->setText("Building...");

    m_engine->buildPointerMap(secretMessage, language);
}

void DistributedStegDialog::onReconstructClicked()
{
    QString mapText = m_mapInputEdit->toPlainText().trimmed();
    if (mapText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter or load pointer map JSON.");
        return;
    }

    QByteArray jsonData = mapText.toUtf8();

    // Check if it starts with '[' (old format) or '{' (new wrapper format)
    QString trimmed = QString::fromUtf8(jsonData).trimmed();

    if (!trimmed.startsWith("[") && !trimmed.startsWith("{")) {
        // Treat as encrypted
        PassphraseDialog dialog(false, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString passphrase = dialog.getPassphrase();
            QByteArray decoded = QByteArray::fromBase64(jsonData);
            QByteArray decrypted = m_crypto.decryptData(decoded, passphrase);
            if (!decrypted.isEmpty()) {
                jsonData = decrypted;
            } else {
                QMessageBox::warning(this, "Error", "Decryption failed. Wrong passphrase?");
                m_reconstructBtn->setEnabled(true);
                m_reconstructBtn->setText("Reconstruct Message");
                return;
            }
        } else {
            m_reconstructBtn->setEnabled(true);
            m_reconstructBtn->setText("Reconstruct Message");
            return;
        }
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QString language = "en";
    QJsonArray pointers;

    if (doc.isObject()) {
        // New format with language wrapper
        QJsonObject wrapper = doc.object();
        language = wrapper["language"].toString("en");
        pointers = wrapper["pointers"].toArray();
    } else if (doc.isArray()) {
        // Old format (backward compatible)
        pointers = doc.array();
    } else {
        QMessageBox::warning(this, "Error", "Invalid JSON format.");
        m_reconstructBtn->setEnabled(true);
        m_reconstructBtn->setText("Reconstruct Message");
        return;
    }

    // Set the language in the engine
    m_engine->setLanguage(language);

    m_recoveredMessageEdit->clear();
    m_reconstructBtn->setEnabled(false);
    m_reconstructBtn->setText("Reconstructing...");

    QString message = m_engine->reconstructMessage(pointers);
    m_recoveredMessageEdit->setPlainText(message);
    m_currentMessage = message;

    if (m_engine->lastError().isEmpty()) {
        QMessageBox::information(this, "Success", "Message reconstructed successfully.");
    } else {
        QMessageBox::warning(this, "Error", m_engine->lastError());
    }

    m_reconstructBtn->setEnabled(true);
    m_reconstructBtn->setText("Reconstruct Message");
}


void DistributedStegDialog::onCopyMap()
{
    QString map = m_mapDisplayEdit->toPlainText();
    if (map.isEmpty()) {
        QMessageBox::warning(this, "Error", "No pointer map to copy.");
        return;
    }
    QApplication::clipboard()->setText(map);
}

void DistributedStegDialog::onSaveMap()
{
    QString map = m_mapDisplayEdit->toPlainText();
    if (map.isEmpty()) {
        QMessageBox::warning(this, "Error", "No pointer map to save.");
        return;
    }

    // Choose default extension based on encryption
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString defaultExt = m_encryptionEnabled ? ".txt" : ".json";
    QString defaultFileName = QString("%1_dist_text%2").arg(timestamp).arg(defaultExt);
    QString defaultPath = Constants::fusedImagesPath + "/" + defaultFileName;

    // Build filter string
    QString filter;
    if (m_encryptionEnabled) {
        filter = "Text Files (*.txt);;All Files (*)";
    } else {
        filter = "JSON Files (*.json);;All Files (*)";
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Pointer Map",
        defaultPath, filter);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }
    file.write(map.toUtf8());
    file.close();

    QMessageBox::information(this, "Success", "Pointer map saved.");
}

void DistributedStegDialog::onLoadMap()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Pointer Map",
            Constants::fusedImagesPath,
            "Pointer Map Files (*.json *.txt);;JSON Files (*.json);;Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    m_mapInputEdit->setPlainText(QString::fromUtf8(data));
}

void DistributedStegDialog::onCopyMessage()
{
    QString message = m_recoveredMessageEdit->toPlainText();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "Error", "No recovered message to copy.");
        return;
    }
    QApplication::clipboard()->setText(message);
}

void DistributedStegDialog::onSaveMessage()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    QString message = m_recoveredMessageEdit->toPlainText();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "Error", "No recovered message to save.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Recovered Message",
        Constants::extractedImagesPath + "/" + timestamp + "_dist_extracted.txt", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }
    file.write(message.toUtf8());
    file.close();

    QMessageBox::information(this, "Success", "Message saved.");
}

void DistributedStegDialog::onTabChanged(int index)
{
    // Clear outputs when switching tabs
    if (index == 0) {
        // Switched to Hide tab - nothing to clear
    } else if (index == 1) {
        // Switched to Extract tab
    }
}

void DistributedStegDialog::onProgressUpdated(int current, int total)
{
    m_progressLabel->setText(QString("Progress: %1/%2 words mapped").arg(current).arg(total));
    if (current == total && total > 0) {
        m_progressLabel->setText("Complete!");
    }
}



void DistributedStegDialog::onMapReady(const QJsonArray &map)
{
    m_currentMap = map;

    QJsonObject wrapper;
    wrapper["language"] = m_languageCombo->currentData().toString();
    wrapper["pointers"] = map;


    QJsonDocument doc(wrapper);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    if (m_encryptionEnabled && !m_encryptionPassphrase.isEmpty()) {
        QByteArray encrypted = m_crypto.encryptData(jsonData, m_encryptionPassphrase);
        if (!encrypted.isEmpty()) {
            jsonData = encrypted.toBase64();
            QMessageBox::information(this, "Encryption",
                "Pointer map encrypted. Save this data - you'll need the passphrase to decrypt it.");
        } else {
            QMessageBox::warning(this, "Error", "Encryption failed.");
        }
    }

    m_mapDisplayEdit->setPlainText(QString::fromUtf8(jsonData));

    m_buildMapBtn->setEnabled(true);
    m_buildMapBtn->setText("Build Pointer Map");

    QMessageBox::information(this, "Success",
        QString("Pointer map built with %1 word mappings.").arg(map.size()));
}


void DistributedStegDialog::onMessageReconstructed(const QString &message)
{
    m_recoveredMessageEdit->setPlainText(message);
    m_currentMessage = message;
}

void DistributedStegDialog::onEngineError(const QString &error)
{
    m_buildMapBtn->setEnabled(true);
    m_buildMapBtn->setText("Build Pointer Map");
    m_reconstructBtn->setEnabled(true);
    m_reconstructBtn->setText("Reconstruct Message");
    m_engine->reset();
    QMessageBox::warning(this, "Engine Error", error);
}



void DistributedStegDialog::validateSecretMessage()
{
    QString text = m_secretMessageEdit->toPlainText();
    QString filtered;

    // Only allow: a-z, A-Z, 0-9, and space
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];
        if (ch.isLetterOrNumber() || ch == ' ') {
            filtered.append(ch);
        }
    }

    // Only replace if different
    if (filtered != text) {
        bool oldState = m_secretMessageEdit->blockSignals(true);
        m_secretMessageEdit->setPlainText(filtered);
        m_secretMessageEdit->blockSignals(oldState);

        // Move cursor to end
        QTextCursor cursor = m_secretMessageEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_secretMessageEdit->setTextCursor(cursor);
    }
}


void DistributedStegDialog::resetForNewOperation()
{
    // Reset engine
    m_engine->reset();

    // Clear all UI fields
    m_secretMessageEdit->clear();
    m_mapDisplayEdit->clear();
    m_progressLabel->setText("Ready");
    m_buildMapBtn->setEnabled(true);
    m_buildMapBtn->setText("Build Pointer Map");

    // Clear extract tab fields
    m_mapInputEdit->clear();
    m_recoveredMessageEdit->clear();

    // Reset to hide tab
    m_tabWidget->setCurrentIndex(0);

    // Clear any stored state
    m_currentMap = QJsonArray();
    m_currentMessage.clear();

    m_encryptCheckBox->blockSignals(true);
    m_encryptCheckBox->setChecked(false);
    m_encryptCheckBox->blockSignals(false);
    m_encryptionEnabled = false;
    m_encryptionPassphrase.clear();
}

void DistributedStegDialog::onEncryptToggled(bool checked)
{

    if (checked) {
        PassphraseDialog dialog(true, this);
        if (dialog.exec() == QDialog::Accepted) {
            m_encryptionPassphrase = dialog.getPassphrase();
            m_encryptionEnabled = true;
        } else {
            // User cancelled - uncheck the box
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

void DistributedStegDialog::setupCornerWidget()
{
    QPushButton *infoButton = new QPushButton("ⓘ", this);
    infoButton->setFixedSize(16, 16);
    infoButton->setObjectName("infoButton");

    infoButton->setToolTip(
        "<html><body style='white-space: nowrap;'>"
        "<b>🌐 Distributed Steganography</b><br>"
        "━━━━━━━━━━━━━━━━━━━━━━━━━━━━<br><br>"

        "<b>⚠️ Important:</b><br>"
        "• Avoid excessively long messages<br>"
        "• Keep text short and generic<br>"
        "• Avoid phrases like 'I am', 'we are', 'you will'<br>"
        "• Neutral content works best<br><br>"

        "<b>📤 Build Map:</b><br>"
        "• Enter message (A-Z, 0-9, space)<br>"
        "• Select Wikipedia language<br>"
        "• Optional: Encrypt with passphrase<br>"
        "• Click 'Build Pointer Map'<br><br>"

        "<b>📥 Reconstruct:</b><br>"
        "• Paste or load pointer map JSON<br>"
        "• Encrypted maps prompt for passphrase<br>"
        "• Click 'Reconstruct Message'<br><br>"

        "<b>🔐 Encryption:</b><br>"
        "• AES-256-CBC<br>"
        "• Base64 encoded output<br>"
        "• Passphrase required for decryption<br><br>"

        "<b>📁 Files:</b><br>"
        "• Plain: .json | Encrypted: .txt<br>"
        "• Copy to clipboard supported<br>"
        "• Load existing maps from file<br><br>"

        "<b>⏱️ Note:</b><br>"
        "• Wikipedia API rate limits apply<br>"
        "• Progress shown during mapping<br>"
        "</body></html>"
    );

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
