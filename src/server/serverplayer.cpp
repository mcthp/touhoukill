#include "serverplayer.h"
#include "card.h"
#include "engine.h"
#include "gamerule.h"
#include "general.h"
#include "recorder.h"
#include "room.h"
#include "settings.h"
#include "skill.h"
#include "util.h"

using namespace QSanProtocol;
using namespace JsonUtils;

const int ServerPlayer::S_NUM_SEMAPHORES = 6;

ServerPlayer::ServerPlayer(Room *room)
    : Player(room)
    , m_expectedReplySerial(0)
    , m_isClientResponseReady(false)
    , m_isWaitingReply(false)
    , socket(nullptr)
    , room(room)
    , recorder(nullptr)
    , _m_phases_index(0)
    , ready(false)
{
    semas = new QSemaphore *[S_NUM_SEMAPHORES];
    for (int i = 0; i < S_NUM_SEMAPHORES; i++)
        semas[i] = new QSemaphore(0);
}

void ServerPlayer::drawCard(const Card *card)
{
    m_handcards << card;
}

Room *ServerPlayer::getRoom() const
{
    return room;
}

void ServerPlayer::broadcastSkillInvoke(const QString &card_name) const
{
    room->broadcastSkillInvoke(card_name, isMale(), -1);
}

void ServerPlayer::broadcastSkillInvoke(const Card *card) const
{
    if (card->mute())
        return;

    QString skill_name = card->skillName();
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill == nullptr) {
        if (card->face()->commonEffectName().isNull())
            broadcastSkillInvoke(card->faceName());
        else
            room->broadcastSkillInvoke(card->face()->commonEffectName(), QStringLiteral("common"));
        return;
    } else {
        int index = skill->getAudioEffectIndex(this, card);
        if (index == 0)
            return;

        if (index == -1 || index == -2) {
            if (card->face()->commonEffectName().isNull())
                broadcastSkillInvoke(card->faceName());
            else
                room->broadcastSkillInvoke(card->face()->commonEffectName(), QStringLiteral("common"));
        } else
            room->broadcastSkillInvoke(skill_name, index);
    }
}

int ServerPlayer::getRandomHandCardId() const
{
    return getRandomHandCard()->effectiveID();
}

const Card *ServerPlayer::getRandomHandCard() const
{
    int index = QRandomGenerator::global()->generate() % m_handcards.length();
    return m_handcards.at(index);
}

void ServerPlayer::obtainCard(const Card *card, bool unhide)
{
    CardMoveReason reason(CardMoveReason::S_REASON_GOTCARD, objectName());
    room->obtainCard(this, card, reason, unhide);
}

void ServerPlayer::throwAllEquips()
{
    QList<const Card *> equips = equipCards();

    if (equips.isEmpty())
        return;

    Card *card = room->cloneCard(QStringLiteral("DummyCard"));
    foreach (const Card *equip, equips) {
        if (!isJilei(card))
            card->addSubcard(equip);
    }
    if (!card->subcards().empty())
        room->throwCard(card, this);
    room->cardDeleting(card);
}

void ServerPlayer::throwAllHandCards()
{
    int card_length = handcardNum();
    room->askForDiscard(this, QString(), card_length, card_length);
}

void ServerPlayer::throwAllHandCardsAndEquips()
{
    int card_length = getCardCount(true);
    room->askForDiscard(this, QString(), card_length, card_length, false, true);
}

void ServerPlayer::throwAllMarks(bool visible_only)
{
    foreach (QString mark_name, marks().keys()) {
        if (!mark_name.startsWith(QStringLiteral("@")))
            continue;

        int n = marks().value(mark_name, 0);
        if (n != 0)
            room->setPlayerMark(this, mark_name, 0);
    }

    //    if (!visible_only)
    //        marks.clear();
}

void ServerPlayer::clearOnePrivatePile(const QString &pile_name)
{
    if (!pileNames().contains(pile_name))
        return;

    Card *dummy = room->cloneCard(QStringLiteral("DummyCard"));
    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, objectName());
    bool notifyLog = true;
    QString new_name = pile_name;
    if (new_name.startsWith(QStringLiteral("#"))) {
        foreach (QString flag, flagList()) {
            if (flag == new_name.mid(1) + QStringLiteral("_InTempMoving")) {
                notifyLog = false;
                break;
            }
        }
    }
    room->throwCard(dummy, reason, nullptr, nullptr, notifyLog);
    room->cardDeleting(dummy);
}

void ServerPlayer::clearPrivatePiles()
{
    foreach (QString pile_name, pileNames())
        clearOnePrivatePile(pile_name);
}

void ServerPlayer::bury()
{
    clearFlags();
    clearHistory();
    throwAllCards();
    throwAllMarks();
    clearPrivatePiles();
    room->clearPlayerCardLimitation(this, false);
    room->setPlayerProperty(this, "dyingFactor", 0);
    if (isRemoved())
        room->setPlayerProperty(this, "removed", false);

    this->tag.remove(QStringLiteral("Huashen_skill"));
    this->tag.remove(QStringLiteral("Huashen_target"));
}

void ServerPlayer::throwAllCards()
{
    Card *card = isKongcheng() ? room->cloneCard(QStringLiteral("DummyCard")) : wholeHandCards();
    foreach (const Card *equip, equipCards())
        card->addSubcard(equip);
    if (!card->subcards().empty())
        room->throwCard(card, this);
    room->cardDeleting(card);

    QList<const Card *> tricks = judgingAreaCards();
    foreach (const Card *trick, tricks) {
        CardMoveReason reason(CardMoveReason::S_REASON_THROW, objectName());
        room->throwCard(trick, reason, nullptr);
    }
}

void ServerPlayer::drawCards(int n, const QString &reason)
{
    room->drawCards(this, n, reason);
}

bool ServerPlayer::askForSkillInvoke(const QString &skill_name, const QVariant &data, const QString &prompt)
{
    return room->askForSkillInvoke(this, skill_name, data, prompt);
}

bool ServerPlayer::askForSkillInvoke(const Skill *skill, const QVariant &data, const QString &prompt)
{
    return room->askForSkillInvoke(this, skill, data, prompt);
}

QList<int> ServerPlayer::forceToDiscard(int discard_num, bool include_equip, bool is_discard)
{
    QList<int> to_discard;

    QString flags = QStringLiteral("h");
    if (include_equip)
        flags.append(QStringLiteral("e"));

    QList<const Card *> all_cards = getCards(flags);
    qShuffle(all_cards);

    for (int i = 0; i < all_cards.length(); i++) {
        if (!is_discard || !isJilei(all_cards.at(i)))
            to_discard << all_cards.at(i)->id();
        if (to_discard.length() == discard_num)
            break;
    }

    return to_discard;
}

void ServerPlayer::setSocket(ClientSocket *socket)
{
    if (this->socket != nullptr) {
        disconnect(this->socket);
        this->socket->disconnect(this);
        this->socket->disconnectFromHost();
        this->socket->deleteLater();
    }

    disconnect(this, SLOT(sendMessage(QString)));

    if (socket != nullptr) {
        connect(socket, &ClientSocket::disconnected, this, &ServerPlayer::disconnected);
        connect(socket, &ClientSocket::message_got, this, &ServerPlayer::getMessage);
        connect(this, &ServerPlayer::message_ready, this, &ServerPlayer::sendMessage);
    }

    this->socket = socket;
}

void ServerPlayer::getMessage(const char *message)
{
    QString request = QString::fromUtf8(message);
    if (request.endsWith(QStringLiteral("\n")))
        request.chop(1);

    emit request_got(request);
}

void ServerPlayer::unicast(const QString &message)
{
    emit message_ready(message);

    if (recorder != nullptr)
        recorder->recordLine(message);
}

void ServerPlayer::startNetworkDelayTest()
{
    test_time = QDateTime::currentDateTime();
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_NETWORK_DELAY_TEST);
    invoke(&packet);
}

qint64 ServerPlayer::endNetworkDelayTest()
{
    return test_time.msecsTo(QDateTime::currentDateTime());
}

void ServerPlayer::startRecord()
{
    delete recorder;
    recorder = new Recorder(this);
}

void ServerPlayer::saveRecord(const QString &filename)
{
    if (recorder != nullptr)
        recorder->save(filename);
}

void ServerPlayer::addToSelected(const QString &general)
{
    selected.append(general);
}

QStringList ServerPlayer::getSelected() const
{
    return selected;
}

QString ServerPlayer::findReasonable(const QStringList &generals, bool no_unreasonable)
{
    foreach (QString name, generals) {
        if (Config.GameMode == QStringLiteral("zombie_mode")) {
            QStringList ban_list = Config.value(QStringLiteral("Banlist/Zombie")).toStringList();
            if (ban_list.contains(name))
                continue;
        }
        if (Config.GameMode.endsWith(QStringLiteral("p")) || Config.GameMode.endsWith(QStringLiteral("pd")) || Config.GameMode.endsWith(QStringLiteral("pz"))) {
            QStringList ban_list = Config.value(QStringLiteral("Banlist/Roles")).toStringList();
            if (ban_list.contains(name))
                continue;
        }

        return name;
    }

    if (no_unreasonable)
        return QString();

    return generals.first();
}

void ServerPlayer::clearSelected()
{
    selected.clear();
}

void ServerPlayer::sendMessage(const QString &message)
{
    if (socket != nullptr) {
#ifndef QT_NO_DEBUG
        printf("%s", qPrintable(objectName()));
#endif
        socket->send(message);
    }
}

void ServerPlayer::invoke(const AbstractPacket *packet)
{
    unicast(packet->toString());
}

void ServerPlayer::invoke(const char *method, const QString &arg)
{
    unicast(QStringLiteral("%1 %2").arg(QString::fromUtf8(method), arg));
}

QString ServerPlayer::reportHeader() const
{
    QString name = objectName();
    return QStringLiteral("%1 ").arg(name.isEmpty() ? tr("Anonymous") : name);
}

QList<const Card *> ServerPlayer::getCards(const QString &flags) const
{
    QList<const Card *> cards;
    if (flags.contains(QStringLiteral("h")) && flags.contains(QStringLiteral("s")))
        cards << m_handcards;
    else if (flags.contains(QStringLiteral("h"))) {
        foreach (const Card *c, m_handcards) {
            if (!shownHandcards().contains(c->effectiveID()))
                cards << c;
        }
    } else if (flags.contains(QStringLiteral("s"))) {
        foreach (const Card *c, m_handcards) {
            if (shownHandcards().contains(c->effectiveID()))
                cards << c;
        }
    }

    if (flags.contains(QStringLiteral("e")))
        cards << equipCards();
    if (flags.contains(QStringLiteral("j")))
        cards << judgingAreaCards();

    return cards;
}

Card *ServerPlayer::wholeHandCards() const
{
    if (isKongcheng())
        return nullptr;

    Card *dummy_card = room->cloneCard(QStringLiteral("DummyCard"));
    foreach (const Card *card, m_handcards)
        dummy_card->addSubcard(card->id());

    return dummy_card;
}

bool ServerPlayer::hasNullification() const
{
    foreach (const Card *card, m_handcards) {
        if (card->face()->isKindOf("Nullification"))
            return true;
    }

    if (hasTreasure(QStringLiteral("wooden_ox"))) {
        foreach (int id, pile(QStringLiteral("wooden_ox"))) {
            if (room->getCard(id)->face()->isKindOf("Nullification"))
                return true;
        }
    }

    if (hasSkill(QStringLiteral("chaoren"))) {
        bool ok = false;
        int id = property("chaoren").toInt(&ok);
        if (ok && id > -1 && room->getCard(id)->face()->isKindOf("Nullification"))
            return true;
    }

    foreach (const Skill *skill, skills(true)) {
        if (hasSkill(skill->objectName())) {
            if (skill->inherits("ViewAsSkill")) {
                const ViewAsSkill *vsskill = qobject_cast<const ViewAsSkill *>(skill);
                if (vsskill->isEnabledAtResponse(this, CardUseStruct::CARD_USE_REASON_RESPONSE_USE, QStringLiteral("nullification")))
                    return true;
            }
        }
    }

    return false;
}

bool ServerPlayer::pindian(ServerPlayer *target, const QString &reason, const Card *card1)
{
    room->tryPause();

    LogMessage log;
    log.type = QStringLiteral("#Pindian");
    log.from = this;
    log.to << target;
    room->sendLog(log);

    LogMessage log2;
    bool card1_result_logged = false;

    const Card *card2 = nullptr;
    PindianStruct pindian_struct;
    pindian_struct.from = this;
    pindian_struct.to = target;
    if (card1 != nullptr)
        pindian_struct.from_card = card1;
    pindian_struct.reason = reason;

    PindianStruct *pindian = &pindian_struct; //for tmp record.
    if (card1 == nullptr) {
        card1 = room->askForPindian(this, this, target, reason, pindian);
        if ((card1 != nullptr) && isShownHandcard(card1->effectiveID())) {
            log2.type = QStringLiteral("$PindianResult");
            log2.from = pindian_struct.from;
            log2.card_str = QString::number(card1->effectiveID());
            room->sendLog(log2);
            card1_result_logged = true;
        }

        CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, objectName(), target->objectName(), pindian_struct.reason, QString());
        room->moveCardTo(card1, this, nullptr, QSanguosha::PlaceTable, reason1, false);

        card2 = room->askForPindian(target, this, target, reason, pindian);

    } else {
        if (card1->isVirtualCard()) {
            int card_id = card1->effectiveID();
            card1 = room->getCard(card_id);
        }
        if ((card1 != nullptr) && isShownHandcard(card1->effectiveID())) {
            log2.type = QStringLiteral("$PindianResult");
            log2.from = pindian_struct.from;
            log2.card_str = QString::number(card1->effectiveID());
            room->sendLog(log2);
            card1_result_logged = true;
        }

        CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, objectName(), target->objectName(), pindian_struct.reason, QString());
        room->moveCardTo(card1, this, nullptr, QSanguosha::PlaceTable, reason1, false);

        card2 = room->askForPindian(target, this, target, reason, pindian);
    }
    if (card2 != nullptr) {
        CardMoveReason reason2(CardMoveReason::S_REASON_PINDIAN, target->objectName());
        room->moveCardTo(card2, target, nullptr, QSanguosha::PlaceTable, reason2, false);
    }

    //check whether card is empty
    if (card1 == nullptr || card2 == nullptr) {
        if (card1 != nullptr) {
            CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, this->objectName(), target->objectName(), pindian_struct.reason, QString());
            room->moveCardTo(card1, qobject_cast<ServerPlayer *>(pindian_struct.from), nullptr, QSanguosha::PlaceDiscardPile, reason1, true);
        }
        if (card2 != nullptr) {
            CardMoveReason reason2(CardMoveReason::S_REASON_PINDIAN, pindian_struct.to->objectName());
            room->moveCardTo(card2, qobject_cast<ServerPlayer *>(pindian_struct.to), nullptr, QSanguosha::PlaceDiscardPile, reason2, true);
        }
        //need trigger choice made?
        return false;
    }

    pindian_struct.from_card = card1;
    pindian_struct.to_card = card2;
    pindian_struct.from_number = static_cast<int>(card1->number());
    pindian_struct.to_number = static_cast<int>(card2->number());

    if (!card1_result_logged) {
        log2.type = QStringLiteral("$PindianResult");
        log2.from = pindian_struct.from;
        log2.card_str = QString::number(pindian_struct.from_card->effectiveID());
        room->sendLog(log2);
    }

    log2.type = QStringLiteral("$PindianResult");
    log2.from = pindian_struct.to;
    log2.card_str = QString::number(pindian_struct.to_card->effectiveID());
    room->sendLog(log2);

    RoomThread *thread = room->getThread();
    PindianStruct *pindian_star = &pindian_struct;
    QVariant data = QVariant::fromValue(pindian_star);
    Q_ASSERT(thread != nullptr);
    thread->trigger(QSanguosha::PindianVerifying, data);

    PindianStruct *new_star = data.value<PindianStruct *>();
    pindian_struct.from_number = new_star->from_number;
    pindian_struct.to_number = new_star->to_number;
    pindian_struct.success = (new_star->from_number > new_star->to_number);

    log.type = pindian_struct.success ? QStringLiteral("#PindianSuccess") : QStringLiteral("#PindianFailure");
    log.from = this;
    log.to.clear();
    log.to << target;
    log.card_str.clear();
    room->sendLog(log);

    JsonArray arg;
    arg << (int)S_GAME_EVENT_REVEAL_PINDIAN;
    arg << objectName();
    arg << pindian_struct.from_card->effectiveID();
    arg << target->objectName();
    arg << pindian_struct.to_card->effectiveID();
    arg << pindian_struct.success;
    arg << reason;
    room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);

    pindian_star = &pindian_struct;
    data = QVariant::fromValue(pindian_star);
    thread->trigger(QSanguosha::Pindian, data);

    if (room->getCardPlace(pindian_struct.from_card->effectiveID()) == QSanguosha::PlaceTable) {
        CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, pindian_struct.from->objectName(), pindian_struct.to->objectName(), pindian_struct.reason, QString());
        room->moveCardTo(pindian_struct.from_card, qobject_cast<ServerPlayer *>(pindian_struct.from), nullptr, QSanguosha::PlaceDiscardPile, reason1, true);
    }

    if (room->getCardPlace(pindian_struct.to_card->effectiveID()) == QSanguosha::PlaceTable) {
        CardMoveReason reason2(CardMoveReason::S_REASON_PINDIAN, pindian_struct.to->objectName());
        room->moveCardTo(pindian_struct.to_card, qobject_cast<ServerPlayer *>(pindian_struct.to), nullptr, QSanguosha::PlaceDiscardPile, reason2, true);
    }

    ChoiceMadeStruct s;
    s.player = this;
    s.type = ChoiceMadeStruct::Pindian;
    s.args << reason << objectName() << QString::number(pindian_struct.from_card->effectiveID()) << target->objectName() << QString::number(pindian_struct.to_card->effectiveID());
    QVariant decisionData = QVariant::fromValue(s);
    thread->trigger(QSanguosha::ChoiceMade, decisionData);

    return pindian_struct.success;
}

void ServerPlayer::turnOver()
{
    setFaceUp(!faceUp());
    room->broadcastProperty(this, "faceup");

    LogMessage log;
    log.type = QStringLiteral("#TurnOver");
    log.from = this;
    log.arg = faceUp() ? QStringLiteral("face_up") : QStringLiteral("face_down");
    room->sendLog(log);

    Q_ASSERT(room->getThread() != nullptr);
    QVariant v = QVariant::fromValue(this);
    room->getThread()->trigger(QSanguosha::TurnedOver, v);
}

bool ServerPlayer::changePhase(QSanguosha::Phase from, QSanguosha::Phase to)
{
    RoomThread *thread = room->getThread();
    Q_ASSERT(room->getThread() != nullptr);

    setPhase(QSanguosha::PhaseNone);

    PhaseChangeStruct phase_change;
    phase_change.player = this;
    phase_change.from = from;
    phase_change.to = to;
    QVariant data = QVariant::fromValue(phase_change);

    bool skip = thread->trigger(QSanguosha::EventPhaseChanging, data);
    if (skip && to != QSanguosha::PhaseNotActive) {
        setPhase(from);
        return true;
    }

    setPhase(to);
    room->broadcastProperty(this, "phase");

    if (!phases.isEmpty())
        phases.removeFirst();

    QVariant thisVariant = QVariant::fromValue(this);

    if (!thread->trigger(QSanguosha::EventPhaseStart, thisVariant)) {
        if (phase() != QSanguosha::PhaseNotActive)
            thread->trigger(QSanguosha::EventPhaseProceeding, thisVariant);
    }
    if (phase() != QSanguosha::PhaseNotActive)
        thread->trigger(QSanguosha::EventPhaseEnd, thisVariant);

    return false;
}

void ServerPlayer::play(QList<QSanguosha::Phase> set_phases)
{
    if (!set_phases.isEmpty()) {
        if (!set_phases.contains(QSanguosha::PhaseNotActive))
            set_phases << QSanguosha::PhaseNotActive;
    } else {
        set_phases << QSanguosha::PhaseRoundStart << QSanguosha::PhaseStart << QSanguosha::PhaseJudge << QSanguosha::PhaseDraw << QSanguosha::PhasePlay << QSanguosha::PhaseDiscard
                   << QSanguosha::PhaseFinish << QSanguosha::PhaseNotActive;
    }

    phases = set_phases;
    _m_phases_state.clear();
    for (int i = 0; i < phases.size(); i++) {
        PhaseStruct _phase;
        _phase.phase = phases[i];
        _m_phases_state << _phase;
    }

    for (int i = 0; i < _m_phases_state.size(); i++) {
        if (isDead() || hasFlag(QStringLiteral("Global_TurnTerminated"))) {
            changePhase(phase(), QSanguosha::PhaseNotActive);
            break;
        }

        _m_phases_index = i;
        PhaseChangeStruct phase_change;
        phase_change.player = this;
        phase_change.from = phase();
        phase_change.to = phases[i];

        RoomThread *thread = room->getThread();
        setPhase(QSanguosha::PhaseNone);
        QVariant data = QVariant::fromValue(phase_change);

        bool skip = thread->trigger(QSanguosha::EventPhaseChanging, data);
        phase_change = data.value<PhaseChangeStruct>();
        _m_phases_state[i].phase = phases[i] = phase_change.to;

        setPhase(phases[i]);
        room->broadcastProperty(this, "phase");

        if (phases[i] != QSanguosha::PhaseNotActive && (skip || _m_phases_state[i].skipped != 0)) {
            PhaseSkippingStruct s;
            s.isCost = _m_phases_state[i].skipped < 0;
            s.phase = phases[i];
            s.player = this;
            QVariant d = QVariant::fromValue(s);
            bool cancel_skip = thread->trigger(QSanguosha::EventPhaseSkipping, d);
            if (!cancel_skip)
                continue;
        }

        QVariant thisVariant = QVariant::fromValue(this);

        if (!thread->trigger(QSanguosha::EventPhaseStart, thisVariant)) {
            if (phase() != QSanguosha::PhaseNotActive)
                thread->trigger(QSanguosha::EventPhaseProceeding, thisVariant);
        }
        if (phase() != QSanguosha::PhaseNotActive)
            thread->trigger(QSanguosha::EventPhaseEnd, thisVariant);
        else
            break;
    }
}

QList<QSanguosha::Phase> &ServerPlayer::getPhases()
{
    return phases;
}

int ServerPlayer::getPhasesIndex() const
{
    return _m_phases_index;
}

void ServerPlayer::skip(QSanguosha::Phase phase, bool isCost, bool sendLog)
{
    for (int i = _m_phases_index; i < _m_phases_state.size(); i++) {
        if (_m_phases_state[i].phase == phase) {
            if (_m_phases_state[i].skipped != 0) {
                if (isCost && _m_phases_state[i].skipped == 1)
                    _m_phases_state[i].skipped = -1;
                return;
            }
            _m_phases_state[i].skipped = (isCost ? -1 : 1);
            //defaultly skip all phases even someone has same pahses.
        }
    }

    static QStringList phase_strings;
    if (phase_strings.isEmpty())
        phase_strings << QStringLiteral("round_start") << QStringLiteral("start") << QStringLiteral("judge") << QStringLiteral("draw") << QStringLiteral("play")
                      << QStringLiteral("discard") << QStringLiteral("finish") << QStringLiteral("not_active");
    int index = static_cast<int>(phase);

    if (sendLog) {
        LogMessage log;
        log.type = QStringLiteral("#SkipPhase");
        log.from = this;
        log.arg = phase_strings.at(index);
        room->sendLog(log);
    }
}

void ServerPlayer::insertPhases(QList<QSanguosha::Phase> new_phases, int index)
{
    if (index == -1)
        index = _m_phases_index;
    for (int i = 0; i < new_phases.size(); i++) {
        PhaseStruct _phase;
        _phase.phase = new_phases[i];
        phases.insert(index + i, new_phases[i]);
        _m_phases_state.insert(index + i, _phase);
    }
}

void ServerPlayer::exchangePhases(QSanguosha::Phase phase1, QSanguosha::Phase phase2)
{
    PhaseStruct _phase1;
    PhaseStruct _phase2;

    int index1 = phases.indexOf(phase1);
    int index2 = phases.indexOf(phase2);
    // make sure that "_m_phases_state" has already contain informations from "phases"
    if (index1 > -1 && index2 > -1) {
        _phase1 = _m_phases_state[index1];
        _phase2 = _m_phases_state[index2];

        phases.removeAt(index1);
        phases.insert(index1, phase2);
        _m_phases_state.removeAt(index1);
        _m_phases_state.insert(index1, _phase2);

        phases.removeAt(index2);
        phases.insert(index2, phase1);
        _m_phases_state.removeAt(index2);
        _m_phases_state.insert(index2, _phase1);
    }
}

bool ServerPlayer::isSkipped(QSanguosha::Phase phase)
{
    for (int i = _m_phases_index; i < _m_phases_state.size(); i++) {
        if (_m_phases_state[i].phase == phase)
            return (_m_phases_state[i].skipped != 0);
    }
    return false;
}

void ServerPlayer::gainMark(const QString &m, int n)
{
    MarkChangeStruct change;
    change.name = m;
    change.num = n;
    change.player = this;
    QVariant n_data = QVariant::fromValue(change);
    if (m.startsWith(QStringLiteral("@"))) {
        if (room->getThread()->trigger(QSanguosha::PreMarkChange, n_data))
            return;
        n = n_data.value<MarkChangeStruct>().num;
    }
    if (n == 0)
        return;
    if (n < 0) {
        loseMark(m, -n);
        return;
    }

    int value = mark(m) + n;

    LogMessage log;
    log.type = QStringLiteral("#GetMark");
    log.from = this;
    log.arg = m;
    log.arg2 = QString::number(n);

    room->sendLog(log);
    room->setPlayerMark(this, m, value);

    if (m.startsWith(QStringLiteral("@")))
        room->getThread()->trigger(QSanguosha::MarkChanged, n_data);
}

void ServerPlayer::loseMark(const QString &m, int n)
{
    if (mark(m) == 0)
        return;
    MarkChangeStruct change;
    change.name = m;
    change.num = -n;
    change.player = this;

    QVariant n_data = QVariant::fromValue(change);

    if (m.startsWith(QStringLiteral("@"))) {
        if (room->getThread()->trigger(QSanguosha::PreMarkChange, n_data))
            return;
        n = -(n_data.value<MarkChangeStruct>().num);
    }

    if (n == 0)
        return;
    if (n < 0) {
        gainMark(m, -n);
        return;
    }

    int value = mark(m) - n;
    if (value < 0) {
        value = 0;
        n = mark(m);
    }

    LogMessage log;
    log.type = QStringLiteral("#LoseMark");
    log.from = this;
    log.arg = m;
    log.arg2 = QString::number(n);

    room->sendLog(log);
    room->setPlayerMark(this, m, value);

    if (m.startsWith(QStringLiteral("@")))
        room->getThread()->trigger(QSanguosha::MarkChanged, n_data);
}

void ServerPlayer::loseAllMarks(const QString &mark_name)
{
    loseMark(mark_name, mark(mark_name));
}

#if 0
void ServerPlayer::addSkill(const QString &skill_name, bool head_skill)
{
    Player::addSkill(skill_name, head_skill);
    JsonArray args;
    args << QSanProtocol::S_GAME_EVENT_ADD_SKILL;
    args << objectName();
    args << skill_name;
    args << head_skill;

    if (isHegemonyGameMode(room->getMode()))
        room->doNotify(this, QSanProtocol::S_COMMAND_LOG_EVENT, args);
    else
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void ServerPlayer::loseSkill(const QString &skill_name, bool head_skill)
{
    Player::loseSkill(skill_name, head_skill);
    JsonArray args;
    args << QSanProtocol::S_GAME_EVENT_LOSE_SKILL;
    args << objectName();
    args << skill_name;
    args << head_skill;
    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}
#endif

int ServerPlayer::getGeneralMaxHp() const
{
    int max_hp = 0;

    if (getGeneral2() == nullptr)
        max_hp = general()->maxHp();
    else {
        int first = general()->maxHp();
        int second = getGeneral2()->maxHp();

        int plan = Config.MaxHpScheme;

        switch (plan) {
        case 3:
            max_hp = (first + second) / 2;
            break;
        case 2:
            max_hp = qMax(first, second);
            break;
        case 1:
            max_hp = qMin(first, second);
            break;
        default:
            max_hp = first + second - Config.Scheme0Subtraction;
            break;
        }

        max_hp = qMax(max_hp, 1);
    }

    if (room->hasWelfare(this))
        max_hp++;

    return max_hp;
}

QString ServerPlayer::getIp() const
{
    if (socket != nullptr)
        return socket->peerAddress();
    else
        return QString();
}

quint32 ServerPlayer::ipv4Address() const
{
    if (socket != nullptr)
        return socket->ipv4Address();
    else
        return 0U;
}

void ServerPlayer::introduceTo(ServerPlayer *player)
{
    QString screen_name = screenName();
    QString avatar = property("avatar").toString();

    JsonArray introduce_str;
    introduce_str << objectName() << screen_name.toUtf8().toBase64() << avatar;

    if (player != nullptr)
        room->doNotify(player, S_COMMAND_ADD_PLAYER, introduce_str);
    else {
        QList<ServerPlayer *> players = room->getPlayers();
        players.removeOne(this);
        room->doBroadcastNotify(players, S_COMMAND_ADD_PLAYER, introduce_str);
    }

#if 0
    // TODO: It seems like the following code should goto marshal()
    if (!isHegemonyGameMode(room->getMode()))
        return;
    if (hasShownGeneral()) {
        foreach (const QString skill_name, skills.keys()) {
            if (Sanguosha->getSkill(skill_name)->isVisible()) {
                JsonArray args1;
                args1 << (int)S_GAME_EVENT_ADD_SKILL;
                args1 << objectName();
                args1 << skill_name;
                args1 << true;
                room->doNotify(player, S_COMMAND_LOG_EVENT, args1);
            }

            foreach (const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_skill->isVisible()) {
                    JsonArray args2;
                    args2 << (int)S_GAME_EVENT_ADD_SKILL;
                    args2 << objectName();
                    args2 << related_skill->objectName();
                    args2 << true;
                    room->doNotify(player, S_COMMAND_LOG_EVENT, args2);
                }
            }
        }
    }
    if (hasShownGeneral2()) {
        foreach (const QString skill_name, skills2.keys()) {
            if (Sanguosha->getSkill(skill_name)->isVisible()) {
                JsonArray args1;
                args1 << S_GAME_EVENT_ADD_SKILL;
                args1 << objectName();
                args1 << skill_name;
                args1 << false;
                room->doNotify(player, S_COMMAND_LOG_EVENT, args1);
            }

            foreach (const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_skill->isVisible()) {
                    JsonArray args2;
                    args2 << (int)S_GAME_EVENT_ADD_SKILL;
                    args2 << objectName();
                    args2 << related_skill->objectName();
                    args2 << false;
                    room->doNotify(player, S_COMMAND_LOG_EVENT, args2);
                }
            }
        }
    }
#endif
}

void ServerPlayer::marshal(ServerPlayer *player) const
{
    room->notifyProperty(player, this, "maxhp");
    room->notifyProperty(player, this, "hp");
    room->notifyProperty(player, this, "dyingFactor");
    room->notifyProperty(player, this, "general_showed");
    room->notifyProperty(player, this, "general2_showed");
    room->notifyProperty(player, this, "inital_seat");

    if (isHegemonyGameMode(room->getMode())) {
        QVariant RoleConfirmedTag = room->getTag(this->objectName() + QStringLiteral("_RoleConfirmed"));
        bool roleConfirmed = RoleConfirmedTag.canConvert<bool>() && RoleConfirmedTag.toBool();
        if (player == this || haveShownOneGeneral() || roleConfirmed) {
            room->notifyProperty(player, this, "kingdom");
            room->notifyProperty(player, this, "role");
        } else {
            room->notifyProperty(player, this, "kingdom", QStringLiteral("god"));
        }
    } else {
        if (kingdom() != general()->kingdom())
            room->notifyProperty(player, this, "kingdom");
    }

    if (isAlive()) {
        room->notifyProperty(player, this, "seat");
        if (phase() != QSanguosha::PhaseNotActive)
            room->notifyProperty(player, this, "phase");
    } else {
        room->notifyProperty(player, this, "alive");
        room->notifyProperty(player, this, "role");
        room->doNotify(player, S_COMMAND_KILL_PLAYER, QVariant(objectName()));
    }

    if (!faceUp())
        room->notifyProperty(player, this, "faceup");

    if (isChained())
        room->notifyProperty(player, this, "chained");

    room->notifyProperty(player, this, "removed");
    room->notifyProperty(player, this, "gender");

    QList<ServerPlayer *> players;
    players << player;

    QList<CardsMoveStruct> moves;

    if (!isKongcheng()) {
        CardsMoveStruct move;
        foreach (const Card *card, m_handcards) {
            move.card_ids << card->id();
            if (player == this) {
                Card *c = room->getCard(card->id());
                if (c->isModified())
                    room->notifyUpdateCard(player, card->id(), c);
            }
        }
        move.from_place = QSanguosha::PlaceDrawPile;
        move.to_player_name = objectName();
        move.to_place = QSanguosha::PlaceHand;

        if (player == this)
            move.to = player;

        moves << move;
    }

    if (hasEquip()) {
        CardsMoveStruct move;
        foreach (const Card *card, equipCards()) {
            move.card_ids << card->id();
            Card *c = room->getCard(card->id());
            if (c->isModified())
                room->notifyUpdateCard(player, card->id(), c);
        }
        move.from_place = QSanguosha::PlaceDrawPile;
        move.to_player_name = objectName();
        move.to_place = QSanguosha::PlaceEquip;

        moves << move;
    }

    if (!judgingArea().isEmpty()) {
        CardsMoveStruct move;
        foreach (int card_id, judgingArea()) {
            move.card_ids << card_id;
            Card *c = room->getCard(card_id);
            if (c->isModified())
                room->notifyUpdateCard(player, card_id, c);
        }
        move.from_place = QSanguosha::PlaceDrawPile;
        move.to_player_name = objectName();
        move.to_place = QSanguosha::PlaceDelayedTrick;

        moves << move;
    }

    if (!moves.isEmpty()) {
        room->notifyMoveCards(true, moves, false, players);
        room->notifyMoveCards(false, moves, false, players);
    }

    if (!pileNames().isEmpty()) {
        CardsMoveStruct move;
        move.from_place = QSanguosha::PlaceDrawPile;
        move.to_player_name = objectName();
        move.to_place = QSanguosha::PlaceSpecial;
        foreach (QString p, pileNames()) {
            move.card_ids.clear();
            move.card_ids.append(pile(p).values());
            move.to_pile_name = p;

            QList<CardsMoveStruct> moves2;
            moves2 << move;

            bool open = pileOpen(p, player->objectName());

            room->notifyMoveCards(true, moves2, open, players);
            room->notifyMoveCards(false, moves2, open, players);
        }
    }

    JsonArray arg_shownhandcard;
    arg_shownhandcard << objectName();
    arg_shownhandcard << JsonUtils::toJsonArray(shownHandcards().values());
    room->doNotify(player, S_COMMAND_SET_SHOWN_HANDCARD, arg_shownhandcard);

    JsonArray arg_brokenIds;
    arg_brokenIds << objectName();
    arg_brokenIds << JsonUtils::toJsonArray(brokenEquips().values());
    room->doNotify(player, S_COMMAND_SET_BROKEN_EQUIP, arg_brokenIds);

    //need remove mark of hidden limit skill
    QStringList hegemony_limitmarks;
    if (isHegemonyGameMode(room->getMode())) {
        foreach (const Skill *skill, skills(false))
            if (skill->isLimited() && mark(skill->limitMark()) > 0 && (this != player && !haveShownSkill(skill)))
                hegemony_limitmarks.append(skill->limitMark());
    }

    foreach (QString mark_name, marks().keys()) {
        if (mark_name.startsWith(QStringLiteral("@")) && !hegemony_limitmarks.contains(mark_name)) {
            int value = mark(mark_name);
            if (value > 0) {
                JsonArray arg_mark;
                arg_mark << objectName();
                arg_mark << mark_name;
                arg_mark << value;
                room->doNotify(player, S_COMMAND_SET_MARK, arg_mark);
            }
        }
    }

    if (!isHegemonyGameMode(room->getMode())) {
        foreach (const Skill *skill, skills(true)) {
            //should not nofity the lord skill
            if (skill->isLordSkill() && !hasLordSkill(skill->objectName()))
                continue;
            QString skill_name = skill->objectName();
            JsonArray arg_acquire;
            arg_acquire << S_GAME_EVENT_ACQUIRE_SKILL;
            arg_acquire << objectName();
            arg_acquire << skill_name;
            arg_acquire << inHeadSkills(skill_name);
            room->doNotify(player, S_COMMAND_LOG_EVENT, arg_acquire);
        }
    }

    foreach (const QString &invalid_name, invalidedSkills()) {
        JsonArray arg_invalid;
        arg_invalid << objectName() << invalid_name << true;
        room->doNotify(player, S_COMMAND_SET_SKILL_INVALIDITY, arg_invalid);
    }

    //for AvatarTooltip
    JsonArray arg_tooltip;
    arg_tooltip << QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
    room->doNotify(player, QSanProtocol::S_COMMAND_LOG_EVENT, arg_tooltip);

    //since "banling", we should notify hp after notifying skill
    if (this->hasSkill(QStringLiteral("banling"))) {
        room->notifyProperty(player, this, "renhp");
        room->notifyProperty(player, this, "linghp");
    }

    //    if (this->hasSkill(QStringLiteral("anyun"), true)) {
    //        QString g = hidden_generals.join(QStringLiteral("|"));
    //        JsonArray arg;
    //        arg << objectName();
    //        if (this == player)
    //            arg << g;
    //        else
    //            arg << (int)hidden_generals.length();

    //        room->doNotify(player, S_COMMAND_SET_HIDDEN_GENERAL, arg);
    //    }

    foreach (QString reason, disableShow(true)) { //for disableshow
        JsonArray arg;
        arg << objectName();
        arg << true;
        arg << QStringLiteral("h");
        arg << reason;
        room->doNotify(player, S_COMMAND_DISABLE_SHOW, arg);
    }
    foreach (QString reason, disableShow(false)) {
        JsonArray arg;
        arg << objectName();
        arg << true;
        arg << QStringLiteral("d");
        arg << reason;
        room->doNotify(player, S_COMMAND_DISABLE_SHOW, arg);
    }

    foreach (QString flag, flagList())
        room->notifyProperty(player, this, "flags", flag);

    foreach (QString item, histories().keys()) {
        int value = histories().value(item);
        if (value > 0) {
            JsonArray arg;
            arg << item;
            arg << value;

            room->doNotify(player, S_COMMAND_ADD_HISTORY, arg);
        }
    }

    if (!isHegemonyGameMode(room->getMode()) && hasShownRole()) {
        room->notifyProperty(player, this, "role");
        room->notifyProperty(player, this, "role_shown"); // notify client!!
    }

    //for huashen  like skill pingyi
    QString huashen_skill = this->tag.value(QStringLiteral("Huashen_skill"), QString()).toString();
    QString huashen_target = this->tag.value(QStringLiteral("Huashen_target"), QString()).toString();
    QString huashen_place = this->tag.value(QStringLiteral("Huashen_place"), QString()).toString();
    if (!huashen_skill.isNull() && !huashen_target.isNull()) {
        JsonArray huanshen_arg;
        huanshen_arg << (int)QSanProtocol::S_GAME_EVENT_HUASHEN;
        huanshen_arg << objectName();
        if ((huashen_place != QStringLiteral("deputy"))) {
            huanshen_arg << huashen_target;
            huanshen_arg << huashen_skill;
        }
        huanshen_arg << QString() << QString();
        if (!(huashen_place != QStringLiteral("deputy"))) {
            huanshen_arg << huashen_target;
            huanshen_arg << huashen_skill;
        }
        room->doNotify(player, QSanProtocol::S_COMMAND_LOG_EVENT, huanshen_arg);
    }

    // for chaoren
    if (player == this && hasSkill(QStringLiteral("chaoren")))
        room->notifyProperty(player, this, "chaoren");
}

void ServerPlayer::addToPile(const QString &pile_name, const Card *card, bool open, const QList<ServerPlayer *> &open_players)
{
    IDSet card_ids;
    if (card->isVirtualCard())
        card_ids = card->subcards();
    else
        card_ids << card->effectiveID();
    return addToPile(pile_name, card_ids, open, open_players);
}

void ServerPlayer::addToPile(const QString &pile_name, int card_id, bool open, const QList<ServerPlayer *> &open_players)
{
    IDSet card_ids;
    card_ids << card_id;
    return addToPile(pile_name, card_ids, open, open_players);
}

void ServerPlayer::addToPile(const QString &pile_name, const IDSet &card_ids, bool open, const QList<ServerPlayer *> &open_players)
{
    return addToPile(pile_name, card_ids, open, CardMoveReason(), open_players);
}

void ServerPlayer::addToPile(const QString &pile_name, const IDSet &card_ids, bool open, const CardMoveReason &reason, QList<ServerPlayer *> open_players)
{
    CardsMoveStruct move;
    move.card_ids = card_ids.values(); // FIXME: Replace here with IDSet
    move.to = this;
    move.to_place = QSanguosha::PlaceSpecial;
    move.to_pile_name = pile_name;
    move.reason = reason;

    room->moveCardsAtomic(move, open);
}

void ServerPlayer::gainAnExtraTurn()
{
    room->getThread()->setNextExtraTurn(this);
}

void ServerPlayer::showHiddenSkill(const QString &skill_name)
{
    if (skill_name.isNull())
        return;

    if (isHegemonyGameMode(room->getMode())) {
        if (!haveShownGeneral() && ownGeneralCardSkill(skill_name) && inHeadSkills(skill_name))
            showGeneral();
        else if (!hasShownGeneral2() && ownGeneralCardSkill(skill_name) && inDeputySkills(skill_name))
            showGeneral(false);
    } else {
        //for yibian
        ServerPlayer *reimu = room->findPlayerBySkillName(QStringLiteral("yibian"));
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if ((reimu != nullptr) && !hasShownRole() && !skill->isEternal() && !skill->isAttachedSkill() && !hasEquipSkill(skill_name)) {
            //&& ownSkill(skill_name)
            QString role = getRoleString();
            room->touhouLogmessage(QStringLiteral("#YibianShow"), this, role, room->getAllPlayers());
            room->broadcastProperty(this, "role");
            room->setPlayerProperty(this, "role_shown", true); //important! to notify client

            room->setPlayerProperty(this, "general_showed", true);
            room->setPlayerProperty(this, "general2_showed", true);
        }

        if (hasSkill(skill_name)) {
            QStringList generals;
            QString generalName;
            if (generals.isEmpty())
                return;
            else if (generals.length() == 1)
                generalName = generals.first();
            else {
                generalName = room->askForChoice(this, QStringLiteral("showSameHiddenSkills"), generals.join(QStringLiteral("+")));
            }

            if (!generalName.isNull()) {
                room->touhouLogmessage(QStringLiteral("#ShowHiddenGeneral"), this, generalName);

                JsonArray arg;
                arg << (int)QSanProtocol::S_GAME_EVENT_HUASHEN;
                arg << objectName();
                if (inHeadSkills(QStringLiteral("anyun"))) {
                    arg << generalName;
                    arg << skill_name;
                }
                arg << QString() << QString();
                if (!inHeadSkills(QStringLiteral("anyun"))) {
                    arg << generalName;
                    arg << skill_name;
                }
                room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, arg);

                // shown_hidden_general = generalName;
                JsonArray arg1;
                arg1 << objectName();
                arg1 << generalName;
                room->doBroadcastNotify(S_COMMAND_SET_SHOWN_HIDDEN_GENERAL, arg1);

                foreach (const Skill *skill, Sanguosha->getGeneral(generalName)->skills()) {
                    if (!skill->isLordSkill() && !skill->isAttachedSkill() && !skill->isLimited() && !skill->isEternal())
                        room->handleAcquireDetachSkills(this, skill->objectName(), true);
                }
                room->filterCards(this, this->getCards(QStringLiteral("hes")), true);

                //keep showing huashen for a short time
                if (phase() == QSanguosha::PhaseFinish)
                    room->getThread()->delay(1000);
            }
        }
    }
}

QStringList ServerPlayer::checkTargetModSkillShow(const CardUseStruct &use)
{
    if (use.card == nullptr || use.card->face()->type() == QSanguosha::TypeSkill)
        return QStringList();
    if (!isHegemonyGameMode(room->getMode())) {
    }

    QList<const TargetModSkill *> tarmods;
    if (isHegemonyGameMode(room->getMode())) {
        foreach (const Skill *skill, use.from->skills(false)) {
            if (skill->inherits("TargetModSkill") && use.from->hasSkill(skill) && !use.from->haveShownSkill(skill)) { //main_skill??
                const TargetModSkill *tarmod = qobject_cast<const TargetModSkill *>(skill);
                tarmods << tarmod;
            }
        }
    }

    if (tarmods.isEmpty())
        return QStringList();

    QSet<QString> showExtraTarget;
    QSet<QString> disShowExtraTarget;
    QSet<QString> showResidueNum;
    QSet<QString> disShowResidueNum;
    QSet<QString> showDistanceLimit;
    QSet<QString> showTargetFix; // only for skill tianqu
    QSet<QString> showTargetProhibit; //only for skill tianqu
    //check extra target
    int num = use.to.length() - 1;
    if (num >= 1) {
        foreach (const TargetModSkill *tarmod, tarmods) {
            if (tarmod->getExtraTargetNum(use.from, use.card) >= num)
                showExtraTarget << tarmod->objectName();
            else
                disShowExtraTarget << tarmod->objectName();
        }
    }

    //check ResidueNum
    //only consider the folloing cards
    if (use.card->face()->isKindOf("Slash") || use.card->face()->isKindOf("Analeptic")) {
        num = 0;
        if (use.card->face()->isKindOf("Slash"))
            num = use.from->slashCount() - 1;
        else if (use.card->face()->isKindOf("Analeptic"))
            num = use.from->analapticCount() - 1;

        if (num >= 1) {
            foreach (const TargetModSkill *tarmod, tarmods) {
                if (tarmod->getResidueNum(use.from, use.card) >= num)
                    showResidueNum << tarmod->objectName();
                else
                    disShowResidueNum << tarmod->objectName();
            }
        }
    }

    //check DistanceLimit
    //only consider the folloing cards
    if (use.card->face()->isKindOf("Slash") || use.card->face()->isKindOf("SupplyShortage") || use.card->face()->isKindOf("Snatch")) {
        int distance = 1;
        foreach (Player *p, use.to) {
            if (use.from->distanceTo(p) > distance)
                distance = use.from->distanceTo(p);

            distance = distance - 1;
            if (distance >= 1) {
                foreach (const TargetModSkill *tarmod, tarmods) {
                    if (tarmod->getDistanceLimit(use.from, use.card) >= distance)
                        showDistanceLimit << tarmod->objectName();
                    //else
                    //    disShowDistanceLimit << tarmod->objectName();
                }
            }
        }
    }

    //check TargetFix
    //only consider the folloing cards
    //Peach , EquipCard , ExNihilo, Analeptic, Lightning

    use.card->addFlag(QStringLiteral("IgnoreFailed"));
    if (use.card->face()->targetFixed(use.from, use.card) && !use.to.contains(use.from) && !use.card->face()->isKindOf("AOE") && !use.card->face()->isKindOf("GlobalEffect")) {
        //        if (isHiddenSkill(QStringLiteral("tianqu")) && room->currentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY)
        //            showTargetFix << QStringLiteral("tianqu");
    }
    use.card->addFlag(QStringLiteral("-IgnoreFailed"));

    //check prohibit

    QList<const Player *> ps;
    foreach (Player *p, use.to)
        ps << p;
    foreach (Player *p, use.to) {
        auto useToExceptp = ps;
        useToExceptp.removeAll(p);
        if (use.from->isProhibited(p, use.card) && room->currentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            showTargetProhibit << QStringLiteral("tianqu");
            if (use.from->isProhibited(p, use.card, useToExceptp) && room->currentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
                break;
            } else if (use.card->face()->isKindOf("Peach")) {
                if (!p->isWounded() && room->currentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
                    showTargetProhibit << QStringLiteral("tianqu");
                    break;
                }
                if (p != use.from && (!p->hasLordSkill(QStringLiteral("yanhui")) || p->kingdom() != QStringLiteral("zhan"))
                    && room->currentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
                    showTargetProhibit << QStringLiteral("tianqu");
                    break;
                }
            } else if (use.card->face()->isKindOf("DelayedTrick") && p->containsTrick(use.card->faceName())
                       && room->currentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
                showTargetProhibit << QStringLiteral("tianqu");
                break;
            }
        }
    }
    QSet<QString> shows = showExtraTarget | showDistanceLimit | showResidueNum | showTargetFix | showTargetProhibit;
    shows = shows - disShowExtraTarget - disShowResidueNum;

    return shows.values();
}

bool ServerPlayer::CompareByActionOrder(ServerPlayer *a, ServerPlayer *b)
{
    Room *room = a->getRoom();
    return room->getFront(a, b) == a;
}

void ServerPlayer::showGeneral(bool head_general, bool trigger_event, bool sendLog, bool /*unused*/)
{
    QStringList names = room->getTag(objectName()).toStringList();
    if (names.isEmpty())
        return;
    QString general_name;

    QVariant RoleConfirmedTag = room->getTag(this->objectName() + QStringLiteral("_RoleConfirmed"));

    bool roleConfirmed = RoleConfirmedTag.canConvert<bool>() && RoleConfirmedTag.toBool();
    bool notify_role = !roleConfirmed;

    room->tryPause();

    if (head_general) {
        //ignore anjiang
        if (generalName() != QStringLiteral("anjiang"))
            return;

        setSkillsPreshowed(QStringLiteral("h"));
        room->setPlayerProperty(this, "general_showed", true);

        general_name = names.first();

        JsonArray arg;
        arg << (int)S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << false;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, general_name);

        //change skinhero
        int skin_id = room->getTag(general_name + QStringLiteral("_skin_id")).toInt();
        JsonArray val;
        val << (int)QSanProtocol::S_GAME_EVENT_SKIN_CHANGED;
        val << objectName();
        val << general_name;
        val << skin_id;
        val << true;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, val);

        if (!property("Duanchang").toString().split(QStringLiteral(",")).contains(QStringLiteral("head"))) {
            sendSkillsToOthers();
            foreach (const Skill *skill, getHeadSkillList()) {
                if (skill->isLimited() && !skill->limitMark().isEmpty() && (!skill->isLordSkill() || hasLordSkill(skill->objectName())) && haveShownSkill(skill)) {
                    JsonArray arg;
                    arg << objectName();
                    arg << skill->limitMark();
                    arg << mark(skill->limitMark());
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_SET_MARK, arg);
                }
            }
        }

    } else {
        //ignore anjiang
        if (getGeneral2Name() != QStringLiteral("anjiang"))
            return;

        setSkillsPreshowed(QStringLiteral("d"));
        room->setPlayerProperty(this, "general2_showed", true);

        general_name = names.last();

        JsonArray arg;
        arg << (int)S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << true;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, general_name);

        //change skinhero
        int skin_id = room->getTag(general_name + QStringLiteral("_skin_id")).toInt();
        JsonArray val;
        val << (int)QSanProtocol::S_GAME_EVENT_SKIN_CHANGED;
        val << objectName();
        val << general_name;
        val << skin_id;
        val << false;
        room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, val);

        if (!property("Duanchang").toString().split(QStringLiteral(",")).contains(QStringLiteral("deputy"))) {
            sendSkillsToOthers(false);
            foreach (const Skill *skill, getDeputySkillList()) {
                if (skill->isLimited() && !skill->limitMark().isEmpty() && (!skill->isLordSkill() || hasLordSkill(skill->objectName())) && haveShownSkill(skill)) {
                    JsonArray arg;
                    arg << objectName();
                    arg << skill->limitMark();
                    arg << mark(skill->limitMark());
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_SET_MARK, arg);
                }
            }
        }
    }

    if (notify_role) {
        //count careerist

        // need to refactor to getKingdom() == "careerist"

        QString role = getRoleString();
        int i = 1;
        foreach (ServerPlayer *p, room->getOtherPlayers(this, true)) {
            if (p->getRoleString() == role) {
                QVariant RoleConfirmedTag1 = room->getTag(p->objectName() + QStringLiteral("_RoleConfirmed"));
                bool roleConfirmed1 = RoleConfirmedTag1.canConvert<bool>() && RoleConfirmedTag1.toBool();
                if (roleConfirmed1 && p->getRoleString() != QStringLiteral("careerist"))
                    ++i;
            }
        }

        if (role != QStringLiteral("careerist")) {
            if ((i + 1) > (room->getPlayers().length() / 2)) { // set hidden careerist
                foreach (ServerPlayer *p, room->getOtherPlayers(this, true)) {
                    QVariant RoleConfirmedTag1 = room->getTag(p->objectName() + QStringLiteral("_RoleConfirmed"));
                    bool roleConfirmed1 = RoleConfirmedTag1.canConvert<bool>() && RoleConfirmedTag1.toBool();
                    if (p->isAlive() && !roleConfirmed1 && role == p->getRoleString()) {
                        p->setRole(QStringLiteral("careerist"));
                        room->notifyProperty(p, p, "role", QStringLiteral("careerist"));
                    }
                }
            }
        }

        if (i > (room->getPlayers().length() / 2))
            role = QStringLiteral("careerist");

        room->setPlayerProperty(this, "role", role);
        room->setTag(this->objectName() + QStringLiteral("_RoleConfirmed"), true);
    }

    if (sendLog) {
        LogMessage log;
        log.type = QStringLiteral("#HegemonyReveal");
        log.from = this;
        log.arg = generalName();
        if (Config.Enable2ndGeneral || isHegemonyGameMode(room->getMode())) {
            log.type = QStringLiteral("#HegemonyRevealDouble");
            log.arg2 = getGeneral2Name();
        }
        room->sendLog(log);
    }

    if (trigger_event) {
        Q_ASSERT(room->getThread() != nullptr);

        if (room->getTag(QStringLiteral("TheFirstToShowRewarded")).isNull()) {
            setMark(QStringLiteral("TheFirstToShowReward"), 1);
            room->setTag(QStringLiteral("TheFirstToShowRewarded"), true);
        }

        ShowGeneralStruct s;
        s.player = this;
        s.isHead = head_general;
        s.isShow = true;
        QVariant _head = QVariant::fromValue(s);
        room->getThread()->trigger(QSanguosha::GeneralShown, _head);
    }

    room->filterCards(this, getCards(QStringLiteral("hes")), true);
}

void ServerPlayer::hideGeneral(bool head_general)
{
    room->tryPause();

    if (head_general) {
        if (generalName() == QStringLiteral("anjiang"))
            return;

        setSkillsPreshowed(QStringLiteral("h"), false);
        // dirty hack for temporary convenience.
        room->setPlayerProperty(this, "flags", QStringLiteral("hiding"));
        room->setPlayerProperty(this, "general_showed", false);
        room->setPlayerProperty(this, "flags", QStringLiteral("-hiding"));

        JsonArray arg;
        arg << (int)S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << QStringLiteral("anjiang");
        arg << false;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, QStringLiteral("anjiang"));

        // disconnectSkillsFromOthers();

        foreach (const Skill *skill, skills(false)) {
            if (skill->isLimited() && !skill->limitMark().isEmpty() && (!skill->isLordSkill() || hasLordSkill(skill->objectName())) && !haveShownSkill(skill)
                && mark(skill->limitMark()) > 0) {
                JsonArray arg;
                arg << objectName();
                arg << skill->limitMark();
                arg << 0;
                foreach (ServerPlayer *p, room->getOtherPlayers(this, true))
                    room->doNotify(p, QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }

    } else {
        if (getGeneral2Name() == QStringLiteral("anjiang"))
            return;

        setSkillsPreshowed(QStringLiteral("d"), false);
        // dirty hack for temporary convenience
        room->setPlayerProperty(this, "flags", QStringLiteral("hiding"));
        room->setPlayerProperty(this, "general2_showed", false);
        room->setPlayerProperty(this, "flags", QStringLiteral("-hiding"));

        JsonArray arg;
        arg << (int)S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << QStringLiteral("anjiang");
        arg << true;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, QStringLiteral("anjiang"));

        // disconnectSkillsFromOthers(false);

        foreach (const Skill *skill, skills(false)) {
            if (skill->isLimited() && !skill->limitMark().isEmpty() && (!skill->isLordSkill() || hasLordSkill(skill->objectName())) && !haveShownSkill(skill)
                && mark(skill->limitMark()) > 0) {
                JsonArray arg;
                arg << objectName();
                arg << skill->limitMark();
                arg << 0;
                foreach (ServerPlayer *p, room->getOtherPlayers(this, true))
                    room->doNotify(p, QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }
    }

    LogMessage log;
    log.type = QStringLiteral("#BasaraConceal");
    log.from = this;
    log.arg = generalName();
    log.arg2 = getGeneral2Name();
    room->sendLog(log);

    Q_ASSERT(room->getThread() != nullptr);

    ShowGeneralStruct s;
    s.player = this;
    s.isHead = head_general;
    s.isShow = false;
    QVariant _head = QVariant::fromValue(s);
    room->getThread()->trigger(QSanguosha::GeneralHidden, _head);

    room->filterCards(this, getCards(QStringLiteral("he")), true);
    setSkillsPreshowed(head_general ? QStringLiteral("h") : QStringLiteral("d"));
}

void ServerPlayer::removeGeneral(bool head_general)

{
    QString general_name;
    QString from_general;
    room->tryPause();
    room->setEmotion(this, QStringLiteral("remove"));

    QStringList names = room->getTag(objectName()).toStringList();

    if (head_general) {
        if (!haveShownGeneral())
            showGeneral(); //zoushi?

        from_general = generalName();
        if (from_general.contains(QStringLiteral("sujiang")))
            return;

        from_general = names.first();
        QSanguosha::Gender gender = Sanguosha->getGeneral(from_general)->gender();
        general_name = gender == QSanguosha::Male ? QStringLiteral("sujiang") : QStringLiteral("sujiangf"); //need image

        room->setPlayerProperty(this, "general_showed", true);

        JsonArray arg;
        arg << (int)S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << false;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, general_name);

        setSkillsPreshowed(QStringLiteral("h"), false);
        // disconnectSkillsFromOthers();

        foreach (const Skill *skill, getHeadSkillList()) {
            if (skill != nullptr)
                room->detachSkillFromPlayer(this, skill->objectName(), false, false, false, true); //sendlog  head deputy
        }
    } else {
        if (!hasShownGeneral2())
            showGeneral(false);

        from_general = getGeneral2Name();
        if (from_general.contains(QStringLiteral("sujiang")))
            return;
        from_general = names.last();
        QSanguosha::Gender gender = Sanguosha->getGeneral(from_general)->gender();

        general_name = gender == QSanguosha::Male ? QStringLiteral("sujiang") : QStringLiteral("sujiangf");

        room->setPlayerProperty(this, "general2_showed", true);

        JsonArray arg;
        arg << (int)S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << true;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, general_name);

        setSkillsPreshowed(QStringLiteral("d"), false);
        // disconnectSkillsFromOthers(false);

        foreach (const Skill *skill, getDeputySkillList()) {
            if (skill != nullptr)
                room->detachSkillFromPlayer(this, skill->objectName(), false, false, false, false);
        }
    }

    LogMessage log;
    log.type = QStringLiteral("#BasaraRemove");
    log.from = this;
    log.arg = head_general ? QStringLiteral("head_general") : QStringLiteral("deputy_general");
    log.arg2 = from_general;
    room->sendLog(log);

    Q_ASSERT(room->getThread() != nullptr);
    ShowGeneralStruct s;
    s.player = this;
    s.isHead = head_general;
    s.isShow = false;
    QVariant _from = QVariant::fromValue(s);

    room->getThread()->trigger(QSanguosha::GeneralRemoved, _from);

    room->filterCards(this, getCards(QStringLiteral("hes")), true);
}

void ServerPlayer::sendSkillsToOthers(bool head_skill)
{
    QStringList names = room->getTag(objectName()).toStringList();
    if (names.isEmpty())
        return;
    const QSet<const Skill *> skills = head_skill ? getHeadSkillList() : getDeputySkillList();

    foreach (const Skill *skill, skills) {
        JsonArray args;
        args << QSanProtocol::S_GAME_EVENT_ADD_SKILL;
        args << objectName();
        args << skill->objectName();
        args << head_skill;
        foreach (ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }
}

int ServerPlayer::getPlayerNumWithSameKingdom(const QString & /*unused*/, const QString &_to_calculate) const
{
    QString to_calculate = _to_calculate;

    if (to_calculate.isEmpty()) {
        if (getRoleString() == QStringLiteral("careerist"))
            to_calculate = QStringLiteral("careerist");
        else
            to_calculate = getRoleString();
    }

    QList<ServerPlayer *> players = room->getAlivePlayers();

    int num = 0;
    foreach (ServerPlayer *p, players) {
        if (!p->haveShownOneGeneral())
            continue;
        if (p->getRoleString() == QStringLiteral("careerist")) { // if player is careerist, DO NOT COUNT AS SOME KINGDOM!!!!!
            if (to_calculate == QStringLiteral("careerist"))
                num = 1;
            continue;
        }
        if (p->getRoleString() == to_calculate) //hegemony
            ++num;
    }

    return qMax(num, 0);
}

bool ServerPlayer::askForGeneralShow(bool one, bool refusable)
{
    if (haveShownAllGenerals())
        return false;

    QStringList choices;

    if (!haveShownGeneral() && disableShow(true).isEmpty())
        choices << QStringLiteral("show_head_general");
    if (!hasShownGeneral2() && disableShow(false).isEmpty())
        choices << QStringLiteral("show_deputy_general");
    if (choices.isEmpty())
        return false;
    if (!one && choices.length() == 2)
        choices << QStringLiteral("show_both_generals");
    if (refusable)
        choices.append(QStringLiteral("cancel"));

    QString choice = room->askForChoice(this, QStringLiteral("GameRule_AskForGeneralShow"), choices.join(QStringLiteral("+")));

    if (choice == QStringLiteral("show_head_general") || choice == QStringLiteral("show_both_generals"))
        showGeneral();
    if (choice == QStringLiteral("show_deputy_general") || choice == QStringLiteral("show_both_generals"))
        showGeneral(false);

    return choice.startsWith(QStringLiteral("s"));
}

bool ServerPlayer::inSiegeRelation(const ServerPlayer *skill_owner, const ServerPlayer *victim) const
{
    if (isFriendWith(victim) || !isFriendWith(skill_owner) || !victim->haveShownOneGeneral())
        return false;
    if (this == skill_owner)
        return (getNextAlive() == victim && getNextAlive(2)->isFriendWith(this)) || (getLastAlive() == victim && getLastAlive(2)->isFriendWith(this));
    else
        return (getNextAlive() == victim && getNextAlive(2) == skill_owner) || (getLastAlive() == victim && getLastAlive(2) == skill_owner);
}

bool ServerPlayer::inFormationRalation(ServerPlayer *teammate) const
{
    QList<const Player *> teammates = getFormation();
    return teammates.length() > 1 && teammates.contains(teammate);
}

void ServerPlayer::summonFriends(const QString &type)
{
    room->tryPause();

    if (room->alivePlayerCount() < 4)
        return;
    LogMessage log;
    log.type = QStringLiteral("#InvokeSkill");
    log.from = this;
    log.arg = QStringLiteral("GameRule_AskForArraySummon");
    room->sendLog(log);
    LogMessage log2;
    log2.type = QStringLiteral("#SummonType");
    log2.arg = (type == QStringLiteral("Siege")) ? QStringLiteral("summon_type_siege") : QStringLiteral("summon_type_formation");
    room->sendLog(log2);

    if (type == QStringLiteral("Siege")) {
        if (isFriendWith(getNextAlive()) && isFriendWith(getLastAlive()))
            return;
        bool failed = true;
        if (!isFriendWith(getNextAlive()) && getNextAlive()->haveShownOneGeneral()) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getNextAlive(2));
            if (!target->haveShownOneGeneral()) {
                QString prompt = target->willBeFriendWith(this) ? QStringLiteral("SiegeSummon") : QStringLiteral("SiegeSummon!");
                bool success = room->askForSkillInvoke(target, prompt);
                LogMessage log;
                log.type = QStringLiteral("#SummonResult");
                log.from = target;
                log.arg = success ? QStringLiteral("summon_success") : QStringLiteral("summon_failed");
                room->sendLog(log);
                if (success) {
                    target->askForGeneralShow();
                    failed = false;
                }
            }
        }
        if (!isFriendWith(getLastAlive()) && getLastAlive()->haveShownOneGeneral()) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getLastAlive(2));
            if (!target->haveShownOneGeneral()) {
                QString prompt = target->willBeFriendWith(this) ? QStringLiteral("SiegeSummon") : QStringLiteral("SiegeSummon!");
                bool success = room->askForSkillInvoke(target, prompt);
                LogMessage log;
                log.type = QStringLiteral("#SummonResult");
                log.from = target;
                log.arg = success ? QStringLiteral("summon_success") : QStringLiteral("summon_failed");
                room->sendLog(log);
                if (success) {
                    target->askForGeneralShow();
                    failed = false;
                }
            }
        }
        if (failed)
            room->setPlayerFlag(this, QStringLiteral("Global_SummonFailed"));

    } else if (type == QStringLiteral("Formation")) {
        int n = room->players(false, false).length();
        int asked = n;
        bool failed = true;
        for (int i = 1; i < n; ++i) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getNextAlive(i));
            if (isFriendWith(target))
                continue;
            else if (!target->haveShownOneGeneral()) {
                QString prompt = target->willBeFriendWith(this) ? QStringLiteral("FormationSummon") : QStringLiteral("FormationSummon!");
                bool success = room->askForSkillInvoke(target, prompt);
                LogMessage log;
                log.type = QStringLiteral("#SummonResult");
                log.from = target;
                log.arg = success ? QStringLiteral("summon_success") : QStringLiteral("summon_failed");
                room->sendLog(log);

                if (success) {
                    target->askForGeneralShow();
                    room->doBattleArrayAnimate(target); //player success animation
                    failed = false;
                } else {
                    asked = i;
                    break;
                }
            } else {
                asked = i;
                break;
            }
        }

        n -= asked;
        for (int i = 1; i < n; ++i) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getLastAlive(i));
            if (isFriendWith(target))
                continue;
            else {
                if (!target->haveShownOneGeneral()) {
                    QString prompt = target->willBeFriendWith(this) ? QStringLiteral("FormationSummon") : QStringLiteral("FormationSummon!");
                    bool success = room->askForSkillInvoke(target, prompt);
                    LogMessage log;
                    log.type = QStringLiteral("#SummonResult");
                    log.from = target;
                    log.arg = success ? QStringLiteral("summon_success") : QStringLiteral("summon_failed");
                    room->sendLog(log);

                    if (success) {
                        target->askForGeneralShow();
                        room->doBattleArrayAnimate(target); //player success animation
                        failed = false;
                    }
                }
                break;
            }
        }
        if (failed)
            room->setPlayerFlag(this, QStringLiteral("Global_SummonFailed"));
    }
}

bool ServerPlayer::isReady() const
{
    return ready;
}

void ServerPlayer::setReady(bool ready)
{
    this->ready = ready;
}
