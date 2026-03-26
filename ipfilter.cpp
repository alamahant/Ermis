#include "ipfilter.h"
#include <QHostInfo>
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>
#include"constants.h"
#include "icmpstegengine.h"

///////// IpFilter /////////

IpFilter::IpFilter(QObject* parent)
    : QObject(parent)
{
}

void IpFilter::setEntries(const QStringList& entries)
{
    m_entries = entries;
    parseEntries();
}

void IpFilter::parseEntries()
{
    m_ipEntries.clear();
    m_cidrEntries.clear();
    m_domainEntries.clear();

    for (const QString& entry : m_entries) {
        if (entry.contains("/")) {
            QStringList parts = entry.split("/");
            if (parts.size() == 2) {
                QHostAddress network(parts[0]);
                bool ok;
                quint8 mask = parts[1].toUShort(&ok);
                if (!network.isNull() && ok && mask <= 32) {
                    m_cidrEntries.append(qMakePair(network, mask));
                }
            }
        } else {
            QHostAddress ip(entry);
            if (!ip.isNull()) {
                m_ipEntries.append(ip);
            } else {
                m_domainEntries.append(entry);
            }
        }
    }
}

bool IpFilter::cidrMatches(const QHostAddress& ip, const QHostAddress& network, quint8 mask) const
{
    if (ip.protocol() != QAbstractSocket::IPv4Protocol ||
        network.protocol() != QAbstractSocket::IPv4Protocol)
        return false;

    quint32 ipInt = ip.toIPv4Address();
    quint32 netInt = network.toIPv4Address();
    quint32 maskInt = mask == 0 ? 0 : 0xFFFFFFFF << (32 - mask);
    return (ipInt & maskInt) == (netInt & maskInt);
}

bool IpFilter::matches(const QString& ipOrDomain) const
{
    QHostAddress addr(ipOrDomain);

    if (!addr.isNull()) {
        if (m_ipEntries.contains(addr)) return true;

        for (const auto& c : m_cidrEntries) {
            if (cidrMatches(addr, c.first, c.second)) return true;
        }
    }

    if (m_domainEntries.contains(ipOrDomain, Qt::CaseInsensitive)) return true;

    return false;
}

bool IpFilter::isAllowed(const QString& ipOrDomain) const
{
    if (!m_enabled) return true;

    bool inList = matches(ipOrDomain);

    if (m_mode == BlockMode) {
        return !inList;
    } else {
        return inList;
    }
}

QList<QHostAddress> IpFilter::domainToIPv4(const QString& domain)
{
    QList<QHostAddress> ipv4Addresses;

    QHostInfo hostInfo = QHostInfo::fromName(domain);
    if (hostInfo.error() == QHostInfo::NoError) {
        const auto& addresses = hostInfo.addresses();
        for (const QHostAddress& addr : addresses) {
            if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                ipv4Addresses.append(addr);
            }
        }
    }

    return ipv4Addresses;
}

///////// IpFilterDialog /////////

IpFilterDialog::IpFilterDialog(IpFilter* filter, QWidget* parent)
    : QDialog(parent), m_filter(filter)
{
    setWindowTitle("IP Filter Settings");
    setModal(true);
    setMinimumWidth(500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Enable filter checkbox
    QHBoxLayout* enableLayout = new QHBoxLayout();
    m_enableFilterCheckBox = new QCheckBox("Enable IP Filter", this);
    enableLayout->addWidget(m_enableFilterCheckBox);
    enableLayout->addStretch();
    mainLayout->addLayout(enableLayout);

    // Separator
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(line);

    // Mode selection
    QHBoxLayout* modeLayout = new QHBoxLayout();
    m_blockModeRadio = new QRadioButton("Block these IPs", this);
    m_allowModeRadio = new QRadioButton("Only allow these IPs", this);
    m_blockModeRadio->setChecked(true);
    modeLayout->addWidget(m_blockModeRadio);
    modeLayout->addWidget(m_allowModeRadio);
    modeLayout->addStretch();
    mainLayout->addLayout(modeLayout);

    // Description
    QLabel* infoLabel = new QLabel(
        "Add IP addresses, CIDR ranges, or domains.\n"
        "Block mode: listed items are blocked.\n"
        "Allow mode: only listed items are allowed.",
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: gray; font-size: 9pt; }");
    mainLayout->addWidget(infoLabel);

    // Entries list
    m_entriesList = new QListWidget(this);
    m_entriesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_entriesList->setMinimumHeight(200);
    mainLayout->addWidget(m_entriesList);

    // Add entry
    QHBoxLayout* addLayout = new QHBoxLayout();
    m_newEntryEdit = new QLineEdit(this);
    m_newEntryEdit->setPlaceholderText("Enter IP, domain, or CIDR (e.g., 192.168.1.1, example.com, 10.0.0.0/24)");
    m_addBtn = new QPushButton("Add", this);
    addLayout->addWidget(m_newEntryEdit);
    addLayout->addWidget(m_addBtn);
    mainLayout->addLayout(addLayout);

    // Remove button
    QHBoxLayout* removeLayout = new QHBoxLayout();
    m_removeBtn = new QPushButton("Remove Selected", this);
    removeLayout->addWidget(m_removeBtn);
    removeLayout->addStretch();
    mainLayout->addLayout(removeLayout);

    // Buttons for extra features
    QHBoxLayout* extraLayout = new QHBoxLayout();
    m_resolveBtn = new QPushButton("Resolve Domain", this);
    m_testIpBtn = new QPushButton("Test IP", this);
    extraLayout->addWidget(m_resolveBtn);
    extraLayout->addWidget(m_testIpBtn);
    extraLayout->addStretch();
    mainLayout->addLayout(extraLayout);

    // Save button
    m_saveBtn = new QPushButton("Save", this);
    m_saveBtn->setMinimumHeight(35);
    mainLayout->addWidget(m_saveBtn);

    // Load settings
    loadSettings();

    // Connect signals
    connect(m_addBtn, &QPushButton::clicked, this, &IpFilterDialog::onAddEntry);
    connect(m_removeBtn, &QPushButton::clicked, this, &IpFilterDialog::onRemoveEntry);
    connect(m_entriesList, &QListWidget::itemSelectionChanged, this, &IpFilterDialog::onSelectionChanged);
    connect(m_saveBtn, &QPushButton::clicked, this, &IpFilterDialog::onSave);
    // Add these two lines
    connect(m_resolveBtn, &QPushButton::clicked, this, &IpFilterDialog::onResolveDomain);
    connect(m_testIpBtn, &QPushButton::clicked, this, &IpFilterDialog::onTestIp);
    onSelectionChanged();
}

void IpFilterDialog::loadSettings()
{
    QSettings settings;

    bool enabled = settings.value("IPFilterEnabled", false).toBool();
    m_enableFilterCheckBox->setChecked(enabled);

    int mode = settings.value("IPFilterMode", 0).toInt();
    if (mode == 0) {
        m_blockModeRadio->setChecked(true);
    } else {
        m_allowModeRadio->setChecked(true);
    }

    QStringList entries = settings.value("IPFilterEntries").toStringList();
    for (const QString& entry : entries) {
        m_entriesList->addItem(entry);
    }
}

void IpFilterDialog::onAddEntry()
{
    QString entry = m_newEntryEdit->text().trimmed();
    if (entry.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter an IP, domain, or CIDR.");
        return;
    }

    for (int i = 0; i < m_entriesList->count(); ++i) {
        if (m_entriesList->item(i)->text().compare(entry, Qt::CaseInsensitive) == 0) {
            QMessageBox::warning(this, "Warning", "This entry already exists.");
            return;
        }
    }

    m_entriesList->addItem(entry);
    m_newEntryEdit->clear();
}

void IpFilterDialog::onRemoveEntry()
{
    qDeleteAll(m_entriesList->selectedItems());
}

void IpFilterDialog::onSelectionChanged()
{
    m_removeBtn->setEnabled(!m_entriesList->selectedItems().isEmpty());
}

void IpFilterDialog::onSave()
{
    QStringList entries;
    for (int i = 0; i < m_entriesList->count(); ++i) {
        entries << m_entriesList->item(i)->text();
    }

    bool enabled = m_enableFilterCheckBox->isChecked();
    bool isBlockMode = m_blockModeRadio->isChecked();

    m_filter->setEnabled(enabled);
    m_filter->setMode(isBlockMode ? IpFilter::BlockMode : IpFilter::AllowMode);
    m_filter->setEntries(entries);



    // Update constants for engine
    Constants::ipFilterEnabled = enabled;
    Constants::ipFilterMode = isBlockMode ? 0 : 1;
    Constants::ipFilterEntries = entries;

    QSettings settings;
    settings.setValue("IPFilterEnabled", enabled);
    settings.setValue("IPFilterMode", isBlockMode ? 0 : 1);
    settings.setValue("IPFilterEntries", entries);



    QMessageBox::information(this, "Saved", "IP filter updated.");
    accept();
}

void IpFilterDialog::onResolveDomain()
{
    auto selectedItems = m_entriesList->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a domain to resolve.");
        return;
    }

    QString domain = selectedItems.first()->text().trimmed();
    if (domain.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Selected item is empty.");
        return;
    }

    // Check if it's already an IP address
    QHostAddress ip(domain);
    if (!ip.isNull()) {
        QMessageBox::information(this, "Resolve Domain",
            "Selected item is an IP address, not a domain.");
        return;
    }

    // Check if it's a CIDR range
    if (domain.contains("/")) {
        QMessageBox::information(this, "Resolve Domain",
            "Selected item is a CIDR range, not a domain.");
        return;
    }

    // Resolve the domain
    QHostInfo hostInfo = QHostInfo::fromName(domain);

    if (hostInfo.error() != QHostInfo::NoError) {
        QMessageBox::warning(this, "Resolve Domain",
            QString("Failed to resolve '%1':\n%2")
            .arg(domain)
            .arg(hostInfo.errorString()));
        return;
    }

    QList<QHostAddress> addresses = hostInfo.addresses();
    if (addresses.isEmpty()) {
        QMessageBox::information(this, "Resolve Domain",
            QString("No IP addresses found for '%1'").arg(domain));
        return;
    }

    // Separate IPv4 and IPv6
    QStringList ipv4List;
    QStringList ipv6List;

    for (const QHostAddress& addr : addresses) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
            ipv4List << addr.toString();
        } else if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
            ipv6List << addr.toString();
        }
    }

    QString result;
    if (!ipv4List.isEmpty()) {
        result += "IPv4:\n" + ipv4List.join("\n");
    }
    if (!ipv6List.isEmpty()) {
        if (!result.isEmpty()) result += "\n\n";
        result += "IPv6:\n" + ipv6List.join("\n");
    }
    if (result.isEmpty()) {
        result = "No IPv4 or IPv6 addresses found.";
    }

    QMessageBox::information(this, "Resolve Domain",
        QString("Domain: %1\n\n%2").arg(domain).arg(result));
}

void IpFilterDialog::onTestIp()
{
    bool ok;
    QString ipStr = QInputDialog::getText(this, "Test IP",
        "Enter IP address to test:", QLineEdit::Normal, "", &ok);

    if (!ok || ipStr.isEmpty()) return;

    QHostAddress testIp(ipStr);
    if (testIp.isNull()) {
        QMessageBox::warning(this, "Warning", "Invalid IP address.");
        return;
    }

    // Check if filter is enabled
    if (!m_filter->isEnabled()) {
        QMessageBox::information(this, "Test Result",
            "Filter is currently disabled.\n"
            "All IPs are allowed.");
        return;
    }

    bool allowed = m_filter->isAllowed(ipStr);

    QString mode = (m_filter->mode() == IpFilter::BlockMode) ? "block mode" : "allow mode";
    QString result = allowed ? "ALLOWED" : "BLOCKED";

    QMessageBox::information(this, "Test Result",
        QString("IP %1 is %2\n\n"
                "Filter: %3\n"
                "Mode: %4\n"
                "Entries: %5")
        .arg(ipStr)
        .arg(result)
        .arg(m_filter->isEnabled() ? "Enabled" : "Disabled")
        .arg(mode)
        .arg(m_filter->getEntries().isEmpty() ? "(empty)" :
             QString::number(m_filter->getEntries().size()) + " entries"));
}
