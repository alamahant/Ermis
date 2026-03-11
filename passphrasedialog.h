#ifndef PASSPHRASEDIALOG_H
#define PASSPHRASEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class PassphraseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PassphraseDialog(bool isEncryption, QWidget *parent = nullptr);
    QString getPassphrase() const;
    bool rememberPassphrase() const;

private:
    QLineEdit *passphraseEdit;
    QLineEdit *confirmPassphraseEdit;
    QCheckBox *rememberCheckBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
    bool isEncryptionMode;
};

#endif // PASSPHRASEDIALOG_H
