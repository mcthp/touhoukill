#include "chatwidget.h"
#include "engine.h"
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPainter>

MyPixmapItem::MyPixmapItem(const QPixmap &pixmap, QGraphicsItem *parentItem)
    : QGraphicsPixmapItem(pixmap, parentItem)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    initFaceBoardPos();
    initEasyTextPos();
    easytext = Sanguosha->getChattingEasyTexts();
}

void MyPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setVisible(false);
    QString msg;
    int result = mouseCanClick(event->pos().x(), event->pos().y());
    if (result == -1)
        return;
    if (itemName == QStringLiteral("faceboard"))
        msg = QStringLiteral("<#") + QString::number(result + 1) + QStringLiteral("#>");
    else if (itemName == QStringLiteral("easytextboard"))
        msg = easytext.at(result);

    emit my_pixmap_item_msg(msg);
}

void MyPixmapItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (mouseCanClick(event->pos().x(), event->pos().y()) != -1)
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void MyPixmapItem::setSize(int x, int y)
{
    sizex = x;
    sizey = y;
}

int MyPixmapItem::mouseCanClick(int x, int y)
{
    int result = -1;
    if (itemName == QStringLiteral("faceboard"))
        result = mouseOnIcon(x, y);
    else if (itemName == QStringLiteral("easytextboard"))
        result = mouseOnText(x, y);
    return result;
}

int MyPixmapItem::mouseOnIcon(int x, int y)
{
    int result = -1;
    for (int i = 0; i < faceboardPos.size(); i++) {
        QRect rect = faceboardPos.at(i);
        if (rect.contains(x, y)) {
            result = i;
            break;
        }
    }

    return result;
}

int MyPixmapItem::mouseOnText(int x, int y)
{
    int result = -1;
    for (int i = 0; i < easytextPos.size(); i++) {
        QRect rect = easytextPos.at(i);
        if (rect.contains(x, y)) {
            result = i;
            break;
        }
    }
    return result;
}

QRectF MyPixmapItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), QSizeF(sizex, sizey));
}

void MyPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->drawPixmap(boundingRect().toRect(), pixmap());
}

void MyPixmapItem::initFaceBoardPos()
{
    const int start_x = 5;
    const int start_y = 5;
    int x = 0;
    int y = 0;
    int icon_w = 16;
    int icon_h = 16;
    int x_offset = 6;
    int y_offset = 6;

    // total 7 x 8 icons in QList <QRect> faceboardPos;
    for (int j = 0; j < 8; j++) {
        y = j * (icon_h + y_offset) + start_y;
        for (int i = 0; i < 7; i++) {
            x = i * (icon_w + x_offset) + start_x;
            faceboardPos << QRect(x, y, icon_w, icon_h);
        }
    }
}

void MyPixmapItem::initEasyTextPos()
{
    const int start_x = 5;
    const int start_y = 5;
    int y = 0;
    int icon_w = 210;
    int icon_h = 12;
    int y_offset = 10;

    // only 10 text QList <QRect> easytextPos;
    for (int j = 0; j < 10; j++) {
        y = j * (icon_h + y_offset) + start_y;
        easytextPos << QRect(start_x, y, icon_w, icon_h);
    }
}

ChatWidget::ChatWidget()
    : base_pixmap(QStringLiteral("image/system/chatface/base.png"))
{
    setFlags(ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    base = new QGraphicsRectItem(QRectF(base_pixmap.rect()), this);
    QPushButton *returnButton = nullptr;
    QPushButton *chatfaceButton = nullptr;
    QPushButton *easytextButton = nullptr;

    returnButton = addButton(QStringLiteral("returnBt"), -1);
    chatfaceButton = addButton(QStringLiteral("chatfaceBt"), 24);
    easytextButton = addButton(QStringLiteral("easytextBt"), 48 + 1);

    chat_face_board = new MyPixmapItem(QPixmap(QStringLiteral("image/system/chatface/faceboard.png")), this);
    chat_face_board->setSize(160, 180);
    chat_face_board->setPos(-160 + 74, -180 - 1); // 24 + 24 + 24 + 2 = 74
    chat_face_board->setZValue(10000);
    chat_face_board->setVisible(false);
    chat_face_board->itemName = QStringLiteral("faceboard");

    easy_text_board = new MyPixmapItem(QPixmap(QStringLiteral("image/system/chatface/easytextboard.png")), this);
    easy_text_board->setSize(180, 222);
    easy_text_board->setPos(-106, -223);
    easy_text_board->setZValue(10000);
    easy_text_board->setVisible(false);
    easy_text_board->itemName = QStringLiteral("easytextboard");

    connect(chat_face_board, &MyPixmapItem::my_pixmap_item_msg, this, &ChatWidget::chat_widget_msg);
    connect(easy_text_board, &MyPixmapItem::my_pixmap_item_msg, this, &ChatWidget::chat_widget_msg);
    connect(chatfaceButton, &QAbstractButton::clicked, this, &ChatWidget::showFaceBoard);
    connect(easytextButton, &QAbstractButton::clicked, this, &ChatWidget::showEasyTextBoard);
    connect(returnButton, &QAbstractButton::clicked, this, &ChatWidget::sendText);
}

void ChatWidget::showEasyTextBoard()
{
    easy_text_board->setVisible(!easy_text_board->isVisible());
    chat_face_board->setVisible(false);
}

void ChatWidget::showFaceBoard()
{
    chat_face_board->setVisible(!chat_face_board->isVisible());
    easy_text_board->setVisible(false);
}

void ChatWidget::sendText()
{
    chat_face_board->setVisible(false);
    easy_text_board->setVisible(false);
    emit return_button_click();
}

QRectF ChatWidget::boundingRect() const
{
    return QRectF(-1, 0, 24 * 3 + 2, 24);
}

void ChatWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->drawPixmap(base->pos(), base_pixmap);
}

QPushButton *ChatWidget::createButton(const QString &name)
{
    QPushButton *button = new QPushButton;
    button->setEnabled(true);

    QPixmap iconOn(QStringLiteral("image/system/chatface/%1On.png").arg(name));
    QPixmap iconOff(QStringLiteral("image/system/chatface/%1.png").arg(name));

    QIcon icon;
    icon.addPixmap(iconOff, QIcon::Normal, QIcon::Off);
    icon.addPixmap(iconOn, QIcon::Active, QIcon::Off);

    button->setIcon(icon);
    button->setIconSize(iconOn.size());
    button->setFixedSize(iconOn.size());
    button->setObjectName(name);

    button->setAttribute(Qt::WA_TranslucentBackground);
    return button;
}

QPushButton *ChatWidget::addButton(const QString &name, int x)
{
    QPushButton *button = createButton(name);
    addWidget(button, x);
    return button;
}

QGraphicsProxyWidget *ChatWidget::addWidget(QWidget *widget, int x)
{
    QGraphicsProxyWidget *proxy_widget = new QGraphicsProxyWidget(this);
    proxy_widget->setWidget(widget);
    proxy_widget->setParentItem(base);
    proxy_widget->setPos(x, 0);

    return proxy_widget;
}
