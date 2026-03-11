

#include "passphrasedialog.h"
#include <QFormLayout>
#include <QMessageBox>

PassphraseDialog::PassphraseDialog(bool isEncryption, QWidget *parent)
    : QDialog(parent)
    , isEncryptionMode(isEncryption)
{
    setWindowTitle(isEncryption ? "Encryption Passphrase" : "Decryption Passphrase");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Add icon and message
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap(":/icons/lock.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QLabel *messageLabel = new QLabel(isEncryption ?
                                          "Please enter a passphrase to encrypt your data. Remember this passphrase as you will need it to decrypt the data later." :
                                          "This image contains encrypted data. Please enter the passphrase to decrypt it.", this);
    messageLabel->setWordWrap(true);

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(messageLabel, 1);
    mainLayout->addLayout(headerLayout);

    // Add form for passphrase entry
    QFormLayout *formLayout = new QFormLayout();

    passphraseEdit = new QLineEdit(this);
    passphraseEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Passphrase:", passphraseEdit);

    // Only show confirmation for encryption
    if (isEncryption) {
        confirmPassphraseEdit = new QLineEdit(this);
        confirmPassphraseEdit->setEchoMode(QLineEdit::Password);
        formLayout->addRow("Confirm passphrase:", confirmPassphraseEdit);
    } else {
        confirmPassphraseEdit = nullptr;
    }

    rememberCheckBox = new QCheckBox("Remember passphrase for this session", this);
    formLayout->addRow("", rememberCheckBox);

    mainLayout->addLayout(formLayout);

    // Add buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(okButton, &QPushButton::clicked, this, [this]() {
        if (isEncryptionMode && confirmPassphraseEdit) {
            if (passphraseEdit->text().isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter a passphrase");
                return;
            }

            if (passphraseEdit->text() != confirmPassphraseEdit->text()) {
                QMessageBox::warning(this, "Error", "Passphrases do not match");
                return;
            }
        } else if (passphraseEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a passphrase");
            return;
        }

        accept();
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Set focus to passphrase field
    passphraseEdit->setFocus();

    // Set size
    resize(400, sizeHint().height());
}

QString PassphraseDialog::getPassphrase() const
{
    return passphraseEdit->text();
}

bool PassphraseDialog::rememberPassphrase() const
{
    return rememberCheckBox->isChecked();
}

