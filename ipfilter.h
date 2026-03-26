#ifndef IPFILTER_H
#define IPFILTER_H

#include <QObject>
#include <QDialog>
#include <QStringList>
#include <QHostAddress>
#include <QPair>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

class ICMPStegEngine;

class IpFilter : public QObject
{
    Q_OBJECT
public:
    enum Mode { BlockMode, AllowMode };

    explicit IpFilter(QObject* parent = nullptr);

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void setMode(Mode mode) { m_mode = mode; }
    Mode mode() const { return m_mode; }

    void setEntries(const QStringList& entries);
    QStringList getEntries() const { return m_entries; }

    bool isAllowed(const QString& ipOrDomain) const;

    static QList<QHostAddress> domainToIPv4(const QString& domain);

private:
    bool m_enabled = false;
    Mode m_mode = BlockMode;
    QStringList m_entries;

    QList<QHostAddress> m_ipEntries;
    QList<QPair<QHostAddress, quint8>> m_cidrEntries;
    QStringList m_domainEntries;

    void parseEntries();
    bool matches(const QString& ipOrDomain) const;
    bool cidrMatches(const QHostAddress& ip, const QHostAddress& network, quint8 mask) const;
};

class IpFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IpFilterDialog(IpFilter* filter, QWidget* parent = nullptr);

private slots:
    void onSave();
    void onAddEntry();
    void onRemoveEntry();
    void onSelectionChanged();
    void onResolveDomain();
    void onTestIp();

private:
    void loadSettings();

    IpFilter* m_filter = nullptr;

    QCheckBox* m_enableFilterCheckBox;
    QRadioButton* m_blockModeRadio;
    QRadioButton* m_allowModeRadio;
    QListWidget* m_entriesList;
    QLineEdit* m_newEntryEdit;
    QPushButton* m_addBtn;
    QPushButton* m_removeBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_resolveBtn;
    QPushButton* m_testIpBtn;


};

#endif
