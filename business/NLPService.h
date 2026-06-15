//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef NLPSERVICE_H
#define NLPSERVICE_H

#include "data/models/Schedule.h"

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSet>
#include <QString>

class QNetworkReply;

class NLPService : public QObject {
    Q_OBJECT

public:
    explicit NLPService(QObject *parent = nullptr);

public slots:
    void parse(const QString &text);
    void parseEdit(const QString &text, const Schedule &current);
    void cancelPending();
    void clearHistory();

signals:
    void parsed(const Schedule &s);
    void parseFailed(const QString &reason);
    void clarificationNeeded(const QString &question);

private slots:
    void onReply(QNetworkReply *reply);

private:
    QString resolveApiKey() const;

    QNetworkAccessManager m_nam;
    QJsonArray            m_history;
    QSet<QNetworkReply *> m_pendingReplies;
    QSet<QNetworkReply *> m_cancelledReplies;

    static constexpr const char *API_URL =
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
    static constexpr const char *MODEL = "qwen-plus";
};

#endif //NLPSERVICE_H
