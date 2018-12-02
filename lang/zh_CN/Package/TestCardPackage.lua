-- translation for ManeuveringPackage

return {
	["test_card"] = "东方杀测试牌",

	["light_slash"] = "秽杀",
	[":light_slash"] = "基本牌<br />使用时机：出牌阶段限一次。<br />使用目标：你攻击范围内的一名其他角色。<br />作用效果：你明置目标角色的<font color=\"green\"><b>一张</b></font>手牌，然后对其造成<font color=\"green\"><b>1点</b></font>伤害。",

	["power_slash"] = "幻杀",
	[":power_slash"] = "基本牌<br />使用时机：出牌阶段限一次。<br />使用目标：你攻击范围内的一名其他角色。<br />作用效果：你横置目标角色装备区里的<font color=\"green\"><b>一张</b></font>牌，然后对其造成<font color=\"green\"><b>1点</b></font>伤害。",

	["iron_slash"] = "缚杀",
	[":iron_slash"] = "基本牌<br />使用时机：出牌阶段限一次。<br />使用目标：你攻击范围内的一名其他角色。<br />作用效果：你令目标角色横置，然后对其造成<font color=\"green\"><b>1点</b></font>伤害。",

	["magic_analeptic"] = "果酒",
	[":magic_analeptic"] = "基本牌<br />使用方法①<br />使用时机：出牌阶段，每回合限一次。<br />使用目标：包括你在内的一名角色。<br />作用效果：目标角色于此回合内使用的下一张普通锦囊牌的第一个效果值+<font color=\"green\"><b>1</b></font>。<br />◆不处于濒死状态的角色使用【酒】默认是以使用方法①使用【酒】。<br /> <br />使用方法②<br />使用时机：当你处于濒死状态时。<br />使用目标：你。<br />作用效果：你回复<font color=\"green\"><b>1点</b></font>体力。<br />◆处于濒死状态的角色使用【酒】默认是以使用方法②使用【酒】。",

	["super_peach"] = "仙桃",
	[":super_peach"] = "基本牌<br />使用方法①<br />使用时机：出牌阶段。<br />使用目标：一名处于异常状态的角色。<br />作用效果：目标角色暗置其所有明置手牌，然后重置并重置装备区里的所有横牌，最后回复<font color=\"green\"><b>1点</b></font>体力。<br />◆若一名角色的人物牌处于连环状态，或其装备区里有横牌，或其手牌里有明牌，则该角色处于异常状态。 <br /><br />使用方法②<br />使用时机：当一名角色处于濒死状态时。<br />使用目标：一名处于濒死状态的角色。<br />作用效果：目标角色暗置其所有明置手牌，然后重置并重置装备区里的所有横牌，最后回复<font color=\"green\"><b>1点</b></font>体力。",

	["chain_jink"] = "磁闪",
	[":chain_jink"] = "基本牌<br />使用时机：以你为目标的【杀】生效前。<br />使用目标：以你为目标的【杀】。<br />作用效果：你令此【杀】的使用者横置，然后抵消此【杀】。",

	["light_jink"] = "光闪",
	[":light_jink"] = "基本牌<br />使用时机：以你为目标的【杀】生效前。<br />使用目标：以你为目标的【杀】。<br />作用效果：你明置此【杀】的使用者的一张暗置手牌，然后抵消此【杀】。",


	["Gun"] = "狂气之枪",
	[":Gun"] = "装备牌·武器<br />攻击范围：4<br />装备技能：<font color=\"blue\"><b>锁定技，</b></font>当你使用【杀】对其他角色造成伤害后，横置其装备区里的所有牌。",
	["#Gun"]= "%from 对 %to 触发了 %arg 的效果",

	["Pillar"] = "御柱",
	[":Pillar"] = "装备牌·武器<br />攻击范围：3<br />装备技能： <font color=\"blue\"><b>锁定技，</b></font>结束阶段结束时，你选择一项：将一张基本牌当【杀】使用；横置你装备区里的此牌。",
	["@Pillar"] = "你因为<font color=\"green\"><b>御柱</b></font>的效果，请将一张基本牌当【杀】使用，否则你将横置装备区里的此牌",
	["~Pillar"] = "选择一张基本牌 -> 选择【杀】的目标 -> 确定；或取消 -> 横置",
	
	["Hakkero"] = "八卦炉",
	[":Hakkero"] = "装备牌·武器<br />攻击范围：3<br />装备技能：当你使用的【杀】被目标角色使用的【闪】抵消时，你可以弃置其X张牌（X为此【杀】的伤害值基数）。 ",
	
	["JadeSeal"] = "玉玺",
	[":JadeSeal"] = "装备牌·宝物<br />装备技能：出牌阶段限一次。你可以视为使用【知己知彼】。",
	["~JadeSeal"] = "选择【知己知彼】的合法目标 -> 确定",

	["Camouflage"] = "光学迷彩",
	[":Camouflage"] = "装备牌·防具<br />装备技能：<font color=\"blue\"><b>锁定技，</b></font>当你受到伤害时，若场上仅有一张防具牌，防止此伤害；当此牌因使用结算完毕而置入你的装备区后，你弃置场上一张与之牌名不同的防具牌。",
	["#Camouflage"] = "%from 的防具【%arg2】防止了 %arg 点伤害",
	["@camouflage"] = "光学迷彩： 请选择一名装备区里有防具牌的其他角色，弃置其装备区里的防具牌",
	
	
	["Hagoromo"] = "羽衣",
	[":Hagoromo"] = "装备牌·防具<br />装备技能：当你需要使用/打出【闪】时，若当前回合角色不处于连环状态，你可以横置装备区里的此牌，视为使用/打出磁【闪】。",


	["await_exhausted"] = "以逸待劳",
	[":await_exhausted"] = "锦囊牌·非全体性的普通锦囊牌<br />使用时机：出牌阶段。<br />使用目标：一至两名角色。<br />作用效果：目标角色摸<font color=\"green\"><b>两张</b></font>牌，然后弃置<font color=\"green\"><b>两张</b></font>牌。",


	["Pagoda"] = "宝塔",
	[":Pagoda"] = "装备牌·宝物<br />装备技能：你可以将一张黑色手牌当【无懈可击】使用。每回合限一次；<br />当你使用【无懈可击】抵消目标牌后，你可以令此牌对所有目标角色无效。",
	["#PagodaNullified"] = " 因%arg2 效果，  【%arg】 对 %from 的效果被抵消。",

	["alliance_feast"] = "联军盛宴",
	[":alliance_feast"] = "锦囊牌·全体性的普通锦囊牌<br />使用时机：出牌阶段。<br />使用目标：所有角色。<br />作用效果：目标角色暗置其所有明置手牌，然后重置并重置装备区里的所有横牌，最后摸<font color=\"green\"><b>一张</b></font>牌。<br />◆若一名角色的人物牌处于连环状态或其装备区里有横牌或其手牌里有明牌，则该角色处于异常状态。<br />◆对不处于异常状态的角色无效。",

	["fight_together"] = "勠力同心",
	[":fight_together"] = "锦囊牌·非全体性的普通锦囊牌<br />使用时机：出牌阶段。<br />使用目标：距离1以内的至少一名角色。<br />作用效果：你选择一项：\n1.明置目标角色的<font color=\"green\"><b>一张</b></font>手牌。   \n2.横置目标角色装备区里的<font color=\"green\"><b>一张</b></font>牌 。   \n3.令目标角色横置。 <br />执行动作：此回合结束时，若成为过此牌目标的角色可以从你未选择过的一项效果执行相反操作的效果，则其须选择其中一项并执行对应的相反操作的效果（明置手牌对应暗置手牌、横置装备区里的牌对应重置装备区里的横牌、令目标角色横置对应令目标角色重置）。",
	["fight_together_effect"] = "勠力同心(后续)",
	["#FightTogetherEffect"] = "%from 执行 勠力同心(后续)",

	["bone_healing"] = "刮骨疗毒",
	[":bone_healing"] = "锦囊牌·非全体性的普通锦囊牌<br />使用时机：出牌阶段。<br />使用目标：一名处于异常状态的其他角色。<br />作用效果：你对目标角色造成<font color=\"green\"><b>1点</b></font>伤害，若如此做，其暗置所有手牌，然后重置并重置装备区里的所有牌。◆若一名角色的人物牌处于连环状态，或其装备区里有横牌，或其手牌里有明牌，则该角色处于异常状态。",

	["spring_breath"] = "春息",
	[":spring_breath"] = "锦囊牌·一次性的延时类锦囊牌<br />使用时机：出牌阶段。<br />使用目标：一名角色。<br />作用效果：目标角色判定，若结果为<font size=\"5\", color=\"#FF0000\"><b>♥</b></font>2-9，其摸六张牌，否则其将此牌置入其判定区且此阶段内不再执行此牌效果。",

}
