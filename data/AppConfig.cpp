//
// Created by BenzoicAcid on 2026/5/12.
//

#include "data/AppConfig.h"

#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDebug>

AppConfig &AppConfig::instance() {
    static AppConfig inst;
    return inst;
}

QString AppConfig::configPath() const {
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return dataDir + "/config.json";
}

void AppConfig::load() {
    QFile f(configPath());
    if (!f.open(QIODevice::ReadOnly)) return;

    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    m_petId            = obj.value("petId").toString("Muelsyse");
    m_petScale         = obj.value("petScale").toInt(100);
    m_petSleepThresholdMins = obj.value("petSleepThresholdMins").toInt(5);
    m_petInteractionDisabled = obj.value("petInteractionDisabled").toBool(false);
    m_reminderEnabled  = obj.value("reminderEnabled").toBool(true);
    m_volume           = obj.value("volume").toInt(80);
    m_apiKey           = obj.value("apiKey").toString();
}

void AppConfig::save() {
    QJsonObject obj;
    obj["petId"]            = m_petId;
    obj["petScale"]         = m_petScale;
    obj["petSleepThresholdMins"] = m_petSleepThresholdMins;
    obj["petInteractionDisabled"] = m_petInteractionDisabled;
    obj["reminderEnabled"]  = m_reminderEnabled;
    obj["volume"]           = m_volume;
    obj["apiKey"]           = m_apiKey;

    QFile f(configPath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson());
    } else {
        qWarning() << "AppConfig::save failed";
    }
}

QString AppConfig::petId() const            { return m_petId; }
void AppConfig::setPetId(const QString &id) { m_petId = id; }

int AppConfig::petScale() const              { return m_petScale; }
void AppConfig::setPetScale(int scale)       { m_petScale = qBound(60, scale, 180); }

int AppConfig::petSleepThresholdMins() const { return m_petSleepThresholdMins; }
void AppConfig::setPetSleepThresholdMins(int mins) { m_petSleepThresholdMins = qBound(1, mins, 60); }

bool AppConfig::petInteractionDisabled() const { return m_petInteractionDisabled; }
void AppConfig::setPetInteractionDisabled(bool disabled) { m_petInteractionDisabled = disabled; }

bool AppConfig::reminderEnabled() const         { return m_reminderEnabled; }
void AppConfig::setReminderEnabled(bool on)     { m_reminderEnabled = on; }

int AppConfig::volume() const               { return m_volume; }
void AppConfig::setVolume(int vol)           { m_volume = vol; }

QString AppConfig::apiKey() const            { return m_apiKey; }
void AppConfig::setApiKey(const QString &key) { m_apiKey = key; }
