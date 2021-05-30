#include "engine.h"
#include "CardFace.h"
#include "RoomObject.h"
#include "audio.h"
#include "aux-skills.h"
#include "card.h"
#include "client.h"
#include "lua-wrapper.h"
#include "protocol.h"
#include "settings.h"
#include "structs.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QVersionNumber>

#include <random>

Engine *Sanguosha = nullptr;

void Engine::addPackage(const QString &name)
{
    Package *pack = PackageAdder::packages()[name];
    if (pack)
        addPackage(pack);
    else
        qWarning("Package %s cannot be loaded!", qPrintable(name));
}

Engine::Engine()
    : l(LuaMultiThreadEnvironment::luaStateForCurrentThread())
{
    Sanguosha = this;

    JsonDocument doc = JsonDocument::fromFilePath("config/gameconfig.json");
    if (doc.isValid())
        configFile = doc.object();

    QStringList package_names = getConfigFromConfigFile("package_names").toStringList();
    foreach (QString name, package_names)
        addPackage(name);

    CardFactory::registerCardFace(new SurrenderCard);
    CardFactory::registerCardFace(new CheatCard);
    CardFactory::registerCardFace(new ChoosePlayerCard);
    CardFactory::registerCardFace(new YijiCard);

    LordBGMConvertList = getConfigFromConfigFile("bgm_convert_pairs").toStringList();
    LordBackdropConvertList = getConfigFromConfigFile("backdrop_convert_pairs").toStringList();
    LatestGeneralList = getConfigFromConfigFile("latest_generals").toStringList();

    QString locale = getConfigFromConfigFile("locale").toString();
    if (locale.length() == 0)
        locale = "zh_CN";

    loadTranslations(locale);

    // available game modes
    modes["02p"] = tr("2 players");
    modes["03p"] = tr("3 players");
    modes["04p"] = tr("4 players");
    modes["05p"] = tr("5 players");
    modes["06p"] = tr("6 players");
    modes["06pd"] = tr("6 players (2 renegades)");
    modes["07p"] = tr("7 players");
    modes["08p"] = tr("8 players");
    modes["08pd"] = tr("8 players (2 renegades)");
    modes["08pz"] = tr("8 players (0 renegade)");
    modes["09p"] = tr("9 players");
    modes["10pd"] = tr("10 players");
    modes["10p"] = tr("10 players (1 renegade)");
    modes["10pz"] = tr("10 players (0 renegade)");
    modes["hegemony_02"] = tr("hegemony 2 players");
    modes["hegemony_03"] = tr("hegemony 3 players");
    modes["hegemony_04"] = tr("hegemony 4 players");
    modes["hegemony_05"] = tr("hegemony 5 players");
    modes["hegemony_06"] = tr("hegemony 6 players");
    modes["hegemony_07"] = tr("hegemony 7 players");
    modes["hegemony_08"] = tr("hegemony 8 players");
    modes["hegemony_09"] = tr("hegemony 9 players");
    modes["hegemony_10"] = tr("hegemony 10 players");

    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Engine::addTranslationEntry(const QString &key, const QString &value)
{
    translations.insert(key, value);
}

Engine::~Engine()
{
#ifdef AUDIO_SUPPORT
    Audio::quit();
#endif
}

void Engine::loadTranslations(const QString &locale)
{
    JsonDocument doc = JsonDocument::fromFilePath(QStringLiteral("lang/") + locale + QStringLiteral(".json"));
    if (!doc.isValid() || !doc.isArray())
        return;

    JsonArray jarr = doc.array();

    foreach (const QVariant &fileNameV, jarr) {
        QString fileName = fileNameV.toString();
        JsonDocument transDoc = JsonDocument::fromFilePath(fileName);
        if (!transDoc.isValid() || !transDoc.isObject())
            continue;

        JsonObject ob = transDoc.object();

        for (auto it = ob.cbegin(); it != ob.cend(); ++it)
            addTranslationEntry(it.key(), it.value().toString());
    }
}

void Engine::addSkills(const QList<const Skill *> &all_skills)
{
    foreach (const Skill *skill, all_skills) {
        if (skills.contains(skill->objectName()))
            QMessageBox::warning(nullptr, "", tr("Duplicated skill : %1").arg(skill->objectName()));

        skills.insert(skill->objectName(), skill);

        if (skill->inherits("ProhibitSkill"))
            prohibit_skills << qobject_cast<const ProhibitSkill *>(skill);
        else if (skill->inherits("ViewHasSkill"))
            viewhas_skills << qobject_cast<const ViewHasSkill *>(skill);
        else if (skill->inherits("DistanceSkill"))
            distance_skills << qobject_cast<const DistanceSkill *>(skill);
        else if (skill->inherits("MaxCardsSkill"))
            maxcards_skills << qobject_cast<const MaxCardsSkill *>(skill);
        else if (skill->inherits("TargetModSkill"))
            targetmod_skills << qobject_cast<const TargetModSkill *>(skill);
        else if (skill->inherits("AttackRangeSkill"))
            attackrange_skills << qobject_cast<const AttackRangeSkill *>(skill);
        else if (skill->inherits("TriggerSkill")) {
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            if (trigger_skill && trigger_skill->isGlobal())
                global_trigger_skills << trigger_skill;
        } else if (skill->inherits("ViewAsSkill"))
            viewas_skills << qobject_cast<const ViewAsSkill *>(skill);
    }
}

QList<const DistanceSkill *> Engine::getDistanceSkills() const
{
    return distance_skills;
}

QList<const MaxCardsSkill *> Engine::getMaxCardsSkills() const
{
    return maxcards_skills;
}

QList<const TargetModSkill *> Engine::getTargetModSkills() const
{
    return targetmod_skills;
}

QList<const AttackRangeSkill *> Engine::getAttackRangeSkills() const
{
    return attackrange_skills;
}

QList<const TriggerSkill *> Engine::getGlobalTriggerSkills() const
{
    return global_trigger_skills;
}

QList<const ViewAsSkill *> Engine::getViewAsSkills() const
{
    return viewas_skills;
}

void Engine::addPackage(Package *package)
{
    if (packages.contains(package))
        return;

    packages << package;

    // package->setParent(this);
    patterns.insert(package->patterns());
    related_skills.unite(package->relatedSkills());

    foreach (auto face, package->cardFaces()) {
        // TODO: How to register skill card???
        CardFactory::registerCardFace(face);
    }

    cards << package->cards();

    addSkills(package->skills());

    foreach (General *general, package->generals()) {
        // TODO: Shall we split skill and general?
        addSkills(general->findChildren<const Skill *>());
        foreach (QString skill_name, general->getExtraSkillSet()) {
            if (skill_name.startsWith("#"))
                continue;
            foreach (const Skill *related, getRelatedSkills(skill_name))
                general->addSkill(related->objectName());
        }
        generals.insert(general->objectName(), general);
        if (isGeneralHidden(general->objectName()))
            continue;
        if (general->isLord())
            lord_list << general->objectName();
    }
}

void Engine::addBanPackage(const QString &package_name)
{
    ban_package.insert(package_name);
}

QStringList Engine::getBanPackages() const
{
    if (QCoreApplication::instance()->arguments().contains("-server"))
        return Config.BanPackages;
    else {
        if (isHegemonyGameMode(ServerInfo.GameMode)) {
            QStringList ban;
            const QList<const Package *> &packs = getPackages();
            QStringList needPacks;
            needPacks << "hegemonyGeneral"
                      << "hegemony_card";
            foreach (const Package *pa, packs) {
                if (!needPacks.contains(pa->name()))
                    ban << pa->name();
            }
            return ban;
        } else {
            QStringList ban = ban_package.values();
            if (!ban.contains("hegemonyGeneral"))
                ban << "hegemonyGeneral";
            if (!ban.contains("hegemony_card"))
                ban << "hegemony_card";
            return ban;
        }
    }
}

QList<const Package *> Engine::getPackages() const
{
    return packages;
}

const Package *Engine::findPackage(const QString &name) const
{
    foreach (auto pkg, packages) {
        if (pkg->name() == name)
            return pkg;
    }

    return nullptr;
}

QString Engine::translate(const QString &to_translate, bool addHegemony) const
{
    QStringList list = to_translate.split("\\");
    QString res;
    foreach (QString str, list) {
        if (addHegemony && !str.endsWith("_hegemony")) {
            QString strh = str + "_hegemony";
            if (translations.contains(strh))
                res.append(translations.value(strh, strh));
            else
                res.append(translations.value(str, str));
        } else
            res.append(translations.value(str, str));
    }

    return res;
}

int Engine::getRoleIndex() const
{
    if (ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "06_XMode") {
        return 4;
    } else if (isHegemonyGameMode(ServerInfo.GameMode))
        return 5;
    else
        return 1;
}

const CardPattern *Engine::getPattern(const QString &name) const
{
    const CardPattern *ptn = patterns.value(name, NULL);
    if (ptn)
        return ptn;

    ExpPattern *expptn = new ExpPattern(name);
    patterns.insert(name, expptn);
    return expptn;
}

bool Engine::matchExpPattern(const QString &pattern, const Player *player, const Card *card) const
{
    ExpPattern p(pattern);
    return p.match(player, card);
}

Card::HandlingMethod Engine::getCardHandlingMethod(const QString &method_name) const
{
    if (method_name == "use")
        return Card::MethodUse;
    else if (method_name == "response")
        return Card::MethodResponse;
    else if (method_name == "discard")
        return Card::MethodDiscard;
    else if (method_name == "recast")
        return Card::MethodRecast;
    else if (method_name == "pindian")
        return Card::MethodPindian;
    else {
        Q_ASSERT(false);
        return Card::MethodNone;
    }
}

QList<const Skill *> Engine::getRelatedSkills(const QString &skill_name) const
{
    QList<const Skill *> skills;
    foreach (QString name, related_skills.values(skill_name))
        skills << getSkill(name);

    return skills;
}

const Skill *Engine::getMainSkill(const QString &skill_name) const
{
    const Skill *skill = getSkill(skill_name);
    if (!skill || skill->isVisible() || related_skills.keys().contains(skill_name))
        return skill;
    foreach (QString key, related_skills.keys()) {
        foreach (QString name, related_skills.values(key))
            if (name == skill_name)
                return getSkill(key);
    }
    return skill;
}

const General *Engine::getGeneral(const QString &name) const
{
    return generals.value(name, NULL);
}

const QStringList Engine::getGenerals() const
{
    return generals.keys();
}

int Engine::getGeneralCount(bool include_banned) const
{
    if (include_banned)
        return generals.size();

    int total = generals.size();
    QHashIterator<QString, const General *> itor(generals);
    while (itor.hasNext()) {
        itor.next();
        const General *general = itor.value();
        if (getBanPackages().contains(general->getPackage()))
            total--;
        else if (isGeneralHidden(general->objectName()))
            total--;
        else if (isNormalGameMode(ServerInfo.GameMode) && Config.value("Banlist/Roles").toStringList().contains(general->objectName()))
            total--;
        else if (ServerInfo.GameMode == "04_1v3" && Config.value("Banlist/HulaoPass").toStringList().contains(general->objectName()))
            total--;
    }

    return total;
}

bool Engine::isGeneralHidden(const QString &general_name) const
{
    const General *general = getGeneral(general_name);
    if (!general)
        return false;
    if (!general->isVisible())
        return false;
    if (!general->isHidden())
        return Config.ExtraHiddenGenerals.contains(general_name);
    else
        return !Config.RemovedHiddenGenerals.contains(general_name);
}

const CardDescriptor &Engine::getEngineCard(int cardId) const
{
    static CardDescriptor nullDescriptor = {QString(), Card::NoSuit, Card::NumberNA, QString()};

    if (cardId == Card::S_UNKNOWN_CARD_ID)
        return nullDescriptor;
    else if (cardId < 0 || cardId >= cards.length()) {
        Q_ASSERT(!(cardId < 0 || cardId >= cards.length()));
        return nullDescriptor;
    } else {
        return cards[cardId];
    }
}

QString Engine::getVersionNumber() const
{
    return QT_STRINGIFY(VERSIONNUMBER);
}

QString Engine::getVersion() const
{
    return QString("%1:%2").arg(getVersionNumber()).arg(getMODName());
}

QString Engine::getVersionName() const
{
    return "V" QT_STRINGIFY(VERSION);
}

QVersionNumber Engine::getQVersionNumber() const
{
    return QVersionNumber::fromString(QT_STRINGIFY(VERSION));
}

QString Engine::getMODName() const
{
    return "TouhouSatsu";
}

QStringList Engine::getExtensions() const
{
    QStringList extensions;
    const QList<const Package *> &packages = getPackages();
    foreach (const Package *package, packages)
        extensions << package->name();

    return extensions;
}

QStringList Engine::getKingdoms() const
{
    static QStringList kingdoms;

    if (kingdoms.isEmpty())
        kingdoms = getConfigFromConfigFile("kingdoms").toStringList();

    return kingdoms;
}

QStringList Engine::getHegemonyKingdoms() const
{
    static QStringList hegemony_kingdoms;
    if (hegemony_kingdoms.isEmpty())
        hegemony_kingdoms = getConfigFromConfigFile("hegemony_kingdoms").toStringList();

    return hegemony_kingdoms;
}

QColor Engine::getKingdomColor(const QString &kingdom) const
{
    static QMap<QString, QColor> color_map;
    if (color_map.isEmpty()) {
        QVariantMap map = getConfigFromConfigFile("kingdom_colors").toMap();
        QMapIterator<QString, QVariant> itor(map);
        while (itor.hasNext()) {
            itor.next();
            QColor color(itor.value().toString());
            if (!color.isValid()) {
                qWarning("Invalid color for kingdom %s", qPrintable(itor.key()));
                color = QColor(128, 128, 128);
            }
            color_map[itor.key()] = color;
        }

        Q_ASSERT(!color_map.isEmpty());
    }

    return color_map.value(kingdom);
}

QStringList Engine::getChattingEasyTexts() const
{
    static QStringList easy_texts;
    if (easy_texts.isEmpty())
        easy_texts = getConfigFromConfigFile("easy_text").toStringList();

    return easy_texts;
}

QString Engine::getSetupString() const
{
    int timeout = Config.OperationNoLimit ? 0 : Config.OperationTimeout;
    QString flags;
    if (Config.RandomSeat)
        flags.append("R");
    if (Config.EnableCheat)
        flags.append("C");
    if (Config.EnableCheat && Config.FreeChoose)
        flags.append("F");
    if (Config.Enable2ndGeneral || isHegemonyGameMode(Config.GameMode))
        flags.append("S");
    if (Config.EnableSame)
        flags.append("T");
    if (Config.EnableAI)
        flags.append("A");
    if (Config.DisableChat)
        flags.append("M");

    if (Config.MaxHpScheme == 1)
        flags.append("1");
    else if (Config.MaxHpScheme == 2)
        flags.append("2");
    else if (Config.MaxHpScheme == 3)
        flags.append("3");
    else if (Config.MaxHpScheme == 0) {
        char c = Config.Scheme0Subtraction + 5 + 'a'; // from -5 to 12
        flags.append(c);
    }

    QString server_name = Config.ServerName.toUtf8().toBase64();
    QStringList setup_items;
    QString mode = Config.GameMode;
    if (mode == "02_1v1")
        mode = mode + Config.value("1v1/Rule", "2013").toString();
    else if (mode == "06_3v3")
        mode = mode + Config.value("3v3/OfficialRule", "2013").toString();
    setup_items << server_name << Config.GameMode << QString::number(timeout) << QString::number(Config.NullificationCountDown) << getBanPackages().join("+") << flags;

    return setup_items.join(":");
}

QMap<QString, QString> Engine::getAvailableModes() const
{
    return modes;
}

QString Engine::getModeName(const QString &mode) const
{
    if (modes.contains(mode))
        return modes.value(mode);

    return QString();
}

int Engine::getPlayerCount(const QString &mode) const
{
    if (isHegemonyGameMode(mode)) {
        QStringList modestrings = mode.split("_");
        return modestrings.last().toInt(nullptr, 10); //return 2;
    }

    if (modes.contains(mode)) {
        QRegExp rx("(\\d+)");
        int index = rx.indexIn(mode);
        if (index != -1)
            return rx.capturedTexts().first().toInt();
    }

    return -1;
}

QString Engine::getRoles(const QString &mode) const
{
    int n = getPlayerCount(mode);

    if (mode == "02_1v1") {
        return "ZN";
    } else if (mode == "04_1v3") {
        return "ZFFF";
    }
    if (isHegemonyGameMode(mode)) {
        QString role;
        int num = getPlayerCount(mode);
        QStringList roles;
        roles << "W"
              << "S"
              << "G"
              << "Q"; //wei shu wu qun
        for (int i = 0; i < num; ++i) {
            int role_idx = QRandomGenerator::global()->generate() % roles.length();
            role = role + roles[role_idx];
        }
        return role;
    }

    if (modes.contains(mode)) {
        static const char *table1[] = {
            "",          "",

            "ZF", // 2
            "ZFN", // 3
            "ZNFF", // 4
            "ZCFFN", // 5
            "ZCFFFN", // 6
            "ZCCFFFN", // 7
            "ZCCFFFFN", // 8
            "ZCCCFFFFN", // 9
            "ZCCCFFFFFN" // 10
        };

        static const char *table2[] = {
            "",          "",

            "ZF", // 2
            "ZFN", // 3
            "ZNFF", // 4
            "ZCFFN", // 5
            "ZCFFNN", // 6
            "ZCCFFFN", // 7
            "ZCCFFFNN", // 8
            "ZCCCFFFFN", // 9
            "ZCCCFFFFNN" // 10
        };

        const char **table = mode.endsWith("d") ? table2 : table1;
        QString rolechar = table[n];
        if (mode.endsWith("z"))
            rolechar.replace("N", "C");

        return rolechar;
    } else if (mode.startsWith("@")) {
        if (n == 8)
            return "ZCCCNFFF";
        else if (n == 6)
            return "ZCCNFF";
    }
    return QString();
}

QStringList Engine::getRoleList(const QString &mode) const
{
    QString roles = getRoles(mode);

    QStringList role_list;
    for (int i = 0; roles[i] != nullptr; i++) {
        QString role;
        switch (roles[i].toLatin1()) {
        case 'Z':
            role = "lord";
            break;
        case 'C':
            role = "loyalist";
            break;
        case 'N':
            role = "renegade";
            break;
        case 'F':
            role = "rebel";
            break;
        case 'W':
            role = "wei";
            break;
        case 'S':
            role = "shu";
            break;
        case 'G':
            role = "wu";
            break;
        case 'Q':
            role = "qun";
            break;
        }
        role_list << role;
    }

    return role_list;
}

int Engine::getCardCount() const
{
    return cards.length();
}

QStringList Engine::getLords(bool contain_banned) const
{
    QStringList lords;

    // add intrinsic lord
    foreach (QString lord, lord_list) {
        const General *general = generals.value(lord);
        if (getBanPackages().contains(general->getPackage()))
            continue;
        if (!contain_banned) {
            if (ServerInfo.GameMode.endsWith("p") || ServerInfo.GameMode.endsWith("pd") || ServerInfo.GameMode.endsWith("pz"))
                if (Config.value("Banlist/Roles", "").toStringList().contains(lord))
                    continue;
        }
        lords << lord;
    }

    return lords;
}

QStringList Engine::getRandomLords() const
{
    QStringList banlist_ban;

    if (Config.GameMode == "zombie_mode")
        banlist_ban.append(Config.value("Banlist/Zombie").toStringList());
    else if (isNormalGameMode(Config.GameMode))
        banlist_ban.append(Config.value("Banlist/Roles").toStringList());

    QStringList lords;
    QStringList splords_package; //lords  in sp package will be not count as a lord.
    splords_package << "thndj";

    foreach (QString alord, getLords()) {
        if (banlist_ban.contains(alord))
            continue;
        const General *general = getGeneral(alord);
        if (splords_package.contains(general->getPackage()))
            continue;
        lords << alord;
    }

    int lord_num = Config.value("LordMaxChoice", 6).toInt();
    if (lord_num != -1 && lord_num < lords.length()) {
        int to_remove = lords.length() - lord_num;
        for (int i = 0; i < to_remove; i++) {
            lords.removeAt(QRandomGenerator::global()->generate() % lords.length());
        }
    }

    QStringList nonlord_list;
    foreach (QString nonlord, generals.keys()) {
        if (isGeneralHidden(nonlord))
            continue;
        const General *general = generals.value(nonlord);
        if (lord_list.contains(nonlord)) {
            if (!splords_package.contains(general->getPackage()))
                continue;
        }
        if (getBanPackages().contains(general->getPackage()))
            continue;
        if (banlist_ban.contains(general->objectName()))
            continue;

        nonlord_list << nonlord;
    }

    qShuffle(nonlord_list);

    int addcount = 0;
    int extra = Config.value("NonLordMaxChoice", 6).toInt();

    int godmax = Config.value("GodLimit", 1).toInt();
    int godCount = 0;

    if (lord_num == 0 && extra == 0)
        extra = 1;

    bool assign_latest_general = Config.value("AssignLatestGeneral", true).toBool();
    QStringList latest = getLatestGenerals(QSet<QString>(lords.begin(), lords.end()));
    if (assign_latest_general && !latest.isEmpty()) {
        lords << latest.first();
        if (nonlord_list.contains(latest.first()))
            nonlord_list.removeOne(latest.first());
        extra--;
    }

    for (int i = 0; addcount < extra; i++) {
        if (getGeneral(nonlord_list.at(i))->getKingdom() != "touhougod") {
            lords << nonlord_list.at(i);
            addcount++;
        } else if (godmax > 0 && godCount < godmax) {
            lords << nonlord_list.at(i);
            godCount++;
            addcount++;
        }

        if (i == nonlord_list.length() - 1)
            break;
    }

    return lords;
}

QStringList Engine::getLimitedGeneralNames() const
{
    QStringList general_names;
    QHashIterator<QString, const General *> itor(generals);
    if (ServerInfo.GameMode == "04_1v3") {
        QList<const General *> hulao_generals = QList<const General *>();
        foreach (QString pack_name, getConfigFromConfigFile("hulao_packages").toStringList()) {
            const Package *pack = findPackage(pack_name);
            if (pack) {
                foreach (General *general, pack->generals())
                    hulao_generals << general;
            }
        }

        foreach (const General *general, hulao_generals) {
            if (isGeneralHidden(general->objectName()) || general->isTotallyHidden() || general->objectName() == "yuyuko_1v3")
                continue;
            general_names << general->objectName();
        }
    } else {
        while (itor.hasNext()) {
            itor.next();
            if (!isGeneralHidden(itor.value()->objectName()) && !getBanPackages().contains(itor.value()->getPackage()))
                general_names << itor.key();
        }
    }

    return general_names;
}

void Engine::banRandomGods() const
{
    QStringList all_generals = getLimitedGeneralNames();

    qShuffle(all_generals);

    int count = 0;
    int max = Config.value("GodLimit", 1).toInt();

    if (max == -1)
        return;

    QStringList gods;

    foreach (const QString &general, all_generals) {
        if (getGeneral(general)->getKingdom() == "touhougod") {
            gods << general;
            count++;
        }
    };
    int bancount = count - max;
    if (bancount <= 0)
        return;
    QStringList ban_gods = gods.mid(0, bancount);
    Q_ASSERT(ban_gods.count() == bancount);

    QStringList ban_list = Config.value("Banlist/Roles").toStringList();

    ban_list.append(ban_gods);
    Config.setValue("Banlist/Roles", ban_list);
}

QStringList Engine::getRandomGenerals(int count, const QSet<QString> &ban_set) const
{
    QStringList all_generals = getLimitedGeneralNames();
    QSet<QString> general_set = QSet<QString>(all_generals.begin(), all_generals.end());

    Q_ASSERT(all_generals.count() >= count);

    QStringList subtractList;
    bool needsubtract = true;
    if (isNormalGameMode(ServerInfo.GameMode))
        subtractList = (Config.value("Banlist/Roles", "").toStringList());
    else if (ServerInfo.GameMode == "04_1v3")
        subtractList = (Config.value("Banlist/HulaoPass", "").toStringList());
    else if (ServerInfo.GameMode == "06_XMode")
        subtractList = (Config.value("Banlist/XMode", "").toStringList());
    else if (isHegemonyGameMode(ServerInfo.GameMode))
        subtractList = (Config.value("Banlist/Hegemony", "").toStringList());
    else
        needsubtract = false;

    if (needsubtract)
        general_set.subtract(QSet<QString>(subtractList.begin(), subtractList.end()));

    all_generals = general_set.subtract(ban_set).values();

    // shuffle them
    qShuffle(all_generals);

    int addcount = 0;
    QStringList general_list = QStringList();
    int godmax = Config.value("GodLimit", 1).toInt();
    int godCount = 0;
    for (int i = 0; addcount < count; i++) {
        if (getGeneral(all_generals.at(i))->getKingdom() != "touhougod") {
            general_list << all_generals.at(i);
            addcount++;
        } else if (godmax > 0 && godCount < godmax) {
            general_list << all_generals.at(i);
            godCount++;
            addcount++;
        }
        if (i == all_generals.count() - 1)
            break;
    }

    return general_list;
}

QStringList Engine::getLatestGenerals(const QSet<QString> &ban_set) const
{
    QSet<QString> general_set = QSet<QString>(LatestGeneralList.begin(), LatestGeneralList.end());

    QStringList subtractList;
    bool needsubtract = true;
    if (isNormalGameMode(ServerInfo.GameMode))
        subtractList = (Config.value("Banlist/Roles", "").toStringList());
    else if (ServerInfo.GameMode == "04_1v3")
        subtractList = (Config.value("Banlist/HulaoPass", "").toStringList());
    else if (ServerInfo.GameMode == "06_XMode")
        subtractList = (Config.value("Banlist/XMode", "").toStringList());
    else if (isHegemonyGameMode(ServerInfo.GameMode))
        subtractList = (Config.value("Banlist/Hegemony", "").toStringList());
    else
        needsubtract = false;

    if (needsubtract)
        general_set.subtract(QSet<QString>(subtractList.begin(), subtractList.end()));

    QStringList latest_generals = general_set.subtract(ban_set).values();
    if (!latest_generals.isEmpty())
        qShuffle(latest_generals);
    return latest_generals;
}

QList<int> Engine::getRandomCards() const
{
    // TODO: reimplement this function in separated class Mode
    bool exclude_disaters = false, using_2012_3v3 = false, using_2013_3v3 = false;

    if (Config.GameMode == "06_3v3") {
        using_2012_3v3 = (Config.value("3v3/OfficialRule", "2013").toString() == "2012");
        using_2013_3v3 = (Config.value("3v3/OfficialRule", "2013").toString() == "2013");
        exclude_disaters = Config.value("3v3/ExcludeDisasters", true).toBool();
    }

    if (Config.GameMode == "04_1v3")
        exclude_disaters = true;

    Q_UNUSED(exclude_disaters);

    QList<int> list;
    foreach (const CardDescriptor &card, cards) {
        // TODO: deal with this in separated class Mode
        Q_UNUSED(card);
#if 0

        if (exclude_disaters && card.face()->isKindOf("Disaster"))
            continue;

        if (getPackageNameByCard(card) == "New3v3Card" && (using_2012_3v3 || using_2013_3v3))
            list << card->id();
        else if (getPackageNameByCard(card) == "New3v3_2013Card" && using_2013_3v3)
            list << card->id();

        if (!getBanPackages().contains(getPackageNameByCard(card))) {
            if (card->faceName().startsWith("known_both")) {
                if (isHegemonyGameMode(Config.GameMode) && card->faceName() == "known_both_hegemony")
                    list << card->id();
                else if (!isHegemonyGameMode(Config.GameMode) && card->faceName() == "known_both")
                    list << card->id();

            } else if (card->faceName().startsWith("DoubleSword")) {
                if (isHegemonyGameMode(Config.GameMode) && card->faceName() == "DoubleSwordHegemony")
                    list << card->id();
                else if (!isHegemonyGameMode(Config.GameMode) && card->faceName() == "DoubleSword")
                    list << card->id();
            } else
                list << card->id();
        }
#endif
    }
    // remove two crossbows and one nullification?
    if (using_2012_3v3 || using_2013_3v3)
        list.removeOne(98);
    if (using_2013_3v3) {
        list.removeOne(53);
        list.removeOne(54);
    }

    qShuffle(list);

    return list;
}

QString Engine::getRandomGeneralName() const
{
    return generals.keys().at(QRandomGenerator::global()->generate() % generals.size());
}

void Engine::playSystemAudioEffect(const QString &name) const
{
    playAudioEffect(QString("audio/system/%1.ogg").arg(name));
}

void Engine::playAudioEffect(const QString &filename) const
{
#ifdef AUDIO_SUPPORT
    if (!Config.EnableEffects)
        return;
    if (filename.isNull())
        return;

    Audio::play(filename);
#endif
}

void Engine::playSkillAudioEffect(const QString &, int) const
{
    // TODO: move this function to UI
#if 0
    const Skill *skill = skills.value(skill_name, NULL);
    if (skill)
        skill->playAudioEffect(index);
#endif
}

const Skill *Engine::getSkill(const QString &skill_name) const
{
    return skills.value(skill_name, NULL);
}

const Skill *Engine::getSkill(const EquipCard *equip) const
{
    const Skill *skill = nullptr;

    if (equip != nullptr)
        skill = getSkill(equip->name());

    return skill;
}

QStringList Engine::getSkillNames() const
{
    return skills.keys();
}

const TriggerSkill *Engine::getTriggerSkill(const QString &skill_name) const
{
    const Skill *skill = getSkill(skill_name);
    if (skill)
        return qobject_cast<const TriggerSkill *>(skill);
    else
        return nullptr;
}

const ViewAsSkill *Engine::getViewAsSkill(const QString &skill_name) const
{
    const Skill *skill = getSkill(skill_name);
    if (skill == nullptr)
        return nullptr;

    if (skill->inherits("ViewAsSkill"))
        return qobject_cast<const ViewAsSkill *>(skill);
    else if (skill->inherits("DistanceSkill")) { //for hegemony showskill
        const DistanceSkill *distance_skill = qobject_cast<const DistanceSkill *>(skill);
        return distance_skill->getViewAsSkill();
    } else if (skill->inherits("MaxCardsSkill")) { //for hegemony showskill
        const MaxCardsSkill *distance_skill = qobject_cast<const MaxCardsSkill *>(skill);
        return distance_skill->getViewAsSkill();
    } else
        return nullptr;
}

const ProhibitSkill *Engine::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others) const
{
    bool ignore
        = (from->hasSkill("tianqu") && from->getRoomObject()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY && to != from && !card->hasFlag("IgnoreFailed"));
    if (ignore && !card->face()->isKindOf("SkillCard"))
        return nullptr;
    foreach (const ProhibitSkill *skill, prohibit_skills) {
        if (skill->isProhibited(from, to, card, others))
            return skill;
    }

    return nullptr;
}

const ViewHasSkill *Engine::ViewHas(const Player *player, const QString &skill_name, const QString &flag, bool ignore_preshow) const
{
    foreach (const ViewHasSkill *skill, viewhas_skills) {
        if (skill->ViewHas(player, skill_name, flag, ignore_preshow))
            return skill;
    }

    return nullptr;
}

int Engine::correctDistance(const Player *from, const Player *to) const
{
    int correct = 0;

    foreach (const DistanceSkill *skill, distance_skills) {
        correct += skill->getCorrect(from, to);
    }

    return correct;
}

int Engine::correctMaxCards(const Player *target, bool fixed, const QString &except) const
{
    int extra = 0;

    QStringList exceptlist = except.split("|");

    foreach (const MaxCardsSkill *skill, maxcards_skills) {
        if (exceptlist.contains(skill->objectName()))
            continue;

        if (fixed) {
            int f = skill->getFixed(target);
            if (f >= 0)
                return f;
        } else {
            extra += skill->getExtra(target);
        }
    }

    return extra;
}

int Engine::correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const
{
    int x = 0;
    QString cardskill = card->skillName();
    bool checkDoubleHidden = false;
    if (cardskill != nullptr)
        checkDoubleHidden = from->isHiddenSkill(cardskill);

    if (type == TargetModSkill::Residue) {
        foreach (const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                int residue = skill->getResidueNum(from, card);
                if (checkDoubleHidden && from->isHiddenSkill(skill->objectName()) && cardskill != skill->objectName() && !skill->objectName().startsWith("#" + cardskill))
                    continue;
                if (residue >= 998)
                    return residue;
                x += residue;
            }
        }
    } else if (type == TargetModSkill::DistanceLimit) {
        foreach (const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                int distance_limit = skill->getDistanceLimit(from, card);
                if (checkDoubleHidden && from->isHiddenSkill(skill->objectName()) && cardskill != skill->objectName() && !skill->objectName().startsWith("#" + cardskill))
                    continue;

                if (distance_limit >= 998)
                    return distance_limit;
                x += distance_limit;
            }
        }
    } else if (type == TargetModSkill::ExtraTarget) {
        foreach (const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card) && from->getMark("chuangshi_user") == 0) {
                if (checkDoubleHidden && from->isHiddenSkill(skill->objectName()) && cardskill != skill->objectName() && !skill->objectName().startsWith("#" + cardskill))
                    continue;
                x += skill->getExtraTargetNum(from, card);
            }
        }
    }

    return x;
}

int Engine::correctAttackRange(const Player *target, bool include_weapon /* = true */, bool fixed /* = false */) const
{
    int extra = 0;

    foreach (const AttackRangeSkill *skill, attackrange_skills) {
        if (fixed) {
            int f = skill->getFixed(target, include_weapon);
            if (f >= 0)
                return f;
        } else {
            extra += skill->getExtra(target, include_weapon);
        }
    }

    return extra;
}

int Engine::operationTimeRate(QSanProtocol::CommandType command, QVariant msg)
{
    int rate = 2; //default
    JsonArray arg = msg.value<JsonArray>();
    if (command == QSanProtocol::S_COMMAND_RESPONSE_CARD) {
        QString pattern = arg[0].toString();
        if (pattern == "@@qiusuo")
            rate = 4;
    }
    if (command == QSanProtocol::S_COMMAND_EXCHANGE_CARD) {
        QString reason = arg[5].toString();
        if (reason == "qingting")
            rate = 3;
    }
    return rate;
}

SurrenderCard::SurrenderCard()
{
    setTargetFixed(true);
    setDefaultHandlingMethod(Card::MethodNone);
}

void SurrenderCard::onUse(Room *room, const CardUseStruct &use) const
{
    room->makeSurrender(use.from);
}

CheatCard::CheatCard()
{
    setTargetFixed(true);
    setDefaultHandlingMethod(Card::MethodNone);
}

void CheatCard::onUse(Room *room, const CardUseStruct &use) const
{
    QString cheatString = use.card->userString();
    JsonDocument doc = JsonDocument::fromJson(cheatString.toUtf8().constData());
    if (doc.isValid())
        room->cheat(use.from, doc.toVariant());
}

QString Engine::GetMappedKingdom(const QString &role)
{
    static QMap<QString, QString> kingdoms;
    if (kingdoms.isEmpty()) {
        kingdoms["lord"] = "wei";
        kingdoms["loyalist"] = "shu";
        kingdoms["rebel"] = "wu";
        kingdoms["renegade"] = "qun";
    }
    if (kingdoms[role].isEmpty())
        return role;
    return kingdoms[role];
}

QVariant Engine::getConfigFromConfigFile(const QString &key) const
{
    // TODO: special case of "withHeroSkin" and "withBGM"
    return configFile.value(key);
}
