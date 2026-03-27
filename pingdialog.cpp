#include "pingdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QSettings>
#include <QCompleter>
#include"constants.h"
#include"passphrasedialog.h"
#include<QCryptographicHash>
#include<QApplication>
#include<QClipboard>

PingDialog::PingDialog(QWidget *parent)
    : QDialog(parent)
    , m_engine(new ICMPStegEngine(this))
    , ipFilter(new IpFilter(this))
    , udpEngine(new UDPStegEngine(this))
    , dnsEngine(new DNSStegEngine(this))
    , httpEngine(new HTTPStegEngine(this))

{
    setWindowTitle("Network Steganography");
    setMinimumSize(800, 600);
    setupUI();
    
    // Connect engine signals
    connect(m_engine, &ICMPStegEngine::dataReceived,
            this, &PingDialog::onDataReceived);
    connect(m_engine, &ICMPStegEngine::fileReceived,
            this, &PingDialog::onFileReceived);
    connect(m_engine, &ICMPStegEngine::sendProgress,
            this, &PingDialog::onSendProgress);
    connect(m_engine, &ICMPStegEngine::receiveProgress,
            this, &PingDialog::onReceiveProgress);
    connect(m_engine, &ICMPStegEngine::error,
            this, &PingDialog::onError);


    // ===== ADD UDP ENGINE CONNECTIONS =====
        connect(udpEngine, &UDPStegEngine::dataReceived,
                this, &PingDialog::onDataReceived);
        connect(udpEngine, &UDPStegEngine::fileReceived,
                this, &PingDialog::onFileReceived);
        connect(udpEngine, &UDPStegEngine::sendProgress,
                this, &PingDialog::onSendProgress);
        connect(udpEngine, &UDPStegEngine::receiveProgress,
                this, &PingDialog::onReceiveProgress);
        connect(udpEngine, &UDPStegEngine::error,
                this, &PingDialog::onError);
        // ===== END UDP CONNECTIONS =====

        // ===== ADD DNS ENGINE CONNECTIONS =====
            connect(dnsEngine, &DNSStegEngine::dataReceived,
                    this, &PingDialog::onDataReceived);
            connect(dnsEngine, &DNSStegEngine::fileReceived,
                    this, &PingDialog::onFileReceived);
            connect(dnsEngine, &DNSStegEngine::sendProgress,
                    this, &PingDialog::onSendProgress);
            connect(dnsEngine, &DNSStegEngine::receiveProgress,
                    this, &PingDialog::onReceiveProgress);
            connect(dnsEngine, &DNSStegEngine::error,
                    this, &PingDialog::onError);
            // ===== END DNS CONNECTIONS =====


            // ===== ADD HTTP ENGINE CONNECTIONS =====
                connect(httpEngine, &HTTPStegEngine::dataReceived,
                        this, &PingDialog::onDataReceived);
                connect(httpEngine, &HTTPStegEngine::fileReceived,
                        this, &PingDialog::onFileReceived);
                connect(httpEngine, &HTTPStegEngine::sendProgress,
                        this, &PingDialog::onSendProgress);
                connect(httpEngine, &HTTPStegEngine::receiveProgress,
                        this, &PingDialog::onReceiveProgress);
                connect(httpEngine, &HTTPStegEngine::error,
                        this, &PingDialog::onError);
                // ===== END HTTP CONNECTIONS =====
    
    // Load saved target IPs
    QSettings settings;
    QStringList targets = settings.value("ping/targetHistory").toStringList();
    m_targetHistoryCombo->addItems(targets);
    
    updateButtonStates();
    addToLog(" ICMP Ping Steganography ready");
    addToLog("ℹ️ Note: Requires root privileges. Run with sudo or set capabilities.");
    addToLog("📝 REMINDER: IP filter also added to UDP and DNS engines");
    populateListWidgetFromTmp();

    Constants::ipFilterEnabled = settings.value("IPFilterEnabled", false).toBool();
    Constants::ipFilterMode = settings.value("IPFilterMode", 0).toInt();
    Constants::ipFilterEntries = settings.value("IPFilterEntries").toStringList();

    ipFilter->setEnabled(Constants::ipFilterEnabled);
    ipFilter->setMode(Constants::ipFilterMode == 0 ? IpFilter::BlockMode : IpFilter::AllowMode);
    ipFilter->setEntries(Constants::ipFilterEntries);

    Constants::currentProtocol = Constants::ICMP;  // Default to ICMP
    Constants::currentReceiverProtocol = Constants::ICMP;  // Default to ICMP

}

void PingDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    
    // === SEND TAB ===
    m_sendTab = new QWidget();
    QVBoxLayout* sendLayout = new QVBoxLayout(m_sendTab);
    
    // Target IP group
    QGroupBox* targetGroup = new QGroupBox("Target");
    QHBoxLayout* targetLayout = new QHBoxLayout(targetGroup);
    targetLayout->addWidget(new QLabel("IP Address:"));
    
    m_targetHistoryCombo = new QComboBox();
    m_targetHistoryCombo->setEditable(true);
    m_targetHistoryCombo->setInsertPolicy(QComboBox::InsertAtTop);
    m_targetHistoryCombo->setMinimumWidth(200);

    targetLayout->addWidget(m_targetHistoryCombo);
    
    m_targetIpEdit = m_targetHistoryCombo->lineEdit();
    m_targetIpEdit->setPlaceholderText("e.g., 127.0.0.1 or 192.168.1.100");
    targetLayout->addStretch();
    sendLayout->addWidget(targetGroup);

    //
    // Protocol selection
    QHBoxLayout* protocolLayout = new QHBoxLayout();
    protocolLayout->addWidget(new QLabel("Carrier:"));

    m_protocolCombo = new QComboBox();
    m_protocolCombo->addItem("ICMP (ping)");
    m_protocolCombo->addItem("UDP");
    m_protocolCombo->addItem("DNS");
    m_protocolCombo->addItem("HTTP");

    protocolLayout->addWidget(m_protocolCombo);

    protocolLayout->addWidget(new QLabel("Port:"));
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(Constants::bindPort);
    m_portSpinBox->setEnabled(false);
    m_portSpinBox->setMaximumWidth(70);
    protocolLayout->addWidget(m_portSpinBox);
    protocolLayout->addStretch();

    connect(m_portSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {


        if (!m_listenBtn->isEnabled()) {
                return; // not needed to block receiver buttons
                // Block signals temporarily to prevent recursion
                m_portSpinBox->blockSignals(true);

                // Restore the previous value
                m_portSpinBox->setValue(static_cast<int>(Constants::bindPort));

                // Unblock signals
                m_portSpinBox->blockSignals(false);

                // Show warning message
                QMessageBox::warning(this,
                    "Cannot Change Port",
                    "Please stop listening before changing the UDP port.\n\n"
                    "Click 'Stop Listening' first, then change the port.");

                return;
            }
        // Stop both engines (safe to call both)
        m_engine->stopListening();
        udpEngine->stopListening();
        Constants::bindPort = static_cast<quint16>(value);
    });
    sendLayout->addLayout(protocolLayout);
/*
    connect(m_protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Constants::currentProtocol = (index == 0) ? Constants::ICMP : Constants::UDP;
        m_portEdit->setEnabled(Constants::currentProtocol == Constants::UDP);
    });
*/

    connect(m_protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PingDialog::onProtocolChanged);

//
    // Input mode selection
    QHBoxLayout* modeLayout = new QHBoxLayout();
    m_textModeRadio = new QRadioButton("Text Message");
    m_fileModeRadio = new QRadioButton("File Transfer");
    m_textModeRadio->setChecked(true);
    modeLayout->addWidget(m_textModeRadio);
    modeLayout->addWidget(m_fileModeRadio);


    m_encryptCheckBox = new QCheckBox("Encrypt with passphrase");
    modeLayout->addWidget(m_encryptCheckBox);

    modeLayout->addStretch();
    sendLayout->addLayout(modeLayout);
    
    // Text input group
    QGroupBox* textGroup = new QGroupBox("Message");
    QVBoxLayout* textLayout = new QVBoxLayout(textGroup);
    m_textInput = new QPlainTextEdit();
    m_textInput->setPlaceholderText("Enter your secret message here...");
    m_textInput->setMinimumHeight(150);
    textLayout->addWidget(m_textInput);
    sendLayout->addWidget(textGroup);
    
    // File input group
    QGroupBox* fileGroup = new QGroupBox("File");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    QHBoxLayout* fileBrowseLayout = new QHBoxLayout();
    m_filePathEdit = new QLineEdit();
    m_filePathEdit->setPlaceholderText("Select a file to send...");
    m_filePathEdit->setReadOnly(true);
    fileBrowseLayout->addWidget(m_filePathEdit);
    
    m_browseBtn = new QPushButton("Browse...");
    connect(m_browseBtn, &QPushButton::clicked, this, &PingDialog::onBrowseFile);
    fileBrowseLayout->addWidget(m_browseBtn);
    fileLayout->addLayout(fileBrowseLayout);
    sendLayout->addWidget(fileGroup);
    
    // Send button and progress
    QHBoxLayout* sendBtnLayout = new QHBoxLayout();
    m_sendBtn = new QPushButton("Send");
    m_sendBtn->setMinimumHeight(40);
    m_sendBtn->setEnabled(false);
    connect(m_sendBtn, &QPushButton::clicked, [this]() {
        if (m_textModeRadio->isChecked()) {
            onSendText();
            m_encryptCheckBox->setChecked(false);
        }else{
            onSendFile();
            m_encryptCheckBox->setChecked(false);
        }
    });

    m_cancelSendBtn = new QPushButton("Cancel");
    m_cancelSendBtn->setMinimumHeight(40);
    m_cancelSendBtn->setEnabled(false);
    connect(m_cancelSendBtn, &QPushButton::clicked, this, &PingDialog::onCancelSend);


    sendBtnLayout->addStretch();
    sendBtnLayout->addWidget(m_sendBtn);
    sendBtnLayout->addWidget(m_cancelSendBtn);
    sendBtnLayout->addStretch();
    sendLayout->addLayout(sendBtnLayout);
    
    m_sendProgress = new QProgressBar();
    m_sendProgress->setVisible(false);
    sendLayout->addWidget(m_sendProgress);
    
    m_sendStatusLabel = new QLabel();
    sendLayout->addWidget(m_sendStatusLabel);
    
    sendLayout->addStretch();
    
    // === RECEIVE TAB ===
    m_receiveTab = new QWidget();
    QVBoxLayout* receiveLayout = new QVBoxLayout(m_receiveTab);
    

    QHBoxLayout* receiverProtocolLayout = new QHBoxLayout();
    receiverProtocolLayout->addWidget(new QLabel("Receive Protocol:"));

    m_receiverProtocolCombo = new QComboBox();
    m_receiverProtocolCombo->addItem("ICMP (ping)");
    m_receiverProtocolCombo->addItem("UDP");
    m_receiverProtocolCombo->addItem("DNS");
    m_receiverProtocolCombo->addItem("HTTP");

    connect(m_receiverProtocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PingDialog::onReceiverProtocolChanged);
    receiverProtocolLayout->addWidget(m_receiverProtocolCombo);

    receiverProtocolLayout->addWidget(new QLabel("Port:"));
    m_receiverPortSpinBox = new QSpinBox();
    m_receiverPortSpinBox->setRange(1, 65535);
    m_receiverPortSpinBox->setValue(Constants::bindPort);
    m_receiverPortSpinBox->setEnabled(false);  // Initially disabled
    m_receiverPortSpinBox->setMaximumWidth(80);


    connect(m_receiverPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {


        if (!m_listenBtn->isEnabled()) {
                // Block signals temporarily to prevent recursion
                m_receiverPortSpinBox->blockSignals(true);

                // Restore the previous value
                m_receiverPortSpinBox->setValue(static_cast<int>(Constants::bindReceiverPort));

                // Unblock signals
                m_receiverPortSpinBox->blockSignals(false);

                // Show warning message
                QMessageBox::warning(this,
                    "Cannot Change Port",
                    "Please stop listening before changing the UDP port.\n\n"
                    "Click 'Stop Listening' first, then change the port.");

                return;
            }
        // Stop both engines (safe to call both)
        m_engine->stopListening();
        udpEngine->stopListening();
        Constants::bindReceiverPort = static_cast<quint16>(value);
    });


    receiverProtocolLayout->addWidget(m_receiverPortSpinBox);
    receiverProtocolLayout->addStretch();
    receiveLayout->addLayout(receiverProtocolLayout);

    // Control buttons
    QHBoxLayout* ctrlLayout = new QHBoxLayout();
    m_listenBtn = new QPushButton("Start Listening");
    m_listenBtn->setMinimumHeight(35);
    connect(m_listenBtn, &QPushButton::clicked, this, &PingDialog::onStartListening);
    ctrlLayout->addWidget(m_listenBtn);
    
    m_stopBtn = new QPushButton("Stop Listening");
    m_stopBtn->setMinimumHeight(35);
    m_stopBtn->setEnabled(false);
    connect(m_stopBtn, &QPushButton::clicked, this, &PingDialog::onStopListening);
    ctrlLayout->addWidget(m_stopBtn);
    
    m_clearLogBtn = new QPushButton("Clear Log");
    connect(m_clearLogBtn, &QPushButton::clicked, this, &PingDialog::onClearLog);
    ctrlLayout->addWidget(m_clearLogBtn);



    // NEW: Blocked IPs button
    m_blockedIpBtn = new QPushButton(" Blocked IPs");
    connect(m_blockedIpBtn, &QPushButton::clicked, this, &PingDialog::onBlockedIpSettings);
    ctrlLayout->addWidget(m_blockedIpBtn);

    // NEW: Clear All button
    m_clearAllBtn = new QPushButton("Reset All");
    connect(m_clearAllBtn, &QPushButton::clicked, this, &PingDialog::onClearAll);
    ctrlLayout->addWidget(m_clearAllBtn);

    QPushButton* clearTempButton = new QPushButton("Clear Temp Folder");
    connect(clearTempButton, &QPushButton::clicked, this, [this]() {
        // Warn user
        auto reply = QMessageBox::question(this,
                        "Clear Temp Folder",
                        "This will permanently delete all received files in the temp folder. Continue?",
                        QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes)
            return;

        QDir tempDir(Constants::ermistmpPath);
        if (!tempDir.exists()) {
            QMessageBox::information(this, "Info", "Temp folder does not exist.");
            return;
        }

        QStringList files = tempDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        int deletedCount = 0;

        for (const QString& filename : files) {
            QString filepath = tempDir.filePath(filename);
            if (QFile::remove(filepath)) {
                deletedCount++;
            } else {
            }
        }

        QMessageBox::information(this, "Done",
                                 QString("Deleted %1 files from temp folder").arg(deletedCount));

        // Optionally, refresh the list widget
        populateListWidgetFromTmp();
    });
    ctrlLayout->addWidget(clearTempButton);

    ctrlLayout->addStretch();
    receiveLayout->addLayout(ctrlLayout);
    
    // Status
    m_listenStatusLabel = new QLabel("Not listening");
    m_listenStatusLabel->setStyleSheet("color: gray;");
    receiveLayout->addWidget(m_listenStatusLabel);
    

    // ===== ADD TEXT DISPLAY SECTION =====
    QGroupBox* textDisplayGroup = new QGroupBox("Received Text");
    QVBoxLayout* textDisplayLayout = new QVBoxLayout(textDisplayGroup);

    m_receivedTextEdit = new QPlainTextEdit();
    m_receivedTextEdit->setReadOnly(true);
    m_receivedTextEdit->setPlaceholderText("Received plain text will appear here...");
    m_receivedTextEdit->setMinimumHeight(100);
    textDisplayLayout->addWidget(m_receivedTextEdit);

    QHBoxLayout* textButtonsLayout = new QHBoxLayout();
    QPushButton* clearTextBtn = new QPushButton("Clear Text");
    QPushButton* copyTextBtn = new QPushButton("Copy to Clipboard");

    connect(clearTextBtn, &QPushButton::clicked, [this]() {
        m_receivedTextEdit->clear();
        addToLog(" Received text display cleared");
    });

    connect(copyTextBtn, &QPushButton::clicked, [this]() {
        QString text = m_receivedTextEdit->toPlainText();
        if (!text.isEmpty()) {
            QApplication::clipboard()->setText(text);
            addToLog(" Copied received text to clipboard");
            QMessageBox::information(this, "Copied", "Text copied to clipboard.");
        } else {
            QMessageBox::information(this, "Nothing to copy", "No text to copy.");
        }
    });

    textButtonsLayout->addWidget(clearTextBtn);
    textButtonsLayout->addWidget(copyTextBtn);
    textButtonsLayout->addStretch();
    textDisplayLayout->addLayout(textButtonsLayout);

    receiveLayout->addWidget(textDisplayGroup);
    // ===== END TEXT DISPLAY SECTION =====


    // Received files list
    QGroupBox* filesGroup = new QGroupBox("Received Files");
    QVBoxLayout* filesLayout = new QVBoxLayout(filesGroup);
    m_receivedFilesList = new QListWidget();
    filesLayout->addWidget(m_receivedFilesList);
    
    QHBoxLayout* fileActionsLayout = new QHBoxLayout();
    m_saveSelectedBtn = new QPushButton("Save Selected As...");
    connect(m_saveSelectedBtn, &QPushButton::clicked, this, &PingDialog::onSaveReceivedFile);
    fileActionsLayout->addWidget(m_saveSelectedBtn);
    filesLayout->addLayout(fileActionsLayout);
    receiveLayout->addWidget(filesGroup);
    
    fileActionsLayout->addStretch();
    filesLayout->addLayout(fileActionsLayout);

    // Log display
    QGroupBox* logGroup = new QGroupBox("Event Log");
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    m_logDisplay = new QPlainTextEdit();
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setMaximumBlockCount(MAX_LOG_ENTRIES);
    logLayout->addWidget(m_logDisplay);
    receiveLayout->addWidget(logGroup);
    
    // Add tabs
    m_tabWidget->addTab(m_sendTab, "Send Data");
    m_tabWidget->addTab(m_receiveTab, "Receive Data");
    
    connect(m_targetIpEdit, &QLineEdit::textChanged, this, &PingDialog::updateSendButtonState);
    connect(m_textInput, &QPlainTextEdit::textChanged, this, &PingDialog::updateSendButtonState);
    connect(m_filePathEdit, &QLineEdit::textChanged, this, &PingDialog::updateSendButtonState);
    connect(m_textModeRadio, &QRadioButton::toggled, this, &PingDialog::updateSendButtonState);
    connect(m_fileModeRadio, &QRadioButton::toggled, this, &PingDialog::updateSendButtonState);
    connect(m_receivedFilesList, &QListWidget::itemSelectionChanged, [this]() {
        m_saveSelectedBtn->setEnabled(m_receivedFilesList->currentItem() != nullptr);
    });
    connect(m_encryptCheckBox, &QCheckBox::toggled, this, &PingDialog::onEncryptToggled);

    mainLayout->addWidget(m_tabWidget);
}

// ===== SEND TAB SLOTS =====

void PingDialog::onSendText()
{
    QString text = m_textInput->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter text to send");
        return;
    }

    QString target = m_targetIpEdit->text();
    if (target.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter target IP address");
        return;
    }

    QByteArray data = text.toUtf8();
    data.prepend("TEXT");  // ← ALWAYS add marker first

    if (m_encryptCheckBox->isChecked()) {
        // Encryption will wrap "TEXT" + data
        // No need to modify data further
    }

    // Save to history
    QSettings settings;
    QStringList targets = settings.value("ping/targetHistory").toStringList();
    targets.removeAll(target);
    targets.prepend(target);
    while (targets.size() > 10) targets.removeLast();
    settings.setValue("ping/targetHistory", targets);

    addToLog(QString(" Sending %1text to %2 (%3 bytes)")
             .arg(m_encryptCheckBox->isChecked() ? "encrypted " : "")
             .arg(target).arg(data.size()));

    m_sendProgress->setVisible(true);
    m_sendProgress->setValue(0);
    m_sendStatusLabel->setText("Sending...");
    m_sendBtn->setEnabled(false);

    if (Constants::currentProtocol == Constants::UDP) {
        udpEngine->sendData(target, data, Constants::bindPort);
    } else if(Constants::currentProtocol == Constants::ICMP){
        m_engine->sendData(target, data);
    }else if(Constants::currentProtocol == Constants::DNS){
        dnsEngine->sendData(target, data, Constants::bindPort);

    }else{
        httpEngine->sendData(target, data, Constants::bindPort);

    }

    //m_engine->sendData(target, data);
}

void PingDialog::onSendFile()
{
    QString filePath = m_filePathEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a file to send");
        return;
    }
    
    QString target = m_targetIpEdit->text();
    if (target.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter target IP address");
        return;
    }
    
    QFileInfo info(filePath);
    addToLog(QString(" Sending file to %1: %2 (%3 bytes)")
             .arg(target).arg(info.fileName()).arg(info.size()));
    
    m_sendProgress->setVisible(true);
    m_sendProgress->setValue(0);
    m_sendStatusLabel->setText("Sending file...");
    m_sendBtn->setEnabled(false);
    m_cancelling = false;



    if (Constants::currentProtocol == Constants::UDP) {
            udpEngine->sendFile(target, filePath, Constants::bindPort);
            m_cancelSendBtn->setEnabled(true);

        } else if (Constants::currentProtocol == Constants::ICMP) {
            m_engine->sendFile(target, filePath);
            m_cancelSendBtn->setEnabled(true);

        }else if(Constants::currentProtocol == Constants::DNS){
        dnsEngine->sendFile(target, filePath, Constants::bindPort);
        m_cancelSendBtn->setEnabled(true);
    }else{
        httpEngine->sendFile(target, filePath, Constants::bindPort);
        m_cancelSendBtn->setEnabled(true);
    }

    //m_engine->sendFile(target, filePath);
}

void PingDialog::onBrowseFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
        "Select File to Send", QDir::homePath());
    if (!filePath.isEmpty()) {
        m_filePathEdit->setText(filePath);
        updateSendButtonState();  // Update button state after file selection
    }
}


void PingDialog::onStartListening()
{
#ifdef FLATPAK_BUILD
    if (Constants::currentReceiverProtocol == Constants::ICMP) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("ICMP Protocol Warning");
        msgBox.setText(
            "ICMP (ping) protocol requires access to raw sockets.\n\n"
            "Due to Flatpak sandbox restrictions, raw socket access is difficult to grant.\n\n"
            "For better compatibility in Flatpak, you can safely use:\n"
            "  • UDP protocol\n"
            "  • DNS protocol\n"
            "  • HTTP/HTTPS steganography protocols\n\n"
            "These protocols work without special permissions.\n\n"
            "Do you want to continue with ICMP anyway?"
        );
        msgBox.setIcon(QMessageBox::Warning);

        QPushButton *continueButton = msgBox.addButton("Continue Anyway", QMessageBox::AcceptRole);
        QPushButton *cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
        msgBox.setDefaultButton(cancelButton);

        msgBox.exec();

        if (msgBox.clickedButton() != continueButton) {
            return; // User cancelled
        }
    }
#endif

    bool success = false;

    if (Constants::currentReceiverProtocol == Constants::UDP) {
        success = udpEngine->startListening(Constants::bindReceiverPort);
    } else if(Constants::currentReceiverProtocol == Constants::ICMP) {
        success = m_engine->startListening();
    }else if (Constants::currentReceiverProtocol == Constants::DNS){
        success = dnsEngine->startListening(Constants::bindReceiverPort);

    }else{
        success = httpEngine->startListening(Constants::bindReceiverPort);

    }

    if (success) {
        addToLog(" Started listening");
        m_listenBtn->setEnabled(false);
        m_stopBtn->setEnabled(true);
        m_listenStatusLabel->setText("Listening...");
        m_listenStatusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        QMessageBox::warning(this, "Error", "Failed to start listening");
    }
}


void PingDialog::onStopListening()
{
    m_engine->stopListening();
    udpEngine->stopListening();
    dnsEngine->stopListening();
    httpEngine->stopListening();
    addToLog(" Stopped listening");
    m_listenBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_listenStatusLabel->setText("Not listening");
    m_listenStatusLabel->setStyleSheet("color: gray;");
}

void PingDialog::onClearLog()
{
    m_logDisplay->clear();
}

void PingDialog::onSendProgress(int packet, int total)
{
    if (m_cancelling) return;

    int percent = (packet * 100) / total;
    m_sendProgress->setValue(percent);
    m_sendStatusLabel->setText(QString("Sending: %1/%2 packets (%3%)")
                               .arg(packet).arg(total).arg(percent));
    
    if (packet == total) {
        m_sendProgress->setVisible(false);
        m_sendStatusLabel->setText("Send complete!");
        m_sendBtn->setEnabled(true);
        addToLog("✅ Send complete");
        m_cancelSendBtn->setEnabled(false);
    }
}

void PingDialog::onReceiveProgress(int packet, int total)
{
    m_listenStatusLabel->setText(QString("Receiving: %1/%2 packets")
                                 .arg(packet + 1).arg(total));
}

void PingDialog::onError(const QString& message)
{
    addToLog("❌ Error: " + message, true);
    
    m_sendProgress->setVisible(false);
    m_sendStatusLabel->setText("Send failed");
    m_sendBtn->setEnabled(true);
}

// ===== UTILITY METHODS =====

void PingDialog::addToLog(const QString& msg, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formatted = QString("[%1] %2").arg(timestamp).arg(msg);
    
    m_logDisplay->appendPlainText(formatted);
    
    if (isError) {
        // Flash error state
        m_logDisplay->setStyleSheet("color: red;");
        QTimer::singleShot(1000, [this]() { m_logDisplay->setStyleSheet(""); });
    }
}

void PingDialog::updateButtonStates()
{
    m_engine->stopListening();
    udpEngine->stopListening();
    // Initial button states
    m_stopBtn->setEnabled(false);
    m_sendBtn->setEnabled(false);
    m_saveSelectedBtn->setEnabled(false);
}


void PingDialog::updateSendButtonState()
{
    // Guard against null pointers
    if (!m_targetIpEdit || !m_textModeRadio || !m_fileModeRadio ||
        !m_textInput || !m_filePathEdit || !m_browseBtn || !m_sendBtn) {
        return;
    }

    bool hasTarget = !m_targetIpEdit->text().isEmpty();

    // Exclusive mode - only one input type enabled
    if (m_textModeRadio->isChecked()) {
        // Text mode: text input enabled, file input disabled
        if (m_textInput) m_textInput->setEnabled(true);
        if (m_filePathEdit) m_filePathEdit->setEnabled(false);
        if (m_browseBtn) m_browseBtn->setEnabled(false);

        // Button enabled only if text is not empty
        if (m_sendBtn) {
            m_sendBtn->setEnabled(hasTarget && m_textInput &&
                                 !m_textInput->toPlainText().isEmpty());
        }
    } else {
        // File mode: file input enabled, text input disabled
        if (m_textInput) m_textInput->setEnabled(false);
        if (m_filePathEdit) m_filePathEdit->setEnabled(true);
        if (m_browseBtn) m_browseBtn->setEnabled(true);

        // Button enabled only if file path is not empty
        if (m_sendBtn) {
            m_sendBtn->setEnabled(hasTarget && m_filePathEdit &&
                                 !m_filePathEdit->text().isEmpty());
        }
    }
}


void PingDialog::onClearAll()
{
    // --- BLOCK UI SIGNALS ---
    if (m_receivedFilesList)
        m_receivedFilesList->blockSignals(true);

    // --- FORCE RESET ALL NETWORK ENGINES ---
    m_engine->forceReset();
    udpEngine->forceReset();
    dnsEngine->forceReset();
    httpEngine->forceReset();
    // --- RESET BUTTON STATES TO "NOT LISTENING" ---
    m_listenBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_listenStatusLabel->setText("Not listening");
    m_listenStatusLabel->setStyleSheet("color: gray;");

    // Reset send button states
    m_sendBtn->setEnabled(false);
    m_cancelSendBtn->setEnabled(false);
    m_cancelling = false;

    // --- CLEAR UI ELEMENTS ---
    if (m_logDisplay) {
        m_logDisplay->clear();
    }

    if (m_receivedFilesList) {
        m_receivedFilesList->clear();
    }

    if (m_saveSelectedBtn) {
        m_saveSelectedBtn->setEnabled(false);
    }

    if (m_sendProgress) {
        m_sendProgress->reset();
        m_sendProgress->setVisible(false);
    }

    if (m_sendStatusLabel) {
        m_sendStatusLabel->clear();
    }

    if (m_receivedTextEdit) {
        m_receivedTextEdit->clear();
    }

    if (m_textInput) {
        m_textInput->clear();
    }

    // --- CLEAR UNDERLYING DATA ---
    m_lastReceivedData.clear();
    m_lastReceivedSource.clear();

    // --- UNBLOCK SIGNALS ---
    if (m_receivedFilesList)
        m_receivedFilesList->blockSignals(false);

    // --- UPDATE BUTTON STATES ---
    updateSendButtonState();

    if (m_logDisplay)
        addToLog(" All received data cleared and network engines reset");
}



void PingDialog::onSaveReceivedFile()
{
    QListWidgetItem* current = m_receivedFilesList->currentItem();
    if (!current) {
        QMessageBox::warning(this, "Error", "Please select a file to save");
        return;
    }

    // Check if it's a text message (using TextRole)
    QByteArray textData = current->data(Qt::UserRole + 3).toByteArray();
    if (!textData.isEmpty()) {
        // It's a text message
        QString defaultName = QString("received_text_%1.txt")
                             .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

        QString fileName = QFileDialog::getSaveFileName(this,
            "Save Text Message",
            Constants::receivedICMPPath + "/" + defaultName,
            "Text Files (*.txt);;All Files (*)");

        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(textData);
                file.close();
                addToLog(" Saved text to: " + fileName);
            } else {
                QMessageBox::warning(this, "Error", "Could not save file");
            }
        }
        return;
    }

    // It's a file (existing code)
    QString sourcePath = current->data(Qt::UserRole).toString();
    QString originalFilename = current->data(Qt::UserRole + 2).toString();
    QString sourceIp = current->data(Qt::UserRole + 1).toString();



    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Received File As",
        Constants::receivedICMPPath + "/" + originalFilename,
        "All Files (*)");

    if (!fileName.isEmpty()) {
        if (QFile::copy(sourcePath, fileName)) {
            addToLog(QString(" Saved file from %1 to: %2").arg(sourceIp).arg(fileName));
            if (QFile::remove(sourcePath)) {
                            // Remove from list since file is gone
                            delete current;
            }

        } else {
            QMessageBox::warning(this, "Error", "Could not save file");
        }
    }
}

void PingDialog::onFileReceived(const QString& path, const QString& source)
{
    // Clear text data since we received a file
    m_lastReceivedData.clear();

    QFileInfo info(path);


    if (info.fileName().startsWith("text_")) {
           QFile textFile(path);
           if (textFile.open(QIODevice::ReadOnly)) {
               QByteArray content = textFile.readAll();
               m_receivedTextEdit->setPlainText(QString::fromUtf8(content));
               textFile.close();
           }
    }


    addToLog(QString(" File received from %1: %2 (%3 bytes)")
             .arg(source).arg(info.fileName()).arg(info.size()));

    // Create display text with IP
    QString displayText = QString("%1 (from %2)")
                         .arg(info.fileName())
                         .arg(source);

    // Add to list with filename and IP
    QListWidgetItem* item = new QListWidgetItem(displayText);
    item->setData(Qt::UserRole, path);              // Store full path for saving
    item->setData(Qt::UserRole + 1, source);        // Store sender IP
    item->setData(Qt::UserRole + 2, info.fileName()); // Store original filename
    //item->setIcon(QIcon(":/icons/file.png"));
    m_receivedFilesList->addItem(item);

    // Enable the save button
    m_saveSelectedBtn->setEnabled(true);

    // Auto-show receive tab
    m_tabWidget->setCurrentIndex(1);
}


void PingDialog::onEncryptToggled(bool checked)
{
    if(Constants::currentProtocol == Constants::HTTP) { return; }  // we moved to sslsocket

    if (checked) {
        PassphraseDialog dialog(true, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString passphrase = dialog.getPassphrase();
            //m_engine->setEncryptionKey(passphrase);

        if (Constants::currentProtocol == Constants::UDP) {
                udpEngine->setEncryptionKey(passphrase);
        } else if(Constants::currentProtocol == Constants::ICMP) {
                m_engine->setEncryptionKey(passphrase);
        }else if (Constants::currentProtocol == Constants::DNS){
            dnsEngine->setEncryptionKey(passphrase);

        }else{
            httpEngine->setEncryptionKey(passphrase);

        }

            if (dialog.rememberPassphrase()) {
                // Store in dialog if you want, but engine has it
                m_rememberedPassphrase = passphrase;
                m_hasRememberedPassphrase = true;
            }
            addToLog(" Encryption enabled");
        } else {
            // User cancelled - uncheck the box
            m_encryptCheckBox->setChecked(false);
        }
    } else {
        m_engine->clearEncryption();
        udpEngine->clearEncryption();
        dnsEngine->clearEncryption();
        httpEngine->clearEncryption();
        addToLog(" Encryption disabled");
    }
}


void PingDialog::onDataReceived(const QByteArray& data, const QString& source)
{
    QByteArray payload = data;

    // ===== HANDLE ENCRYPTION =====
    if (data.startsWith("ENCR")) {
        addToLog(" Encrypted data received");

        PassphraseDialog dialog(false, this);
        if (dialog.exec() != QDialog::Accepted) {
            addToLog(" Decryption cancelled");
            return;
        }

        StegEngine crypto;
        payload = crypto.decryptData(data.mid(4), dialog.getPassphrase());

        if (payload.isEmpty()) {
            addToLog("❌ Decryption failed");
            QMessageBox::warning(this, "Error", "Decryption failed");
            return;
        }

        addToLog("✅ Decryption successful");

        // ===== SAVE DECRYPTED TEXT TO TEMP =====
        if (payload.startsWith("TEXT")) {
            QByteArray text = payload.mid(4);
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            QString savePath = Constants::ermistmpPath + "/" +
            QString("text_%1.txt   (%3)").arg(timestamp).arg(source);
            m_receivedTextEdit->setPlainText(QString::fromUtf8(text));  // Use 'text', not 'payload'
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(text);
                file.close();
                onFileReceived(savePath, source);

                return;
            }
        }
        // ===== END ADD
    }
    // ===== TYPE HANDLING =====
    if (payload.startsWith("FILE")) {

        QByteArray filePayload = payload.mid(4);

        QDataStream ds(filePayload);
        ds.setVersion(QDataStream::Qt_6_0);

        QString filename;
        qint64 fileSize;
        QByteArray hash;

        ds >> filename >> fileSize >> hash;

        if (ds.status() != QDataStream::Ok) {
            addToLog("❌ Corrupted file header");
            return;
        }

        QByteArray fileData = filePayload.mid(ds.device()->pos(), fileSize);

        QByteArray computedHash = QCryptographicHash::hash(fileData, QCryptographicHash::Sha256);

        if (computedHash != hash) {
            addToLog("❌ File hash mismatch");
            return;
        }

        QString savePath = Constants::ermistmpPath + "/" +
        QString("ermis_%1_%2   (%3)")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"))
        .arg(filename.arg(source));


        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileData);
            file.close();

            addToLog(QString(" File received: %1 (%2 bytes)")
                     .arg(filename).arg(fileSize));

            onFileReceived(savePath, source);
        }

        return;
    }

    else if (payload.startsWith("TEXT")) {

        QByteArray text = payload.mid(4);

        m_lastReceivedData = text;
        m_lastReceivedSource = source;

        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

        QListWidgetItem* item = new QListWidgetItem(
            QString("text_%1 (from %2)").arg(timestamp).arg(source)
        );

        item->setData(Qt::UserRole + 3, text);
        item->setData(Qt::UserRole + 1, source);
        m_receivedFilesList->addItem(item);
        m_receivedTextEdit->setPlainText(QString::fromUtf8(text));

        addToLog(" Text received: " + QString::fromUtf8(text).left(200));
        return;
    }
    // ===== FALLBACK =====
    addToLog("⚠️ Unknown format, treating as raw text");

    m_lastReceivedData = payload;
}



void PingDialog::populateListWidgetFromTmp()
{
    QDir tmpDir(Constants::ermistmpPath);
    if (!tmpDir.exists()) {
        return;
    }

    // Clear current UI and internal containers first
    m_receivedFilesList->clear();
    m_receivedFilesList->clear();

    QStringList files = tmpDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& fileName : files) {
        QString filePath = tmpDir.filePath(fileName);

        // Add item to QListWidget
        QListWidgetItem* item = new QListWidgetItem(fileName);
        item->setData(Qt::UserRole, filePath);       // store full path
        item->setData(Qt::UserRole + 2, fileName);   // store original filename
        //item->setIcon(QIcon(":/icons/file.png"));
        m_receivedFilesList->addItem(item);



    }

    // Enable the save button if there are items
    m_saveSelectedBtn->setEnabled(!files.isEmpty());

    // Optionally switch to receive tab
    if (!files.isEmpty()) {
        //m_tabWidget->setCurrentIndex(1);
    }

}


void PingDialog::onBlockedIpSettings()
{

    IpFilterDialog dlg(ipFilter, this);
    if (dlg.exec() == QDialog::Accepted) {
        // Update constants from the filter object
        Constants::ipFilterEnabled = ipFilter->isEnabled();
        Constants::ipFilterMode = (ipFilter->mode() == IpFilter::BlockMode) ? 0 : 1;
        Constants::ipFilterEntries = ipFilter->getEntries();
    }
}

void PingDialog::onCancelSend()
{
    m_cancelling = true;
    m_engine->cancelCurrentTransfer();
    udpEngine->cancelCurrentTransfer();
    dnsEngine->cancelCurrentTransfer();
    httpEngine->cancelCurrentTransfer();
    addToLog("❌ Send cancelled by user");
    m_sendBtn->setEnabled(true);
    m_cancelSendBtn->setEnabled(false);
    m_sendProgress->setVisible(false);
    m_sendStatusLabel->setText("Cancelled");
}

void PingDialog::onProtocolChanged(int index)
{
    // Save the new protocol
    //Constants::currentProtocol = (index == 0) ? Constants::ICMP : Constants::UDP;
    if (index == 0) {
        Constants::currentProtocol = Constants::ICMP;
    } else if (index == 1) {
        Constants::currentProtocol = Constants::UDP;
    } else if (index == 2) {
        Constants::currentProtocol = Constants::DNS;
    }else{
       Constants::currentProtocol = Constants::HTTP;
    }
    // Update UI
    m_portSpinBox->setEnabled(Constants::currentProtocol == Constants::UDP || Constants::currentProtocol == Constants::DNS
                              || Constants::currentProtocol == Constants::HTTP);

    // Check if currently listening
    bool wasListening = (!m_listenBtn->isEnabled() && m_stopBtn->isEnabled());

    if (wasListening) {
        addToLog(" Protocol switched while listening - restarting...");

        // Stop both engines (safe to call both)
        m_engine->stopListening();
        udpEngine->stopListening();
        dnsEngine->stopListening();
        httpEngine->stopListening();
        // Update UI to show stopped state
        m_listenBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
        m_listenStatusLabel->setText("Not listening");
        m_listenStatusLabel->setStyleSheet("color: gray;");

        // Restart listening with new protocol
        bool success = false;
        if (Constants::currentProtocol == Constants::UDP) {
            success = udpEngine->startListening(Constants::bindPort);
        } else if(Constants::currentProtocol == Constants::ICMP)  {
            success = m_engine->startListening();
        }else if(Constants::currentProtocol == Constants::DNS){
            success = dnsEngine->startListening(Constants::bindPort);

        }else{
            success = httpEngine->startListening(Constants::bindPort);

        }

        if (success) {

            addToLog(QString("Restarted listening on %1")
                     .arg(Constants::currentProtocol == Constants::UDP ? "UDP" :
                          Constants::currentProtocol == Constants::DNS ? "DNS" :
                          Constants::currentProtocol == Constants::HTTP ? "HTTP" : "ICMP"));

            m_listenBtn->setEnabled(false);
            m_stopBtn->setEnabled(true);
            m_listenStatusLabel->setText("Listening...");
            m_listenStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        } else {
            addToLog("❌ Failed to restart listening after protocol switch", true);
            QMessageBox::warning(this, "Error",
                "Failed to restart listening after protocol switch.\n"
                "Please start listening manually.");
        }
    } else {
        addToLog(QString("Sender's Protocol changed to %1")
                 .arg(Constants::currentProtocol == Constants::UDP ? "UDP" :
                      Constants::currentProtocol == Constants::DNS ? "DNS" :
                      Constants::currentProtocol == Constants::HTTP ? "HTTP" : "ICMP"));
    }
}


void PingDialog::onReceiverProtocolChanged(int index)
{
    // Save the new protocol
   // Constants::currentReceiverProtocol = (index == 0) ? Constants::ICMP : Constants::UDP;

    if (index == 0) {
        Constants::currentReceiverProtocol = Constants::ICMP;
    } else if (index == 1) {
        Constants::currentReceiverProtocol = Constants::UDP;
    } else if (index == 2) {
        Constants::currentReceiverProtocol = Constants::DNS;
    }else {
       Constants::currentReceiverProtocol = Constants::HTTP;
    }

    // Update UI
    m_receiverPortSpinBox->setEnabled(Constants::currentReceiverProtocol == Constants::UDP ||
                                      Constants::currentReceiverProtocol == Constants::DNS ||
                                      Constants::currentReceiverProtocol == Constants::HTTP);

    // Check if currently listening
    bool wasListening = (!m_listenBtn->isEnabled() && m_stopBtn->isEnabled());

    if (wasListening) {
        addToLog(" Protocol switched while listening - restarting...");

        // Stop both engines (safe to call both)
        m_engine->stopListening();
        udpEngine->stopListening();
        dnsEngine->stopListening();
        httpEngine->stopListening();
        // Update UI to show stopped state
        m_listenBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
        m_listenStatusLabel->setText("Not listening");
        m_listenStatusLabel->setStyleSheet("color: gray;");

        // Restart listening with new protocol
        bool success = false;
        if (Constants::currentReceiverProtocol == Constants::UDP) {
            success = udpEngine->startListening(Constants::bindReceiverPort);
        } else if(Constants::currentReceiverProtocol == Constants::ICMP) {
            success = m_engine->startListening();
        }else if (Constants::currentReceiverProtocol == Constants::DNS){
            success = dnsEngine->startListening(Constants::bindReceiverPort);

        }else{
            success = httpEngine->startListening(Constants::bindReceiverPort);

        }

        if (success) {

            addToLog(QString("Restarted listening on %1")
                     .arg(Constants::currentReceiverProtocol == Constants::UDP ? "UDP" :
                          Constants::currentReceiverProtocol == Constants::DNS ? "DNS" :
                          Constants::currentReceiverProtocol == Constants::HTTP ? "HTTP" : "ICMP"));

            m_listenBtn->setEnabled(false);
            m_stopBtn->setEnabled(true);
            m_listenStatusLabel->setText("Listening...");
            m_listenStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        } else {
            addToLog("❌ Failed to restart listening after protocol switch", true);
            QMessageBox::warning(this, "Error",
                "Failed to restart listening after protocol switch.\n"
                "Please start listening manually.");
        }
    } else {
        addToLog(QString("Receiver's Protocol changed to %1")
                 .arg(Constants::currentReceiverProtocol == Constants::UDP ? "UDP" :
                      Constants::currentReceiverProtocol == Constants::DNS ? "DNS" :
                      Constants::currentReceiverProtocol == Constants::HTTP ? "HTTP" : "ICMP"));
    }
}
