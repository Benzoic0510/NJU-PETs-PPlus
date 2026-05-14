//
// Created by BenzoicAcid on 2026/5/12.
//

#include "business/NLPService.h"
#include "data/AppConfig.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

static const QString SYSTEM_PROMPT_TEMPLATE = QStringLiteral(
    "你是一个日程解析助手。将用户的自然语言描述解析为 JSON 格式的日程信息。\n"
    "当前时间：%1\n\n"
    "返回格式（只返回 JSON，不要有其他文字，不要用 markdown 代码块）：\n"
    "{\n"
    "  \"title\": \"日程标题\",\n"
    "  \"startTime\": \"ISO 8601 格式，如 2026-05-14T15:00:00\",\n"
    "  \"endTime\": \"ISO 8601 格式\",\n"
    "  \"location\": \"地点，没有则为空字符串\",\n"
    "  \"remindMins\": 15\n"
    "}"
);

NLPService::NLPService(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &NLPService::onReply);
}

QString NLPService::resolveApiKey() const {
    const QByteArray envKey = qgetenv("DASHSCOPE_API_KEY");
    if (!envKey.isEmpty()) return QString::fromUtf8(envKey);
    return AppConfig::instance().apiKey();
}

void NLPService::parse(const QString &text) {
    const QString apiKey = resolveApiKey();
    if (apiKey.isEmpty()) {
        emit parseFailed("未配置 API Key，请设置环境变量 DASHSCOPE_API_KEY 或在 config.json 中填写 apiKey");
        return;
    }

    const QString systemPrompt = SYSTEM_PROMPT_TEMPLATE.arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")
    );

    const QJsonObject body {
        {"model", MODEL},
        {"messages", QJsonArray {
            QJsonObject {{"role", "system"}, {"content", systemPrompt}},
            QJsonObject {{"role", "user"},   {"content", text}}
        }}
    };

    QNetworkRequest req{QUrl{QString(API_URL)}};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    m_nam.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
}

void NLPService::onReply(QNetworkReply *reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit parseFailed(reply->errorString());
        return;
    }

    const QByteArray raw = reply->readAll();
    const QJsonObject root = QJsonDocument::fromJson(raw).object();

    // 提取 choices[0].message.content
    const QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty()) {
        emit parseFailed("API 返回格式异常：choices 为空");
        return;
    }
    QString content = choices[0].toObject()
                          .value("message").toObject()
                          .value("content").toString()
                          .trimmed();

    // 去掉可能的 markdown 代码块
    if (content.startsWith("```")) {
        content = content.section('\n', 1);       // 去掉第一行 ```json
        content = content.left(content.lastIndexOf("```")).trimmed();
    }

    const QJsonObject obj = QJsonDocument::fromJson(content.toUtf8()).object();
    if (obj.isEmpty()) {
        emit parseFailed("无法解析模型返回的 JSON：" + content);
        return;
    }

    Schedule s;
    s.title      = obj.value("title").toString();
    s.startTime  = QDateTime::fromString(obj.value("startTime").toString(), Qt::ISODate);
    s.endTime    = QDateTime::fromString(obj.value("endTime").toString(), Qt::ISODate);
    s.location   = obj.value("location").toString();
    s.remindMins = obj.value("remindMins").toInt(15);

    emit parsed(s);
}
