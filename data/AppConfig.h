//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QMap>

class AppConfig {
public:
    static AppConfig &instance();

    void load();
    void save();

    QString petId() const;
    void setPetId(const QString &id);

    int petScale() const;
    void setPetScale(int scale);

    int petSleepThresholdMins() const;
    void setPetSleepThresholdMins(int mins);

    bool petInteractionDisabled() const;
    void setPetInteractionDisabled(bool disabled);

    bool reminderEnabled() const;
    void setReminderEnabled(bool on);

    QString apiKey() const;
    void setApiKey(const QString &key);

    QMap<QString, QString> soundMapping() const;
    void setSoundMapping(const QMap<QString, QString> &mapping);

    QString soundForEvent(const QString &eventKey) const;
    void setSoundForEvent(const QString &eventKey, const QString &filePath);

private:
    AppConfig() = default;
    AppConfig(const AppConfig &) = delete;
    AppConfig &operator=(const AppConfig &) = delete;

    QString configPath() const;

    QString m_petId = "Muelsyse";
    int m_petScale = 100;
    int m_petSleepThresholdMins = 5;
    bool m_petInteractionDisabled = false;
    bool m_reminderEnabled = true;
    QString m_apiKey;
    QMap<QString, QString> m_soundMapping;
};

#endif //APPCONFIG_H