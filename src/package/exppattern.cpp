#include "exppattern.h"
#include "engine.h"

ExpPattern::ExpPattern(const QString &exp)
{
    this->exp = exp;
}

bool ExpPattern::match(const Player *player, const Card *card) const
{
    foreach (QString one_exp, exp.split(QLatin1Char('#')))
        if (matchOne(player, card, one_exp))
            return true;

    return false;
}

// '|' means 'and', '#' means 'or'.
// the expression splited by '#' has 3 parts,
// 1st part means the card name, and ',' means more than one options.
// 2nd patt means the card suit, and ',' means more than one options.
// 3rd part means the card number, and ',' means more than one options,
// the number uses '~' to make a scale for valid expressions
bool ExpPattern::matchOne(const Player *player, const Card *card, const QString &exp) const
{
    QStringList factors = exp.split(QLatin1Char('|'));

    // TODO: HACK!!!!
    if (factors.isEmpty())
        return true;

    bool checkpoint = false;
    QStringList card_types = factors.at(0).split(QLatin1Char(','));
    foreach (QString or_name, card_types) {
        checkpoint = false;
        foreach (QString name, or_name.split(QLatin1Char('+'))) {
            if (name == QStringLiteral(".")) {
                checkpoint = true;
            } else {
                bool isInt = false;
                bool positive = true;
                if (name.startsWith(QLatin1Char('^'))) {
                    positive = false;
                    name = name.mid(1);
                }
                //sometimes, the first character need to Upper
                QString kindOfName = name.left(1).toUpper() + name.right(name.length() - 1);
                if (card->face()->isKindOf(kindOfName.toLocal8Bit().data()) || (card->face()->name() == name) || (QStringLiteral("%") + card->faceName() == name)
                    || (card->effectiveID() == name.toInt(&isInt) && isInt))
                    checkpoint = positive;
                else
                    checkpoint = !positive;
            }
            if (!checkpoint)
                break;
        }
        if (checkpoint)
            break;
    }
    if (!checkpoint)
        return false;
    if (factors.size() < 2)
        return true;

    checkpoint = false;
    QStringList card_suits = factors.at(1).split(QLatin1Char(','));
    foreach (QString suit, card_suits) {
        if (suit == QStringLiteral(".")) {
            checkpoint = true;
            break;
        }
        bool positive = true;
        if (suit.startsWith(QLatin1Char('^'))) {
            positive = false;
            suit = suit.mid(1);
        }
        if (card->suitString() == suit || (card->isBlack() && suit == QStringLiteral("black")) || (card->isRed() && suit == QStringLiteral("red")))
            checkpoint = positive;
        else
            checkpoint = !positive;
        if (checkpoint)
            break;
    }
    if (!checkpoint)
        return false;
    if (factors.size() < 3)
        return true;

    checkpoint = false;
    QStringList card_numbers = factors.at(2).split(QLatin1Char(','));
    int cdn = static_cast<int>(card->number());

    foreach (QString number, card_numbers) {
        if (number == QStringLiteral(".")) {
            checkpoint = true;
            break;
        }
        bool isInt = false;
        if (number.contains(QLatin1Char('~'))) {
            QStringList params = number.split(QLatin1Char('~'));
            int from = 0, to = 0;
            if (params.at(0).isEmpty())
                from = 1;
            else
                from = params.at(0).toInt();
            if (params.at(1).isEmpty())
                to = 13;
            else
                to = params.at(1).toInt();

            if (from <= cdn && cdn <= to)
                checkpoint = true;
        } else if (number.toInt(&isInt) == cdn && isInt) {
            checkpoint = true;
        } else if ((number == QStringLiteral("A") && cdn == 1) || (number == QStringLiteral("J") && cdn == 11) || (number == QStringLiteral("Q") && cdn == 12)
                   || (number == QStringLiteral("K") && cdn == 13)) {
            checkpoint = true;
        }
        if (checkpoint)
            break;
    }
    if (!checkpoint)
        return false;
    if (factors.size() < 4)
        return true;

    checkpoint = false;
    QString place = factors.at(3);
    if (!player || place == QStringLiteral("."))
        checkpoint = true;
    if (!checkpoint) {
        bool findOneShow = false; //only for check palce "show"
        bool needCheckShow = place.split(QStringLiteral(",")).contains(QStringLiteral("show")); //only for check palce "show"

        IDSet ids;
        if (card->isVirtualCard())
            ids = card->subcards();
        else
            ids.insert(card->effectiveID());

        if (!ids.isEmpty()) {
            foreach (int id, ids) {
                if (findOneShow)
                    break;
                checkpoint = false;
                const Card *card = player->getRoomObject()->getCard(id);
                foreach (QString p, place.split(QStringLiteral(","))) {
                    if (p == QStringLiteral("equipped") && player->hasEquip(card)) {
                        checkpoint = true;
                    } else if (p == QStringLiteral("hand") && card->effectiveID() >= 0) {
                        foreach (const Card *c, player->getHandcards()) {
                            if (c->effectiveID() == id) {
                                checkpoint = true;
                                break;
                            }
                        }
                    } else if (p == QStringLiteral("handOnly") && card->effectiveID() >= 0) { // exclude shownHandCard
                        foreach (const Card *c, player->getHandcards()) {
                            if (c->effectiveID() == id && !player->getShownHandcards().contains(id)) {
                                checkpoint = true;
                                break;
                            }
                        }
                    } else if (p.startsWith(QStringLiteral("%"))) {
                        p = p.mid(1);
                        foreach (const Player *pl, player->getAliveSiblings())
                            if (!pl->getPile(p).isEmpty() && pl->getPile(p).contains(id)) {
                                checkpoint = true;
                                break;
                            }
                    } else if ((p == QStringLiteral("sqchuangshi")) && card->effectiveID() >= 0 && !player->hasEquip(card)) {
                        checkpoint = true;
                    } else if (p == QStringLiteral("shehuo") && card->effectiveID() >= 0 && !player->hasEquip(card)) {
                        checkpoint = true;
                    } else if (p == QStringLiteral("benwo") && (card->isVirtualCard() || !player->getHandcards().contains(player->getRoomObject()->getCard(card->id())))) {
                        return false;
                    } else if (!player->getPile(p).isEmpty() && player->getPile(p).contains(id)) {
                        checkpoint = true;
                    }
                    if (p == QStringLiteral("show")) {
                        if (player->getShownHandcards().contains(id)) {
                            checkpoint = true;
                            findOneShow = true;
                        }
                    }
                    if (checkpoint)
                        break;
                }
                if (!checkpoint && !needCheckShow)
                    break;
            }
        }
    }

    if (!checkpoint)
        return false;
    if (factors.size() < 5)
        return true;

    // @@Compatibility
    QString color = factors.at(4);
    if (color == QStringLiteral("."))
        return true;
    else if (color == QStringLiteral("red") && card->isRed())
        return true;
    else if (color == QStringLiteral("black") && card->isBlack())
        return true;
    else if (color == QStringLiteral("colorless") && card->suit() == Card::NoSuit)
        return true;

    return false;
}
