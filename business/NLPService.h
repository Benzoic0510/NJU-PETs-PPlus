//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef NLPSERVICE_H
#define NLPSERVICE_H

#include "data/models/Schedule.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class QNetworkReply;

class NLPService : public QObject {
    Q_OBJECT

public:
    explicit NLPService(QObject *parent = nullptr);

    void parse(const QString &text);

signals:
    void parsed(const Schedule &s);
    void parseFailed(const QString &reason);

private slots:
    void onReply(QNetworkReply *reply);

private:
    QString resolveApiKey() const;

    QNetworkAccessManager m_nam;

    static constexpr const char *API_URL =
        "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
    static constexpr const char *MODEL = "qwen-plus";
};

#endif //NLPSERVICE_H
