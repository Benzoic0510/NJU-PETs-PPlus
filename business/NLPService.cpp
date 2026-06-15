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

static const QString EDIT_PROMPT_TEMPLATE = QStringLiteral(
    "你是一个日程修改助手。今天是 %1。\n\n"
    "当前日程：\n"
    "  标题：%2\n"
    "  类型：%7\n"
    "  开始：%3\n"
    "  结束：%4\n"
    "  地点：%5\n"
    "  提醒：%6\n\n"
    "根据用户的描述输出修改后的完整日程。"
    "每次只返回一个 JSON 对象，不要有任何其他内容，不要使用 markdown 代码块。\n\n"
    "规则：\n"
    "1. 输入与修改日程无关 → 返回：\n"
    "   {\"type\":\"irrelevant\",\"message\":\"我只能帮你修改日程哦~\"}\n\n"
    "2. 信息不够清晰且无法从当前日程推断 → 返回：\n"
    "   {\"type\":\"incomplete\",\"message\":\"（友好提问）\"}\n\n"
    "3. 可以输出修改结果 → 返回（保留原 isDDL 类型，除非用户明确要求改变）：\n"
    "   {\"type\":\"schedule\",\"isDDL\":false,\"title\":\"...\",\"startTime\":\"YYYY-MM-DDTHH:mm:ss\","
    "\"endTime\":\"YYYY-MM-DDTHH:mm:ss\",\"location\":\"\",\"remindMins\":15}\n\n"
    "重要：用户未提及的字段，直接使用当前日程的原值，不要修改也不要追问。\n"
    "时间计算以今天 %1 为基准。\n\n"
    "时间严格性要求：\n"
    "- 输出 schedule 前必须能确定完整日期、开始时间、结束时间。\n"
    "- 用户只说 1点~11点、1:00~11:59 且未说明上午/下午/晚上/凌晨时，不要猜；除非当前日程原时间有相同 12 小时制小时并可直接保留上午/下午，否则返回 incomplete 追问“是上午还是下午”。\n"
    "- “上午/早上”按 06:00-11:59，“中午”默认 12:00，“下午”按 12:00-17:59，“晚上/今晚”按 18:00-23:59，“凌晨”按 00:00-05:59。\n"
    "- 用户只说日期但普通日程没有具体时刻时，返回 incomplete；DDL 可以只给日期，截止时间为当天 23:59:59。"
);

static const QString SYSTEM_PROMPT_TEMPLATE = QStringLiteral(
    "你是一个日程记录助手，只负责帮用户添加日程或截止日期。今天是 %1。\n\n"
    "每次只返回一个 JSON 对象，不要有任何其他内容，不要使用 markdown 代码块。\n\n"
    "规则：\n"
    "1. 输入与记录日程无关（闲聊、问天气、提问等）→ 返回：\n"
    "   {\"type\":\"irrelevant\",\"message\":\"我只能帮你记录日程哦~\"}\n\n"
    "2. 想记日程但缺少开始时间或事件名称 → 返回：\n"
    "   {\"type\":\"incomplete\",\"message\":\"（根据缺少的信息自然地提问，语气友好）\"}\n\n"
    "3. 信息完整且包含「截止」「DDL」「deadline」「到期」「提交」等截止语义 → 返回：\n"
    "   {\"type\":\"schedule\",\"isDDL\":true,\"title\":\"...\","
    "\"startTime\":\"YYYY-MM-DDT23:59:59\",\"location\":\"\",\"remindMins\":1440}\n\n"
    "4. 信息完整（普通日程）→ 返回：\n"
    "   {\"type\":\"schedule\",\"isDDL\":false,\"title\":\"...\",\"startTime\":\"YYYY-MM-DDTHH:mm:ss\","
    "\"endTime\":\"YYYY-MM-DDTHH:mm:ss\",\"location\":\"\",\"remindMins\":15}\n\n"
    "补充说明：\n"
    "- isDDL 为 true 时：startTime 为截止日期当天 23:59:59，无需 endTime，remindMins 默认 1440（提前1天）\n"
    "- 普通日程缺少结束时间默认 +1 小时，无需追问\n"
    "- 缺少地点 location 填空字符串，无需追问\n"
    "- 时间计算以今天 %1 为基准\n\n"
    "时间严格性要求：\n"
    "- 输出 schedule 前必须能确定完整日期、开始时间、结束时间。\n"
    "- 用户只说 1点~11点、1:00~11:59 且未说明上午/下午/晚上/凌晨时，不要猜上午或下午，必须返回 incomplete 追问“是上午还是下午”。例如“明天3点开会”必须追问；“明天下午3点开会”输出 15:00。\n"
    "- “上午/早上”按 06:00-11:59，“中午”默认 12:00，“下午”按 12:00-17:59，“晚上/今晚”按 18:00-23:59，“凌晨”按 00:00-05:59。\n"
    "- “12点”必须结合中午/晚上/凌晨判断；无法判断时返回 incomplete。\n"
    "- 用户只说日期但普通日程没有具体时刻时，返回 incomplete；DDL 可以只给日期，截止时间为当天 23:59:59。"
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

    // 追加用户消息到历史
    m_history.append(QJsonObject{{"role", "user"}, {"content", text}});

    const QString today = QDateTime::currentDateTime().toString("yyyy年MM月dd日");
    const QString systemPrompt = SYSTEM_PROMPT_TEMPLATE.arg(today).arg(today);

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", systemPrompt}});
    for (const auto &v : std::as_const(m_history))
        messages.append(v);

    const QJsonObject body{{"model", MODEL}, {"messages", messages}};

    QNetworkRequest req{QUrl{QString(API_URL)}};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QNetworkReply *reply = m_nam.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_pendingReplies.insert(reply);
}

void NLPService::clearHistory() {
    m_history = QJsonArray();
}

void NLPService::cancelPending() {
    const auto replies = m_pendingReplies;
    for (QNetworkReply *reply : replies) {
        if (!reply)
            continue;
        m_cancelledReplies.insert(reply);
        reply->abort();
    }
}

void NLPService::parseEdit(const QString &text, const Schedule &current) {
    const QString apiKey = resolveApiKey();
    if (apiKey.isEmpty()) {
        emit parseFailed("未配置 API Key，请设置环境变量 DASHSCOPE_API_KEY 或在设置页填写");
        return;
    }

    m_history.append(QJsonObject{{"role", "user"}, {"content", text}});

    const QString today  = QDateTime::currentDateTime().toString("yyyy年MM月dd日");
    const QString loc    = current.location.isEmpty() ? "（无）" : current.location;
    const QString remind = current.remindMins > 0
                               ? QString("提前 %1 分钟").arg(current.remindMins)
                               : "不提醒";

    const QString systemPrompt = EDIT_PROMPT_TEMPLATE
        .arg(today)
        .arg(current.title)
        .arg(current.startTime.toString("yyyy-MM-dd HH:mm"))
        .arg(current.endTime.toString("yyyy-MM-dd HH:mm"))
        .arg(loc)
        .arg(remind)
        .arg(current.isDDL ? "截止日期(DDL)" : "普通日程");

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", systemPrompt}});
    for (const auto &v : std::as_const(m_history))
        messages.append(v);

    const QJsonObject body{{"model", MODEL}, {"messages", messages}};

    QNetworkRequest req{QUrl{QString(API_URL)}};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QNetworkReply *reply = m_nam.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_pendingReplies.insert(reply);
}

void NLPService::onReply(QNetworkReply *reply) {
    const bool cancelled = m_cancelledReplies.remove(reply);
    m_pendingReplies.remove(reply);
    reply->deleteLater();

    if (cancelled) {
        if (!m_history.isEmpty()) m_history.removeLast();
        emit parseFailed("已取消解析");
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        // 撤回刚才加入历史的用户消息，让用户可以重试
        if (!m_history.isEmpty()) m_history.removeLast();
        emit parseFailed("网络错误：" + reply->errorString());
        return;
    }

    const QByteArray raw = reply->readAll();
    const QJsonObject root = QJsonDocument::fromJson(raw).object();

    const QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty()) {
        if (!m_history.isEmpty()) m_history.removeLast();
        emit parseFailed("API 返回格式异常");
        return;
    }

    QString content = choices[0].toObject()
                          .value("message").toObject()
                          .value("content").toString()
                          .trimmed();

    // 去掉可能的 markdown 代码块
    if (content.startsWith("```")) {
        content = content.section('\n', 1);
        content = content.left(content.lastIndexOf("```")).trimmed();
    }

    // 追加 AI 回复到历史
    m_history.append(QJsonObject{{"role", "assistant"}, {"content", content}});

    const QJsonObject obj = QJsonDocument::fromJson(content.toUtf8()).object();
    if (obj.isEmpty()) {
        emit parseFailed("无法解析模型返回的内容，请重试");
        return;
    }

    const QString type = obj["type"].toString();

    if (type == "irrelevant") {
        emit parseFailed(obj["message"].toString("我只能帮你记录日程哦~"));
    } else if (type == "incomplete") {
        emit clarificationNeeded(obj["message"].toString("请提供更多信息"));
    } else if (type == "schedule") {
        Schedule s;
        s.isDDL      = obj["isDDL"].toBool(false);
        s.title      = obj["title"].toString();
        s.startTime  = QDateTime::fromString(obj["startTime"].toString(), Qt::ISODate);
        s.endTime    = s.isDDL ? s.startTime
                               : QDateTime::fromString(obj["endTime"].toString(), Qt::ISODate);
        s.location   = obj["location"].toString();
        s.remindMins = obj["remindMins"].toInt(s.isDDL ? 1440 : 15);

        if (s.title.isEmpty() || !s.startTime.isValid()) {
            emit parseFailed("日程信息解析失败，请重新描述");
        } else {
            m_history = QJsonArray();  // 成功后清空历史，下次是新对话
            emit parsed(s);
        }
    } else {
        emit parseFailed("模型返回了意外格式，请重试");
    }
}
