//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/mainmenu/MainMenu.h"
#include "presentation/calendar/CalendarPanel.h"
#include "presentation/other/OtherPanel.h"
#include "presentation/selector/PetSelector.h"
#include "presentation/settings/SettingsPanel.h"
#include "presentation/common/Theme.h"
#include "data/AppConfig.h"

#include <QEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QTimer>
#include <QVariantAnimation>
#include <QVBoxLayout>

#include <functional>
#include <memory>

namespace {

QColor mixColor(const QColor &from, const QColor &to, qreal amount) {
    const qreal t = qBound<qreal>(0.0, amount, 1.0);
    return QColor::fromRgbF(
        from.redF()   + (to.redF()   - from.redF())   * t,
        from.greenF() + (to.greenF() - from.greenF()) * t,
        from.blueF()  + (to.blueF()  - from.blueF())  * t,
        from.alphaF() + (to.alphaF() - from.alphaF()) * t
    );
}

qreal easeOutCubicValue(qreal value) {
    const qreal t = 1.0 - qBound<qreal>(0.0, value, 1.0);
    return 1.0 - t * t * t;
}

qreal interpolate(qreal from, qreal to, qreal progress) {
    return from + (to - from) * qBound<qreal>(0.0, progress, 1.0);
}

class TopNavButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)
    Q_PROPERTY(qreal checkedProgress READ checkedProgress WRITE setCheckedProgress)

public:
    explicit TopNavButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent)
    {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(34);
        setMinimumWidth(72);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        m_hoverAnim = new QPropertyAnimation(this, "hoverProgress", this);
        m_hoverAnim->setDuration(160);
        m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);

        m_checkedAnim = new QPropertyAnimation(this, "checkedProgress", this);
        m_checkedAnim->setDuration(180);
        m_checkedAnim->setEasingCurve(QEasingCurve::OutCubic);

        connect(this, &QPushButton::toggled, this, [this](bool on) {
            animate(m_checkedAnim, m_checkedProgress, on ? 1.0 : 0.0);
        });
    }

    qreal hoverProgress() const { return m_hoverProgress; }
    qreal checkedProgress() const { return m_checkedProgress; }

    void setHoverProgress(qreal progress) {
        m_hoverProgress = qBound<qreal>(0.0, progress, 1.0);
        update();
    }

    void setCheckedProgress(qreal progress) {
        m_checkedProgress = qBound<qreal>(0.0, progress, 1.0);
        update();
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        QPushButton::enterEvent(event);
        animate(m_hoverAnim, m_hoverProgress, 1.0);
    }

    void leaveEvent(QEvent *event) override {
        QPushButton::leaveEvent(event);
        animate(m_hoverAnim, m_hoverProgress, 0.0);
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const QRectF bgRect = QRectF(rect()).adjusted(0.5, 1.0, -0.5, -1.0);
        QColor hoverBg(Theme::PrimaryMid);
        hoverBg.setAlphaF(0.72 * m_hoverProgress);

        if (m_hoverProgress > 0.0) {
            p.setPen(Qt::NoPen);
            p.setBrush(hoverBg);
            p.drawRoundedRect(bgRect, Theme::Radius, Theme::Radius);
        }

        if (m_checkedProgress > 0.0) {
            QColor selectedBg(Theme::BgPrimary);
            selectedBg.setAlphaF(m_checkedProgress);
            p.setPen(Qt::NoPen);
            p.setBrush(selectedBg);
            p.drawRoundedRect(bgRect, Theme::Radius, Theme::Radius);
        }

        QFont textFont = font();
        textFont.setPointSize(10);
        textFont.setBold(m_checkedProgress > 0.5);
        p.setFont(textFont);
        p.setPen(mixColor(QColor(Theme::BgPrimary), QColor(Theme::PrimaryDark), m_checkedProgress));
        p.drawText(rect(), Qt::AlignCenter, text());
    }

private:
    void animate(QPropertyAnimation *anim, qreal from, qreal to) {
        anim->stop();
        anim->setStartValue(from);
        anim->setEndValue(to);
        anim->start();
    }

    qreal m_hoverProgress = 0.0;
    qreal m_checkedProgress = 0.0;
    QPropertyAnimation *m_hoverAnim = nullptr;
    QPropertyAnimation *m_checkedAnim = nullptr;
};

class TopWindowButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)

public:
    explicit TopWindowButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent)
    {
        setFixedSize(34, 30);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);

        m_hoverAnim = new QPropertyAnimation(this, "hoverProgress", this);
        m_hoverAnim->setDuration(150);
        m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);
    }

    qreal hoverProgress() const { return m_hoverProgress; }

    void setHoverProgress(qreal progress) {
        m_hoverProgress = qBound<qreal>(0.0, progress, 1.0);
        update();
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        QPushButton::enterEvent(event);
        animate(1.0);
    }

    void leaveEvent(QEvent *event) override {
        QPushButton::leaveEvent(event);
        animate(0.0);
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QColor bg(Theme::PrimaryMid);
        bg.setAlphaF(0.82 * m_hoverProgress);
        if (m_hoverProgress > 0.0) {
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 6, 6);
        }

        QFont textFont = font();
        textFont.setPointSize(11);
        textFont.setBold(true);
        p.setFont(textFont);
        p.setPen(QColor(Theme::BgPrimary));
        p.drawText(rect(), Qt::AlignCenter, text());
    }

private:
    void animate(qreal to) {
        m_hoverAnim->stop();
        m_hoverAnim->setStartValue(m_hoverProgress);
        m_hoverAnim->setEndValue(to);
        m_hoverAnim->start();
    }

    qreal m_hoverProgress = 0.0;
    QPropertyAnimation *m_hoverAnim = nullptr;
};

class PetRevealWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal revealScale READ revealScale WRITE setRevealScale)
    Q_PROPERTY(qreal revealOpacity READ revealOpacity WRITE setRevealOpacity)

public:
    explicit PetRevealWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(300);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    qreal revealScale() const { return m_revealScale; }
    qreal revealOpacity() const { return m_revealOpacity; }

    void setRevealScale(qreal scale) {
        m_revealScale = scale;
        update();
    }

    void setRevealOpacity(qreal opacity) {
        m_revealOpacity = qBound<qreal>(0.0, opacity, 1.0);
        update();
    }

    void setPet(const QString &petId) {
        m_petName = petId;
        m_petImage = QPixmap(QString(":/sprites/%1/%1.png").arg(petId));
        update();
    }

    void playPop() {
        stopAnimations();

        setRevealScale(0.82);
        setRevealOpacity(0.0);

        m_scaleAnim = new QVariantAnimation(this);
        m_scaleAnim->setDuration(400);
        m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_scaleAnim->setKeyValueAt(0.0, 0.94);
        m_scaleAnim->setKeyValueAt(0.6, 1.03);
        m_scaleAnim->setKeyValueAt(1.0, 1.0);
        connect(m_scaleAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealScale(value.toReal());
        });
        connect(m_scaleAnim, &QVariantAnimation::finished, this, [this]() {
            setRevealScale(1.0);
            m_scaleAnim = nullptr;
        });
        m_scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

        m_opacityAnim = new QVariantAnimation(this);
        m_opacityAnim->setDuration(260);
        m_opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_opacityAnim->setStartValue(0.0);
        m_opacityAnim->setEndValue(1.0);
        connect(m_opacityAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealOpacity(value.toReal());
        });
        connect(m_opacityAnim, &QVariantAnimation::finished, this, [this]() {
            setRevealOpacity(1.0);
            m_opacityAnim = nullptr;
        });
        m_opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void playExit(const std::function<void()> &finished) {
        stopAnimations();
        setRevealScale(1.0);
        setRevealOpacity(1.0);

        m_scaleAnim = new QVariantAnimation(this);
        m_scaleAnim->setDuration(100);
        m_scaleAnim->setEasingCurve(QEasingCurve::InCubic);
        m_scaleAnim->setStartValue(1.0);
        m_scaleAnim->setEndValue(0.92);
        connect(m_scaleAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealScale(value.toReal());
        });
        connect(m_scaleAnim, &QVariantAnimation::finished, this, [this, finished]() {
            setRevealScale(1.0);
            setRevealOpacity(1.0);
            m_scaleAnim = nullptr;
            if (finished)
                finished();
        });
        m_scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

        m_opacityAnim = new QVariantAnimation(this);
        m_opacityAnim->setDuration(180);
        m_opacityAnim->setEasingCurve(QEasingCurve::InCubic);
        m_opacityAnim->setStartValue(1.0);
        m_opacityAnim->setEndValue(0.0);
        connect(m_opacityAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealOpacity(value.toReal());
        });
        connect(m_opacityAnim, &QVariantAnimation::finished, this, [this]() {
            m_opacityAnim = nullptr;
        });
        m_opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setOpacity(m_revealOpacity);

        const QPointF center(width() / 2.0, height() / 2.0);
        p.translate(center);
        p.scale(m_revealScale, m_revealScale);
        p.translate(-center);

        const int imageSide = 190;
        const QRect imageRect((width() - imageSide) / 2, 10, imageSide, imageSide);
        if (!m_petImage.isNull()) {
            p.drawPixmap(imageRect, m_petImage.scaled(
                imageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            p.setPen(QColor(Theme::TextTertiary));
            QFont fallbackFont = p.font();
            fallbackFont.setPointSize(13);
            fallbackFont.setBold(true);
            p.setFont(fallbackFont);
            p.drawText(imageRect, Qt::AlignCenter, "NJU-PETs++");
        }

        QFont nameFont = p.font();
        nameFont.setPointSize(15);
        nameFont.setBold(true);
        p.setFont(nameFont);
        p.setPen(QColor(Theme::TextPrimary));
        p.drawText(QRect(0, 214, width(), 24), Qt::AlignCenter, m_petName);

        QFont hintFont = p.font();
        hintFont.setPointSize(9);
        hintFont.setBold(false);
        p.setFont(hintFont);
        p.setPen(QColor(Theme::TextTertiary));
        p.drawText(QRect(18, 246, width() - 36, 42),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   "在右侧切换桌宠形象");
    }

private:
    void stopAnimations() {
        if (m_scaleAnim) {
            m_scaleAnim->stop();
            m_scaleAnim->deleteLater();
            m_scaleAnim = nullptr;
        }
        if (m_opacityAnim) {
            m_opacityAnim->stop();
            m_opacityAnim->deleteLater();
            m_opacityAnim = nullptr;
        }
    }

    QString m_petName;
    QPixmap m_petImage;
    qreal m_revealScale = 1.0;
    qreal m_revealOpacity = 1.0;
    QVariantAnimation *m_scaleAnim = nullptr;
    QVariantAnimation *m_opacityAnim = nullptr;
};

class ContextPopHost : public QWidget {
public:
    explicit ContextPopHost(QWidget *content, QWidget *parent = nullptr)
        : QWidget(parent), m_content(content)
    {
        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);
        root->addWidget(m_content);
    }

    qreal revealScale() const { return m_revealScale; }
    qreal revealOpacity() const { return m_revealOpacity; }

    void setRevealScale(qreal scale) {
        m_revealScale = scale;
        update();
    }

    void setRevealOpacity(qreal opacity) {
        m_revealOpacity = qBound<qreal>(0.0, opacity, 1.0);
        update();
    }

    void playPop() {
        if (!m_content || width() <= 0 || height() <= 0)
            return;

        stopAnimation();
        m_content->show();
        m_snapshotRect = m_content->geometry();
        m_snapshot = m_content->grab();
        if (m_snapshot.isNull() || m_snapshotRect.isEmpty()) {
            m_content->show();
            return;
        }

        m_animating = true;
        m_content->hide();
        setRevealScale(0.94);
        setRevealOpacity(0.0);

        m_scaleAnim = new QVariantAnimation(this);
        m_scaleAnim->setDuration(400);
        m_scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_scaleAnim->setKeyValueAt(0.0, 0.94);
        m_scaleAnim->setKeyValueAt(0.4, 1.02);
        m_scaleAnim->setKeyValueAt(1.0, 1.0);
        connect(m_scaleAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealScale(value.toReal());
        });
        connect(m_scaleAnim, &QVariantAnimation::finished, this, [this]() {
            setRevealScale(1.0);
            finishAnimation();
            m_scaleAnim = nullptr;
        });
        m_scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

        m_opacityAnim = new QVariantAnimation(this);
        m_opacityAnim->setDuration(260);
        m_opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_opacityAnim->setStartValue(0.0);
        m_opacityAnim->setEndValue(1.0);
        connect(m_opacityAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealOpacity(value.toReal());
        });
        connect(m_opacityAnim, &QVariantAnimation::finished, this, [this]() {
            setRevealOpacity(1.0);
            m_opacityAnim = nullptr;
        });
        m_opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void playExit(const std::function<void()> &finished) {
        if (!m_content || width() <= 0 || height() <= 0) {
            if (finished)
                finished();
            return;
        }

        stopAnimation();
        m_content->show();
        m_snapshotRect = m_content->geometry();
        m_snapshot = m_content->grab();
        if (m_snapshot.isNull() || m_snapshotRect.isEmpty()) {
            if (finished)
                finished();
            return;
        }

        m_animating = true;
        m_content->hide();
        setRevealScale(1.0);
        setRevealOpacity(1.0);

        m_scaleAnim = new QVariantAnimation(this);
        m_scaleAnim->setDuration(100);
        m_scaleAnim->setEasingCurve(QEasingCurve::InCubic);
        m_scaleAnim->setStartValue(1.0);
        m_scaleAnim->setEndValue(0.92);
        connect(m_scaleAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealScale(value.toReal());
        });
        connect(m_scaleAnim, &QVariantAnimation::finished, this, [this, finished]() {
            m_scaleAnim = nullptr;
            finishExitAnimation();
            if (finished)
                finished();
        });
        m_scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

        m_opacityAnim = new QVariantAnimation(this);
        m_opacityAnim->setDuration(180);
        m_opacityAnim->setEasingCurve(QEasingCurve::InCubic);
        m_opacityAnim->setStartValue(1.0);
        m_opacityAnim->setEndValue(0.0);
        connect(m_opacityAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setRevealOpacity(value.toReal());
        });
        connect(m_opacityAnim, &QVariantAnimation::finished, this, [this]() {
            m_opacityAnim = nullptr;
        });
        m_opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void preparePop() {
        stopAnimation();
        if (m_content)
            m_content->hide();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QWidget::paintEvent(event);
        if (!m_animating || m_snapshot.isNull())
            return;

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setOpacity(m_revealOpacity);

        const QPoint center = m_snapshotRect.center();
        p.translate(center);
        p.scale(m_revealScale, m_revealScale);
        p.translate(-center);
        p.drawPixmap(m_snapshotRect.topLeft(), m_snapshot);
    }

private:
    void stopAnimation() {
        if (m_scaleAnim) {
            m_scaleAnim->stop();
            m_scaleAnim->deleteLater();
            m_scaleAnim = nullptr;
        }
        if (m_opacityAnim) {
            m_opacityAnim->stop();
            m_opacityAnim->deleteLater();
            m_opacityAnim = nullptr;
        }
        finishAnimation();
    }

    void finishAnimation() {
        m_animating = false;
        if (m_content)
            m_content->show();
        m_snapshot = QPixmap();
        setRevealOpacity(1.0);
        setRevealScale(1.0);
        update();
    }

    void finishExitAnimation() {
        m_animating = false;
        if (m_content)
            m_content->hide();
        m_snapshot = QPixmap();
        setRevealOpacity(1.0);
        setRevealScale(1.0);
        update();
    }

    QWidget *m_content = nullptr;
    QPixmap m_snapshot;
    QRect m_snapshotRect;
    qreal m_revealScale = 1.0;
    qreal m_revealOpacity = 1.0;
    bool m_animating = false;
    QVariantAnimation *m_scaleAnim = nullptr;
    QVariantAnimation *m_opacityAnim = nullptr;
};

class SettingsTagHost : public QWidget {
public:
    explicit SettingsTagHost(QPushButton *button, QWidget *parent = nullptr)
        : QWidget(parent), m_button(button)
    {
        setFixedHeight(34);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_button->setParent(this);
        m_effect = new QGraphicsOpacityEffect(m_button);
        m_button->setGraphicsEffect(m_effect);
        m_effect->setOpacity(1.0);
        updateButtonGeometry();
    }

    QPushButton *button() const { return m_button; }

    void prepareEnter() {
        stopAnimation();
        ++m_generation;
        setOffset(StartOffset);
        if (m_effect)
            m_effect->setOpacity(0.0);
    }

    void playEnter(int delayMs) {
        stopAnimation();
        setOffset(StartOffset);
        if (m_effect)
            m_effect->setOpacity(0.0);

        const int generation = ++m_generation;
        QTimer::singleShot(delayMs, this, [this, generation]() {
            if (generation != m_generation)
                return;

            m_anim = new QVariantAnimation(this);
            m_anim->setDuration(AnimMs);
            m_anim->setEasingCurve(QEasingCurve::Linear);
            m_anim->setStartValue(0.0);
            m_anim->setEndValue(1.0);
            connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
                const qreal t = qBound<qreal>(0.0, value.toReal(), 1.0);
                if (m_effect)
                    m_effect->setOpacity(easeOutCubicValue(t));

                qreal offset = 0.0;
                if (t < ForwardPart)
                    offset = interpolate(StartOffset, OvershootOffset, easeOutCubicValue(t / ForwardPart));
                else
                    offset = interpolate(OvershootOffset, 0.0,
                                         easeOutCubicValue((t - ForwardPart) / (1.0 - ForwardPart)));
                setOffset(offset);
            });
            connect(m_anim, &QVariantAnimation::finished, this, [this]() {
                setOffset(0.0);
                if (m_effect)
                    m_effect->setOpacity(1.0);
                m_anim = nullptr;
            });
            m_anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        updateButtonGeometry();
    }

private:
    void stopAnimation() {
        if (m_anim) {
            m_anim->stop();
            m_anim->deleteLater();
            m_anim = nullptr;
        }
    }

    void setOffset(qreal offset) {
        m_offset = offset;
        updateButtonGeometry();
    }

    void updateButtonGeometry() {
        if (!m_button)
            return;
        m_button->setGeometry(qRound(m_offset), 0, width(), height());
    }

    QPushButton *m_button = nullptr;
    QGraphicsOpacityEffect *m_effect = nullptr;
    QVariantAnimation *m_anim = nullptr;
    qreal m_offset = 0.0;
    int m_generation = 0;

    static constexpr int DelayStepMs = 60;
    static constexpr int AnimMs = 240;
    static constexpr qreal StartOffset = -60.0;
    static constexpr qreal OvershootOffset = 6.0;
    static constexpr qreal ForwardPart = 0.6;

public:
    static int delayForIndex(int index) { return index * DelayStepMs; }
    static int animMs() { return AnimMs; }
    static qreal startOffset() { return StartOffset; }
    static qreal overshootOffset() { return OvershootOffset; }
    static qreal forwardPart() { return ForwardPart; }
};

} // namespace

MainMenu::MainMenu(ScheduleService *svc, NLPService *nlp, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("NJU-PETs++");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    resize(Theme::WindowW, Theme::WindowH);
    setMinimumSize(720, 480);
    setupUi(svc, nlp);
}

void MainMenu::showSchedulePage() {
    show();
    raise();
    activateWindow();
    switchPage(1);
}

void MainMenu::setupUi(ScheduleService *svc, NLPService *nlp) {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Custom title / navigation bar ───────────────────
    m_titleBar = new QWidget;
    m_titleBar->setFixedHeight(56);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setStyleSheet(
        "QWidget#titleBar {"
        "  background:" + QString(Theme::PrimaryDark) + ";"
        "  border-bottom: 1px solid " + Theme::Primary + ";"
        "}"
    );
    m_titleBar->setProperty("windowDragArea", true);
    m_titleBar->installEventFilter(this);

    auto *barLayout = new QHBoxLayout(m_titleBar);
    barLayout->setContentsMargins(16, 0, 12, 0);
    barLayout->setSpacing(0);

    auto *appTitle = new QLabel("NJU-PETs++");
    appTitle->setFixedWidth(176);
    appTitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    appTitle->setStyleSheet(
        "font-size: 17px; font-weight:700;"
        "color:" + QString(Theme::BgPrimary) + ";"
        "letter-spacing:0;"
    );
    appTitle->setProperty("windowDragArea", true);
    appTitle->installEventFilter(this);
    barLayout->addWidget(appTitle);

    auto *navWrap = new QWidget;
    navWrap->setProperty("windowDragArea", true);
    navWrap->installEventFilter(this);
    auto *navLayout = new QHBoxLayout(navWrap);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(8);
    navLayout->addStretch();

    m_navGrp = new QButtonGroup(this);
    m_navGrp->setExclusive(true);

    const QStringList labels = {"启动", "日程", "设置", "其他"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = makeNavBtn(labels[i]);
        m_navGrp->addButton(btn, i);
        navLayout->addWidget(btn);
        if (i == 0) btn->setChecked(true);
    }
    navLayout->addStretch();
    barLayout->addWidget(navWrap, 1);

    auto *windowControls = new QWidget;
    windowControls->setFixedWidth(176);
    windowControls->setProperty("windowDragArea", true);
    windowControls->installEventFilter(this);
    auto *controlLayout = new QHBoxLayout(windowControls);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(6);
    controlLayout->addStretch();

    auto *minBtn = new TopWindowButton("−");
    auto *closeBtn = new TopWindowButton("×");
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    controlLayout->addWidget(minBtn);
    controlLayout->addWidget(closeBtn);
    barLayout->addWidget(windowControls);

    root->addWidget(m_titleBar);

    // ── Content shell ───────────────────────────────────
    auto *contentShell = new QWidget;
    contentShell->setObjectName("contentShell");
    contentShell->setStyleSheet(
        "QWidget#contentShell { background:" + QString(Theme::PrimaryBg) + "; }");
    auto *contentLayout = new QHBoxLayout(contentShell);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_contextPanel = new QWidget;
    m_contextPanel->setObjectName("contextPanel");
    m_contextPanel->setFixedWidth(260);
    m_contextPanel->setStyleSheet(
        "QWidget#contextPanel {"
        "  background:" + QString(Theme::BgPrimary) + ";"
        "  border-right: 1px solid " + Theme::Border + ";"
        "}");
    auto *contextLayout = new QVBoxLayout(m_contextPanel);
    contextLayout->setContentsMargins(0, 0, 0, 0);
    contextLayout->setSpacing(0);

    m_contextStack = new QStackedWidget;
    m_contextStack->setStyleSheet("background: transparent;");
    contextLayout->addWidget(m_contextStack);
    contentLayout->addWidget(m_contextPanel);

    auto *rightWrap = new QWidget;
    rightWrap->setObjectName("rightContentWrap");
    rightWrap->setStyleSheet("#rightContentWrap { background: transparent; }");
    auto *rightWrapLayout = new QVBoxLayout(rightWrap);
    rightWrapLayout->setContentsMargins(18, 18, 18, 18);
    rightWrapLayout->setSpacing(0);

    m_rightSurface = new QWidget;
    m_rightSurface->setObjectName("rightContentSurface");
    auto *surfaceLayout = new QVBoxLayout(m_rightSurface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: transparent;");

    auto *selector = new PetSelector;
    selector->setStyleSheet("background: transparent;");
    connect(selector, &PetSelector::petSelected, this, [this](const QString &petId) {
        updatePetContext(petId);
        emit petSelected(petId);
    });

    auto *calendarPanel = new CalendarPanel(svc, nlp, nullptr, false);
    calendarPanel->setStyleSheet("background: transparent;");
    m_calendarPanel = calendarPanel;
    connect(svc, &ScheduleService::scheduleAdded,   calendarPanel, &CalendarPanel::refresh);
    connect(svc, &ScheduleService::scheduleUpdated, calendarPanel, &CalendarPanel::refresh);
    connect(svc, &ScheduleService::scheduleRemoved, calendarPanel, &CalendarPanel::refresh);
    auto *settingsPanel = new SettingsPanel;
    settingsPanel->setStyleSheet("background: transparent;");
    m_settingsPanel = settingsPanel;
    connect(settingsPanel, &SettingsPanel::petScaleChanged,
            this,          &MainMenu::petScaleChanged);
    connect(settingsPanel, &SettingsPanel::petSleepThresholdChanged,
            this,          &MainMenu::petSleepThresholdChanged);
    connect(settingsPanel, &SettingsPanel::petInteractionDisabledChanged,
            this,          &MainMenu::petInteractionDisabledChanged);
    auto *otherPanel = new OtherPanel(svc);
    otherPanel->setStyleSheet("background: transparent;");
    m_otherPanel = otherPanel;

    auto *scheduleContextReveal = new ContextPopHost(calendarPanel->contextPanel());
    m_scheduleReveal = scheduleContextReveal;

    m_contextStack->addWidget(makePetContext());                 // 0 启动
    m_contextStack->addWidget(scheduleContextReveal);            // 1 日程
    m_contextStack->addWidget(makeSettingsContext(settingsPanel));// 2 设置
    m_contextStack->addWidget(makeOtherContext(otherPanel));     // 3 其他

    m_stack->addWidget(selector);                                // 0 启动
    m_stack->addWidget(calendarPanel);                           // 1 日程
    m_stack->addWidget(settingsPanel);                           // 2 设置
    m_stack->addWidget(otherPanel);                              // 3 其他

    surfaceLayout->addWidget(m_stack);
    rightWrapLayout->addWidget(m_rightSurface);
    contentLayout->addWidget(rightWrap, 1);
    root->addWidget(contentShell, 1);
    updateRightSurfaceStyle(0);

    connect(m_navGrp, &QButtonGroup::idClicked, this, &MainMenu::switchPage);
}

QPushButton *MainMenu::makeNavBtn(const QString &text) {
    return new TopNavButton(text);
}

QPushButton *MainMenu::makeContextBtn(const QString &text) {
    auto *btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setFixedHeight(34);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setStyleSheet(
        "QPushButton {"
        "  text-align:left; padding:0 20px;"
        "  border:none; border-radius:8px;"
        "  background:transparent;"
        "  color:" + QString(Theme::TextSecondary) + ";"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background:" + QString(Theme::PrimaryBg) + ";"
        "  color:" + QString(Theme::PrimaryDark) + ";"
        "}"
        "QPushButton:checked {"
        "  background:transparent;"
        "  color:" + QString(Theme::PrimaryDark) + ";"
        "  font-weight:600;"
        "}"
    );
    return btn;
}

QWidget *MainMenu::makePetContext() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 22, 18, 18);
    root->setSpacing(12);

    auto *reveal = new PetRevealWidget;
    m_petReveal = reveal;
    root->addWidget(reveal);
    root->addStretch();

    updatePetContext(AppConfig::instance().petId());
    QTimer::singleShot(0, reveal, [reveal]() {
        reveal->playPop();
    });
    return page;
}

QWidget *MainMenu::makeSettingsContext(SettingsPanel *settingsPanel) {
    auto *page = new QWidget;
    m_settingsContext = page;
    m_settingsTagHosts.clear();
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(14, 22, 14, 18);
    root->setSpacing(4);

    auto *group = new QButtonGroup(page);
    group->setExclusive(true);

    const QStringList labels = {"宠物", "日程", "音效"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = makeContextBtn(labels[i]);
        auto *host = new SettingsTagHost(btn, page);
        m_settingsTagHosts.append(host);
        group->addButton(btn, i);
        root->addWidget(host);
        if (i == 0) btn->setChecked(true);
    }

    m_settingsIndicator = new QWidget(page);
    m_settingsIndicator->setFixedSize(4, 20);
    m_settingsIndicator->setStyleSheet(
        "background:" + QString(Theme::Primary) + ";"
        "border-radius:2px;");
    m_settingsIndicator->raise();
    m_settingsIndicator->hide();

    connect(group, &QButtonGroup::idClicked, this, [this, settingsPanel](int id) {
        const int targetIndex = qBound(0, id, m_settingsTagHosts.size() - 1);
        if (targetIndex == m_settingsCurrentIndex)
            return;

        m_settingsCurrentIndex = targetIndex;
        settingsPanel->setCategory(id);
        moveSettingsIndicator(m_settingsCurrentIndex, true);
    });
    root->addStretch();
    return page;
}

QWidget *MainMenu::makePlaceholder(const QString &text) {
    auto *w   = new QWidget;
    auto *lbl = new QLabel(text);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet(
        "color:" + QString(Theme::TextTertiary) + ";"
        "font-size: 19px;"
    );
    auto *lay = new QVBoxLayout(w);
    lay->addWidget(lbl);
    return w;
}

QWidget *MainMenu::makeOtherContext(OtherPanel *otherPanel) {
    auto *page = new QWidget;
    m_otherContext = page;
    m_otherTagHosts.clear();
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(14, 22, 14, 18);
    root->setSpacing(4);

    auto *group = new QButtonGroup(page);
    group->setExclusive(true);

    const QStringList labels = {"版本信息", "数据管理", "帮助与反馈"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = makeContextBtn(labels[i]);
        auto *host = new SettingsTagHost(btn, page);
        m_otherTagHosts.append(host);
        group->addButton(btn, i);
        root->addWidget(host);
        if (i == 0) btn->setChecked(true);
    }

    m_otherIndicator = new QWidget(page);
    m_otherIndicator->setFixedSize(4, 20);
    m_otherIndicator->setStyleSheet(
        "background:" + QString(Theme::Primary) + ";"
        "border-radius:2px;");
    m_otherIndicator->raise();
    m_otherIndicator->hide();

    connect(group, &QButtonGroup::idClicked, this, [this, otherPanel](int id) {
        const int targetIndex = qBound(0, id, m_otherTagHosts.size() - 1);
        if (targetIndex == m_otherCurrentIndex)
            return;

        m_otherCurrentIndex = targetIndex;
        otherPanel->setCategory(id);
        moveOtherIndicator(m_otherCurrentIndex, true);
    });
    root->addStretch();
    return page;
}

void MainMenu::switchPage(int id) {
    if (!m_stack || !m_contextStack) return;

    const int target = qBound(0, id, 3);
    if (m_pageSwitching) {
        m_pendingPage = (target == m_currentPage) ? -1 : target;
        if (m_navGrp) {
            if (auto *button = m_navGrp->button(m_currentPage)) {
                QSignalBlocker blocker(m_navGrp);
                button->setChecked(true);
            }
        }
        return;
    }

    if (target == m_currentPage)
        return;

    m_pendingPage = -1;
    m_pageSwitching = true;
    const quint64 token = ++m_switchToken;

    auto makeFinishBranch = [this, target, token](int branchCount) {
        auto remaining = std::make_shared<int>(branchCount);
        auto completed = std::make_shared<QVector<bool>>(branchCount, false);
        return [this, target, token, remaining, completed](int branch) {
            if (token != m_switchToken || branch < 0 || branch >= completed->size() || completed->at(branch))
                return;

            (*completed)[branch] = true;
            --(*remaining);
            if (*remaining == 0)
                completePageSwitch(target, token);
        };
    };

    if (m_currentPage == 1 && m_calendarPanel) {
        auto finishBranch = makeFinishBranch(2);
        playContextExit(m_currentPage, [finishBranch]() { finishBranch(0); });
        m_calendarPanel->playTimelineExit([finishBranch]() { finishBranch(1); });
        QTimer::singleShot(700, this, [finishBranch]() {
            finishBranch(0);
            finishBranch(1);
        });
        return;
    }
    if (m_currentPage == 2 && m_settingsPanel) {
        auto finishBranch = makeFinishBranch(2);
        playContextExit(m_currentPage, [finishBranch]() { finishBranch(0); });
        m_settingsPanel->playGroupsExit([finishBranch]() { finishBranch(1); });
        QTimer::singleShot(700, this, [finishBranch]() {
            finishBranch(0);
            finishBranch(1);
        });
        return;
    }
    if (m_currentPage == 3 && m_otherPanel) {
        auto finishBranch = makeFinishBranch(2);
        playContextExit(m_currentPage, [finishBranch]() { finishBranch(0); });
        m_otherPanel->playGroupsExit([finishBranch]() { finishBranch(1); });
        QTimer::singleShot(700, this, [finishBranch]() {
            finishBranch(0);
            finishBranch(1);
        });
        return;
    }

    auto finishBranch = makeFinishBranch(1);
    playContextExit(m_currentPage, [finishBranch]() {
        finishBranch(0);
    });
    QTimer::singleShot(700, this, [finishBranch]() {
        finishBranch(0);
    });
}

void MainMenu::completePageSwitch(int id, quint64 token) {
    if (token != m_switchToken)
        return;

    if (!m_stack || !m_contextStack) {
        m_pageSwitching = false;
        return;
    }

    const int target = qBound(0, id, 3);
    if (target == 1 && m_scheduleReveal)
        static_cast<ContextPopHost *>(m_scheduleReveal)->preparePop();
    if (target == 1 && m_calendarPanel)
        m_calendarPanel->prepareTimelineEnter();
    if (target == 2)
        prepareSettingsContextEnter();
    if (target == 2 && m_settingsPanel)
        m_settingsPanel->prepareGroupsEnter();
    if (target == 3)
        prepareOtherContextEnter();
    if (target == 3 && m_otherPanel)
        m_otherPanel->prepareGroupsEnter();
    prepareRightSurfaceEnter(target);

    m_stack->setCurrentIndex(target);
    m_contextStack->setCurrentIndex(target);
    if (target == 2)
        prepareSettingsContextEnter();
    if (target == 2 && m_settingsPanel)
        m_settingsPanel->prepareGroupsEnter();
    if (target == 3)
        prepareOtherContextEnter();
    if (target == 3 && m_otherPanel)
        m_otherPanel->prepareGroupsEnter();
    updateRightSurfaceStyle(target);

    static constexpr int ContextWidths[] = {260, 320, 120, 150};
    animateContextWidth(ContextWidths[target]);
    m_currentPage = target;
    if (m_navGrp) {
        if (auto *button = m_navGrp->button(target)) {
            QSignalBlocker blocker(m_navGrp);
            button->setChecked(true);
        }
    }

    playContextEnter(target, token);
    if (target == 2 && m_settingsPanel) {
        QTimer::singleShot(40, m_settingsPanel, [this, token]() {
            if (token == m_switchToken && m_currentPage == 2 && m_settingsPanel)
                m_settingsPanel->playGroupsEnter();
        });
    }
    if (target == 3 && m_otherPanel) {
        QTimer::singleShot(40, m_otherPanel, [this, token]() {
            if (token == m_switchToken && m_currentPage == 3 && m_otherPanel)
                m_otherPanel->playGroupsEnter();
        });
    }
    playRightSurfaceEnter(target, token);
    if (target == 1 && m_calendarPanel) {
        QTimer::singleShot(40, m_calendarPanel, [this, token]() {
            if (token == m_switchToken && m_currentPage == 1 && m_calendarPanel)
                m_calendarPanel->playTimelineEnter();
        });
    }
    m_pageSwitching = false;

    const int pending = m_pendingPage;
    m_pendingPage = -1;
    if (pending >= 0 && pending != m_currentPage) {
        QTimer::singleShot(0, this, [this, pending, token]() {
            if (token == m_switchToken && !m_pageSwitching)
                switchPage(pending);
        });
    }
}

void MainMenu::playContextExit(int id, const std::function<void()> &finished) {
    if (id == 0) {
        if (auto *reveal = qobject_cast<PetRevealWidget *>(m_petReveal)) {
            reveal->playExit(finished);
            return;
        }
    } else if (id == 1 && m_scheduleReveal) {
        static_cast<ContextPopHost *>(m_scheduleReveal)->playExit(finished);
        return;
    }

    if (finished)
        finished();
}

void MainMenu::playContextEnter(int id, quint64 token) {
    if (id == 0) {
        if (auto *reveal = qobject_cast<PetRevealWidget *>(m_petReveal))
            reveal->playPop();
    } else if (id == 1 && m_scheduleReveal) {
        QTimer::singleShot(40, m_scheduleReveal, [this, token]() {
            if (token == m_switchToken && m_contextStack && m_contextStack->currentIndex() == 1 && m_scheduleReveal)
                static_cast<ContextPopHost *>(m_scheduleReveal)->playPop();
        });
    } else if (id == 2) {
        QTimer::singleShot(40, this, [this, token]() {
            if (token == m_switchToken && m_contextStack && m_contextStack->currentIndex() == 2)
                playSettingsContextEnter();
        });
    } else if (id == 3) {
        QTimer::singleShot(40, this, [this, token]() {
            if (token == m_switchToken && m_contextStack && m_contextStack->currentIndex() == 3)
                playOtherContextEnter();
        });
    }
}

void MainMenu::prepareSettingsContextEnter() {
    for (QWidget *host : m_settingsTagHosts) {
        if (auto *tagHost = static_cast<SettingsTagHost *>(host))
            tagHost->prepareEnter();
    }

    if (!m_settingsContext || !m_settingsIndicator || m_settingsTagHosts.isEmpty())
        return;

    if (auto *layout = m_settingsContext->layout())
        layout->activate();

    if (m_settingsIndicatorAnim) {
        m_settingsIndicatorAnim->stop();
        m_settingsIndicatorAnim->deleteLater();
        m_settingsIndicatorAnim = nullptr;
    }

    const int currentIndex = qBound(0, m_settingsCurrentIndex, m_settingsTagHosts.size() - 1);
    QWidget *host = m_settingsTagHosts[currentIndex];
    const QPoint hostPos = host->mapTo(m_settingsContext, QPoint(0, 0));
    const int targetX = hostPos.x();
    const int targetY = hostPos.y() + (host->height() - m_settingsIndicator->height()) / 2;

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(m_settingsIndicator->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_settingsIndicator);
        m_settingsIndicator->setGraphicsEffect(effect);
    }
    effect->setOpacity(0.0);
    m_settingsIndicator->move(targetX + qRound(SettingsTagHost::startOffset()), targetY);
    m_settingsIndicator->show();
    m_settingsIndicator->raise();
}

void MainMenu::playSettingsContextEnter() {
    for (int i = 0; i < m_settingsTagHosts.size(); ++i) {
        if (auto *tagHost = static_cast<SettingsTagHost *>(m_settingsTagHosts[i]))
            tagHost->playEnter(SettingsTagHost::delayForIndex(i));
    }

    if (!m_settingsContext || !m_settingsIndicator || m_settingsTagHosts.isEmpty())
        return;

    const int currentIndex = qBound(0, m_settingsCurrentIndex, m_settingsTagHosts.size() - 1);
    QWidget *host = m_settingsTagHosts[currentIndex];
    const QPoint hostPos = host->mapTo(m_settingsContext, QPoint(0, 0));
    const int targetX = hostPos.x();
    const int targetY = hostPos.y() + (host->height() - m_settingsIndicator->height()) / 2;

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(m_settingsIndicator->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_settingsIndicator);
        m_settingsIndicator->setGraphicsEffect(effect);
    }
    effect->setOpacity(0.0);
    m_settingsIndicator->move(targetX + qRound(SettingsTagHost::startOffset()), targetY);
    m_settingsIndicator->show();
    m_settingsIndicator->raise();

    if (m_settingsIndicatorAnim) {
        m_settingsIndicatorAnim->stop();
        m_settingsIndicatorAnim->deleteLater();
    }

    QTimer::singleShot(SettingsTagHost::delayForIndex(currentIndex), m_settingsIndicator,
                       [this, effect, targetX, targetY, currentIndex]() {
        if (m_currentPage != 2 || m_settingsCurrentIndex != currentIndex || !m_settingsIndicator)
            return;

        m_settingsIndicatorAnim = new QVariantAnimation(this);
        m_settingsIndicatorAnim->setDuration(SettingsTagHost::animMs());
        m_settingsIndicatorAnim->setEasingCurve(QEasingCurve::Linear);
        m_settingsIndicatorAnim->setStartValue(0.0);
        m_settingsIndicatorAnim->setEndValue(1.0);
        connect(m_settingsIndicatorAnim, &QVariantAnimation::valueChanged, this,
                [this, effect, targetX, targetY](const QVariant &value) {
            const qreal t = qBound<qreal>(0.0, value.toReal(), 1.0);
            effect->setOpacity(easeOutCubicValue(t));

            qreal offset = 0.0;
            const qreal forwardPart = SettingsTagHost::forwardPart();
            if (t < forwardPart)
                offset = interpolate(SettingsTagHost::startOffset(), SettingsTagHost::overshootOffset(),
                                     easeOutCubicValue(t / forwardPart));
            else
                offset = interpolate(SettingsTagHost::overshootOffset(), 0.0,
                                     easeOutCubicValue((t - forwardPart) / (1.0 - forwardPart)));

            if (m_settingsIndicator)
                m_settingsIndicator->move(targetX + qRound(offset), targetY);
        });
        connect(m_settingsIndicatorAnim, &QVariantAnimation::finished, this, [this, effect, targetX, targetY]() {
            effect->setOpacity(1.0);
            if (m_settingsIndicator) {
                m_settingsIndicator->move(targetX, targetY);
                m_settingsIndicator->raise();
            }
            m_settingsIndicatorAnim = nullptr;
        });
        m_settingsIndicatorAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainMenu::moveSettingsIndicator(int index, bool animated) {
    if (!m_settingsContext || !m_settingsIndicator || index < 0 || index >= m_settingsTagHosts.size())
        return;

    QWidget *host = m_settingsTagHosts[index];
    if (!host)
        return;

    const QPoint hostPos = host->mapTo(m_settingsContext, QPoint(0, 0));
    const int targetX = hostPos.x();
    const int targetY = hostPos.y() + (host->height() - m_settingsIndicator->height()) / 2;

    m_settingsIndicator->show();
    m_settingsIndicator->raise();

    if (m_settingsIndicatorAnim) {
        m_settingsIndicatorAnim->stop();
        m_settingsIndicatorAnim->deleteLater();
        m_settingsIndicatorAnim = nullptr;
    }

    if (!animated) {
        m_settingsIndicator->move(targetX, targetY);
        return;
    }

    m_settingsIndicatorAnim = new QVariantAnimation(this);
    m_settingsIndicatorAnim->setDuration(180);
    m_settingsIndicatorAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_settingsIndicatorAnim->setStartValue(m_settingsIndicator->pos().y());
    m_settingsIndicatorAnim->setEndValue(targetY);
    m_settingsIndicator->move(targetX, m_settingsIndicator->y());
    connect(m_settingsIndicatorAnim, &QVariantAnimation::valueChanged, this, [this, targetX](const QVariant &value) {
        if (m_settingsIndicator)
            m_settingsIndicator->move(targetX, value.toInt());
    });
    connect(m_settingsIndicatorAnim, &QVariantAnimation::finished, this, [this, targetX, targetY]() {
        if (m_settingsIndicator)
            m_settingsIndicator->move(targetX, targetY);
        m_settingsIndicatorAnim = nullptr;
    });
    m_settingsIndicatorAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainMenu::prepareOtherContextEnter() {
    for (QWidget *host : m_otherTagHosts) {
        if (auto *tagHost = static_cast<SettingsTagHost *>(host))
            tagHost->prepareEnter();
    }

    if (!m_otherContext || !m_otherIndicator || m_otherTagHosts.isEmpty())
        return;

    if (auto *layout = m_otherContext->layout())
        layout->activate();

    if (m_otherIndicatorAnim) {
        m_otherIndicatorAnim->stop();
        m_otherIndicatorAnim->deleteLater();
        m_otherIndicatorAnim = nullptr;
    }

    const int currentIndex = qBound(0, m_otherCurrentIndex, m_otherTagHosts.size() - 1);
    QWidget *host = m_otherTagHosts[currentIndex];
    const QPoint hostPos = host->mapTo(m_otherContext, QPoint(0, 0));
    const int targetX = hostPos.x();
    const int targetY = hostPos.y() + (host->height() - m_otherIndicator->height()) / 2;

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(m_otherIndicator->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_otherIndicator);
        m_otherIndicator->setGraphicsEffect(effect);
    }
    effect->setOpacity(0.0);
    m_otherIndicator->move(targetX + qRound(SettingsTagHost::startOffset()), targetY);
    m_otherIndicator->show();
    m_otherIndicator->raise();
}

void MainMenu::playOtherContextEnter() {
    for (int i = 0; i < m_otherTagHosts.size(); ++i) {
        if (auto *tagHost = static_cast<SettingsTagHost *>(m_otherTagHosts[i]))
            tagHost->playEnter(SettingsTagHost::delayForIndex(i));
    }

    if (!m_otherContext || !m_otherIndicator || m_otherTagHosts.isEmpty())
        return;

    const int currentIndex = qBound(0, m_otherCurrentIndex, m_otherTagHosts.size() - 1);
    QWidget *host = m_otherTagHosts[currentIndex];
    const QPoint hostPos = host->mapTo(m_otherContext, QPoint(0, 0));
    const int targetX = hostPos.x();
    const int targetY = hostPos.y() + (host->height() - m_otherIndicator->height()) / 2;

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(m_otherIndicator->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(m_otherIndicator);
        m_otherIndicator->setGraphicsEffect(effect);
    }
    effect->setOpacity(0.0);
    m_otherIndicator->move(targetX + qRound(SettingsTagHost::startOffset()), targetY);
    m_otherIndicator->show();
    m_otherIndicator->raise();

    if (m_otherIndicatorAnim) {
        m_otherIndicatorAnim->stop();
        m_otherIndicatorAnim->deleteLater();
    }

    QTimer::singleShot(SettingsTagHost::delayForIndex(currentIndex), m_otherIndicator,
                       [this, effect, targetX, targetY, currentIndex]() {
        if (m_currentPage != 3 || m_otherCurrentIndex != currentIndex || !m_otherIndicator)
            return;

        m_otherIndicatorAnim = new QVariantAnimation(this);
        m_otherIndicatorAnim->setDuration(SettingsTagHost::animMs());
        m_otherIndicatorAnim->setEasingCurve(QEasingCurve::Linear);
        m_otherIndicatorAnim->setStartValue(0.0);
        m_otherIndicatorAnim->setEndValue(1.0);
        connect(m_otherIndicatorAnim, &QVariantAnimation::valueChanged, this,
                [this, effect, targetX, targetY](const QVariant &value) {
            const qreal t = qBound<qreal>(0.0, value.toReal(), 1.0);
            effect->setOpacity(easeOutCubicValue(t));

            qreal offset = 0.0;
            const qreal forwardPart = SettingsTagHost::forwardPart();
            if (t < forwardPart)
                offset = interpolate(SettingsTagHost::startOffset(), SettingsTagHost::overshootOffset(),
                                     easeOutCubicValue(t / forwardPart));
            else
                offset = interpolate(SettingsTagHost::overshootOffset(), 0.0,
                                     easeOutCubicValue((t - forwardPart) / (1.0 - forwardPart)));

            if (m_otherIndicator)
                m_otherIndicator->move(targetX + qRound(offset), targetY);
        });
        connect(m_otherIndicatorAnim, &QVariantAnimation::finished, this, [this, effect, targetX, targetY]() {
            effect->setOpacity(1.0);
            if (m_otherIndicator) {
                m_otherIndicator->move(targetX, targetY);
                m_otherIndicator->raise();
            }
            m_otherIndicatorAnim = nullptr;
        });
        m_otherIndicatorAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainMenu::moveOtherIndicator(int index, bool animated) {
    if (!m_otherContext || !m_otherIndicator || index < 0 || index >= m_otherTagHosts.size())
        return;

    QWidget *host = m_otherTagHosts[index];
    if (!host)
        return;

    const QPoint hostPos = host->mapTo(m_otherContext, QPoint(0, 0));
    const int targetX = hostPos.x();
    const int targetY = hostPos.y() + (host->height() - m_otherIndicator->height()) / 2;

    m_otherIndicator->show();
    m_otherIndicator->raise();

    if (m_otherIndicatorAnim) {
        m_otherIndicatorAnim->stop();
        m_otherIndicatorAnim->deleteLater();
        m_otherIndicatorAnim = nullptr;
    }

    if (!animated) {
        m_otherIndicator->move(targetX, targetY);
        return;
    }

    m_otherIndicatorAnim = new QVariantAnimation(this);
    m_otherIndicatorAnim->setDuration(180);
    m_otherIndicatorAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_otherIndicatorAnim->setStartValue(m_otherIndicator->pos().y());
    m_otherIndicatorAnim->setEndValue(targetY);
    m_otherIndicator->move(targetX, m_otherIndicator->y());
    connect(m_otherIndicatorAnim, &QVariantAnimation::valueChanged, this, [this, targetX](const QVariant &value) {
        if (m_otherIndicator)
            m_otherIndicator->move(targetX, value.toInt());
    });
    connect(m_otherIndicatorAnim, &QVariantAnimation::finished, this, [this, targetX, targetY]() {
        if (m_otherIndicator)
            m_otherIndicator->move(targetX, targetY);
        m_otherIndicatorAnim = nullptr;
    });
    m_otherIndicatorAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainMenu::animateContextWidth(int targetWidth) {
    if (!m_contextPanel) return;

    if (!m_contextWidthAnim) {
        m_contextWidthAnim = new QVariantAnimation(this);
        m_contextWidthAnim->setDuration(260);
        m_contextWidthAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_contextWidthAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            m_contextPanel->setFixedWidth(value.toInt());
        });
    }

    if (targetWidth > 0)
        m_contextPanel->show();

    m_contextWidthAnim->stop();
    m_contextWidthAnim->setStartValue(m_contextPanel->width());
    m_contextWidthAnim->setEndValue(targetWidth);
    m_contextWidthAnim->start();
}

void MainMenu::prepareRightSurfaceEnter(int id) {
    if (!m_rightSurface)
        return;

    if (m_rightSurfaceHeightAnim) {
        m_rightSurfaceHeightAnim->stop();
        m_rightSurfaceHeightAnim->deleteLater();
        m_rightSurfaceHeightAnim = nullptr;
    }

    if (id == 1) {
        setRightSurfaceTopAligned(true);
        m_rightSurface->setFixedHeight(0);
    } else {
        setRightSurfaceTopAligned(false);
        m_rightSurface->setMinimumHeight(0);
        m_rightSurface->setMaximumHeight(QWIDGETSIZE_MAX);
    }
}

void MainMenu::playRightSurfaceEnter(int id, quint64 token) {
    if (!m_rightSurface)
        return;

    if (id != 1) {
        m_rightSurface->setMinimumHeight(0);
        m_rightSurface->setMaximumHeight(QWIDGETSIZE_MAX);
        return;
    }

    QTimer::singleShot(40, m_rightSurface, [this, token]() {
        if (token != m_switchToken || m_currentPage != 1 || !m_rightSurface)
            return;

        const QWidget *wrap = m_rightSurface->parentWidget();
        int targetHeight = qMax(0, height() - 92);
        if (wrap) {
            targetHeight = wrap->height();
            if (auto *layout = wrap->layout()) {
                const QMargins margins = layout->contentsMargins();
                targetHeight -= margins.top() + margins.bottom();
            }
        }
        targetHeight = qMax(0, targetHeight);
        if (targetHeight <= 0) {
            m_rightSurface->setMinimumHeight(0);
            m_rightSurface->setMaximumHeight(QWIDGETSIZE_MAX);
            return;
        }

        if (m_rightSurfaceHeightAnim) {
            m_rightSurfaceHeightAnim->stop();
            m_rightSurfaceHeightAnim->deleteLater();
        }

        m_rightSurfaceHeightAnim = new QVariantAnimation(this);
        m_rightSurfaceHeightAnim->setDuration(240);
        m_rightSurfaceHeightAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_rightSurfaceHeightAnim->setStartValue(m_rightSurface->height());
        m_rightSurfaceHeightAnim->setEndValue(targetHeight);
        connect(m_rightSurfaceHeightAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            if (m_rightSurface)
                m_rightSurface->setFixedHeight(value.toInt());
        });
        connect(m_rightSurfaceHeightAnim, &QVariantAnimation::finished, this, [this, targetHeight, token]() {
            if (token == m_switchToken && m_currentPage == 1 && m_rightSurface) {
                m_rightSurface->setFixedHeight(targetHeight);
                m_rightSurface->setMaximumHeight(QWIDGETSIZE_MAX);
                m_rightSurface->setMinimumHeight(0);
                setRightSurfaceTopAligned(false);
            }
            m_rightSurfaceHeightAnim = nullptr;
        });
        m_rightSurfaceHeightAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainMenu::setRightSurfaceTopAligned(bool aligned) {
    if (!m_rightSurface || !m_rightSurface->parentWidget())
        return;

    auto *layout = m_rightSurface->parentWidget()->layout();
    if (!layout)
        return;

    if (auto *item = layout->itemAt(0)) {
        item->setAlignment(aligned ? Qt::AlignTop : Qt::Alignment());
        layout->invalidate();
    }
}

void MainMenu::updatePetContext(const QString &petId) {
    if (auto *reveal = qobject_cast<PetRevealWidget *>(m_petReveal))
        reveal->setPet(petId);
}

void MainMenu::updateRightSurfaceStyle(int pageId) {
    if (!m_rightSurface) return;

    if (pageId == 0 || pageId == 2 || pageId == 3) {
        m_rightSurface->setStyleSheet(
            "#rightContentSurface {"
            "  background: transparent;"
            "  border: none;"
            "}");
    } else {
        m_rightSurface->setStyleSheet(
            "#rightContentSurface {"
            "  background:" + QString(Theme::BgPrimary) + ";"
            "  border: 1px solid " + Theme::Border + ";"
            "  border-radius: 8px;"
            "}");
    }
}

bool MainMenu::eventFilter(QObject *watched, QEvent *event) {
    if (watched->property("windowDragArea").toBool()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragStartGlobal = mouseEvent->globalPosition().toPoint();
                m_windowStartPos = frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_dragging) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            move(m_windowStartPos + mouseEvent->globalPosition().toPoint() - m_dragStartGlobal);
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

#include "MainMenu.moc"
