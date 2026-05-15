//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

class AppConfig {
public:
    static AppConfig &instance();

    void load();
    void save();

    QString petId() const;
    void setPetId(const QString &id);

    bool reminderEnabled() const;
    void setReminderEnabled(bool on);

    int volume() const;
    void setVolume(int vol);

    QString apiKey() const;
    void setApiKey(const QString &key);

private:
    AppConfig() = default;
    AppConfig(const AppConfig &) = delete;
    AppConfig &operator=(const AppConfig &) = delete;

    QString configPath() const;

    QString m_petId = "whale";
    bool m_reminderEnabled = true;
    int m_volume = 80;
    QString m_apiKey;
};

#endif //APPCONFIG_H


