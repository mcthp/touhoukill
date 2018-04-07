-- translation for StandardPackage

local t = {
	["standard_cards"] = "标准版",

	["slash"] = "杀",
	[":slash"] = "基本牌<br />出牌时机：出牌阶段<br />使用目标：攻击范围内的一名其他角色<br />作用效果：对目标角色造成<font color=\"green\"><b>1点</b></font>伤害",
	["slash-jink"] = "%src 使用了【杀】，请使用一张【闪】",
	["@multi-jink-start"] = "%src 使用了【杀】，你须连续使用 %arg 张【闪】",
	["@multi-jink"] = "%src 使用了【杀】，你须再使用 %arg 张【闪】",
	["@slash_extra_targets"] = "请选择此【杀】的额外目标",

	["jink"] = "闪",
	[":jink"] = "基本牌<br />出牌时机：以你为目标的【杀】结算时<br />使用目标：以你为目标的【杀】<br />作用效果：抵消目标【杀】的效果",
	["#NoJink"] = "%from 不能使用【<font color=\"yellow\"><b>闪</b></font>】响应此【<font color=\"yellow\"><b>杀</b></font>】",

	["peach"] = "桃",
	[":peach"] = "基本牌<br />出牌时机：1、出牌阶段；2、有角色处于濒死状态时<br />使用目标：1、包括你的一名已受伤角色；2、处于濒死状态的一名角色<br />作用效果：目标角色回复<font color=\"green\"><b>1点</b></font>体力",

	["Crossbow"] = "诸葛连弩",
	[":Crossbow"] = "装备牌·武器<br />攻击范围：１<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>你于出牌阶段内使用【杀】无数量限制。",

	["DoubleSword"] = "雌雄双股剑",
	[":DoubleSword"] = "装备牌·武器<br />攻击范围：２<br />武器特效：每当你使用【杀】指定一个目标后，若两者所属势力不相同且其中一方所属势力与主公的所属势力相同时，你可以令其选择一项：弃置一张手牌，或令你摸一张牌。",
	["double-sword-card"] = "%src 发动了【雌雄双股剑】效果，你须弃置一张手牌，或令 %src 摸一张牌",

	["QinggangSword"] = "青釭剑",
	[":QinggangSword"] = "装备牌·武器<br />攻击范围：２<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>每当你使用【杀】指定一个目标后，你无视其防具。",

	["Blade"] = "青龙偃月刀",
	[":Blade"] = "装备牌·武器<br />攻击范围：３<br />武器特效：当你使用的【杀】指定目标后，你可以判定，若结果为红色，此【杀】不能被【闪】响应。",
	["blade-slash"] = "你可以发动【青龙偃月刀】再对 %src 使用一张【杀】",
	["#BladeUse"] = "%from 对 %to 发动了【<font color=\"yellow\"><b>青龙偃月刀</b></font>】效果",

	["Spear"] = "丈八蛇矛",
	[":Spear"] = "装备牌·武器<br />攻击范围：３<br />武器特效：你可以将两张手牌当【杀】使用或打出",

	["Axe"] = "贯石斧",
	[":Axe"] = "装备牌·武器<br />攻击范围：３<br />武器特效：每当你使用的【杀】被【闪】抵消后，你可以弃置两张牌，则此【杀】继续造成伤害。",
	["@Axe"] = "你可以弃置两张牌令此【杀】继续造成伤害",
	["~Axe"] = "选择两张牌→点击确定",

	["Halberd"] = "方天画戟",
	[":Halberd"] = "装备牌·武器<br />攻击范围：４<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>若你使用的【杀】是最后的手牌，你使用此【杀】选择目标的个数上限+2。",

	["KylinBow"] = "麒麟弓",
	[":KylinBow"] = "装备牌·武器<br />攻击范围：５<br />武器特效：每当你使用【杀】对目标角色造成伤害时，你可以弃置其装备区内的一张坐骑牌。",
	["KylinBow:dhorse"] = "+1坐骑",
	["KylinBow:ohorse"] = "-1坐骑",

	["EightDiagram"] = "八卦阵",
	[":EightDiagram"] = "装备牌·防具<br />防具效果：每当你需要使用或打出【闪】时，你可以进行判定，若结果为红色，你视为使用或打出了一张【闪】。",

	["standard_ex_cards"] = "标准版EX",

	["RenwangShield"] = "仁王盾",
	[":RenwangShield"] = "装备牌·防具<br />防具效果：<font color=\"blue\"><b>锁定技，</b></font>黑色【杀】对你无效",

	["IceSword"] = "寒冰剑",
	[":IceSword"] = "装备牌·武器<br />攻击范围：２<br />武器特效：每当你使用【杀】对目标角色造成伤害时，若该角色有牌，你可以防止此伤害，然后依次弃置其两张牌。",

	["Horse"] = "坐骑",
	[":+1 horse"] = "装备牌·坐骑<br />坐骑效果：其他角色与你的距离+1。",
	["JueYing"] = "绝影",
	["DiLu"] = "的卢",
	["ZhuaHuangFeiDian"] = "爪黄飞电",
	[":-1 horse"] = "装备牌·坐骑<br />坐骑效果：你与其他角色的距离-1。",
	["ChiTu"] = "赤兔",
	["DaYuan"] = "大宛",
	["ZiXing"] = "紫骍",

	["amazing_grace"] = "五谷丰登",
	[":amazing_grace"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有角色。<br />作用效果：你亮出牌堆顶等于现存角色数量的牌，每名目标角色获得其中<font color=\"green\"><b>一张</b></font>牌。",

	["god_salvation"] = "桃园结义",
	[":god_salvation"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有角色。<br />作用效果：每名目标角色回复<font color=\"green\"><b>1点</b></font>体力",

	["savage_assault"] = "南蛮入侵",
	[":savage_assault"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有其他角色。<br />作用效果：每名目标角色须依次打出<font color=\"green\"><b>一张</b></font>【杀】，否则受到<font color=\"green\"><b>1点</b></font>伤害",
	["savage-assault-slash"] = "%src 使用了【南蛮入侵】，请打出 %dest 张【杀】来响应。 <br />本次询问 <font color=\"green\"><b>第%arg 张</b></font>【杀】",

	["archery_attack"] = "万箭齐发",
	[":archery_attack"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有其他角色。<br />作用效果：每名目标角色须依次打出<font color=\"green\"><b>一张</b></font>【闪】，否则受到<font color=\"green\"><b>1点</b></font>伤害",
	["archery-attack-jink"] = "%src 使用了【万箭齐发】，请打出 %dest 张【闪】以响应。 <br />本次询问 <font color=\"green\"><b>第%arg 张</b></font>【闪】",

	["collateral"] = "借刀杀人",
	--[":collateral"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：装备区内有武器牌的且攻击范围内有另一名角色的一名其他角色A。（你需要选择另一名在A攻击范围内的角色B）。<br />作用效果：A须对B使用一张【杀】，否则你获得A装备区内的武器牌。",
	[":collateral"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：装备区内有武器牌的一名其他角色。。<br />作用效果：你选择一名在目标角色攻击范围内的角色，然后目标角色选择一项：对其使用一张【杀】；你获得目标角色装备区内的武器牌。",

	["collateral-chooseVictim"] = "选择 <font color=\"green\"><b>%src</b></font> 执行【借刀杀人】时 使用【杀】的目标",
	["collateral-slash"] = "%dest 使用了【借刀杀人】，请对 %src 使用一张【杀】",
	["#CollateralSlash"] = "【借刀杀人】的目标角色 %from 需使用【<font color=\"yellow\"><b>杀</b></font>】的目标被指定为 %to",

	["duel"] = "决斗",
	[":duel"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名其他角色<br />作用效果：由目标角色开始，你与其轮流地进行以下操作： 依次打出<font color=\"green\"><b>一张</b></font>【杀】。首先不打出【杀】的一方受到对方造成的<font color=\"green\"><b>1点</b></font>伤害。",
	["duel-slash"] = "%src 对你【决斗】，你需要打出 %dest 张【杀】。<br />本次询问 <font color=\"green\"><b>第%arg 张</b></font>【杀】",

	["ex_nihilo"] = "无中生有",
	[":ex_nihilo"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：包括你的一名角色<br />作用效果：摸<font color=\"green\"><b>两张</b></font>牌。",

	["snatch"] = "顺手牵羊",
	[":snatch"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：距离1的一名角色<br />作用效果：你获得其区域里的<font color=\"green\"><b>一张</b></font>牌",

	["dismantlement"] = "过河拆桥",
	[":dismantlement"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名其他角色。<br />作用效果：你弃置其区域里的<font color=\"green\"><b>一张</b></font>牌",

	["nullification"] = "无懈可击",
	[":nullification"] = "锦囊牌<br />出牌时机：目标锦囊对目标角色生效前<br />使用目标：目标锦囊。<br />作用效果：抵消目标锦囊牌对一名角色产生的效果，或抵消另一张【无懈可击】产生的效果。",

	["indulgence"] = "乐不思蜀",
	[":indulgence"] = "一次性延时锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名其他角色。<br />作用效果：目标角色进行判定，若结果不为<font color=\"red\">♥</font>，目标角色跳过出牌阶段。",

	["lightning"] = "闪电",
	[":lightning"] = "流动性延时锦囊牌<br />出牌时机：出牌阶段<br />使用目标：你<br />作用效果：目标角色进行判定，若结果为♠2~9，则目标角色受到3点无来源的雷电伤害；若结果不为♠2~9，将之移动到其下家的判定区里。",

	["limitation_broken"] = "界限突破卡牌",

	["wooden_ox"] = "木牛流马",
	[":wooden_ox"] = "装备牌·宝物<br /><b>宝物技能</b>：<br />" ..
					"1. 阶段技。你可以将一张手牌置于装备区的【木牛流马】下：若如此做，你可以将【木牛流马】移动至一名其他角色的装备区。<br />" ..
					"2. 你可以将装备区的【木牛流马】下的牌视为手牌使用或打出。<br />" ..
					"◆每当你失去装备区的【木牛流马】后，若【木牛流马】未移动至装备区，其下的牌置入弃牌堆，否则这些牌仍置于【木牛流马】下。<br />◆【木牛流马】下的牌为移出游戏。",
	["@wooden_ox-move"] = "你可以将【木牛流马】移动至一名其他角色的装备区",
	["#WoodenOx"] = "%from 使用/打出了 %arg 张 %arg2 牌",


	["BreastPlate"] = "护心镜",
	--[":BreastPlate"] = "装备牌·防具<br /><b>防具技能</b>：当你于回合外受到伤害时，若此伤害不小于X点（X为你的体力值），你可以获得装备区里的此牌，若如此做，防止此伤害。",
	[":BreastPlate"] = "装备牌·防具<br /><b>防具技能</b>：当你受到伤害时，若此伤害不小于X点（X为你的体力值），你可以横置装备区里的此牌。若如此做，防止此伤害。",
	["#BreastPlate"] = "%from 防止了 %to 对其造成的 %arg 点伤害[%arg2]",


	["Triblade"] = "三尖两刃刀",
	[":Triblade"] = "装备牌·武器\n\n攻击范围：3<br/>技能：每当你使用【杀】对目标角色造成伤害后，你可以弃置一张手牌并选择目标角色距离为1的一名其他角色，对其造成1点伤害。",
	["Tribladeskill"] = "三尖两刃刀",
	["@Triblade"] = "你可以发动【三尖两刃刀】的效果",
	["~Triblade"] = "选择一张牌→选择一名角色→点击确定",
	
	
	["lure_tiger"] = "调虎离山",
	[":lure_tiger"] = "锦囊牌\n\n使用时机：出牌阶段。\n使用目标：至多两名其他角色。\n作用效果：目标角色进入神隐状态直到回合结束。（其不计入距离和座次计算，且不会扣减体力，且不能使用牌且不是牌的合法目标。）\n执行动作：此牌结算结束时，你摸一张牌。",
	["lure_tiger_effect"] = "调虎离山",
	["#lure_tiger-prohibit"] = "调虎离山",
	
	["drowning"] = "水淹七军",
	[":drowning"] = "锦囊牌\n\n使用时机：出牌阶段。\n使用目标：所有装备区里有牌的其他角色。\n作用效果：目标角色须弃置其装备区里的<font color=\"green\"><b>一张</b></font>牌。",
	["@drowning"] = "你须弃置装备区里的一张牌",
	
	["known_both"] = "知己知彼",
	[":known_both"] = "锦囊牌\n\n使用时机：出牌阶段。\n使用目标：至多两名有暗置手牌的其他角色。\n作用效果：你明置目标角色的<font color=\"green\"><b>一张</b></font>暗置手牌。\n执行动作：此牌结算结束时，若你为当前回合人，直到此回合结束，其他角色不能使用或打出其明置手牌。\n◆此牌能重铸。",
	["#KnownBothLimit"] = "本回合，%from 以外的角色将无法使用 明置手牌 " ,

	["saving_energy"] = "养精蓄锐",
	[":saving_energy"] = "一次性延时锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名角色。<br />作用效果：目标角色进行判定，若结果不为<font color=\"black\"><b>♠</b></font>，目标角色跳过此回合的弃牌阶段，并将此牌置于人物牌旁。此回合结束时，其获得此牌。",
	["saving_energy_effect"] = "养精蓄锐",
	
	
	["DeathSickle"] = "死神镰刀",
	[":DeathSickle"] = "装备牌·武器\n\n攻击范围：2<br/>技能：当你使用【杀】指定一名目标后，若其体力下限为1，你可令其体力下限+1直到此【杀】结算完毕。",
	["#DeathSickle"] = "%from 体力下限增加了%arg2 点， 当前体力下限 为 %arg 点。",
}

local ohorses = { "ChiTu", "DaYuan", "ZiXing"}
local dhorses = { "ZhuaHuangFeiDian", "DiLu", "JueYing", "HuaLiu" }

for _, horse in ipairs(ohorses) do
	t[":" .. horse] = t[":-1 horse"]
end

for _, horse in ipairs(dhorses) do
	t[":" .. horse] = t[":+1 horse"]
end

return t