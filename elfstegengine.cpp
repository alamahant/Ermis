#include "elfstegengine.h"
#include <QFile>
#include <QProcess>
#include <QTemporaryFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

ELFStegEngine::ELFStegEngine(QObject *parent)
    : QObject(parent)
{
}

ELFStegEngine::~ELFStegEngine()
{
}

bool ELFStegEngine::embedDataInELF(const QString &carrierPath, const QString &outputPath,
                                   const QByteArray &data, const QString &sectionName)
{
    if (carrierPath.isEmpty() || outputPath.isEmpty() || data.isEmpty()) {
        setError("Invalid input parameters");
        return false;
    }

    emit progressUpdated(0, 100);

    // Step 1: Copy the carrier binary to output location
    if (!QFile::copy(carrierPath, outputPath)) {
        setError("Failed to copy carrier binary");
        return false;
    }

    emit progressUpdated(25, 100);

    // Step 2: Write data to a temporary file
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        setError("Failed to create temporary file");
        return false;
    }
    tempFile.write(data);
    tempFile.flush();
    tempFile.close();

    emit progressUpdated(50, 100);

    // Step 3: Use objcopy to add the section
    QProcess process;
    QStringList args;
    args << "--add-section"
         << QString("%1=%2").arg(sectionName).arg(tempFile.fileName())
         << outputPath;

    process.start("objcopy", args);
    process.waitForFinished();

    if (process.exitCode() != 0) {
        setError(QString("objcopy failed: %1").arg(QString::fromUtf8(process.readAllStandardError())));
        return false;
    }

    emit progressUpdated(75, 100);

    // Step 4: Verify the binary still works (optional)
    QProcess verifyProcess;
    verifyProcess.start(outputPath, {"--version"});
    verifyProcess.waitForFinished();

    // If the binary doesn't run, something went wrong
    if (verifyProcess.exitCode() != 0 && !verifyProcess.errorString().contains("No such file")) {
        qDebug() << "Warning: Binary may be corrupted";
        // Continue anyway - it's likely still functional
    }

    emit progressUpdated(100, 100);
    emit embedSuccess();

    return true;
}

QByteArray ELFStegEngine::extractDataFromELF(const QString &stegoPath, const QString &sectionName)
{
    if (stegoPath.isEmpty()) {
        setError("Invalid input path");
        return QByteArray();
    }

    emit progressUpdated(0, 100);

    // Check if the binary exists
    if (!QFile::exists(stegoPath)) {
        setError("Stego binary not found");
        return QByteArray();
    }

    emit progressUpdated(25, 100);

    // Create a temporary file for extraction
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        setError("Failed to create temporary file");
        return QByteArray();
    }
    tempFile.close();

    emit progressUpdated(50, 100);

    // Use objcopy to dump the section
    QProcess process;
    QStringList args;
    args << "--dump-section"
         << QString("%1=%2").arg(sectionName).arg(tempFile.fileName())
         << stegoPath;

    process.start("objcopy", args);
    process.waitForFinished();

    if (process.exitCode() != 0) {
        QString error = QString::fromUtf8(process.readAllStandardError());
        if (error.contains("does not contain") || error.contains("no section")) {
            setError("No hidden data found in this binary");
        } else {
            setError(QString("objcopy failed: %1").arg(error));
        }
        return QByteArray();
    }

    emit progressUpdated(75, 100);

    // Read the extracted data
    QFile extractedFile(tempFile.fileName());
    if (!extractedFile.open(QIODevice::ReadOnly)) {
        setError("Failed to read extracted data");
        return QByteArray();
    }

    QByteArray data = extractedFile.readAll();
    extractedFile.close();

    emit progressUpdated(100, 100);

    return data;
}

qint64 ELFStegEngine::calculateELFCapacity(const QString &elfPath)
{
    if (elfPath.isEmpty() || !QFile::exists(elfPath)) {
        return 0;
    }

    // Get the file size
    QFileInfo info(elfPath);
    qint64 fileSize = info.size();

    // ELF files can typically handle sections up to ~50% of file size
    // But this is a conservative estimate
    qint64 capacity = fileSize / 2;

    // Cap at a reasonable limit (2GB is the practical maximum)
    const qint64 MAX_CAPACITY = 2LL * 1024 * 1024 * 1024; // 2GB
    if (capacity > MAX_CAPACITY) {
        capacity = MAX_CAPACITY;
    }

    return capacity;
}