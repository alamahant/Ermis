
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , engine(new StegEngine(this))
    , ui(new Ui::MainWindow)
    , isTextInput(true)
    , hasRememberedPassphrase(false)
{
    ui->setupUi(this);
    setupUi();
    createMenus();
    createConnections();

    setWindowTitle("Steganize Me");
    setMinimumSize(800, 600);

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
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(tabWidget);

    // Create Hide Data tab
    QWidget *hideTab = new QWidget(tabWidget);
    tabWidget->addTab(hideTab, "Hide Data");

    // Image preview area
    originalImageLabel = new QLabel("Original Image", hideTab);
    originalImageLabel->setAlignment(Qt::AlignCenter);
    originalImageLabel->setMinimumSize(320, 240);
    originalImageLabel->setFrameShape(QFrame::Box);

    modifiedImageLabel = new QLabel("Modified Image", hideTab);
    modifiedImageLabel->setAlignment(Qt::AlignCenter);
    modifiedImageLabel->setMinimumSize(320, 240);
    modifiedImageLabel->setFrameShape(QFrame::Box);

    QHBoxLayout *previewLayout = new QHBoxLayout();
    previewLayout->addWidget(originalImageLabel);
    previewLayout->addWidget(modifiedImageLabel);

    // Image file selection
    openImageButton = new QPushButton("Select Carrier Image", hideTab);
    saveImageButton = new QPushButton("Save Modified Image", hideTab);
    saveImageButton->setEnabled(false);

    QHBoxLayout *imageButtonsLayout = new QHBoxLayout();
    imageButtonsLayout->addWidget(openImageButton);
    imageButtonsLayout->addWidget(saveImageButton);

    // Data input options
    QGroupBox *dataInputGroup = new QGroupBox("Data to Hide", hideTab);

    textInputRadio = new QRadioButton("Text Input", dataInputGroup);
    fileInputRadio = new QRadioButton("File Input", dataInputGroup);
    textInputRadio->setChecked(true);

    QHBoxLayout *radioLayout = new QHBoxLayout();
    radioLayout->addWidget(textInputRadio);
    radioLayout->addWidget(fileInputRadio);

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

    // Add encryption checkbox
    encryptionCheckBox = new QCheckBox("Encrypt data", dataInputGroup);

    hideButton = new QPushButton("Hide Data", dataInputGroup);
    hideButton->setEnabled(false);

    capacityLabel = new QLabel("Capacity: 0/0 bytes (0%)", dataInputGroup);

    QVBoxLayout *dataInputLayout = new QVBoxLayout(dataInputGroup);
    dataInputLayout->addLayout(radioLayout);
    dataInputLayout->addWidget(textInput);
    dataInputLayout->addLayout(fileInputLayout);
    dataInputLayout->addWidget(encryptionCheckBox);
    dataInputLayout->addWidget(hideButton);
    dataInputLayout->addWidget(capacityLabel);

    // Arrange Hide tab layout
    QVBoxLayout *hideTabLayout = new QVBoxLayout(hideTab);
    hideTabLayout->addLayout(previewLayout);
    hideTabLayout->addLayout(imageButtonsLayout);
    hideTabLayout->addWidget(dataInputGroup);

    // Create Extract Data tab
    QWidget *extractTab = new QWidget(tabWidget);
    tabWidget->addTab(extractTab, "Extract Data");

    // Stego image preview
    stegoImageLabel = new QLabel("Steganographic Image", extractTab);
    stegoImageLabel->setAlignment(Qt::AlignCenter);
    stegoImageLabel->setMinimumSize(320, 240);
    stegoImageLabel->setFrameShape(QFrame::Box);

    openStegoImageButton = new QPushButton("Open Image with Hidden Data", extractTab);

    // Extraction controls
    extractButton = new QPushButton("Extract Data", extractTab);
    extractButton->setEnabled(false);

    extractedTextOutput = new QPlainTextEdit(extractTab);
    extractedTextOutput->setPlaceholderText("Extracted text will appear here...");
    extractedTextOutput->setReadOnly(true);

    saveExtractedFileButton = new QPushButton("Extract and Save Data to File", extractTab);
    saveExtractedFileButton->setEnabled(false);
    saveExtractedFileButton->setToolTip(tr("Extract data from the image and save it to a file"));

    // Arrange Extract tab layout
    QVBoxLayout *extractTabLayout = new QVBoxLayout(extractTab);
    extractTabLayout->addWidget(stegoImageLabel);
    extractTabLayout->addWidget(openStegoImageButton);
    extractTabLayout->addWidget(extractButton);
    extractTabLayout->addWidget(extractedTextOutput);
    extractTabLayout->addWidget(saveExtractedFileButton);
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

    QMenu *helpMenu = menuBar()->addMenu("&Help");

    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Steganography Tool",
                           "A simple tool for hiding data in images.\n\n"
                           "Version 1.0");
    });
}

void MainWindow::createConnections()
{
    // Carrier image selection
    connect(openImageButton, &QPushButton::clicked, this, &MainWindow::openCarrierImage);
    connect(saveImageButton, &QPushButton::clicked, this, &MainWindow::saveModifiedImage);

    // Input method selection
    connect(textInputRadio, &QRadioButton::toggled, this, &MainWindow::toggleInputMethod);
    connect(browseFileButton, &QPushButton::clicked, this, &MainWindow::selectDataFile);

    // Connect encryption checkbox to update the hide button text
    connect(encryptionCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        hideButton->setText(checked ? "Hide Encrypted Data" : "Hide Data");
    });

    // Steganography operations with encryption support
    connect(hideButton, &QPushButton::clicked, [this]() {
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
        } else {
            QString filePath = filePathInput->text();
            QFile file(filePath);
            if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Could not open the selected file");
                return;
            }
            data = file.readAll();
            file.close();
        }

        // Check if encryption is enabled
        if (encryptionCheckBox->isChecked()) {
            // Show passphrase dialog
            PassphraseDialog dialog(true, this);
            if (dialog.exec() == QDialog::Accepted) {
                QString passphrase = dialog.getPassphrase();

                // Encrypt the data
                QByteArray encryptedData = encryptData(data, passphrase);
                if (encryptedData.isEmpty()) {
                    QMessageBox::warning(this, "Error", "Encryption failed");
                    return;
                }

                // Prepend the ENCR marker
                encryptedData.prepend("ENCR");

                // Check if the encrypted data will fit
                int capacity = calculateImageCapacity(originalImage);
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
                // User canceled the dialog
                return;
            }
        }

        // Check if the data will fit
        int capacity = calculateImageCapacity(originalImage);
        if (data.size() > capacity) {
            QMessageBox::warning(this, "Error",
                                 QString("Data size (%1 bytes) exceeds image capacity (%2 bytes)")
                                     .arg(data.size()).arg(capacity));
            return;
        }

        // Embed the data
        if (embedDataInImage(data)) {
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
    });

    // Extract tab connections

    connect(openStegoImageButton, &QPushButton::clicked, [this]() {
        // Use QStandardPaths for the default open location
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (defaultPath.isEmpty()) {
            defaultPath = QDir::homePath();
        }
        defaultPath += "/SteganizeMe/Images";

        QString fileName = QFileDialog::getOpenFileName(this, "Open Image with Hidden Data",
                                                        defaultPath,
                                                        "Image Files (*.png *.jpg *.bmp)");
        if (fileName.isEmpty()) {
            return;
        }

        stegoImage.load(fileName);
        if (!stegoImage.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(stegoImage);
            pixmap = pixmap.scaled(stegoImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            stegoImageLabel->setPixmap(pixmap);
            saveExtractedFileButton->setEnabled(true);
            extractButton->setEnabled(true);
            statusBar()->showMessage("Image loaded successfully: " + fileName);
        } else {
            statusBar()->showMessage("Failed to load image");
            QMessageBox::warning(this, "Error", "Failed to load the selected image");
        }
    });





    //connect(extractButton, &QPushButton::clicked, this, &MainWindow::extractData);

    connect(saveExtractedFileButton, &QPushButton::clicked, [this]() {
        // First, extract the data
        if (stegoImage.isNull()) {
            QMessageBox::warning(this, "Error", "Please load an image with hidden data first");
            return;
        }

        QByteArray extractedData = extractDataFromImage();
        if (extractedData.isEmpty()) {
            QMessageBox::warning(this, "Error", "No hidden data found or data extraction failed");
            return;
        }

        // Check if data is encrypted
        if (extractedData.startsWith("ENCR")) {
            // If we have a remembered passphrase, try it
            if (hasRememberedPassphrase) {
                QByteArray decryptedData = decryptData(extractedData.mid(4), rememberedPassphrase);
                if (!decryptedData.isEmpty()) {
                    // Use the decrypted data instead
                    extractedData = decryptedData;
                } else {
                    // If we get here, the remembered passphrase didn't work
                    hasRememberedPassphrase = false;
                    rememberedPassphrase.clear();
                    // Show passphrase dialog
                    PassphraseDialog dialog(false, this);
                    if (dialog.exec() == QDialog::Accepted) {
                        QString passphrase = dialog.getPassphrase();
                        decryptedData = decryptData(extractedData.mid(4), passphrase);
                        if (!decryptedData.isEmpty()) {
                            // Use the decrypted data instead
                            extractedData = decryptedData;
                            // Remember passphrase if requested
                            if (dialog.rememberPassphrase()) {
                                rememberedPassphrase = passphrase;
                                hasRememberedPassphrase = true;
                            }
                        } else {
                            QMessageBox::warning(this, "Error", "Decryption failed. Check your passphrase.");
                            return;
                        }
                    } else {
                        // User canceled the dialog
                        return;
                    }
                }
            } else {
                // Show passphrase dialog
                PassphraseDialog dialog(false, this);
                if (dialog.exec() == QDialog::Accepted) {
                    QString passphrase = dialog.getPassphrase();
                    QByteArray decryptedData = decryptData(extractedData.mid(4), passphrase);
                    if (!decryptedData.isEmpty()) {
                        // Use the decrypted data instead
                        extractedData = decryptedData;
                        // Remember passphrase if requested
                        if (dialog.rememberPassphrase()) {
                            rememberedPassphrase = passphrase;
                            hasRememberedPassphrase = true;
                        }
                    } else {
                        QMessageBox::warning(this, "Error", "Decryption failed. Check your passphrase.");
                        return;
                    }
                } else {
                    // User canceled the dialog
                    return;
                }
            }
        }

        // Now prompt for save location
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (defaultPath.isEmpty()) {
            defaultPath = QDir::homePath();
        }
        defaultPath += "/SteganizeMe";
        QDir().mkpath(defaultPath);

        // Create a default filename with timestamp
        QString defaultFilename = QString("%1/extracted_data_%2")
                                      .arg(defaultPath)
                                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

        // Try to determine if it's text data for appropriate extension
        bool isTextFile = true;
        QString text = QString::fromUtf8(extractedData);
        for (const QChar &c : text) {
            if (!c.isPrint() && !c.isSpace()) {
                isTextFile = false;
                break;
            }
        }
        defaultFilename += isTextFile ? ".txt" : ".bin";

        QString fileName = QFileDialog::getSaveFileName(this, "Save Extracted Data", defaultFilename);

        // If user cancels, use the default filename
        if (fileName.isEmpty()) {
            fileName = defaultFilename;
        }

        // Ensure the directory exists
        QDir().mkpath(QFileInfo(fileName).absolutePath());

        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {  // No Text flag to handle binary data
            // Write the raw binary data directly
            qint64 bytesWritten = file.write(extractedData);
            file.close();

            if (bytesWritten == extractedData.size()) {
                statusBar()->showMessage("Extracted data saved successfully to: " + fileName);

                // If it's a text file, display its content
                if (isTextFile) {
                    // Display in the text edit
                    extractedTextOutput->setPlainText(text);
                    // Show a message
                    QMessageBox::information(this, "Text File Detected",
                                             "The extracted data appears to be text and has been saved to: " + fileName);
                } else {
                    QMessageBox::information(this, "Binary File Saved",
                                             "The extracted binary data has been saved to: " + fileName);
                }
            } else {
                QMessageBox::warning(this, "Error", "Could not save all the extracted data");
            }
        } else {
            QMessageBox::warning(this, "Error", "Could not save the extracted data");
        }
    });



    // Add this connection to update capacity when text changes
    connect(textInput, &QPlainTextEdit::textChanged, this, &MainWindow::updateCapacityStatus);

    // Add this connection to update capacity when file path changes
    connect(filePathInput, &QLineEdit::textChanged, this, &MainWindow::updateCapacityStatus);
}




void MainWindow::openCarrierImage() {
    // Use QStandardPaths for the default open location
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    defaultPath += "/SteganizeMe/Images";
    QDir().mkpath(defaultPath);
    QString fileName = QFileDialog::getOpenFileName(this, "Open Carrier Image",
                                                    defaultPath,
                                                    "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        originalImage.load(fileName);
        if (!originalImage.isNull()) {
            // Convert to a format that supports alpha channel if needed
            if (originalImage.format() != QImage::Format_ARGB32 &&
                originalImage.format() != QImage::Format_RGB32) {
                originalImage = originalImage.convertToFormat(QImage::Format_ARGB32);
            }
            QPixmap pixmap = QPixmap::fromImage(originalImage);
            pixmap = pixmap.scaled(originalImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            originalImageLabel->setPixmap(pixmap);
            modifiedImageLabel->clear();
            modifiedImageLabel->setText("Modified Image (not yet created)");
            hideButton->setEnabled(true);
            saveImageButton->setEnabled(false);
            updateCapacityStatus();
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

    // Use QStandardPaths for the default save location
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (defaultPath.isEmpty()) {
        defaultPath = QDir::homePath();
    }
    defaultPath += "/SteganizeMe/Images";
    QDir().mkpath(defaultPath);

    // Create a default filename with timestamp
    QString defaultFilename = QString("%1/stego_image_%2.png")
                                  .arg(defaultPath)
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
    QDir().mkpath(QFileInfo(fileName).absolutePath());

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



void MainWindow::hideData()
{
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
    } else {
        QString filePath = filePathInput->text();
        QFile file(filePath);
        if (!file.exists()) {
            QMessageBox::warning(this, "Error", "Selected file does not exist");
            return;
        }

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error", "Could not open the selected file");
            return;
        }

        data = file.readAll();
        file.close();
    }

    // Check if encryption is enabled
    if (encryptionCheckBox->isChecked()) {
        // Show passphrase dialog
        PassphraseDialog dialog(true, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString passphrase = dialog.getPassphrase();

            // Encrypt the data
            QByteArray encryptedData = encryptData(data, passphrase);
            if (encryptedData.isEmpty()) {
                QMessageBox::warning(this, "Error", "Encryption failed");
                return;
            }

            // Prepend the ENCR marker
            encryptedData.prepend("ENCR");

            // Check if the encrypted data will fit
            int capacity = calculateImageCapacity(originalImage);
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
            // User canceled the dialog
            return;
        }
    }

    int capacity = calculateImageCapacity(originalImage);

    if (data.size() > capacity) {
        QMessageBox::warning(this, "Error",
                             QString("Data size (%1 bytes) exceeds image capacity (%2 bytes)")
                                 .arg(data.size()).arg(capacity));
        return;
    }

    if (embedDataInImage(data)) {
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

void MainWindow::updateCapacityStatus()
{
    if (originalImage.isNull()) {
        capacityLabel->setText("Capacity: 0/0 bytes (0%)");
        return;
    }

    int capacity = calculateImageCapacity(originalImage);
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

void MainWindow::toggleInputMethod()
{
    isTextInput = textInputRadio->isChecked();

    textInput->setEnabled(isTextInput);
    filePathInput->setEnabled(!isTextInput);
    browseFileButton->setEnabled(!isTextInput);

    updateCapacityStatus();
}

void MainWindow::selectDataFile() {
    // Use QStandardPaths for the default open location
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (defaultPath.isEmpty()) {
        defaultPath = QDir::homePath();
    }
    defaultPath += "/SteganizeMe";

    QString fileName = QFileDialog::getOpenFileName(this, "Select File to Hide", defaultPath);

    if (!fileName.isEmpty()) {
        filePathInput->setText(fileName);
        updateCapacityStatus();
    }
}



QByteArray MainWindow::extractDataFromImage()
{
    if (stegoImage.isNull()) {
        return QByteArray();
    }


    // First, extract the header (4 bytes for data size)
    QByteArray header(4, 0);

    // Extract header bits
    for (int byteIndex = 0; byteIndex < 4; byteIndex++) {
        header[byteIndex] = 0; // Initialize to zero
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            // Calculate pixel position
            int bitPosition = byteIndex * 8 + bitIndex;
            int pixelIndex = bitPosition / 3;
            int channelIndex = bitPosition % 3;

            int x = pixelIndex % stegoImage.width();
            int y = pixelIndex / stegoImage.width();

            QColor pixel = stegoImage.pixelColor(x, y);

            // Get the LSB from the appropriate channel
            int bit = 0;
            if (channelIndex == 0) {
                bit = pixel.red() & 1;
            } else if (channelIndex == 1) {
                bit = pixel.green() & 1;
            } else { // channelIndex == 2
                bit = pixel.blue() & 1;
            }

            // Set the bit in the header byte
            header[byteIndex] |= (bit << (7 - bitIndex));
        }
    }

    // Debug the header bytes
             << QString("0x%1").arg(static_cast<unsigned char>(header[0]), 2, 16, QChar('0'))
             << QString("0x%1").arg(static_cast<unsigned char>(header[1]), 2, 16, QChar('0'))
             << QString("0x%1").arg(static_cast<unsigned char>(header[2]), 2, 16, QChar('0'))
             << QString("0x%1").arg(static_cast<unsigned char>(header[3]), 2, 16, QChar('0'));

    // Parse data size from header
    quint32 dataSize = 0;
    dataSize = (static_cast<quint32>(static_cast<unsigned char>(header[0])) << 24) |
               (static_cast<quint32>(static_cast<unsigned char>(header[1])) << 16) |
               (static_cast<quint32>(static_cast<unsigned char>(header[2])) << 8) |
               static_cast<quint32>(static_cast<unsigned char>(header[3]));


    // Sanity check for data size
    int maxCapacity = calculateImageCapacity(stegoImage) - 4; // Subtract header size

    if (dataSize == 0) {
        return QByteArray();
    }

    if (dataSize > maxCapacity) {
        return QByteArray();
    }

    // Now extract the actual data
    QByteArray extractedData(dataSize, 0);

    // Start extracting data after the header
    int startBitPosition = 4 * 8; // Skip header bits

    for (int byteIndex = 0; byteIndex < dataSize; byteIndex++) {
        extractedData[byteIndex] = 0; // Initialize to zero
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            // Calculate bit position in the overall sequence
            int bitPosition = startBitPosition + byteIndex * 8 + bitIndex;

            // Calculate pixel position
            int pixelIndex = bitPosition / 3;
            int channelIndex = bitPosition % 3;

            int x = pixelIndex % stegoImage.width();
            int y = pixelIndex / stegoImage.width();

            QColor pixel = stegoImage.pixelColor(x, y);

            // Get the LSB from the appropriate channel
            int bit = 0;
            if (channelIndex == 0) {
                bit = pixel.red() & 1;
            } else if (channelIndex == 1) {
                bit = pixel.green() & 1;
            } else { // channelIndex == 2
                bit = pixel.blue() & 1;
            }

            // Set the bit in the data byte
            extractedData[byteIndex] |= (bit << (7 - bitIndex));
        }
    }


    // Debug the first few bytes of extracted data
    if (!extractedData.isEmpty()) {
        QString hexPreview;
        for (int i = 0; i < qMin(static_cast<int>(dataSize), 16); i++) {
            hexPreview += QString("0x%1 ").arg(static_cast<unsigned char>(extractedData[i]), 2, 16, QChar('0'));
        }

        // Debug the last character specifically
        if (dataSize > 0) {
            unsigned char lastChar = static_cast<unsigned char>(extractedData[dataSize - 1]);
                     << "ASCII:" << (lastChar >= 32 && lastChar < 127 ? QChar(lastChar) : QChar('?'));
        }
    }

    return extractedData;
}

// Implement the steganography methods
bool MainWindow::embedDataInImage(const QByteArray &data)
{
    if (originalImage.isNull() || data.isEmpty()) {
        return false;
    }

             << originalImage.width() << "x" << originalImage.height();

    // Debug the data being embedded
    QString hexPreview;
    for (int i = 0; i < qMin(data.size(), static_cast<qsizetype>(16)); i++) {
        hexPreview += QString("0x%1 ").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0'));
    }

    // Debug the last character specifically
    if (data.size() > 0) {
        unsigned char lastChar = static_cast<unsigned char>(data[data.size() - 1]);
                 << "ASCII:" << (lastChar >= 32 && lastChar < 127 ? QChar(lastChar) : QChar('?'));
    }

    // Create a copy of the original image to modify
    modifiedImage = originalImage.copy();

    // Prepare header: 4 bytes for data size
    QByteArray header(4, 0);
    quint32 dataSize = data.size();
    header[0] = (dataSize >> 24) & 0xFF;
    header[1] = (dataSize >> 16) & 0xFF;
    header[2] = (dataSize >> 8) & 0xFF;
    header[3] = dataSize & 0xFF;

             << QString("0x%1").arg(static_cast<unsigned char>(header[0]), 2, 16, QChar('0'))
             << QString("0x%1").arg(static_cast<unsigned char>(header[1]), 2, 16, QChar('0'))
             << QString("0x%1").arg(static_cast<unsigned char>(header[2]), 2, 16, QChar('0'))
             << QString("0x%1").arg(static_cast<unsigned char>(header[3]), 2, 16, QChar('0'));

    // Combine header and data
    QByteArray fullData = header + data;

    // Check if the image can hold the data
    int capacity = calculateImageCapacity(originalImage);

    if (fullData.size() > capacity) {
        return false;
    }

    // Embed data using LSB (Least Significant Bit) method
    for (int byteIndex = 0; byteIndex < fullData.size(); byteIndex++) {
        unsigned char currentByte = static_cast<unsigned char>(fullData[byteIndex]);

        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            // Calculate bit position in the overall sequence
            int bitPosition = byteIndex * 8 + bitIndex;

            // Calculate pixel position
            int pixelIndex = bitPosition / 3;
            int channelIndex = bitPosition % 3;

            int x = pixelIndex % modifiedImage.width();
            int y = pixelIndex / modifiedImage.width();

            // Get the bit to embed
            int bit = (currentByte >> (7 - bitIndex)) & 1;

            QColor pixel = modifiedImage.pixelColor(x, y);

            // Embed the bit in the appropriate channel
            if (channelIndex == 0) {
                // Red channel
                int r = (pixel.red() & 0xFE) | bit;
                pixel.setRed(r);
            } else if (channelIndex == 1) {
                // Green channel
                int g = (pixel.green() & 0xFE) | bit;
                pixel.setGreen(g);
            } else { // channelIndex == 2
                // Blue channel
                int b = (pixel.blue() & 0xFE) | bit;
                pixel.setBlue(b);
            }

            modifiedImage.setPixelColor(x, y, pixel);
        }
    }

    return true;
}

int MainWindow::calculateImageCapacity(const QImage &image)
{
    if (image.isNull()) {
        return 0;
    }

    // Each pixel can store 3 bits (one in each RGB channel)
    // We need 4 bytes (32 bits) for the header
    int totalBits = image.width() * image.height() * 3;
    int dataBits = totalBits - 32; // Subtract header size

    // Convert bits to bytes (integer division)
    return dataBits / 8;
}

QByteArray MainWindow::encryptData(const QByteArray &data, const QString &passphrase)
{
    if (data.isEmpty() || passphrase.isEmpty()) {
        return QByteArray();
    }

    // Generate a random salt (16 bytes)
    QByteArray salt;
    salt.resize(16);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(salt.data()), salt.size() / 4);

    // Generate key and IV from passphrase and salt using PBKDF2
    QByteArray key(32, 0); // 256 bits
    QByteArray iv(16, 0);  // 128 bits

    // Using OpenSSL for PBKDF2
    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.toUtf8().constData(),
        passphrase.toUtf8().length(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.length(),
        10000, // iterations
        key.length(),
        reinterpret_cast<unsigned char*>(key.data())
        );

    // Generate IV (can also use PBKDF2 with different salt)
    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.toUtf8().constData(),
        passphrase.toUtf8().length(),
        reinterpret_cast<const unsigned char*>(salt.constData()) + 8, // Use part of salt
        8,
        5000, // fewer iterations for IV
        iv.length(),
        reinterpret_cast<unsigned char*>(iv.data())
        );

    // Set up encryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                       reinterpret_cast<const unsigned char*>(key.constData()),
                       reinterpret_cast<const unsigned char*>(iv.constData()));

    // Encrypt the data
    QByteArray encryptedData;
    encryptedData.resize(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outlen1 = 0, outlen2 = 0;

    EVP_EncryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(encryptedData.data()),
                      &outlen1,
                      reinterpret_cast<const unsigned char*>(data.constData()),
                      data.length());

    EVP_EncryptFinal_ex(ctx,
                        reinterpret_cast<unsigned char*>(encryptedData.data()) + outlen1,
                        &outlen2);

    EVP_CIPHER_CTX_free(ctx);

    encryptedData.resize(outlen1 + outlen2);

    // Format: [salt][iv][encrypted data]
    return salt + iv + encryptedData;
}

QByteArray MainWindow::decryptData(const QByteArray &encryptedData, const QString &passphrase)
{
    if (encryptedData.size() <= 32 || passphrase.isEmpty()) { // 16 (salt) + 16 (IV)
        return QByteArray();
    }

    // Extract salt and IV
    QByteArray salt = encryptedData.left(16);
    QByteArray iv = encryptedData.mid(16, 16);
    QByteArray ciphertext = encryptedData.mid(32);

    // Generate key from passphrase and salt
    QByteArray key(32, 0); // 256 bits

    PKCS5_PBKDF2_HMAC_SHA1(
        passphrase.toUtf8().constData(),
        passphrase.toUtf8().length(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.length(),
        10000, // iterations
        key.length(),
        reinterpret_cast<unsigned char*>(key.data())
        );

    // Set up decryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                       reinterpret_cast<const unsigned char*>(key.constData()),
                       reinterpret_cast<const unsigned char*>(iv.constData()));

    // Decrypt the data
    QByteArray decryptedData;
    decryptedData.resize(ciphertext.size());
    int outlen1 = 0, outlen2 = 0;

    EVP_DecryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(decryptedData.data()),
                      &outlen1,
                      reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                      ciphertext.length());

    // Check if final decryption was successful
    int result = EVP_DecryptFinal_ex(ctx,
                                     reinterpret_cast<unsigned char*>(decryptedData.data()) + outlen1,
                                     &outlen2);

    EVP_CIPHER_CTX_free(ctx);

    // If decryption failed (wrong passphrase), return empty array
    if (result != 1) {
        return QByteArray();
    }

    decryptedData.resize(outlen1 + outlen2);
    return decryptedData;
}


