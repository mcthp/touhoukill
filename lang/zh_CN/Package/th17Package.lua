return {
	["gxs"] = "鬼",
	["th17"] = "鬼形兽",

--***********************************
	
	["keiki"] = "埴安神袿姬" ,
	["#keiki"] = "孤立无援造就的造形神" ,
	["zaoxing"] = "造形",
	[":zaoxing"] = "准备阶段开始时，你可声明一个与上回合不同的花色，直到你下个回合开始前，其他角色使用此花色的牌时，须选择一项：交给你一张同花色的牌，或令你令此牌无效并获得之。",
	["@zaoxing-discard"] = "%src 的造型你须选择交给 %src 一张 %arg2 牌，或你令此 %arg 无效并令 %src 获得之",
	["lingshou"] = "灵守",
	[":lingshou"] = "出牌阶段结束后，你可观看一名角色的手牌，并指定其中至少一张同花色的牌。然后你声明一张基本牌和其使用此牌的合法目标，令其选择是否弃置你指定的牌并视为使用之。",
	["zhuying"] = "祷应",
	[":zhuying"] = "主公技，每名鬼势力的角色出牌阶段限一次，其可选择一名角色并交给你一张手牌，令你可令本回合内其他角色与目标距离-1。",
	
	["eika"] = "戎璎花" ,
	["#eika"] = "河原的偶像水子" ,
	
	["shanlei"] = "善垒",
	[":shanlei"] = "<font color=\"blue\"><b>锁定技，</b></font>回合开始时，若你的手牌数大于你的手牌上限，你将手牌弃置至上限；回合结束时，若你的手牌数小于其他角色，你摸至X张（X为手牌最多的角色的手牌数+1）。",
	["@shanlei-discard"] = "善垒：由于你的手牌数大于你的手牌上限，你必须弃置手牌至上限",
	["bengluo"] = "崩落",
	[":bengluo"] = "一名角色的一个阶段结束时，若你于此阶段内有过不因使用而失去牌后使你的手牌数大于你的手牌上限的情况，你可以将一张手牌当【杀】使用。当此牌对目标造成伤害时，若其手牌数小于你，你可以将手牌调整至你的手牌上限，令此伤害值+1。",
	["@bengluo-kill"] = "崩落：你可以将一张手牌当【杀】使用（无距离限制）",
	["@bengluo-discard"] = "崩落：你可以弃置 %arg 张牌来让【杀】的伤害+1",
	["bengluo:@bengluo-draw"] = "崩落：你可以摸 %arg 张牌来让【杀】的伤害+1",
	
	["urumi"] = "牛崎润美" ,
	["#urumi"] = "古代鱼的携子护卫" ,
	
	["lunni"] = "沦溺",
	[":lunni"] = "其他角色回合开始时，你可以将一张装备牌置入其装备区（若已有同类型的牌则替换之），若如此做，此回合的：出牌段结束时，其获得其装备区里所有的牌；弃牌阶段结束时，你可以获得一张其于此阶段内弃置于弃牌堆里的装备牌。",
	["@lunni-discard"] = "沦溺：你可以将一张装备置入 %src 的装备区（若已有同类型的牌则替换之）",
	["#lunni-eff4"] = "%from 的出牌阶段结束，“沦溺”的效果生效。",
	["#lunni-eff5"] = "%from 的弃牌阶段结束，“沦溺”的效果生效。",
	["quangui"] = "劝归",
	[":quangui"] = "当其他角色因牌的效果受到大于1点的伤害而进入濒死状态时，你可以展示并获得其区域里的一张牌，若获得的是装备牌，其将体力回复至其体力下限。",

	["kutaka"] = "庭渡久侘歌" ,
	["#kutaka"] = "地狱口岸的守护神" ,
	["yvshou"] = "狱守",
	[":yvshou"] = " 其他角色的准备阶段开始时，你可以弃置一张手牌，若如此做，当其于此回合内使用第一张有点数的牌时，若点数不大于之，此牌无效，反之此牌不计入限制的使用次数。",
	["@yvshou-discard"] = "狱守： %src 的回合开始，你可以弃置一张牌发动“御守”",
	["lingdu"] = "灵渡",
	[":lingdu"] = "当你区域里的牌于回合外置入弃牌堆后（包括牌使用或打出结算完毕后），你可以用你区域里另一张牌替换之，若以此法失去了一个区域里最后的一张牌，你摸一张牌。<font color=\"green\"><b>每回合限一次。</b></font>",
	["#judging_area"] = "判定区",
	["@lingdu-discard"] = "灵渡：你可以用你区域里的一张牌来替换它",
	["$lingdulost"] = "%from 发动 %arg，用 %card 进行了交易。",
	
	["yachie"] = "吉吊八千慧" ,
	["#yachie"] = "鬼杰组组长" ,
	["duozhi"] = "夺志",
	[":duozhi"] = "<font color=\"blue\"><b>锁定技，</b></font>当你使用牌结算完毕后，你令所有其他角色于当前回合内不能使用或打出牌。",
	["#duozhi"] = "%to 变成了嘤嘤怪，在当前回合内不能使用或打出牌",
	
	["mayumi"] = "杖刀偶磨弓" ,
	["#mayumi"] = "埴轮兵长" ,
	["lingjun"] = "领军",
	[":lingjun"] = "当你于一个回合内使用的第一张【杀】结算完毕后，你可以选择此牌的一个目标，令攻击范围内有其的其他角色各选择是否将一张基本牌当【杀】对其使用（除其外的角色不是合法目标），然后若此次被以此法转化的牌均是【杀】，你视为对其使用【杀】。",
	["@lingjun-concentratefire"] = "领军：选择一个目标来让别人选择是否集火他",
	["@lingjun-fire"] = "领军： %src 要集火 %dest，你可以将一张基本牌当【杀】对 %dest 使用",
	["ciou"] = "瓷偶",
	[":ciou"] = "<font color=\"blue\"><b>锁定技，</b></font>当你受到伤害时，若受到的是【杀】造成的无属性伤害，此伤害结算结束后你失去1点体力，否则此伤害值-1。",
	["#ciou"] = "%from 的 %arg 碎了一地。",
	
	["saki"] = "骊驹早鬼" ,
	["#saki"] = "劲牙组组长" ,
	["jinji"] = "劲疾",
	[":jinji"] = "<font color=\"blue\"><b>锁定技，</b></font>你与其他角色的距离-X（X为你装备区里的牌数），其他角色与你的距离+Y（Y为你装备区里横置的牌数）。",
	["tianxing"] = "天行",
	[":tianxing"] = "一名其他角色的准备阶段开始时，你可以横置装备区里的一张牌，视为对其使用【杀】。当你使用【杀】对一名角色造成伤害后，其于此回合内不能使用以你为唯一目标的牌。",
	["@tianxing-discard"] = "天行：你可以横置一张装备区的牌，视为你对其使用了一张【杀】",
	["#tianxing"] = "%from 此回合内不能使用牌选择 %to 为唯一目标",
}
