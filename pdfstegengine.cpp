#include "pdfstegengine.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>

PDFStegEngine::PDFStegEngine(QObject *parent) : QObject(parent)
{
}

PDFStegEngine::~PDFStegEngine()
{
}



bool PDFStegEngine::embedDataInPDF(const QString &inputPath,
                                    const QString &outputPath,
                                    const QByteArray &data)
{

    emit progressUpdated(0, 100);

    if (data.isEmpty()) {
        m_lastError = "No data to embed";
        emit errorOccurred(m_lastError);
        return false;
    }

    emit progressUpdated(10, 100);

    // Read input PDF
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Cannot open input PDF: %1").arg(inputFile.errorString());
        emit errorOccurred(m_lastError);
        return false;
    }

    QByteArray pdfData = inputFile.readAll();
    inputFile.close();

    if (pdfData.isEmpty()) {
        m_lastError = "Input PDF is empty";
        emit errorOccurred(m_lastError);
        return false;
    }

    emit progressUpdated(20, 100);

    // Validate PDF
    if (!pdfData.startsWith("%PDF")) {
        m_lastError = "File does not appear to be a valid PDF";
        emit errorOccurred(m_lastError);
        return false;
    }

    // Find insertion point (before %%EOF)
    int eofPos = findInsertionPoint(pdfData);
    if (eofPos == -1) {
        m_lastError = "Cannot find %%EOF marker in PDF";
        emit errorOccurred(m_lastError);
        return false;
    }


    emit progressUpdated(30, 100);

    // Find highest object number to avoid conflicts
    int maxObjNum = findHighestObjectNumber(pdfData);
    int newObjNum = maxObjNum + 1;

    emit progressUpdated(40, 100);

    // Prepare data for embedding
    QByteArray preparedData = prepareDataForEmbedding(data);

    emit progressUpdated(50, 100);

    // Create object stream
    QByteArray objectStream = createObjectStream(newObjNum, preparedData);

    emit progressUpdated(60, 100);

    // Check capacity
    qint64 capacity = calculatePDFCapacity(inputPath);
    if (preparedData.size() > capacity) {
        m_lastError = QString("Data too large. Capacity: %1 bytes, Need: %2 bytes")
                      .arg(capacity).arg(preparedData.size());
        emit errorOccurred(m_lastError);
        return false;
    }

    emit progressUpdated(70, 100);

    // Insert object stream before %%EOF
    pdfData.insert(eofPos, objectStream);

    emit progressUpdated(80, 100);

    // Write output PDF
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        m_lastError = QString("Cannot write output PDF: %1").arg(outputFile.errorString());
        emit errorOccurred(m_lastError);
        return false;
    }

    outputFile.write(pdfData);
    outputFile.close();

    emit progressUpdated(100, 100);

    emit embedSuccess();
    return true;
}

QByteArray PDFStegEngine::extractDataFromPDF(const QString &pdfPath)
{

    emit progressUpdated(0, 100);

    QFile file(pdfPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Cannot open PDF: %1").arg(file.errorString());
        emit errorOccurred(m_lastError);
        return QByteArray();
    }

    emit progressUpdated(20, 100);

    QByteArray pdfData = file.readAll();
    file.close();

    if (pdfData.isEmpty()) {
        m_lastError = "PDF is empty";
        emit errorOccurred(m_lastError);
        return QByteArray();
    }

    emit progressUpdated(40, 100);

    // Search for 'PDFS' magic bytes directly in the whole PDF
    int magicPos = pdfData.indexOf("PDFS");
    if (magicPos == -1) {
        m_lastError = "No hidden data found in PDF";
        emit errorOccurred(m_lastError);
        return QByteArray();
    }

    // The header (original size) is 4 bytes before 'PDFS'
    if (magicPos < 4) {
        m_lastError = "Invalid data format";
        emit errorOccurred(m_lastError);
        return QByteArray();
    }

    // Extract original size
    QByteArray header = pdfData.mid(magicPos - 4, 4);
    quint32 originalSize = (static_cast<quint32>(static_cast<unsigned char>(header[0])) << 24) |
                           (static_cast<quint32>(static_cast<unsigned char>(header[1])) << 16) |
                           (static_cast<quint32>(static_cast<unsigned char>(header[2])) << 8) |
                           static_cast<quint32>(static_cast<unsigned char>(header[3]));


    // Find the compressed data - it starts after 'PDFS' and goes until corrupted by PDF
    // Since I use qUncompress, it will stop at the correct end
    QByteArray compressed = pdfData.mid(magicPos + 4);

    emit progressUpdated(60, 100);

    QByteArray extractedData = qUncompress(compressed);

    if (extractedData.isEmpty()) {
        // Try with a smaller chunk - maybe captured too much
        compressed = pdfData.mid(magicPos + 4, 10000); // Arbitrary limit
        extractedData = qUncompress(compressed);
    }

    if (extractedData.isEmpty()) {
        m_lastError = "Failed to extract data";
        emit errorOccurred(m_lastError);
        return QByteArray();
    }

    emit progressUpdated(90, 100);


    emit progressUpdated(100, 100);
    emit extractSuccess(extractedData);

    return extractedData;
}

bool PDFStegEngine::isValidPDF(const QString &pdfPath) const
{
    QFile file(pdfPath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    
    QByteArray header = file.read(8);
    file.close();
    
    return header.startsWith("%PDF");
}

qint64 PDFStegEngine::calculatePDFCapacity(const QString &pdfPath) const
{
    QFile file(pdfPath);
    if (!file.open(QIODevice::ReadOnly)) return 0;
    
    qint64 fileSize = file.size();
    file.close();
    
    // PDFs can typically hold up to 50% of their size as extra data
    // But realistically, I am limited by available object numbers and stream size
    return fileSize / 2;  // Conservative estimate
}

// ========== PRIVATE HELPER METHODS ==========

int PDFStegEngine::findInsertionPoint(const QByteArray &pdfData)
{
    // Find last %%EOF marker
    int eofPos = pdfData.lastIndexOf("%%EOF");
    
    if (eofPos == -1) {
        // Try to find it with variations
        eofPos = pdfData.lastIndexOf("%%EOF\r\n");
        if (eofPos == -1) {
            eofPos = pdfData.lastIndexOf("%%EOF\n");
        }
    }
    
    return eofPos;
}

int PDFStegEngine::findHighestObjectNumber(const QByteArray &pdfData)
{
    QRegularExpression regex(R"((\d+)\s+0\s+obj)");
    int maxObj = 0;
    
    QRegularExpressionMatchIterator it = regex.globalMatch(QString::fromUtf8(pdfData));
    while (it.hasNext()) {
        int objNum = it.next().captured(1).toInt();
        if (objNum > maxObj) {
            maxObj = objNum;
        }
    }
    
    return maxObj;
}

int PDFStegEngine::findXrefPosition(const QByteArray &pdfData)
{
    // Find xref table position (usually after all objects)
    return pdfData.lastIndexOf("xref");
}

QByteArray PDFStegEngine::createObjectStream(int objNum, const QByteArray &data)
{
    QByteArray stream;
    
    // Object header
    stream += QString("%1 0 obj\n").arg(objNum).toUtf8();
    stream += "<<\n";
    stream += "  /Type /ObjStm\n";
    stream += "  /N 1\n";  // One object in stream
    stream += "  /First 0\n";
    stream += QString("  /Length %1\n").arg(data.size()).toUtf8();
    stream += "  /Filter /FlateDecode\n";
    stream += ">>\n";
    stream += "stream\n";
    stream += data;
    stream += "\nendstream\n";
    stream += "endobj\n\n";
    
    return stream;
}

QByteArray PDFStegEngine::createObjectStreamWithReference(int objNum, int refObjNum, 
                                                            const QByteArray &data)
{
    QByteArray stream;
    
    // Create the object stream
    stream += createObjectStream(objNum, data);
    
    // Create a reference object (zero-size, invisible)
    stream += QString("%1 0 obj\n").arg(refObjNum).toUtf8();
    stream += "<<\n";
    stream += "  /Type /XObject\n";
    stream += "  /Subtype /Form\n";
    stream += "  /BBox [0 0 0 0]\n";  // Zero size - invisible!
    stream += QString("  /Resources << /XObject << /Steg%1 %2 0 R >> >>\n")
                  .arg(objNum).arg(objNum).toUtf8();
    stream += ">>\n";
    stream += "endobj\n\n";
    
    return stream;
}

QByteArray PDFStegEngine::findHiddenObjectStream(const QByteArray &pdfData)
{
    // Look for our object stream marker
    QRegularExpression regex(
        R"((\d+)\s+0\s+obj[^>]*/Type\s*/ObjStm[^>]*/Filter\s*/FlateDecode[^>]*>>\s*stream\n(.*?)\nendstream)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );
    
    QRegularExpressionMatch match = regex.match(QString::fromUtf8(pdfData));
    
    if (match.hasMatch()) {
        QByteArray streamData = match.captured(2).toUtf8();
        return streamData;
    }
    
    // Try alternative pattern (without /FlateDecode)
    QRegularExpression regex2(
        R"((\d+)\s+0\s+obj[^>]*/Type\s*/ObjStm[^>]*>>\s*stream\n(.*?)\nendstream)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption
    );
    
    match = regex2.match(QString::fromUtf8(pdfData));
    if (match.hasMatch()) {
        QByteArray streamData = match.captured(2).toUtf8();
        return streamData;
    }
    
    return QByteArray();
}

QByteArray PDFStegEngine::prepareDataForEmbedding(const QByteArray &data)
{
    // Format: [HEADER_SIZE bytes header][compressed data]
    
    // Compress the data
    QByteArray compressed = qCompress(data, 9);
    
    // Create header with original size info
    QByteArray header(HEADER_SIZE, 0);
    quint32 originalSize = data.size();
    header[0] = (originalSize >> 24) & 0xFF;
    header[1] = (originalSize >> 16) & 0xFF;
    header[2] = (originalSize >> 8) & 0xFF;
    header[3] = originalSize & 0xFF;
    
    // Add magic bytes for identification
    header[4] = 'P';
    header[5] = 'D';
    header[6] = 'F';
    header[7] = 'S';  // PDFSteg marker
    
    // Combine header and compressed data
    QByteArray result = header + compressed;
    

    
    return result;
}

QByteArray PDFStegEngine::extractDataFromStream(const QByteArray &streamData)
{
    if (streamData.size() < HEADER_SIZE) {
        return QByteArray();
    }

    // Find magic bytes 'PDFS' anywhere in the first 100 bytes
    int magicPos = streamData.indexOf("PDFS");
    if (magicPos == -1) {
        return QByteArray();
    }

    // Header is 4 bytes before magic bytes (original size)
    if (magicPos < 4) {
        return QByteArray();
    }

    QByteArray header = streamData.mid(magicPos - 4, HEADER_SIZE);

    // Extract original size from header
    quint32 originalSize = (static_cast<quint32>(static_cast<unsigned char>(header[0])) << 24) |
                           (static_cast<quint32>(static_cast<unsigned char>(header[1])) << 16) |
                           (static_cast<quint32>(static_cast<unsigned char>(header[2])) << 8) |
                           static_cast<quint32>(static_cast<unsigned char>(header[3]));


    // Extract compressed data (starts after magic bytes)
    QByteArray compressed = streamData.mid(magicPos + 4);

    // Decompress
    QByteArray decompressed = qUncompress(compressed);

    if (decompressed.isEmpty()) {
        return QByteArray();
    }

    return decompressed;
}


bool PDFStegEngine::isPDFCorrupted(const QByteArray &pdfData) const
{
    // Basic corruption checks
    if (!pdfData.startsWith("%PDF")) return true;
    if (pdfData.lastIndexOf("%%EOF") == -1) return true;
    
    return false;
}


