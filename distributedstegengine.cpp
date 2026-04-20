#include "distributedstegengine.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include<QElapsedTimer>
#include<QThread>

DistributedStegEngine::DistributedStegEngine(QObject *parent)
    : QObject(parent)
    , m_cancelled(false)
    , m_currentState(Idle)
{
}

DistributedStegEngine::~DistributedStegEngine()
{
    cancel();
#ifdef Q_OS_WIN
    system("taskkill /F /IM curl.exe");
#else
    system("pkill -9 curl");
#endif
}

void DistributedStegEngine::cancel()
{
    m_cancelled = true;
    m_currentState = Idle;
}

QByteArray DistributedStegEngine::execCurl(const QString &url)
{

    QProcess process;
    process.start("curl", QStringList() << "-s" << url);

    if (!process.waitForFinished(30000)) {
        process.kill();
        return QByteArray();
    }

    QByteArray output = process.readAllStandardOutput();
    return output;
}

void DistributedStegEngine::buildPointerMap(const QString &secretMessage, const QString &language)
{

    reset();

    m_lastError.clear();
    m_cancelled = false;

    // Split into words
    QStringList words = secretMessage.toLower().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    //QStringList words = secretMessage.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    //QStringList words = secretMessage.toLower().split(QRegularExpression("\\P{L}+"), Qt::SkipEmptyParts);

    m_remainingWords.clear();
    m_originalIndices.clear();


    for (int i = 0; i < words.size(); ++i) {
        if (!words[i].isEmpty()) {
            m_remainingWords.append(words[i]);
            m_originalIndices.append(i);
        }
    }

    if (m_remainingWords.isEmpty()) {
        m_lastError = "No valid words in secret message";
        emit errorOccurred(m_lastError);
        return;
    }

    m_totalWords = m_remainingWords.size();
    m_currentPointerMap = QJsonArray();
    m_baseUrl = QString("https://%1.wikipedia.org/w/api.php").arg(language);


    emit progressUpdated(0, m_totalWords);

    // Start with search for the first word
    searchForWord(m_remainingWords.first());
}


void DistributedStegEngine::searchForWord(const QString &word)
{

    if (m_cancelled) {
        return;
    }

    static QString lastWord;
    if (lastWord != word) {
        m_retryCount = 0;
        lastWord = word;
    }

    static QElapsedTimer lastRequest;
    if (!lastRequest.isValid()) {
        lastRequest.start();
    } else if (lastRequest.elapsed() < 2000) {
        QThread::msleep(2000 - lastRequest.elapsed());
    }
    lastRequest.restart();

    static int searchCount = 0;
    searchCount++;
    if (searchCount % 10 == 0) {
        QThread::msleep(5000);
    }

    QString url = QString("%1?action=query&list=search&srsearch=%2&srwhat=text&srlimit=10&format=json")
                      .arg(m_baseUrl, QString::fromUtf8(QUrl::toPercentEncoding(word)));


    QByteArray output = execCurl(url);
    if (output.isEmpty()) {
        emit errorOccurred(QString("Search failed for word: %1").arg(word));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output);
    QJsonArray searchResults = doc["query"]["search"].toArray();


    if (searchResults.isEmpty()) {
        emit errorOccurred(QString("Word not found in Wikipedia: '%1'").arg(word));
        m_cancelled = true;
        return;
    }

    // Store ALL search results
    m_searchResultsList.clear();
    for (const QJsonValue &val : searchResults) {
        m_searchResultsList.append(val.toObject());
    }

    // Sort by wordcount (largest first)
    std::sort(m_searchResultsList.begin(), m_searchResultsList.end(),
        [](const QJsonObject &a, const QJsonObject &b) {
            return a["wordcount"].toInt() > b["wordcount"].toInt();
        });

    m_currentResultIndex = 0;
    m_currentSearchWord = word;

    // Try the first result
    tryNextSearchResult();
}

void DistributedStegEngine::tryNextSearchResult()
{

    m_retryCount++;

    if (m_retryCount > MAX_RETRIES) {
        emit errorOccurred(QString("Failed to find article for word '%1' after %2 attempts").arg(m_currentSearchWord).arg(MAX_RETRIES));
        m_cancelled = true;
        return;
    }

    if (m_currentResultIndex >= m_searchResultsList.size()) {
        // No more results - give up on this word
        emit errorOccurred(QString("No suitable article found for word '%1'").arg(m_currentSearchWord));
        m_cancelled = true;
        return;
    }



    QJsonObject article = m_searchResultsList[m_currentResultIndex];
    m_currentTitle = article["title"].toString();

    m_currentResultIndex++;
    fetchArticleText(m_currentTitle, m_currentSearchWord);
}




void DistributedStegEngine::fetchArticleText(const QString &title, const QString &targetWord)
{

    if (m_cancelled) return;

    QString encodedTitle = QString::fromUtf8(QUrl::toPercentEncoding(title));
    QString url = QString("%1?action=query&titles=%2&prop=extracts&explaintext=1&format=json")
                      .arg(m_baseUrl, encodedTitle);

    QByteArray output = execCurl(url);
    if (output.isEmpty()) {
        tryNextSearchResult();
        return;
    }

    if (output.size() == 2171) {
        QThread::msleep(10000);
        tryNextSearchResult();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output);
    QJsonObject pages = doc["query"]["pages"].toObject();

    if (pages.isEmpty()) {
        tryNextSearchResult();
        return;
    }

    QString pageId = pages.keys().first();
    QJsonObject pageObj = pages[pageId].toObject();
    QString text = pageObj["extract"].toString();


    if (!text.contains(targetWord, Qt::CaseInsensitive)) {
        tryNextSearchResult();
        return;
    }


    packWordsIntoArticle(text, m_currentTitle);
}


QString DistributedStegEngine::fetchArticleTextSync(const QString &title)
{

    // Rate limiting - same as searchForWord
    static QElapsedTimer lastRequest;
    if (!lastRequest.isValid()) {
        lastRequest.start();
    } else if (lastRequest.elapsed() < 2000) {
        QThread::msleep(2000 - lastRequest.elapsed());
    }
    lastRequest.restart();

    static int fetchCount = 0;
    fetchCount++;
    if (fetchCount % 10 == 0) {
        QThread::msleep(5000);
    }

    QString encodedTitle = QString::fromUtf8(QUrl::toPercentEncoding(title));
    QString url = QString("%1?action=query&titles=%2&prop=extracts&explaintext=1&format=json")
                      .arg(m_baseUrl, encodedTitle);

    QByteArray output = execCurl(url);
    if (output.isEmpty()) {
        return QString();
    }

    // Check for rate limiting (2171 bytes is typical error page size)
    if (output.size() == 2171) {
        QThread::msleep(10000);
        return fetchArticleTextSync(title);  // Retry
    }

    QJsonDocument doc = QJsonDocument::fromJson(output);
    QJsonObject pages = doc["query"]["pages"].toObject();

    if (pages.isEmpty()) {
        return QString();
    }

    QString pageId = pages.keys().first();
    QJsonObject pageObj = pages[pageId].toObject();

    if (pageObj.contains("missing")) {
        return QString();
    }

    QString extract = pageObj["extract"].toString();
    return extract;
}


void DistributedStegEngine::packWordsIntoArticle(const QString &text, const QString &title)
{

    if (m_cancelled) return;

    QStringList sentences = splitIntoSentences(text);
    QList<int> wordsFound;

    for (int i = 0; i < m_remainingWords.size(); ++i) {
        QString targetWord = m_remainingWords[i];
        int foundSentence = -1;
        int foundPosition = -1;

        for (int s = 0; s < sentences.size(); ++s) {
            QStringList words = extractWords(sentences[s]);
            for (int w = 0; w < words.size(); ++w) {
                if (words[w].toLower() == targetWord) {
                    foundSentence = s;
                    foundPosition = w;
                    break;
                }
            }
            if (foundSentence != -1) break;
        }

        if (foundSentence != -1) {
            QJsonObject pointer;
            pointer["word"] = targetWord;
            pointer["index"] = m_originalIndices[i];
            pointer["article"] = title;
            pointer["sentence"] = foundSentence;
            pointer["position"] = foundPosition;
            m_currentPointerMap.append(pointer);
            wordsFound.append(i);

            int progress = m_currentPointerMap.size();
            emit progressUpdated(progress, m_totalWords);
        } else {
        }
    }

    // Remove found words
    for (int i = wordsFound.size() - 1; i >= 0; --i) {
        int posToRemove = wordsFound[i];
        m_remainingWords.removeAt(posToRemove);
        m_originalIndices.removeAt(posToRemove);
    }


    if (m_remainingWords.isEmpty()) {
        emit mapReady(m_currentPointerMap);
        m_currentState = Idle;
    } else {
        searchForWord(m_remainingWords.first());
    }
}

QStringList DistributedStegEngine::splitIntoSentences(const QString &text) const
{
    QRegularExpression sentenceSplitter(R"((?<=[.!?])\s+)");
    return text.split(sentenceSplitter, Qt::SkipEmptyParts);
}

QStringList DistributedStegEngine::extractWords(const QString &sentence) const
{
    QRegularExpression wordFinder(R"(\b\w+\b)");
    QStringList words;
    QRegularExpressionMatchIterator it = wordFinder.globalMatch(sentence);
    while (it.hasNext()) {
        words.append(it.next().captured());
    }
    return words;
}



QString DistributedStegEngine::reconstructMessage(const QJsonArray &pointerMap)
{

    if (m_baseUrl.isEmpty()) {
        m_baseUrl = "https://en.wikipedia.org/w/api.php";
    }

    m_lastError.clear();

    QMap<QString, QString> articleCache;  // Cache for fetched articles

    QList<QJsonObject> pointers;
    for (const QJsonValue &val : pointerMap) {
        pointers.append(val.toObject());
    }

    std::sort(pointers.begin(), pointers.end(),
        [](const QJsonObject &a, const QJsonObject &b) {
            return a["index"].toInt() < b["index"].toInt();
        });

    QStringList reconstructedWords;
    for (const QJsonObject &pointer : pointers) {
        QString article = pointer["article"].toString();
        int sentence = pointer["sentence"].toInt();
        int position = pointer["position"].toInt();



        // ========== USE CACHE  ==========
        QString text;
        if (articleCache.contains(article)) {
            text = articleCache[article];
        } else {
            text = fetchArticleTextSync(article);
            if (!text.isEmpty()) {
                articleCache[article] = text;
            }
        }
        // ====================================


        //QString text = fetchArticleTextSync(article);
        if (text.isEmpty()) {
            m_lastError = QString("Failed to fetch article: %1").arg(article);
            return QString();
        }

        QStringList sentences = splitIntoSentences(text);
        if (sentence >= sentences.size()) {
            m_lastError = QString("Sentence index out of range for article: %1").arg(article);
            return QString();
        }

        QStringList words = extractWords(sentences[sentence]);
        if (position >= words.size()) {
            m_lastError = QString("Word position out of range for article: %1").arg(article);
            return QString();
        }

        QString word = words[position];
        reconstructedWords.append(word);
    }

    QString result = reconstructedWords.join(" ");
    return result;
}

void DistributedStegEngine::reset()
{
    m_cancelled = false;
    m_currentState = Idle;
    m_lastError.clear();
    m_remainingWords.clear();
    m_originalIndices.clear();
    m_currentPointerMap = QJsonArray();

    // Add these missing resets:
    m_searchResultsList.clear();
    m_currentResultIndex = 0;
    m_currentSearchWord.clear();
    m_retryCount = 0;
    m_totalWords = 0;
    m_baseUrl.clear();
    m_currentTitle.clear();
}
