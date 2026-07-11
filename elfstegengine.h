#ifndef ELFSTEGENGINE_H
#define ELFSTEGENGINE_H

#include <QObject>
#include <QByteArray>
#include <QString>

class ELFStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit ELFStegEngine(QObject *parent = nullptr);
    ~ELFStegEngine();

    // Core operations
    bool embedDataInELF(const QString &carrierPath, const QString &outputPath,
                        const QByteArray &data, const QString &sectionName = ".debug_types");
    QByteArray extractDataFromELF(const QString &stegoPath, const QString &sectionName = ".debug_types");

    // Capacity calculation
    qint64 calculateELFCapacity(const QString &elfPath);

    // Error handling
    QString lastError() const { return m_lastError; }

signals:
    void progressUpdated(int current, int total);
    void errorOccurred(const QString &error);
    void embedSuccess();

private:
    bool createDebugTypesSection(const QString &inputPath, const QString &outputPath,
                                 const QByteArray &data, const QString &sectionName);
    QByteArray readDebugTypesSection(const QString &elfPath, const QString &sectionName);

    QString m_lastError;
    void setError(const QString &error) { m_lastError = error; emit errorOccurred(error); }
};

#endif // ELFSTEGENGINE_H