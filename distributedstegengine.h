#ifndef DISTRIBUTEDSTEGENGINE_H
#define DISTRIBUTEDSTEGENGINE_H

#include <QObject>
#include <QJsonArray>
#include <QStringList>
#include<QCheckBox>

class DistributedStegEngine : public QObject
{
    Q_OBJECT

public:
    explicit DistributedStegEngine(QObject *parent = nullptr);
    ~DistributedStegEngine();

    void buildPointerMap(const QString &secretMessage, const QString &language = "en");
    QString reconstructMessage(const QJsonArray &pointerMap);
    QString lastError() const { return m_lastError; }
    void cancel();
    void reset();
    void setLanguage(const QString &language) {
        m_baseUrl = QString("https://%1.wikipedia.org/w/api.php").arg(language);
    }
signals:
    void progressUpdated(int current, int total);
    void mapReady(const QJsonArray &map);
    void messageReconstructed(const QString &message);
    void errorOccurred(const QString &error);

private:
    QByteArray execCurl(const QString &url);
    void searchForWord(const QString &word);
    void fetchArticleText(const QString &title, const QString &targetWord);
    void packWordsIntoArticle(const QString &text, const QString &title);
    QString fetchArticleTextSync(const QString &title);

    QStringList splitIntoSentences(const QString &text) const;
    QStringList extractWords(const QString &sentence) const;

    QString m_baseUrl;
    QStringList m_remainingWords;
    QList<int> m_originalIndices;
    QJsonArray m_currentPointerMap;
    int m_totalWords = 0;
    bool m_cancelled = false;

    enum State { Idle, Searching, Fetching };
    State m_currentState = Idle;
    QString m_currentTitle;
    QString m_lastError;

    QList<QJsonObject> m_searchResultsList;
    int m_currentResultIndex;
    QString m_currentSearchWord;
    void tryNextSearchResult();
    int m_retryCount = 0;
    int MAX_RETRIES = 10;
};

#endif // DISTRIBUTEDSTEGENGINE_H
