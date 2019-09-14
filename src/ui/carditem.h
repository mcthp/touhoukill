#ifndef _CARD_ITEM_H
#define _CARD_ITEM_H

#include "QSanSelectableItem.h"
#include "SkinBank.h"
#include "card.h"
#include "settings.h"

#include <QAbstractAnimation>
#include <QMutex>
#include <QSize>

class FilterSkill;
class General;

class CardItem : public QSanSelectableItem
{
    Q_OBJECT

public:
    explicit CardItem(const Card *card);
    explicit CardItem(const QString &general_name);
    ~CardItem();

    virtual QRectF boundingRect() const;
    virtual void setEnabled(bool enabled);

    const Card *getCard() const;
    void setCard(const Card *card);
    inline int getId() const
    {
        return m_cardId;
    }

    // For move card animation
    void setHomePos(QPointF home_pos);
    QPointF homePos() const;
    QAbstractAnimation *getGoBackAnimation(bool doFadeEffect, bool smoothTransition = false, int duration = Config.S_MOVE_CARD_ANIMATION_DURATION);
    void goBack(bool playAnimation, bool doFade = true);
    inline QAbstractAnimation *getCurrentAnimation()
    {
        return m_currentAnimation;
    }
    inline void setHomeOpacity(double opacity)
    {
        m_opacityAtHome = opacity;
    }
    inline double getHomeOpacity()
    {
        return m_opacityAtHome;
    }

    void showFrame(const QString &frame);
    void hideFrame();
    void showAvatar(const General *general);
    void hideAvatar();
    void setAutoBack(bool auto_back);
    void changeGeneral(const QString &general_name);
    void setFootnote(const QString &desc);

    inline bool isSelected() const
    {
        return m_isSelected;
    }
    inline void setSelected(bool selected)
    {
        m_isSelected = selected;
    }
    bool isEquipped() const;

    void setFrozen(bool is_frozen);
    inline bool isFrozen()
    {
        return frozen;
    }
    inline void showFootnote()
    {
        _m_showFootnote = true;
    }
    inline void hideFootnote()
    {
        _m_showFootnote = false;
    }
    inline QString getFootnote()
    {
        return footnote;
    }

    static CardItem *FindItem(const QList<CardItem *> &items, int card_id);

    struct UiHelper
    {
        int tablePileClearTimeStamp;
    } m_uiHelper;

    void clickItem()
    {
        emit clicked();
    }

    void setOuterGlowEffectEnabled(const bool &willPlay);
    bool isOuterGlowEffectEnabled() const;

    void setOuterGlowColor(const QColor &color);
    QColor getOuterGlowColor() const;

private slots:
    void currentAnimationDestroyed();
    void animationFinished();

protected:
    void _initialize();
    
    QImage _m_footnoteImage;
    bool _m_showFootnote;
    QString footnote;
    // QGraphicsPixmapItem *_m_footnoteItem;
    
    double m_opacityAtHome;
    bool m_isSelected;
    bool _m_isUnknownGeneral;
    static const int _S_CLICK_JITTER_TOLERANCE;
    static const int _S_MOVE_JITTER_TOLERANCE;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    //virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    bool auto_back, frozen;

private:
    QAbstractAnimation *m_currentAnimation;
    QMutex m_animationMutex;

    int m_cardId;
    QString _m_frameType, _m_avatarName;
    QPointF home_pos;
    QPointF _m_lastMousePressScenePos;
    
    bool outerGlowEffectEnabled;
    QColor outerGlowColor;
    QGraphicsDropShadowEffect *outerGlowEffect;

signals:
    void toggle_discards();
    void clicked();
    void double_clicked();
    void thrown();
    void released();
    void enter_hover();
    void leave_hover();
    void movement_animation_finished();
    void general_changed();
    void hoverChanged(const bool &enter);
};

#endif
