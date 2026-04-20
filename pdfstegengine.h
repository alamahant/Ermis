#ifndef PDFSTEGENGINE_H
#define PDFSTEGENGINE_H

#include <QObject>
#include <QByteArray>
#include <QString>

class PDFStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit PDFStegEngine(QObject *parent = nullptr);
    ~PDFStegEngine();

    // Main public methods (matching StegEngine pattern)
    bool embedDataInPDF(const QString &inputPath, 
                        const QString &outputPath,
                        const QByteArray &data);
    
    QByteArray extractDataFromPDF(const QString &pdfPath);
    
    // Validation and utilities
    bool isValidPDF(const QString &pdfPath) const;
    QString lastError() const { return m_lastError; }
    
    // Capacity calculation
    qint64 calculatePDFCapacity(const QString &pdfPath) const;


signals:
    void progressUpdated(int current, int total);
    void errorOccurred(const QString &error);
    void embedSuccess();
    void extractSuccess(const QByteArray &data);

private:
    QString m_lastError;
    
    // PDF parsing helpers
    int findInsertionPoint(const QByteArray &pdfData);
    int findHighestObjectNumber(const QByteArray &pdfData);
    int findXrefPosition(const QByteArray &pdfData);
    
    // Object stream creation
    QByteArray createObjectStream(int objNum, const QByteArray &data);
    QByteArray createObjectStreamWithReference(int objNum, int refObjNum, const QByteArray &data);
    QByteArray findHiddenObjectStream(const QByteArray &pdfData);
    
    // Helper methods
    bool isPDFCorrupted(const QByteArray &pdfData) const;
    QByteArray prepareDataForEmbedding(const QByteArray &data);
    QByteArray extractDataFromStream(const QByteArray &streamData);
    
    // Constants
    static const int HEADER_SIZE = 8;  // 8 bytes header for our format
    static const int DEFAULT_OBJ_NUM = 999999; // High number to avoid conflicts
};

#endif // PDFSTEGENGINE_H
