
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QStatusBar>
#include<QDateTime>
#include<QDir>
#include<QStandardPaths>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QThread>
#include<QProgressDialog>
#include"constants.h"
#include<QMediaDevices>
#include<QMediaCaptureSession>
#include<QCameraDevice>
#include<QList>
#include<QClipboard>
#include<QApplication>
#include<QUrl>
#include<QRegularExpression>
#include<QSplitter>
#include"helpmenudialog.h"
#include"donationdialog.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , engine(new StegEngine(this))
    , ui(new Ui::MainWindow)
    , isTextInput(true)
    , hasRememberedPassphrase(false)
    , audioPlayer(new AudioPlayer(this))
    , audioEngine(new AudioStegEngine(this))
    , ffmpeg(new FFmpegHandler(this))
{
    setAcceptDrops(true);  // ENABLE DRAG & DROP
    ui->setupUi(this);
    setupUi();
    createMenus();
    createConnections();

    setWindowTitle("Ermis - Steganography");
    setMinimumSize(900, 700);
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setupUi()
{
    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    tabWidget = new QTabWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(tabWidget);

    // ========== HIDE TAB ==========
    hideTab = new QWidget(tabWidget);
    tabWidget->addTab(hideTab, "Hide Data");

    // Image preview area
    originalImageLabel = new QLabel("Original Image", hideTab);
    originalImageLabel->installEventFilter(this);
    originalImageLabel->setAlignment(Qt::AlignCenter);
    originalImageLabel->setMinimumSize(320, 240);
    originalImageLabel->setFrameShape(QFrame::Box);

    modifiedImageLabel = new QLabel("Modified Image", hideTab);
    modifiedImageLabel->installEventFilter(this);
    modifiedImageLabel->setAlignment(Qt::AlignCenter);
    modifiedImageLabel->setMinimumSize(320, 240);
    modifiedImageLabel->setFrameShape(QFrame::Box);

    //QHBoxLayout *previewLayout = new QHBoxLayout();
    //previewLayout->addWidget(originalImageLabel);
    //previewLayout->addWidget(modifiedImageLabel);

    QSplitter *imageSplitter = new QSplitter(Qt::Horizontal, hideTab);
    imageSplitter->setHandleWidth(3);
    imageSplitter->addWidget(originalImageLabel);
    imageSplitter->addWidget(modifiedImageLabel);
    imageSplitter->setSizes({320, 320});  // Equal initial sizes

    imageSplitter->setStyleSheet(
                "QSplitter::handle {"
                "    background-color: #e0c9a6;"  // Same beige-yellow
                "    width: 3px;"
                "}"
                "QSplitter::handle:hover {"
                "    background-color: #c9b386;"
                "}"
                );

    // Then in your topSectionWidget layout, add the splitter instead of previewLayout

    // Image file selection
    openImageButton = new QPushButton("Select Carrier Image", hideTab);
    saveImageButton = new QPushButton("Save Modified Image", hideTab);
    saveImageButton->setEnabled(false);

    QHBoxLayout *imageButtonsLayout = new QHBoxLayout();
    imageButtonsLayout->addWidget(openImageButton);
    imageButtonsLayout->addWidget(saveImageButton);

    // Create widget to hold preview and buttons (top section)
    QWidget *topSectionWidget = new QWidget(hideTab);
    QVBoxLayout *topSectionLayout = new QVBoxLayout(topSectionWidget);
    topSectionLayout->setContentsMargins(0, 0, 0, 0);
    //topSectionLayout->addLayout(previewLayout);
    topSectionLayout->addWidget(imageSplitter);

    topSectionLayout->addLayout(imageButtonsLayout);

    // Data input options
    QGroupBox *dataInputGroup = new QGroupBox("Data to Hide", hideTab);

    textInputRadio = new QRadioButton("Text Input", dataInputGroup);
    fileInputRadio = new QRadioButton("File Input", dataInputGroup);
    textInputRadio->setChecked(true);
    encryptionCheckBox = new QCheckBox("Encrypt data", dataInputGroup);

    QHBoxLayout *radioLayout = new QHBoxLayout();
    radioLayout->addWidget(textInputRadio);
    radioLayout->addWidget(fileInputRadio);
    radioLayout->addWidget(encryptionCheckBox);

    textInput = new QPlainTextEdit(dataInputGroup);
    textInput->setPlaceholderText("Enter text to hide...");

    filePathInput = new QLineEdit(dataInputGroup);
    filePathInput->setPlaceholderText("Select a file to hide...");
    filePathInput->setEnabled(false);

    browseFileButton = new QPushButton("Browse...", dataInputGroup);
    browseFileButton->setEnabled(false);

    QHBoxLayout *fileInputLayout = new QHBoxLayout();
    fileInputLayout->addWidget(filePathInput);
    fileInputLayout->addWidget(browseFileButton);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    hideButton = new QPushButton("Hide Data", dataInputGroup);
    resetButton = new QPushButton("Reset", dataInputGroup);
    buttonsLayout->addWidget(hideButton);
    buttonsLayout->addWidget(resetButton);
    hideButton->setEnabled(false);

    capacityLabel = new QLabel("Capacity: 0/0 bytes (0%)", dataInputGroup);

    QVBoxLayout *dataInputLayout = new QVBoxLayout(dataInputGroup);
    dataInputLayout->addLayout(radioLayout);
    dataInputLayout->addWidget(textInput);
    dataInputLayout->addLayout(fileInputLayout);
    dataInputLayout->addLayout(buttonsLayout);
    dataInputLayout->addWidget(capacityLabel);

    // Create splitter for Hide tab
    QSplitter *hideSplitter = new QSplitter(Qt::Vertical, hideTab);
    hideSplitter->setHandleWidth(3);  // Default is 3-4, make it 8-10
    hideSplitter->addWidget(topSectionWidget);     // Top: preview + buttons
    hideSplitter->addWidget(dataInputGroup);       // Bottom: data input group

    // Set initial sizes (60% top, 40% bottom)
    hideSplitter->setSizes({400, 250});

    hideSplitter->setStyleSheet(
                "QSplitter::handle {"
                "    background-color: #e0c9a6;;"  // Beige-yellow
                "    height: 3px;"
                "}"
                "QSplitter::handle:hover {"
                "    background-color: #c9b386;"  // Slightly darker on hover
                "}"
                );


    // Arrange Hide tab layout
    QVBoxLayout *hideTabLayout = new QVBoxLayout(hideTab);
    hideTabLayout->addWidget(hideSplitter);

    // ========== EXTRACT TAB ==========
    extractTab = new QWidget(tabWidget);
    tabWidget->addTab(extractTab, "Extract Data");

    // Stego image preview
    stegoImageLabel = new QLabel("Steganographic Image", extractTab);
    stegoImageLabel->installEventFilter(this);
    stegoImageLabel->setAlignment(Qt::AlignCenter);
    stegoImageLabel->setMinimumSize(320, 240);
    stegoImageLabel->setFrameShape(QFrame::Box);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    openStegoImageButton = new QPushButton("Open Image with Hidden Data", extractTab);
    cameraButton = new QPushButton("Start Camera", extractTab);
    clipboardButton = new QPushButton("Clipboard Copy", extractTab);
    clipboardButton->setToolTip("Copy extracted text to clipboard");
    buttonLayout->addWidget(openStegoImageButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(clipboardButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cameraButton);

    // Create widget for top section (image + buttons)
    QWidget *extractTopSectionWidget = new QWidget(extractTab);
    QVBoxLayout *extractTopSectionLayout = new QVBoxLayout(extractTopSectionWidget);
    extractTopSectionLayout->setContentsMargins(0, 0, 0, 0);
    extractTopSectionLayout->addWidget(stegoImageLabel);
    extractTopSectionLayout->addLayout(buttonLayout);

    // Extraction controls
    extractButton = new QPushButton("Extract Data", extractTab);
    extractButton->setEnabled(false);
    extractButton->setVisible(false);

    extractedTextOutput = new QPlainTextEdit(extractTab);
    extractedTextOutput->setPlaceholderText("Extracted text will appear here. If too long, please access it via the saved file.");
    extractedTextOutput->setReadOnly(true);

    saveExtractedFileButton = new QPushButton("Extract and Save Data to File", extractTab);
    saveExtractedFileButton->setEnabled(false);
    saveExtractedFileButton->setToolTip(tr("Extract data from the image and save it to a file"));

    // Create splitter for Extract tab
    QSplitter *extractSplitter = new QSplitter(Qt::Vertical, extractTab);
    extractSplitter->setHandleWidth(3);
    extractSplitter->addWidget(extractTopSectionWidget);  // Top: image + buttons
    extractSplitter->addWidget(extractedTextOutput);      // Middle: text output
    //extractSplitter->addWidget(saveExtractedFileButton);  // Bottom: save button

    // Set initial sizes (50% top, 30% middle, 20% bottom)
    extractSplitter->setSizes({400, 250});

    extractSplitter->setStyleSheet(
                "QSplitter::handle {"
                "    background-color: #e0c9a6;;"  // Beige-yellow
                "    height: 3px;"
                "}"
                "QSplitter::handle:hover {"
                "    background-color: #c9b386;"  // Slightly darker on hover
                "}"
                );
    // Arrange Extr exact tab layout
    QVBoxLayout *extractTabLayout = new QVBoxLayout(extractTab);
    extractTabLayout->addWidget(extractSplitter);
    extractTabLayout->addWidget(saveExtractedFileButton);
    mainLayout->addWidget(createAudioPlayerBar());
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *openAction = fileMenu->addAction("&Open Carrier Image...");
    connect(openAction, &QAction::triggered, this, &MainWindow::openCarrierImage);

    QAction *saveAction = fileMenu->addAction("&Save Modified Image...");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveModifiedImage);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *viewMenu = menuBar()->addMenu("&View");
    toggleAudioBarAction = viewMenu->addAction("Show Audio Bar");
    toggleAudioBarAction->setCheckable(true);
    toggleAudioBarAction->setChecked(false);
    connect(toggleAudioBarAction,  &QAction::triggered, this, [this](bool checked){
        isAudioBarVisible = checked;
        if (!checked) {
            if(audioPlayer && audioPlayer->isPlaying()) {
                audioPlayer->stop();
            }
        }
        m_audioToolbar->setVisible(checked);
    });

    QMenu *helpMenu = menuBar()->addMenu("&Help");

    ////////////////////////////////////////////////////
    QAction *aboutAction = helpMenu->addAction("About Ermis");
    connect(aboutAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::About, this);
        dialog.exec();
    });

    // Features
    QAction *featuresAction = helpMenu->addAction("Features");
    connect(featuresAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Features, this);
        dialog.exec();
    });

    // Instructions
    QAction *instructionsAction = helpMenu->addAction("Instructions");
    connect(instructionsAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Instructions, this);
        dialog.exec();
    });


    // whatsnew
    QAction *whatsNewAction = helpMenu->addAction("What's New");
    connect(whatsNewAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::onChangelog, this);
        dialog.exec();
    });

    QAction *supportusAction = helpMenu->addAction("Support Us");
        connect(supportusAction, &QAction::triggered, [this]() {
            DonationDialog dialog(this);
            dialog.exec();
        });
    //////////////////////////////////////////////////

    //prt checkbox
    prtCheckBox = new QCheckBox("PRT Mode", this);
    menuBar()->setCornerWidget(prtCheckBox, Qt::TopRightCorner);
    prtCheckBox->setToolTip("");
    prtCheckBox->setCheckState(Qt::Unchecked);
    prtCheckBox->setVisible(false);
    connect(prtCheckBox, &QCheckBox::toggled, [this](bool checked){
        isPRTMode = checked;

        if(isPRTMode) {
            encryptionCheckBox->setChecked(false);
            textInputRadio->setChecked(true);

            encryptionCheckBox->setEnabled(false);
            fileInputRadio->setEnabled(false);

        }else{
            fileInputRadio->setEnabled(true);
            encryptionCheckBox->setEnabled(true);

        }

    });

    // === ADD AUDIO STEG CHECKBOX ===
    audioStegCheckBox = new QCheckBox("AUDIO", this);
    // Need to add it next to PRT - we'll use a widget container
    QWidget *cornerWidget = new QWidget(this);
    QHBoxLayout *cornerLayout = new QHBoxLayout(cornerWidget);
    cornerLayout->setContentsMargins(0, 0, 10, 0);
    cornerLayout->setSpacing(10);
    cornerLayout->addWidget(audioStegCheckBox);
    cornerLayout->addWidget(prtCheckBox);

    menuBar()->setCornerWidget(cornerWidget, Qt::TopRightCorner);

    audioStegCheckBox->setToolTip("Switch to Audio Steganography mode");
    audioStegCheckBox->setCheckState(Qt::Unchecked);

    connect(audioStegCheckBox, &QCheckBox::toggled, this, &MainWindow::onAudioStegToggled);
}



    void MainWindow::createConnections()
    {
        // Carrier image selection
        //connect(openImageButton, &QPushButton::clicked, this, &MainWindow::openCarrierImage);
        connect(openImageButton, &QPushButton::clicked, [this]() {
            if (isAudioMode) {
                openAudioCarrier();
            } else {
                openCarrierImage();
            }
        });

        //connect(saveImageButton, &QPushButton::clicked, this, &MainWindow::saveModifiedImage);
        connect(saveImageButton, &QPushButton::clicked, [this]() {
            if (isAudioMode) {
                saveModifiedAudio();
            } else {
                saveModifiedImage();
            }
        });
        // Input method selection
        connect(textInputRadio, &QRadioButton::toggled, this, &MainWindow::toggleInputMethod);
        connect(fileInputRadio, &QRadioButton::toggled, this, &MainWindow::toggleInputMethod);
        connect(browseFileButton, &QPushButton::clicked, this, &MainWindow::selectDataFile);

        // Connect encryption checkbox to update the hide button text
        connect(encryptionCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
            hideButton->setText(checked ? "Hide Encrypted Data" : "Hide Data");
        });


        connect(hideButton, &QPushButton::clicked, [this]() {
            qDebug() << "=== HIDE BUTTON CLICKED ===";
            qDebug() << "isAudioMode:" << isAudioMode;
            qDebug() << "originalImage.isNull():" << originalImage.isNull();
            qDebug() << "isTextInput:" << isTextInput;

            if (isAudioMode) {
                hideDataInAudio();  // Call audio hide method
                return;
            }

            if (originalImage.isNull()) {
                QMessageBox::warning(this, "Error", "Please load a carrier image first");
                return;
            }

            QByteArray data;

            if (isTextInput) {
                QString text = textInput->toPlainText();

                if (text.isEmpty()) {
                    QMessageBox::warning(this, "Error", "Please enter text to hide");
                    return;
                }
                data = text.toUtf8();

                QByteArray textData;
                textData.append(static_cast<char>(0x00)); // 0 means it's pure text, not a file
                textData.append(data);
                data = textData;

            } else {
                QString filePath = filePathInput->text();
                QFile file(filePath);
                if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
                    QMessageBox::warning(this, "Error", "Could not open the selected file");
                    return;
                }

                QByteArray fileData = file.readAll();
                file.close();

                // Extract the base filename with extension
                QString fileName = QFileInfo(filePath).fileName(); // e.g., "document.txt"
                QByteArray fileNameBytes = fileName.toUtf8();

                // Prepend filename length and filename to the file data
                data.clear();  // ensure outer data is empty
                data.append(static_cast<char>(fileNameBytes.size())); // 1 byte for length (max 255)
                data.append(fileNameBytes);
                data.append(fileData);
            }

            // Check if encryption is enabled
            if (encryptionCheckBox->isChecked()) {
                PassphraseDialog dialog(true, this);
                if (dialog.exec() == QDialog::Accepted) {
                    QString passphrase = dialog.getPassphrase();

                    // Encrypt the data
                    QByteArray encryptedData = engine->encryptData(data, passphrase);
                    if (encryptedData.isEmpty()) {
                        QMessageBox::warning(this, "Error", "Encryption failed");
                        return;
                    }

                    // Prepend the ENCR marker
                    encryptedData.prepend("ENCR");

                    // Check if the encrypted data will fit
                    int capacity = engine->calculateImageCapacity(originalImage);
                    if (encryptedData.size() > capacity) {
                        QMessageBox::warning(this, "Error",
                        QString("Encrypted data size (%1 bytes) exceeds image capacity (%2 bytes)")
                        .arg(encryptedData.size()).arg(capacity));
                        return;
                    }

                    // Remember passphrase if requested
                    if (dialog.rememberPassphrase()) {
                        rememberedPassphrase = passphrase;
                        hasRememberedPassphrase = true;
                    }

                    // Use the encrypted data
                    data = encryptedData;
                } else {
                    return; // User canceled
                }
            }

            // Check if the data will fit
            int capacity = engine->calculateImageCapacity(originalImage);
            if (data.size() > capacity) {
                QMessageBox::warning(this, "Error",
                QString("Data size (%1 bytes) exceeds image capacity (%2 bytes)")
                .arg(data.size()).arg(capacity));
                return;
            }

            // Embed the data
            if (isPRTMode) {
                if (engine->embedPRTData(originalImage, modifiedImage, data)) {
                    originalModifiedImage = modifiedImage;
                    modifiedZoom = 1.0;
                    QPixmap pixmap = QPixmap::fromImage(modifiedImage);
                    pixmap = pixmap.scaled(modifiedImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    modifiedImageLabel->setPixmap(pixmap);
                    saveImageButton->setEnabled(true);
                    statusBar()->showMessage("PRT artistic code created successfully");
                } else {
                    QMessageBox::warning(this, "Error", "Failed to create PRT artistic code");
                }
            } else {
                if (engine->embedDataInImage(originalImage, modifiedImage, data)) {
                    originalModifiedImage = modifiedImage;
                    modifiedZoom = 1.0;
                    QPixmap pixmap = QPixmap::fromImage(modifiedImage);
                    pixmap = pixmap.scaled(modifiedImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    modifiedImageLabel->setPixmap(pixmap);
                    saveImageButton->setEnabled(true);
                    statusBar()->showMessage(encryptionCheckBox->isChecked() ?
                    "Encrypted data hidden successfully" :
                    "Data hidden successfully");
                } else {
                    QMessageBox::warning(this, "Error", "Failed to hide data in the image");
                }
            }
        });
        // Extract tab connections

        connect(openStegoImageButton, &QPushButton::clicked, [this]() {

            // === AUDIO MODE CHECK FIRST ===
            if (isAudioMode) {
                openAudioStegoFile();
                return;
            }




            QString fileName = QFileDialog::getOpenFileName(this, "Open Image with Hidden Data",
            Constants::fusedImagesPath,
            "Image Files (*.png *.jpg *.bmp)");
            if (fileName.isEmpty()) {
                return;
            }

            stegoImage.load(fileName);
            if (!stegoImage.isNull()) {
                originalStegoImage = stegoImage;
                stegoZoom = 1.0;
                // For detection only - check if it's PRT
                QByteArray detectionData = engine->extractDataFromImage(stegoImage);
                isPRTMode = (detectionData.size() >= 4 &&
                detectionData.startsWith("PRT") && detectionData[3] == 0x01);

                prtCheckBox->setChecked(isPRTMode);

                //
                QPixmap pixmap = QPixmap::fromImage(stegoImage);
                pixmap = pixmap.scaled(stegoImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                stegoImageLabel->setPixmap(pixmap);
                saveExtractedFileButton->setEnabled(true);
                //extractButton->setEnabled(true);
                statusBar()->showMessage("Image loaded successfully: " + fileName);
                currentToExtractPath = fileName;
            } else {
                statusBar()->showMessage("Failed to load image");
                QMessageBox::warning(this, "Error", "Failed to load the selected image");
            }
        });

        connect(saveExtractedFileButton, &QPushButton::clicked, [this]() {

            // === AUDIO MODE CHECK FIRST ===
            if (isAudioMode) {
                extractDataFromAudio();  // Call audio extraction method
                return;
            }

            if (stegoImage.isNull()) {
                QMessageBox::warning(this, "Error", "Please load an image with hidden data first");
                return;
            }

            // Extract raw data from the image
            QByteArray extractedData = isPRTMode
            ? engine->extractPRTData(stegoImage)
            : engine->extractDataFromImage(stegoImage);

            if (extractedData.isEmpty()) {
                QMessageBox::warning(this, "Error", "No hidden data found or data extraction failed");
                return;
            }

            // Check if data is encrypted
            if (!isPRTMode && extractedData.startsWith("ENCR")) {
                QByteArray decryptedData;
                bool passphraseUsed = false;

                // Try remembered passphrase first
                if (hasRememberedPassphrase) {
                    decryptedData = engine->decryptData(extractedData.mid(4), rememberedPassphrase);
                    if (!decryptedData.isEmpty()) passphraseUsed = true;
                }

                // If decryption failed, ask user
                while (!passphraseUsed) {
                    PassphraseDialog dialog(false, this);
                    if (dialog.exec() != QDialog::Accepted) return; // User canceled

                    QString passphrase = dialog.getPassphrase();
                    decryptedData = engine->decryptData(extractedData.mid(4), passphrase);
                    if (!decryptedData.isEmpty()) {
                        if (dialog.rememberPassphrase()) {
                            rememberedPassphrase = passphrase;
                            hasRememberedPassphrase = true;
                        }
                        break;
                    } else {
                        QMessageBox::warning(this, "Error", "Decryption failed. Check your passphrase.");
                    }
                }

                extractedData = decryptedData;
            }

            // --- Extract filename and content ---
            int fileNameLen = static_cast<unsigned char>(extractedData[0]); // first byte = filename length

            QByteArray fileContent;
            QString extractedFileName;

            if (fileNameLen == 0) {
                // It's pure text data
                extractedFileName = QString("extracted_text_%1.txt")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
                fileContent = extractedData.mid(1); // Skip the 0 marker
            } else {
                // It's a file with filename
                extractedFileName = QString::fromUtf8(extractedData.mid(1, fileNameLen));
                fileContent = extractedData.mid(1 + fileNameLen);
            }
            QRegularExpression invalidChars(R"([:,'"])");  // includes colon, comma, single and double quote
            // --- Simple fix: if filename contains ":", use default ---

            if (extractedFileName.contains(invalidChars)) {
                extractedFileName = QString("extracted_data_%1.txt")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            }

            // --- Prepare default save path ---
            QString defaultFilePath = QDir(Constants::extractedImagesPath).filePath(extractedFileName);

            // --- Ask user where to save ---
            QString savePath = QFileDialog::getSaveFileName(this, "Save Extracted File", defaultFilePath);

            // If user cancels, use default
            if (savePath.isEmpty()) savePath = defaultFilePath;

            // --- Write to disk ---
            QFile outFile(savePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                qint64 written = outFile.write(fileContent);
                outFile.close();

                if (written == fileContent.size()) {
                    statusBar()->showMessage("Extracted file saved as: " + savePath);
                    QMessageBox::information(this, "File Saved",
                    "Extracted file saved successfully:\n" + savePath);

                    // If the file is text, display it
                    QString text = QString::fromUtf8(fileContent);
                    bool isText = true;
                    for (const QChar &c : text) {
                        if (!c.isPrint() && !c.isSpace()) {
                            isText = false;
                            break;
                        }
                    }
                    if (isText) {
                        if (text.length() > 10000) {
                            extractedTextOutput->clear();
                            extractedTextOutput->setPlainText(
                            QString("Extracted text is too long to display.\n"
                            "Please access it by opening:\n%1")
                            .arg(savePath)
                            );
                        } else {
                            extractedTextOutput->setPlainText(text);
                        }
                    }

                } else {
                    QMessageBox::warning(this, "Error", "Could not write all data to disk");
                }
            } else {
                QMessageBox::warning(this, "Error", "Failed to open file for writing");
            }
        });

        // Add this connection to update capacity when text changes
        //connect(textInput, &QPlainTextEdit::textChanged, this, &MainWindow::updateCapacityStatus);

        // Add this connection to update capacity when file path changes
        //connect(filePathInput, &QLineEdit::textChanged, this, &MainWindow::updateCapacityStatus);

        connect(textInput, &QPlainTextEdit::textChanged, this, [this](){
            if (isAudioMode)
            updateAudioCapacity();
            else
            updateCapacityStatus();
        });

        connect(filePathInput, &QLineEdit::textChanged, this, [this](){
            if (isAudioMode)
            updateAudioCapacity();
            else
            updateCapacityStatus();
        });

        connect(cameraButton, &QPushButton::clicked, this, &MainWindow::onCameraButtonClicked);
        connect(clipboardButton, &QPushButton::clicked, [this] {
            QApplication::clipboard()->setText(extractedTextOutput->toPlainText());
            statusBar()->showMessage("📋 Copied to clipboard!");
        });
        connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetAll);

    }

    void MainWindow::openCarrierImage() {

        QString initialPath = Constants::picturesPath;

        if (!QDir(initialPath).exists()) {
            initialPath = Constants::imagesPath;
        }

        QString fileName = QFileDialog::getOpenFileName(this, "Open Carrier Image",
        initialPath,
        "Image Files (*.png *.jpg *.bmp)");
        if (!fileName.isEmpty()) {
            originalImage.load(fileName);
            if (!originalImage.isNull()) {
                // Convert to a format that supports alpha channel if needed
                if (originalImage.format() != QImage::Format_ARGB32 &&
                originalImage.format() != QImage::Format_RGB32) {
                    originalImage = originalImage.convertToFormat(QImage::Format_ARGB32);
                }

                originalCarrierImage = originalImage;
                carrierZoom = 1.0;

                QPixmap pixmap = QPixmap::fromImage(originalImage);
                pixmap = pixmap.scaled(originalImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                originalImageLabel->setPixmap(pixmap);
                modifiedImageLabel->clear();
                modifiedImageLabel->setText("Modified Image (not yet created)");
                hideButton->setEnabled(true);
                saveImageButton->setEnabled(false);
                updateCapacityStatus();
                currentCarrierPath = fileName;
                statusBar()->showMessage("Image loaded successfully");
            } else {
                statusBar()->showMessage("Failed to load image");
            }
        }
    }


    void MainWindow::saveModifiedImage() {
        if (modifiedImage.isNull()) {
            QMessageBox::warning(this, "Error", "No modified image to save");
            return;
        }



        // Create a default filename with timestamp
        QString defaultFilename = QString("%1/stego_image_%2.png")
        .arg(Constants::fusedImagesPath)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

        // Create and configure the file dialog
        QFileDialog fileDialog(this, "Save Modified Image", defaultFilename, "PNG Image (*.png)");
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
        fileDialog.setDefaultSuffix("png");

        // Show the dialog and get the result
        if (fileDialog.exec() != QDialog::Accepted) {
            return;
        }

        // Get the selected file path
        QString fileName = fileDialog.selectedFiles().first();
        if (fileName.isEmpty()) {
            return;
        }

        // Ensure PNG extension
        if (!fileName.endsWith(".png", Qt::CaseInsensitive)) {
            fileName += ".png";
        }


        // Ensure the directory exists
        //QDir().mkpath(QFileInfo(fileName).absolutePath());

        // Show a "please wait" message in the status bar immediately
        statusBar()->showMessage("Saving image, please wait...");
        QApplication::processEvents();

        // Create and show progress dialog
        QProgressDialog progress("Saving image...", nullptr, 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setCancelButton(nullptr);
        progress.setMinimumDuration(0);
        progress.show();
        QApplication::processEvents();

        // Use a separate thread for saving to keep UI responsive
        QFuture<bool> future = QtConcurrent::run([this, fileName]() {
            return modifiedImage.save(fileName, "PNG", 5);
        });

        // Create a watcher to monitor the save operation
        QFutureWatcher<bool> watcher;
        QObject::connect(&watcher, &QFutureWatcher<bool>::finished, [&]() {
            progress.close();

            bool success = future.result();
            if (success) {
                statusBar()->showMessage("Image saved successfully to: " + fileName);
                currentModifiedPath = fileName;
            } else {
                QMessageBox::warning(this, "Error", "Could not save the image");
            }
        });

        // Start watching the future
        watcher.setFuture(future);

        // Wait for the operation to complete (with event processing)
        while (!future.isFinished()) {
            QApplication::processEvents();
            QThread::msleep(50);
        }
    }

    void MainWindow::updateCapacityStatus()
    {
        if (originalImage.isNull()) {
            capacityLabel->setText("Capacity: 0/0 bytes (0%)");
            return;
        }

        int capacity = engine->calculateImageCapacity(originalImage);
        int dataSize = 0;

        if (isTextInput) {
            dataSize = textInput->toPlainText().toUtf8().size();
        } else {
            QFile file(filePathInput->text());
            if (file.exists() && file.open(QIODevice::ReadOnly)) {
                dataSize = file.size();
                file.close();
            }
        }

        // If encryption is checked, estimate the encrypted size (add ~30% for encryption overhead)
        if (encryptionCheckBox->isChecked() && dataSize > 0) {
            dataSize = dataSize + 32 + 4; // 32 bytes for salt+IV, 4 bytes for "ENCR" marker
            // Add padding to account for AES block size (16 bytes)
            int remainder = dataSize % 16;
            if (remainder > 0) {
                dataSize += (16 - remainder);
            }
        }

        double percentage = (capacity > 0) ? (dataSize * 100.0 / capacity) : 0;
        capacityLabel->setText(QString("Capacity: %1/%2 bytes (%3%)").arg(dataSize).arg(capacity).arg(percentage, 0, 'f', 1));

        // Update the hide button status based on capacity
        hideButton->setEnabled(capacity > 0 && dataSize > 0 && dataSize <= capacity);
    }


    /*
void MainWindow::toggleInputMethod()
{
    isTextInput = textInputRadio->isChecked();

    textInput->setEnabled(isTextInput);
    filePathInput->setEnabled(!isTextInput);
    browseFileButton->setEnabled(!isTextInput);

    updateCapacityStatus();
}
*/

    void MainWindow::toggleInputMethod()
    {
        isTextInput = textInputRadio->isChecked();

        textInput->setEnabled(isTextInput);
        filePathInput->setEnabled(!isTextInput);
        browseFileButton->setEnabled(!isTextInput);

        if (isAudioMode) {
            updateAudioCapacity();
            hideButton->setEnabled(!m_currentAudioData.isEmpty());
        } else {
            updateCapacityStatus();  // image mode
            hideButton->setEnabled(!originalImage.isNull());
        }
    }


    void MainWindow::selectDataFile() {


        QString fileName = QFileDialog::getOpenFileName(this, "Select File to Hide", Constants::imagesPath);

        if (!fileName.isEmpty()) {
            filePathInput->setText(fileName);
            if(isAudioMode){
                updateAudioCapacity();
            }else {
                updateCapacityStatus();
            }
        }
    }

    //camera
    void MainWindow::setupCamera()
    {

        // Get available cameras
        const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();


        if (!cameras.isEmpty()) {
            camera = new QCamera(cameras.first());


            videoSink = new QVideoSink(this);

            QMediaCaptureSession *captureSession = new QMediaCaptureSession;

            captureSession->setCamera(camera);

            captureSession->setVideoSink(videoSink);

            connect(videoSink, &QVideoSink::videoFrameChanged,
            this, &MainWindow::processCameraFrame);

            camera->start();

            statusBar()->showMessage("Camera started - scanning for PRT codes...");
        } else {
            QMessageBox::warning(this, "No Camera", "No camera device found!");
        }
    }


    void MainWindow::processCameraFrame(const QVideoFrame &frame)
    {
        static int frameCount = 0;
        if (++frameCount % 5 != 0) return;

        QImage image = frame.toImage();

        QByteArray data = engine->extractPRTData(image);
        if (!data.isEmpty()) {

            // THREAD-SAFE UI UPDATE
            QMetaObject::invokeMethod(this, [this, data]() {
                QString text = QString::fromUtf8(data);
                extractedTextOutput->setPlainText(text);
                statusBar()->showMessage("PRT code scanned successfully!");

                // Optional: Show success message like in file mode
                QMessageBox::information(this, "PRT Code Scanned",
                "PRT code successfully scanned from camera!\n\nContent: " + text);

                stopCamera();
            }, Qt::QueuedConnection);

            return; // Important: return after queuing the update
        }
    }

    void MainWindow::resetAll()
    {

        // Stop camera if running
        if (camera) {
            stopCamera();
        }

        // ===== HIDE TAB =====

        // Clear image previews
        originalImageLabel->clear();
        originalImageLabel->setText("Original Image");
        modifiedImageLabel->clear();
        modifiedImageLabel->setText("Modified Image");

        // Clear image data
        originalImage = QImage();
        modifiedImage = QImage();

        // Reset data input
        textInput->clear();
        filePathInput->clear();
        textInputRadio->setChecked(true);
        filePathInput->setEnabled(false);
        browseFileButton->setEnabled(false);

        // Reset encryption
        encryptionCheckBox->setChecked(false);
        audioStegCheckBox->setChecked(false);
        isAudioMode = false;
        // Reset buttons and labels
        hideButton->setEnabled(false);
        saveImageButton->setEnabled(false);
        capacityLabel->setText("Capacity: 0/0 bytes (0%)");

        // ===== EXTRACT TAB =====

        // Clear stego image preview
        stegoImageLabel->clear();
        stegoImageLabel->setText("Steganographic Image");

        // Clear extracted data
        extractedTextOutput->clear();
        stegoImage = QImage();

        // Reset extract buttons
        extractButton->setEnabled(false);
        saveExtractedFileButton->setEnabled(false);

        // Reset camera button text if needed
        cameraButton->setText("Start Camera");

        // ===== RESET MODE STATE =====
        // If you want to reset PRT mode too:
        // isPRTMode = false;
        // if (prtCheckBox) prtCheckBox->setChecked(false);

        // Clear any remembered passphrase
        hasRememberedPassphrase = false;
        rememberedPassphrase.clear();
        //prtCheckBox->setChecked(false);
        statusBar()->showMessage("All fields reset");

        // Remove all selection highlights
        originalImageLabel->setStyleSheet("");
        modifiedImageLabel->setStyleSheet("");
        stegoImageLabel->setStyleSheet("");

        if(m_audioToolbar->isVisible()) {
            toggleAudioBarAction->setChecked(false);
            if(audioPlayer && audioPlayer->isPlaying()) {
                audioPlayer->stop();
            }
            m_audioToolbar->setVisible(false);
        }
    }


    void MainWindow::dragEnterEvent(QDragEnterEvent *event)
    {
        if (!event->mimeData()->hasUrls()) return;

        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty()) return;

        QString filePath = urls.first().toLocalFile();
        if (filePath.isEmpty()) return;

        // Accept if image
        bool isImage = filePath.endsWith(".png", Qt::CaseInsensitive) ||
        filePath.endsWith(".jpg", Qt::CaseInsensitive) ||
        filePath.endsWith(".jpeg", Qt::CaseInsensitive);

        // Accept if audio
        bool isAudio = filePath.endsWith(".mp3", Qt::CaseInsensitive) ||
        filePath.endsWith(".wav", Qt::CaseInsensitive) ||
        filePath.endsWith(".flac", Qt::CaseInsensitive) ||
        filePath.endsWith(".ogg", Qt::CaseInsensitive);

        if (isImage || isAudio) {
            event->acceptProposedAction();
        }
    }

    void MainWindow::dropEvent(QDropEvent *event)
    {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            //processDroppedImage(filePath);
            qDebug()<<"from inside dropevent. isaudiomode : "<< isAudioMode;
            if (isAudioMode) {
                processDroppedAudio(filePath);
            } else {
                processDroppedImage(filePath);
            }
        }

        event->acceptProposedAction();
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {

    }

    void MainWindow::stopCamera()
    {
        if (camera) {
            camera->stop();
            delete camera;
            camera = nullptr;
        }
        statusBar()->showMessage("Camera stopped");
    }
    /*
void MainWindow::onCameraButtonClicked()
{
    if (camera) {
        stopCamera();
    } else {
        setupCamera();
    }
}
*/

    void MainWindow::onCameraButtonClicked()
    {
        return;

        if(!isPRTMode)
        {
            QMessageBox::information(this, "PRT Mode Required",
            "Camera scanning is only available in PRT Mode.\n"
            "Please enable the 'PRT Mode' checkbox to scan beautiful PRT codes.");
            return;
        }

        if (camera) {
            stopCamera();
            Constants::isCameraOn = false;

            cameraButton->setText("📸 Scan PRT Code");
        } else {
            setupCamera();
            Constants::isCameraOn = true;
            cameraButton->setText("⏹️ Stop Camera");
        }
    }


    void MainWindow::processDroppedImage(const QString& filePath)
    {
        if (filePath.isEmpty()) return;

        // Check if it's a supported image format
        if (!filePath.endsWith(".png", Qt::CaseInsensitive) &&
        !filePath.endsWith(".jpg", Qt::CaseInsensitive) &&
        !filePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
            statusBar()->showMessage("Unsupported file format");
            return;
        }

        resetAll();

        // Load the image
        QImage image;
        if (!image.load(filePath)) {
            statusBar()->showMessage("Failed to load image");
            return;
        }

        // Convert to suitable format
        /*
        if (image.format() != QImage::Format_ARGB32 &&
            image.format() != QImage::Format_RGB32) {
            image = image.convertToFormat(QImage::Format_ARGB32);
        }
    */
        // DETECTION: Extract data and check headers
        QByteArray extractedData = engine->extractDataFromImage(image);

        bool isPRTImage = false;
        bool isEncryptedStego = false;
        bool isRegularStego = false;

        if (!extractedData.isEmpty()) {
            // Check for PRT header
            if (extractedData.size() >= 4 &&
            extractedData.startsWith("PRT") && extractedData[3] == 0x01) {
                isPRTImage = true;
                prtCheckBox->setChecked(true);
            }
            // Check for encrypted data
            else if (extractedData.startsWith("ENCR")) {
                isEncryptedStego = true;
            }
            // Regular stego data (no specific header)
            else {
                isRegularStego = true;
            }
        }

        // Route to appropriate tab
        if (isPRTImage || isEncryptedStego || isRegularStego) {
            // Load in extract tab (any type of stego image)
            stegoImage = image;
            originalStegoImage = image;
            stegoZoom = 1.0;
            QPixmap pixmap = QPixmap::fromImage(stegoImage);
            pixmap = pixmap.scaled(stegoImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            stegoImageLabel->setPixmap(pixmap);
            saveExtractedFileButton->setEnabled(true);
            extractButton->setEnabled(true);

            if (extractTab) {
                tabWidget->setCurrentWidget(extractTab);
            }

            // Show appropriate message
            if (isPRTImage) {
                statusBar()->showMessage("PRT artistic code loaded: " + filePath);
            } else if (isEncryptedStego) {
                statusBar()->showMessage("Encrypted stego image loaded: " + filePath);
            } else {
                statusBar()->showMessage("Stego image loaded: " + filePath);
            }
        }
        else {
            // Load in hide tab as carrier image
            originalImage = image;
            originalCarrierImage = image;
            carrierZoom = 1.0;
            QPixmap pixmap = QPixmap::fromImage(originalImage);
            pixmap = pixmap.scaled(originalImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            originalImageLabel->setPixmap(pixmap);
            modifiedImageLabel->clear();
            modifiedImageLabel->setText("Modified Image (not yet created)");
            hideButton->setEnabled(true);
            saveImageButton->setEnabled(false);
            updateCapacityStatus();

            if (hideTab) {
                tabWidget->setCurrentWidget(hideTab);
            }
            statusBar()->showMessage("Carrier image loaded: " + filePath);
            currentCarrierPath = filePath;
        }
    }

    bool MainWindow::eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {

                // Clear previous selections
                originalImageLabel->setStyleSheet("");
                modifiedImageLabel->setStyleSheet("");
                stegoImageLabel->setStyleSheet("");

                // Highlight the clicked label
                QLabel *label = qobject_cast<QLabel*>(obj);
                if (label) {
                    label->setStyleSheet("border: 3px solid #f0d8b0;");
                }

                // Show corresponding path
                if (obj == originalImageLabel)
                statusBar()->showMessage(currentCarrierPath);
                else if (obj == modifiedImageLabel)
                statusBar()->showMessage(currentModifiedPath);
                else if (obj == stegoImageLabel)
                statusBar()->showMessage(currentToExtractPath);

                return true;
            }
        }
        else if (event->type() == QEvent::ContextMenu) {

            if(isAudioMode) return false;

            QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent*>(event);

            if (obj == originalImageLabel && !originalImage.isNull()) {
                QMenu* menu = createImageContextMenu(originalImageLabel, originalImage);
                menu->exec(contextMenuEvent->globalPos());
                delete menu;
                return true;
            }
            else if (obj == modifiedImageLabel && !modifiedImage.isNull()) {
                QMenu* menu = createImageContextMenu(modifiedImageLabel, modifiedImage);
                menu->exec(contextMenuEvent->globalPos());
                delete menu;
                return true;
            }
            else if (obj == stegoImageLabel && !originalStegoImage.isNull()) {
                QMenu* menu = createImageContextMenu(stegoImageLabel, originalStegoImage);
                menu->exec(contextMenuEvent->globalPos());
                delete menu;
                return true;
            }
        }
        else if (event->type() == QEvent::Resize) {
            // Handle resize to apply current aspect mode
            if (obj == originalImageLabel && !originalImage.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(originalImage);
                originalImageLabel->setPixmap(pixmap.scaled(
                originalImageLabel->size(),
                m_originalAspectMode,
                Qt::SmoothTransformation
                ));
            }
            else if (obj == modifiedImageLabel && !modifiedImage.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(modifiedImage);
                modifiedImageLabel->setPixmap(pixmap.scaled(
                modifiedImageLabel->size(),
                m_modifiedAspectMode,
                Qt::SmoothTransformation
                ));
            }
            else if (obj == stegoImageLabel && !originalStegoImage.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(originalStegoImage);
                stegoImageLabel->setPixmap(pixmap.scaled(
                stegoImageLabel->size(),
                m_stegoAspectMode,
                Qt::SmoothTransformation
                ));
            }
        }

        return QMainWindow::eventFilter(obj, event);
    }


    void MainWindow::updatePannedImage(QLabel *label, ImagePan &pan)
    {
        QImage *sourceImage = nullptr;
        double *zoomLevel = nullptr;

        if (label == stegoImageLabel && !originalStegoImage.isNull()) {
            sourceImage = &originalStegoImage;
            zoomLevel = &stegoZoom;
        }
        else if (label == originalImageLabel && !originalCarrierImage.isNull()) {
            sourceImage = &originalCarrierImage;
            zoomLevel = &carrierZoom;
        }
        else if (label == modifiedImageLabel && !modifiedImage.isNull()) {
            sourceImage = &modifiedImage;
            zoomLevel = &modifiedZoom;
        }

        if (!sourceImage || !zoomLevel) return;

        // Calculate zoomed size
        QSize zoomedSize = sourceImage->size() * (*zoomLevel);

        // Create pixmap of label size
        QPixmap displayPixmap(label->size());
        displayPixmap.fill(Qt::black); // or transparent

        QPainter painter(&displayPixmap);

        // Draw zoomed image with pan offset
        QPixmap zoomedPixmap = QPixmap::fromImage(*sourceImage)
        .scaled(zoomedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        painter.drawPixmap(pan.currentOffset, zoomedPixmap);
        painter.end();

        label->setPixmap(displayPixmap);
    }


    QWidget* MainWindow::createAudioPlayerBar()
    {
        // Create toolbar widget
        m_audioToolbar = new QWidget();
        m_audioToolbar->setObjectName("audioToolbar");
        m_audioToolbar->setFixedHeight(35);
        m_audioToolbar->setVisible(false);  // Hidden by default

        // Create toolbar layout
        QHBoxLayout *toolbarLayout = new QHBoxLayout(m_audioToolbar);
        toolbarLayout->setContentsMargins(10, 5, 10, 5);
        toolbarLayout->setSpacing(10);


        m_openMusicButton = new QPushButton(m_audioToolbar);
        m_openMusicButton->setObjectName("openMusicButton");
        m_openMusicButton->setIcon(QIcon(":/resources/icons/music.svg"));
        m_openMusicButton->setFixedSize(44, 24);
        m_openMusicButton->setToolTip("Load audio files");

        // === PLAY/PAUSE/STOP BUTTONS ===
        m_audioPlayButton = new QPushButton(m_audioToolbar);
        m_audioPlayButton->setObjectName("audioPlayButton");
        m_audioPlayButton->setIcon(QIcon(":/resources/icons/play.svg"));
        m_audioPlayButton->setFixedSize(32, 24);
        m_audioPlayButton->setToolTip("Play");

        m_audioPauseButton = new QPushButton(m_audioToolbar);
        m_audioPauseButton->setObjectName("audioPauseButton");
        m_audioPauseButton->setIcon(QIcon(":resources/icons/pause.svg"));
        m_audioPauseButton->setFixedSize(24, 24);
        m_audioPauseButton->setToolTip("Pause");

        m_audioStopButton = new QPushButton(m_audioToolbar);
        m_audioStopButton->setObjectName("audioStopButton");
        m_audioStopButton->setIcon(QIcon(":/resources/icons/square.svg"));
        m_audioStopButton->setFixedSize(24, 24);
        m_audioStopButton->setToolTip("Stop");

        // === VOLUME SLIDER ===
        QLabel *volLabel = new QLabel("Vol:", m_audioToolbar);
        //volLabel->setStyleSheet("color: white;");

        m_audioVolumeSlider = new QSlider(Qt::Horizontal, m_audioToolbar);
        m_audioVolumeSlider->setObjectName("audioVolumeSlider");
        m_audioVolumeSlider->setMinimumWidth(100);
        m_audioVolumeSlider->setMaximumWidth(150);
        m_audioVolumeSlider->setRange(0, 100);
        m_audioVolumeSlider->setValue(70);

        m_audioVolumeLabel = new QLabel("70%", m_audioToolbar);
        //m_audioVolumeLabel->setStyleSheet("color: white; min-width: 35px;");

        // === DURATION SLIDER ===
        m_audioProgressSlider = new QSlider(Qt::Horizontal, m_audioToolbar);
        m_audioProgressSlider->setObjectName("audioProgressSlider");
        m_audioProgressSlider->setRange(0, 100);
        m_audioProgressSlider->setValue(0);

        // === TIME LABEL ===
        m_audioTimeLabel = new QLabel("00:00 / 00:00", m_audioToolbar);
        m_audioTimeLabel->setObjectName("audioTimeLabel");
        //m_audioTimeLabel->setStyleSheet("color: white; font-size: 12px; min-width: 100px;");

        // === ADD ALL TO LAYOUT ===
        toolbarLayout->addWidget(m_openMusicButton);

        toolbarLayout->addWidget(m_audioPlayButton);
        toolbarLayout->addWidget(m_audioPauseButton);
        toolbarLayout->addWidget(m_audioStopButton);

        toolbarLayout->addSpacing(20);  // Some breathing room

        toolbarLayout->addWidget(volLabel);
        toolbarLayout->addWidget(m_audioVolumeSlider);
        toolbarLayout->addWidget(m_audioVolumeLabel);

        toolbarLayout->addSpacing(10);

        toolbarLayout->addWidget(m_audioProgressSlider, 1);  // Stretches
        toolbarLayout->addWidget(m_audioTimeLabel);

        // === STYLESHEET (simplified) ===
        QString toolbarStyle =
        "#audioToolbar {"
        "    background-color: rgba(0, 0, 0, 220);"
        "    border-top: 1px solid rgba(255, 255, 255, 50);"
        "    padding: 2px 8px;"
        "    spacing: 4px;"
        "}"
        "#audioToolbar QLabel {"
        "    color: white;"
        "    font-size: 12px;"
        "}"
        "#audioToolbar QPushButton {"
        "    color: white;"
        "    background-color: rgba(255, 255, 255, 30);"
        "    border: none;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "}"
        "#audioToolbar QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 50);"
        "}"
        "#audioToolbar QPushButton:pressed {"
        "    background-color: rgba(255, 255, 255, 70);"
        "}"
        "#audioToolbar QSlider::groove:horizontal {"
        "    height: 3px;"
        "    background: rgba(255, 255, 255, 50);"
        "    border-radius: 1.5px;"
        "}"
        "#audioToolbar QSlider::handle:horizontal {"
        "    background: white;"
        "    width: 10px;"
        "    height: 10px;"
        "    margin: -3.5px 0;"
        "    border-radius: 5px;"
        "}";

        //m_audioToolbar->setStyleSheet(toolbarStyle);

        // === CONNECTIONS (to AudioPlayer) ===

        connect(m_openMusicButton, &QPushButton::clicked, this, [this](){
            QString fileName = QFileDialog::getOpenFileName(
            this,
            "Select Music File",
            Constants::appDirPath,
            "Audio Files (*.mp3 *.wav *.ogg *.flac *.aac *.m4a *.wma);;All Files (*.*)"
            );

            if (!fileName.isEmpty()) {
                audioPlayer->playFile(fileName);
            }
        });

        // Play/Pause/Stop buttons
        connect(m_audioPlayButton, &QPushButton::clicked,
        audioPlayer, &AudioPlayer::play);
        connect(m_audioPauseButton, &QPushButton::clicked,
        audioPlayer, &AudioPlayer::pause);
        connect(m_audioStopButton, &QPushButton::clicked,
        audioPlayer, &AudioPlayer::stop);

        // Volume control
        connect(m_audioVolumeSlider, &QSlider::valueChanged,
        audioPlayer, &AudioPlayer::setVolume);
        connect(audioPlayer, &AudioPlayer::volumeChanged,
        this, &MainWindow::onAudioVolumeChanged);

        // Progress slider (use sliderReleased for seeking)
        connect(m_audioProgressSlider, &QSlider::sliderReleased, this, [this](){
            int percent = m_audioProgressSlider->value();
            qint64 duration = audioPlayer->duration();
            qint64 newPosition = (duration * percent) / 100;
            audioPlayer->setPosition(newPosition);
        });

        // Player signals to UI
        connect(audioPlayer, &AudioPlayer::positionChanged,
        this, &MainWindow::onAudioPositionChanged);
        connect(audioPlayer, &AudioPlayer::durationChanged,
        this, &MainWindow::onAudioDurationChanged);
        connect(audioPlayer, &AudioPlayer::mediaLoaded,
        this, &MainWindow::onMediaLoaded);
        connect(audioPlayer, &AudioPlayer::playbackStarted,
        this, &MainWindow::onPlaybackStarted);
        connect(audioPlayer, &AudioPlayer::playbackPaused,
        this, &MainWindow::onPlaybackPaused);
        connect(audioPlayer, &AudioPlayer::playbackStopped,
        this, &MainWindow::onPlaybackStopped);

        return m_audioToolbar;

    }


    void MainWindow::openAudioCarrier()
    {
        QString initialAudioPath = Constants::musicPath;

        if (!QDir(initialAudioPath).exists()) {
            initialAudioPath = Constants::appDirPath;
        }

        QString fileName = QFileDialog::getOpenFileName(this, "Select Audio Carrier",
        initialAudioPath,
        "Audio Files (*.mp3 *.wav *.flac *.ogg)");
        if (fileName.isEmpty()) return;

        statusBar()->showMessage("Loading audio file...");
        QApplication::processEvents();

        // Convert to WAV using FFmpeg
        QByteArray wavData;
        if (!ffmpeg->convertToWav(fileName, wavData)) {
            QMessageBox::warning(this, "Error", "Could not convert audio file to WAV format");
            return;
        }

        // Get metadata
        auto info = ffmpeg->getAudioInfoFromData(wavData);
        if (!info.isValid) {
            QMessageBox::warning(this, "Error", "Could not read audio file information");
            return;
        }

        // Store data
        m_currentAudioPath = fileName;
        m_currentAudioData = wavData;
        m_currentSampleRate = info.sampleRate;
        m_currentChannels = info.channels;
        m_currentBitsPerSample = info.bitsPerSample;
        m_currentDuration = info.durationSeconds;
        m_modifiedAudioData.clear();

        // Update UI (reuse existing labels but change text)
        originalImageLabel->setText(QString("Audio: %1\n%2 Hz, %3 ch, %4 bit, %5 s")
        .arg(QFileInfo(fileName).fileName())
        .arg(info.sampleRate)
        .arg(info.channels)
        .arg(info.bitsPerSample)
        .arg(info.durationSeconds));
        modifiedImageLabel->setText("Modified Audio (not yet created)");

        // Clear any pixmaps
        originalImageLabel->setPixmap(QPixmap());
        modifiedImageLabel->setPixmap(QPixmap());
        QIcon placeholderIcon(":/resources/icons/music.svg");
        originalImageLabel->setPixmap(placeholderIcon.pixmap(originalImageLabel->size()));
        //originalImageLabel->setText(fileName);
        currentCarrierPath = fileName;
        updateAudioCapacity();
        hideButton->setEnabled(true);
        saveImageButton->setEnabled(false);

        statusBar()->showMessage("Loaded: " + QFileInfo(fileName).fileName());
    }

    //=============================================================================
    // AUDIO STEGO FILE OPENING (like openStegoImageButton lambda)
    //=============================================================================



    void MainWindow::openAudioStegoFile()
    {
        QString filter = "Audio Files (*.mp3 *.wav *.flac *.ogg);;All Files (*)";
        QString fileName = QFileDialog::getOpenFileName(this, "Open Audio with Hidden Data",
        Constants::fusedImagesPath, filter);
        if (fileName.isEmpty()) return;

        if (fileName.endsWith(".wav", Qt::CaseInsensitive)) {
            QFile file(fileName);
            if(!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Could not read file: " + file.errorString());
                return;
            }
            QByteArray wholeFile = file.readAll();
            file.close();

            // Check header integrity
            qDebug() << "Header first 4 bytes:" << wholeFile.left(4);
            qDebug() << "Should be RIFF:" << (wholeFile.left(4) == "RIFF" ? "OK" : "CORRUPT!");

            // Check data size field (bytes 4-7)
            quint32 riffSize = *reinterpret_cast<const quint32*>(wholeFile.mid(4,4).constData());
            qDebug() << "RIFF size:" << riffSize << "actual file size:" << wholeFile.size();

        }


        statusBar()->showMessage("Loading stego audio file...");
        QApplication::processEvents();

        QByteArray audioData;
        auto info = ffmpeg->getAudioInfo(fileName);
        if (fileName.endsWith(".wav", Qt::CaseInsensitive)) {
            // WAV files: read directly, NO CONVERSION!
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Could not read WAV file");
                return;
            }
            audioData = file.readAll();
            file.close();
        } else {
            // Non-WAV files: convert to WAV first
            if (!ffmpeg->convertToWav(fileName, audioData)) {
                QMessageBox::warning(this, "Error", "Could not process audio file");
                return;
            }
            info = ffmpeg->getAudioInfoFromData(audioData);
        }

        if (!info.isValid) {
            QMessageBox::warning(this, "Error", "Could not read audio file information");
            return;
        }

        // Store data
        m_stegoAudioPath = fileName;
        m_stegoAudioData = audioData;
        m_currentSampleRate = info.sampleRate;
        m_currentChannels = info.channels;
        m_currentBitsPerSample = info.bitsPerSample;

        // Update UI
        /*
        stegoImageLabel->setText(QString("Stego Audio: %1\n%2 Hz, %3 ch, %4 bit")
                                  .arg(QFileInfo(fileName).fileName())
                                  .arg(info.sampleRate)
                                  .arg(info.channels)
                                  .arg(info.bitsPerSample));
                                  */
        QIcon placeholderIcon(":/resources/icons/music.svg");
        stegoImageLabel->setPixmap(placeholderIcon.pixmap(originalImageLabel->size()));

        currentToExtractPath = fileName;
        //stegoImageLabel->setPixmap(QPixmap());
        saveExtractedFileButton->setEnabled(true);
        //extractedTextOutput->clear();




        statusBar()->showMessage("Loaded stego audio: " + QFileInfo(fileName).fileName());
    }


    //=============================================================================
    // SAVE MODIFIED AUDIO (like saveModifiedImage)
    //=============================================================================

    void MainWindow::saveModifiedAudio()
    {
        if (m_modifiedAudioData.isEmpty()) {
            QMessageBox::warning(this, "Error", "No modified audio to save");
            return;
        }

        QString baseName = QFileInfo(m_currentAudioPath).completeBaseName();
        QString defaultFilename = QDir(Constants::fusedImagesPath)
        .filePath(QString("%1_stego.wav").arg(baseName));

        QString fileName = QFileDialog::getSaveFileName(this, "Save Modified Audio",
        defaultFilename,
        "WAV Files (*.wav)");
        if (fileName.isEmpty()) return;

        // Ensure .wav extension
        if (!fileName.endsWith(".wav", Qt::CaseInsensitive)) {
            fileName += ".wav";
        }

        statusBar()->showMessage("Saving audio file...");
        //QProgressDialog progress("Saving audio...", QString(), 0, 0, this);
        //progress.setWindowModality(Qt::ApplicationModal);
        //progress.show();
        QApplication::processEvents();

        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            qint64 written = file.write(m_modifiedAudioData);
            file.close();

            if (written == m_modifiedAudioData.size()) {
                statusBar()->showMessage("Saved: " + QFileInfo(fileName).fileName());
                currentModifiedPath = fileName;
                QIcon placeholderIcon(":/resources/icons/music.svg");
                modifiedImageLabel->setPixmap(placeholderIcon.pixmap(originalImageLabel->size()));
            } else {
                QMessageBox::warning(this, "Error", "Failed to save all audio data");
            }
        } else {
            QMessageBox::warning(this, "Error", "Could not open file for writing");
        }
    }

    //=============================================================================
    // HIDE DATA IN AUDIO (like hideData lambda but for audio)
    //=============================================================================

    void MainWindow::hideDataInAudio()
    {
        if (m_currentAudioData.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please load an audio file first");
            return;
        }

        QByteArray data;
        if (isTextInput) {
            QString text = textInput->toPlainText();
            if (text.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter text to hide");
                return;
            }
            data = text.toUtf8();
        } else {
            QString filePath = filePathInput->text();
            QFile file(filePath);
            if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Could not open the selected file");
                return;
            }
            //data = file.readAll();
            //file.close();
            QByteArray fileData = file.readAll();
            file.close();
            // --- SAME LOGIC AS IMAGE STEG ---
            QString fileName = QFileInfo(filePath).fileName();   // original filename
            QByteArray fileNameBytes = fileName.toUtf8();

            QByteArray combined;
            combined.append(static_cast<char>(fileNameBytes.size())); // filename length
            combined.append(fileNameBytes);                           // filename
            combined.append(fileData);                                // file data

            data = combined;
        }

        // Check if encryption is enabled
        if (encryptionCheckBox->isChecked()) {
            PassphraseDialog dialog(true, this);
            if (dialog.exec() == QDialog::Accepted) {
                QString passphrase = dialog.getPassphrase();
                QByteArray encryptedData = engine->encryptData(data, passphrase);
                if (encryptedData.isEmpty()) {
                    QMessageBox::warning(this, "Error", "Encryption failed");
                    return;
                }
                encryptedData.prepend("ENCR");
                data = encryptedData;

                if (dialog.rememberPassphrase()) {
                    rememberedPassphrase = passphrase;
                    hasRememberedPassphrase = true;
                }
            } else {
                return;
            }
        }

        // Check capacity
        int totalSamples = m_currentAudioData.size() / (m_currentBitsPerSample / 8);
        int capacity = audioEngine->calculateAudioCapacity(totalSamples, m_currentChannels,
        m_currentBitsPerSample, 2);

        if (data.size() > capacity) {
            QMessageBox::warning(this, "Error",
            QString("Data size (%1 bytes) exceeds audio capacity (%2 bytes)")
            .arg(data.size()).arg(capacity));
            return;
        }

        statusBar()->showMessage("Hiding data in audio...");
        QApplication::processEvents();

        QByteArray stegoAudio;
        if (audioEngine->embedDataInAudio(m_currentAudioData, stegoAudio, data,
        m_currentSampleRate, m_currentChannels,
        m_currentBitsPerSample, 2)) {
            m_modifiedAudioData = stegoAudio;
            modifiedImageLabel->setText(QString("Modified Audio (%1 KB)")
            .arg(stegoAudio.size() / 1024));
            modifiedImageLabel->setPixmap(QPixmap());
            saveImageButton->setEnabled(true);
            statusBar()->showMessage("Data hidden successfully");
        } else {
            QMessageBox::warning(this, "Error", "Failed to hide data in audio");
        }
    }

    //=============================================================================
    // EXTRACT DATA FROM AUDIO (like saveExtractedFileButton lambda but for audio)
    //=============================================================================



    void MainWindow::extractDataFromAudio()
    {
        if (m_stegoAudioData.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please load a stego audio file first");
            return;
        }

        statusBar()->showMessage("Extracting data from audio...");
        QApplication::processEvents();

        QByteArray extractedData = audioEngine->extractDataFromAudio(
        m_stegoAudioData,
        m_currentSampleRate,
        m_currentChannels,
        m_currentBitsPerSample,
        2);

        if (extractedData.isEmpty()) {
            QMessageBox::warning(this, "Error", "No hidden data found");
            return;
        }

        // Handle encryption
        if (extractedData.startsWith("ENCR")) {

            if (hasRememberedPassphrase) {
                QByteArray decryptedData =
                engine->decryptData(extractedData.mid(4), rememberedPassphrase);

                if (!decryptedData.isEmpty()) {
                    extractedData = decryptedData;
                } else {
                    hasRememberedPassphrase = false;
                    rememberedPassphrase.clear();
                }
            }

            if (!hasRememberedPassphrase) {

                PassphraseDialog dialog(false, this);

                if (dialog.exec() == QDialog::Accepted) {

                    QString passphrase = dialog.getPassphrase();

                    QByteArray decryptedData =
                    engine->decryptData(extractedData.mid(4), passphrase);

                    if (!decryptedData.isEmpty()) {

                        extractedData = decryptedData;

                        if (dialog.rememberPassphrase()) {
                            rememberedPassphrase = passphrase;
                            hasRememberedPassphrase = true;
                        }

                    } else {
                        QMessageBox::warning(this, "Error", "Decryption failed");
                        return;
                    }

                } else {
                    return;
                }
            }
        }

        // ===== PARSE FILENAME (same logic as image steg) =====

        QString extractedFileName;
        QByteArray fileContent;

        int fileNameLen = static_cast<unsigned char>(extractedData[0]);

        if (fileNameLen > 0 &&
        fileNameLen < 200 &&
        extractedData.size() > (1 + fileNameLen))
        {
            extractedFileName =
            QString::fromUtf8(extractedData.mid(1, fileNameLen));

            fileContent =
            extractedData.mid(1 + fileNameLen);

            // Detect bogus filenames produced by plain text
            //if (extractedFileName.contains(QRegExp(R"([:,'"])"))) {
            if (extractedFileName.contains(QRegularExpression(R"([:,'"])"))) {
                extractedFileName =
                QString("extracted_audio_data_%1.txt")
                .arg(QDateTime::currentDateTime()
                .toString("yyyyMMdd_hhmmss"));

                fileContent = extractedData;
            }

        } else {

            extractedFileName =
            QString("extracted_audio_data_%1.txt")
            .arg(QDateTime::currentDateTime()
            .toString("yyyyMMdd_hhmmss"));

            fileContent = extractedData;
        }

        // Build default save path
        QString defaultFilename =
        QDir(Constants::extractedImagesPath)
        .filePath(extractedFileName);

        QString fileName =
        QFileDialog::getSaveFileName(
        this,
        "Save Extracted Data",
        defaultFilename,
        "All Files (*)");

        if (fileName.isEmpty()) {

            QString text = QString::fromUtf8(fileContent);

            bool isTextFile = true;

            for (const QChar &c : text) {
                if (!c.isPrint() && !c.isSpace()) {
                    isTextFile = false;
                    break;
                }
            }

            if (isTextFile) {
                extractedTextOutput->setPlainText(text);
            } else {
                extractedTextOutput->setPlainText(
                QString("Binary data: %1 bytes")
                .arg(fileContent.size()));
            }

            statusBar()->showMessage("Data extracted but not saved");
            return;
        }

        QFile file(fileName);

        if (file.open(QIODevice::WriteOnly)) {

            qint64 bytesWritten = file.write(fileContent);
            file.close();

            if (bytesWritten == fileContent.size()) {

                statusBar()->showMessage(
                "Extracted data saved successfully to: " + fileName);

                QString text = QString::fromUtf8(fileContent);

                bool isTextFile = true;

                for (const QChar &c : text) {
                    if (!c.isPrint() && !c.isSpace()) {
                        isTextFile = false;
                        break;
                    }
                }

                if (isTextFile) {

                    extractedTextOutput->setPlainText(text);

                    QMessageBox::information(
                    this,
                    "Text File Detected",
                    "The extracted data appears to be text and has been saved to: "
                    + fileName);

                } else {

                    extractedTextOutput->setPlainText(
                    QString("Binary data: %1 bytes")
                    .arg(fileContent.size()));

                    QMessageBox::information(
                    this,
                    "Binary File Saved",
                    "The extracted binary data has been saved to: "
                    + fileName);
                }

            } else {
                QMessageBox::warning(this,
                "Error",
                "Could not save all the extracted data");
            }

        } else {

            QMessageBox::warning(this,
            "Error",
            "Could not save the extracted data");
        }

        m_extractedData = fileContent;
    }


    //=============================================================================
    // UPDATE AUDIO CAPACITY (like updateCapacityStatus)
    //=============================================================================

    void MainWindow::updateAudioCapacity()
    {
        if (!capacityLabel) return;

        if (m_currentAudioData.isEmpty()) {
            capacityLabel->setText("Capacity: 0/0 bytes (0%)");
            return;
        }

        int totalSamples = m_currentAudioData.size() / (m_currentBitsPerSample / 8);
        int capacity = audioEngine->calculateAudioCapacity(totalSamples, m_currentChannels,
        m_currentBitsPerSample, 2);

        int dataSize = 0;
        if (isTextInput) {
            dataSize = textInput->toPlainText().toUtf8().size();
        } else {
            QFile file(filePathInput->text());
            if (file.exists() && file.open(QIODevice::ReadOnly)) {
                dataSize = file.size();
                file.close();
            }
        }

        if (encryptionCheckBox->isChecked() && dataSize > 0) {
            dataSize = dataSize + 32 + 4;
            int remainder = dataSize % 16;
            if (remainder > 0) dataSize += (16 - remainder);
        }

        double percentage = (capacity > 0) ? (dataSize * 100.0 / capacity) : 0;
        capacityLabel->setText(QString("Capacity: %1/%2 bytes (%3%)")
        .arg(dataSize).arg(capacity).arg(percentage, 0, 'f', 1));

        //hideButton->setEnabled(capacity > 0 && dataSize > 0 && dataSize <= capacity);
    }

    //=============================================================================
    // TOGGLE AUDIO INPUT METHOD (like toggleInputMethod)
    //=============================================================================

    void MainWindow::toggleAudioInputMethod()
    {
        isTextInput = textInputRadio->isChecked();
        textInput->setEnabled(isTextInput);
        filePathInput->setEnabled(!isTextInput);
        browseFileButton->setEnabled(!isTextInput);
        updateAudioCapacity();
    }

    //=============================================================================
    // SELECT AUDIO DATA FILE (like selectDataFile)
    //=============================================================================

    void MainWindow::selectAudioDataFile()
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Select File to Hide", QDir::homePath());
        if (!fileName.isEmpty()) {
            filePathInput->setText(fileName);
            updateAudioCapacity();
        }
    }

    //=============================================================================
    // PROCESS DROPPED AUDIO (like processDroppedImage)
    //=============================================================================



    void MainWindow::processDroppedAudio(const QString &filePath)
    {
        if (filePath.isEmpty()) return;

        // Check supported audio formats
        if (!filePath.endsWith(".mp3", Qt::CaseInsensitive) &&
        !filePath.endsWith(".wav", Qt::CaseInsensitive) &&
        !filePath.endsWith(".flac", Qt::CaseInsensitive) &&
        !filePath.endsWith(".ogg", Qt::CaseInsensitive)) {
            statusBar()->showMessage("Unsupported audio format");
            return;
        }

        // Convert to WAV for processing
        QByteArray wavData;
        if (!ffmpeg->convertToWav(filePath, wavData)) {
            statusBar()->showMessage("Could not process audio file");
            return;
        }

        // Get audio info
        auto info = ffmpeg->getAudioInfoFromData(wavData);
        if (!info.isValid) {
            statusBar()->showMessage("Could not read audio information");
            return;
        }

        // DETECTION: Extract data and check if it's a stego audio
        QByteArray extractedData = audioEngine->extractDataFromAudio(
        wavData, info.sampleRate, info.channels, info.bitsPerSample, 2);

        bool isStegoAudio = false;
        bool isEncryptedStego = false;

        if (!extractedData.isEmpty()) {
            isStegoAudio = true;
            if (extractedData.startsWith("ENCR")) {
                isEncryptedStego = true;
            }
        }

        // Route to appropriate tab based on detection AND current mode
        if (isAudioMode) {
            // In AUDIO mode
            if (isStegoAudio) {
                // Load in extract tab (stego audio)
                m_stegoAudioData = wavData;
                m_currentSampleRate = info.sampleRate;
                m_currentChannels = info.channels;
                m_currentBitsPerSample = info.bitsPerSample;
                currentToExtractPath = filePath;

                // Update UI
                QIcon placeholderIcon(":/resources/icons/music.svg");
                stegoImageLabel->setPixmap(placeholderIcon.pixmap(stegoImageLabel->size()));
                saveExtractedFileButton->setEnabled(true);

                if (extractTab) {
                    tabWidget->setCurrentWidget(extractTab);
                }

                statusBar()->showMessage(isEncryptedStego ?
                "Encrypted stego audio loaded: " + filePath :
                "Stego audio loaded: " + filePath);
            }
            else {
                // Load in hide tab as carrier audio
                openAudioCarrierFromDrop(filePath);
            }
        }
        else {
            // In IMAGE mode - route to appropriate audio function based on detection
            if (isStegoAudio) {
                // Switch to audio mode first?
                QMessageBox::information(this, "Audio Detected",
                "This appears to be a stego audio file. Switching to Audio Mode.");
                audioStegCheckBox->setChecked(true); // This will trigger onAudioStegToggled

                // Then load as stego
                m_stegoAudioData = wavData;
                m_currentSampleRate = info.sampleRate;
                m_currentChannels = info.channels;
                m_currentBitsPerSample = info.bitsPerSample;
                currentToExtractPath = filePath;

                QIcon placeholderIcon(":/resources/icons/music.svg");
                stegoImageLabel->setPixmap(placeholderIcon.pixmap(stegoImageLabel->size()));
                saveExtractedFileButton->setEnabled(true);

                if (extractTab) {
                    tabWidget->setCurrentWidget(extractTab);
                }
            }
            else {
                // Just a regular audio file - ask user what to do
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Audio File Dropped",
                "This is an audio file. Switch to Audio Mode?",
                QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::Yes) {
                    audioStegCheckBox->setChecked(true);
                    openAudioCarrierFromDrop(filePath);
                }
            }
        }
    }


    void MainWindow::onAudioStegToggled(bool checked)
    {
        isAudioMode = checked;
        qDebug() << "isaudiomode :" << isAudioMode;

        // Update UI elements based on mode
        if (checked) {
            // Switching to AUDIO mode
            // Change button texts
            if (openImageButton) openImageButton->setText("Select Audio Carrier");
            if (saveImageButton) saveImageButton->setText("Save Modified Audio");
            if (openStegoImageButton) openStegoImageButton->setText("Open Audio with Hidden Data");

            // Clear any image pixmaps and show text instead
            if (originalImageLabel) {
                originalImageLabel->setPixmap(QPixmap());
                originalImageLabel->setText("Original Audio (not loaded)");
            }
            if (modifiedImageLabel) {
                modifiedImageLabel->setPixmap(QPixmap());
                modifiedImageLabel->setText("Modified Audio (not yet created)");
            }

            // Update capacity label
            if (capacityLabel) {
                capacityLabel->setText("Capacity: 0/0 bytes (0%)");
            }

            if (stegoImageLabel) {
                stegoImageLabel->setPixmap(QPixmap());
                stegoImageLabel->setText("Steganographic Audio (not loaded)");
            }

            // Disable camera button
            if (cameraButton) cameraButton->setEnabled(false);

        } else {
            // Switching back to IMAGE mode
            // Restore button texts
            if (openImageButton) openImageButton->setText("Select Carrier Image");
            if (saveImageButton) saveImageButton->setText("Save Modified Image");
            if (openStegoImageButton) openStegoImageButton->setText("Open Image with Hidden Data");

            // Restore image labels
            if (originalImageLabel) {
                originalImageLabel->setPixmap(QPixmap());
                originalImageLabel->setText("Original Image");
            }
            if (modifiedImageLabel) {
                modifiedImageLabel->setPixmap(QPixmap());
                modifiedImageLabel->setText("Modified Image");
            }

            // Re-enable camera button
            if (cameraButton) cameraButton->setEnabled(true);

            // CRITICAL FIX: Re-enable and show text input based on current radio button state
            toggleInputMethod();

            // Restore image capacity
            updateCapacityStatus();
        }

        // Force layout update
        if (mainLayout) {
            mainLayout->activate();
        }
        this->adjustSize();
    }


    void MainWindow::openAudioCarrierFromDrop(const QString &filePath)
    {
        if (filePath.isEmpty()) return;

        m_stegoAudioData.clear();
        currentToExtractPath.clear();

        statusBar()->showMessage("Loading audio file...");
        QApplication::processEvents();

        // Convert to WAV using FFmpeg
        QByteArray wavData;
        if (!ffmpeg->convertToWav(filePath, wavData)) {
            QMessageBox::warning(this, "Error", "Could not convert audio file to WAV format");
            return;
        }

        // Get metadata
        auto info = ffmpeg->getAudioInfoFromData(wavData);
        if (!info.isValid) {
            QMessageBox::warning(this, "Error", "Could not read audio file information");
            return;
        }

        // Store data
        m_currentAudioPath = filePath;
        m_currentAudioData = wavData;
        m_currentSampleRate = info.sampleRate;
        m_currentChannels = info.channels;
        m_currentBitsPerSample = info.bitsPerSample;
        m_currentDuration = info.durationSeconds;
        m_modifiedAudioData.clear();

        // Update UI (reuse existing labels but change text)
        originalImageLabel->setText(QString("Audio: %1\n%2 Hz, %3 ch, %4 bit, %5 s")
        .arg(QFileInfo(filePath).fileName())
        .arg(info.sampleRate)
        .arg(info.channels)
        .arg(info.bitsPerSample)
        .arg(info.durationSeconds));
        modifiedImageLabel->setText("Modified Audio (not yet created)");

        // Clear any pixmaps
        originalImageLabel->setPixmap(QPixmap());
        modifiedImageLabel->setPixmap(QPixmap());
        QIcon placeholderIcon(":/resources/icons/music.svg");
        originalImageLabel->setPixmap(placeholderIcon.pixmap(originalImageLabel->size()));

        currentCarrierPath = filePath;
        updateAudioCapacity();
        hideButton->setEnabled(true);
        saveImageButton->setEnabled(false);

        statusBar()->showMessage("Loaded: " + QFileInfo(filePath).fileName());
    }

    void MainWindow::onAudioVolumeChanged(int volume)
    {
        m_audioVolumeSlider->setValue(volume);
        m_audioVolumeLabel->setText(QString("%1%").arg(volume));
    }

    void MainWindow::onAudioPositionChanged(qint64 position)
    {
        // Only update if user isn't dragging
        if (!m_audioProgressSlider->isSliderDown()) {
            qint64 duration = audioPlayer->duration();
            if (duration > 0) {
                int percent = (position * 100) / duration;
                m_audioProgressSlider->setValue(percent);
            }
            updateTimeLabel(position, audioPlayer->duration());
        }
    }

    void MainWindow::onAudioDurationChanged(qint64 duration)
    {
        updateTimeLabel(audioPlayer->position(), duration);
    }

    void MainWindow::onMediaLoaded(const QString &fileName)
    {
        statusBar()->showMessage("Loaded: " + fileName, 3000);
        // Optionally enable play button
        m_audioPlayButton->setEnabled(true);
    }

    void MainWindow::onPlaybackStarted()
    {
        m_audioPlayButton->setEnabled(false);
        m_audioPauseButton->setEnabled(true);
        m_audioStopButton->setEnabled(true);
    }

    void MainWindow::onPlaybackPaused()
    {
        m_audioPlayButton->setEnabled(true);
        m_audioPauseButton->setEnabled(false);
        m_audioStopButton->setEnabled(true);
    }

    void MainWindow::onPlaybackStopped()
    {
        m_audioPlayButton->setEnabled(true);
        m_audioPauseButton->setEnabled(false);
        m_audioStopButton->setEnabled(false);
        m_audioProgressSlider->setValue(0);
        updateTimeLabel(0, 0);
    }



    QString MainWindow::formatTime(qint64 milliseconds)
    {
        if (milliseconds <= 0) return "00:00";

        int totalSeconds = milliseconds / 1000;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
    }

    void MainWindow::updateTimeLabel(qint64 position, qint64 duration)
    {
        QString currentStr = formatTime(position);
        QString totalStr = formatTime(duration);
        m_audioTimeLabel->setText(QString("%1 / %2").arg(currentStr).arg(totalStr));
    }


    QMenu* MainWindow::createImageContextMenu(QLabel* label, const QImage& image)
    {
        QMenu* menu = new QMenu(this);

        QAction* copyAction = menu->addAction("Copy to Clipboard");
        menu->addSeparator();

        QMenu* aspectMenu = menu->addMenu("Aspect Ratio");
        QAction* actionKeepAspect = aspectMenu->addAction("Default (Keep Aspect)");
        QAction* actionZoom = aspectMenu->addAction("Zoom (Fill)");
        QAction* actionStretch = aspectMenu->addAction("Stretch (Ignore Aspect)");

        // Determine which label this is
        Qt::AspectRatioMode* currentMode = nullptr;
        if (label == originalImageLabel) {
            currentMode = &m_originalAspectMode;
        } else if (label == modifiedImageLabel) {
            currentMode = &m_modifiedAspectMode;
        } else if (label == stegoImageLabel) {
            currentMode = &m_stegoAspectMode;
        }

        // Check current mode
        if (currentMode) {
            actionKeepAspect->setCheckable(true);
            actionZoom->setCheckable(true);
            actionStretch->setCheckable(true);

            actionKeepAspect->setChecked(*currentMode == Qt::KeepAspectRatio);
            actionZoom->setChecked(*currentMode == Qt::KeepAspectRatioByExpanding);
            actionStretch->setChecked(*currentMode == Qt::IgnoreAspectRatio);
        }

        // Connect actions
        connect(actionKeepAspect, &QAction::triggered, this, [this, label, image, currentMode]() {
            if (currentMode) *currentMode = Qt::KeepAspectRatio;
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                label->setPixmap(pixmap.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            statusBar()->showMessage("Aspect: Default (Keep Aspect)", 2000);
        });

        connect(actionZoom, &QAction::triggered, this, [this, label, image, currentMode]() {
            if (currentMode) *currentMode = Qt::KeepAspectRatioByExpanding;
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                label->setPixmap(pixmap.scaled(label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            }
            statusBar()->showMessage("Aspect: Zoom - edges may be cropped", 2000);
        });

        connect(actionStretch, &QAction::triggered, this, [this, label, image, currentMode]() {
            if (currentMode) *currentMode = Qt::IgnoreAspectRatio;
            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                label->setPixmap(pixmap.scaled(label->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            statusBar()->showMessage("Aspect: Stretched", 2000);
        });

        connect(copyAction, &QAction::triggered, this, [label]() {
            if (!label->pixmap().isNull()) {
                QApplication::clipboard()->setPixmap(label->pixmap());
            }
        });

        return menu;
    }
