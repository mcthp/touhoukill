return
{

	["hegemonyGeneral"] = "国战标准包",
    ["reimu_hegemony"] = "博丽灵梦",
    ["marisa_hegemony"] = "雾雨魔理沙",

--***********************
--春（花）
	["byakuren_hegemony"] = "圣白莲",
	["nue_hegemony"]= "封兽鵺",
	["toramaru_hegemony"] = "寅丸星",
	["murasa_hegemony"] = "村纱水蜜",
	
	["ichirin_hegemony"] = "云居一轮",--国战修改
	["lizhi_hegemony"] = "理智",
	[":lizhi_hegemony"] = "你使用的【杀】结算完毕后，若此杀未造成伤害，你可令一名与你阵营相同的角色获得此牌。",
	["@lizhi"] = "理智： 你使用的【杀】未造成伤害，你可令一名与你阵营相同的角色获得此牌。 ",
	
	["nazrin_hegemony"] = "娜兹玲",
	
	["miko_hegemony"] = "丰聪耳神子",--国战修改
	["qingting_hegemony"] = "倾听",
	[":qingting_hegemony"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以令有手牌的所有其他角色各将一张手牌交给你，然后你交给这些角色各一张手牌。",
	["chiling_hegemony"] = "敕令",
	[":chiling_hegemony"] = "<font color=\"purple\"><b>主将技，</b></font>此武将牌减少单个阴阳鱼。其他角色获得你的牌后，你可令其使用其中一张牌。",
	["@chiling_hegemony"] = "敕令： 你从 <font color=\"#00FF00\"><b>%src </b></font> 处获得了牌，你可以使用一张牌。",
	["shezheng_hegemony"] = "摄政",
	[":shezheng_hegemony"] = "<font color=\"purple\"><b>副将技，</b></font>若你装备区没有武器牌，你视为装备了【雌雄剑（国）】。",

	
	
	["mamizou_hegemony"] = "二岩猯藏",
	["futo_hegemony"] = "物部布都",
	
	
	["toziko_hegemony"] = "苏我屠自古",--国战修改
	["fenlei_hegemony"] = "忿雷",
	[":fenlei_hegemony"] = "当你脱离濒死状态后，你可令一名其他角色受到无来源的1点雷电伤害。",
	["@fenlei"] = "你可发动“忿雷”，令一名其他角色受到无来源的1点雷电伤害",
	
	["seiga_hegemony"]= "霍青娥",
	["yoshika_hegemony"]= "宫古芳香",
	
	["kogasa_hegemony"] = "多多良小伞",
	["jingxia_hegemony"] = "惊吓" ,--国战削弱
	[":jingxia_hegemony"] = "当你受到1点伤害后，你可以选择一项：依次弃置来源的两张牌；或弃置场上的一张牌。" ,
	["jingxia_hegemony:dismiss"] = "不发动技能" ,
	["jingxia_hegemony:discard"] = "弃置来源的两张牌" ,
	["jingxia_hegemony:discardfield"] = "弃置场上的一张牌" ,
	
	["kyouko_hegemony"] = "幽谷响子",
	["kokoro_hegemony"] = "秦心",
	
--***********************
--夏（月）

	["remilia_hegemony"] = "蕾米莉亚",
	["skltkexue_hegemony"] = "渴血",
	[":skltkexue_hegemony"] = "你进入濒死状态时，可以亮出此牌。当你向其他角色求【桃】时，若其体力值大于其体力下限，其可以失去1点体力，摸两张牌，然后令你回复1点体力。",
	
	["flandre_hegemony"] = "芙兰朵露",
	["sakuya_hegemony"] = "十六夜咲夜",
	["patchouli_hegemony"] = "帕秋莉",
	["!patchouli_hegemony"] = "帕秋莉·诺蕾姬",
	["meirin_hegemony"] = "红美铃",
	["beishui_hegemony"] = "背水",
	[":beishui_hegemony"] = "你可以将X张牌当任意基本牌使用（X为你的体力值和你所属阵营角色数中的较大值）。<font color=\"green\"><b>每阶段限一次。</b></font>",
	
	["koakuma_hegemony"] = "小恶魔",

	--辉夜 国战削弱
	["kaguya_hegemony"] = "蓬莱山辉夜",
	["xuyu_hegemony"] = "须臾",
	[":xuyu_hegemony"] = "<font color=\"purple\"><b>主将技，</b></font>锁定技，当你失去最后的手牌后，你移除副将，获得技能“永恒”。",
	--["yongheng_hegemony"] = "永恒",
	--[":yongheng_hegemony"] = "<font color=\"blue\"><b>锁定技，</b></font>你跳过弃牌阶段并将手牌调整至x张；当你于回合外获得或失去手牌后，你将手牌调整至x张。（x为你的体力上限）",
	

	["eirin_hegemony"] = "八意永琳",
	["yaoshi_hegemony"] = "药矢",
	[":yaoshi_hegemony"] = "当你使用牌对一名角色造成伤害时，你可防止此伤害，令其回复1点体力。",
	["#yaoshi_log"] = "因“%arg”的效果，%from防止了%arg2点伤害.",
	
	["mokou_hegemony"] = "藤原妹红",
	
	["reisen_hegemony"]="铃仙",
	["!reisen_hegemony"]="铃仙·优昙华院·因幡",
	
	--国战改动
	["keine_hegemony"] = "上白泽慧音",
	["xushi_hegemony"] = "虚史",
	[":xushi_hegemony"] = "当复数名角色成为牌的目标时，你可以取消其中一个目标。",
	["@xushi_hegemony_targetchosen"] = "虚史： 【%src】 有复数个目标， 你是否取消其中一个目标？",
	["#XushiHegemonySkillAvoid"] = "因为 “%arg”的效果，取消了 【%arg2】 的目标 %from ",
	
	--因幡帝 国战削弱
	["tewi_hegemony"] = "因幡天为",
	["xingyun_hegemony"] = "幸运",
	[":xingyun_hegemony"] = "当你获得牌后，你可以展示其中一张<font size=\"5\", color=\"#FF0000\"><b>♥</b></font>牌，你选择一项：回复1点体力；或令一名角色摸一张牌。",
	["@xingyun_hegemony"] = "你可以发动“幸运”，展示获得的牌中的一张<font size=\"5\", color=\"#FF0000\"><b>♥</b></font>牌。",
	["~xingyun_hegemony"] = "选择此次获得的一张红桃牌 ->确定",
	["xingyun:letdraw"] = "令一名角色摸一张牌" ,
	["xingyun:recover"] = "你回复1点体力" ,
	["@xingyun-select"]= "选择一名角色，令其摸一张牌。",
	
	["keine_sp_hegemony"] = "白泽",

	["toyohime_hegemony"] = "绵月丰姬",
	["yueshi_hegemony"] = "月使",
	[":yueshi_hegemony"] = "<font color=\"purple\"><b>主将技，</b></font>此武将牌减少单个阴阳鱼。你视为拥有“睿智”（当普通锦囊牌对你结算结束时，若你已受伤，你可以判定，若结果为红色，你回复1点体力）。",
	["yueshi_hegemony:invoke"]= "普通锦囊牌【%src】对你结算结束了，你可以发动“睿智”。",
	
	["yorihime_hegemony"] = "绵月依姬",


--***********************
--秋（风）	
	["kanako_hegemony"] = "八坂神奈子",
	["qiankun_kanako"] = "乾坤",
	[":qiankun_kanako"] = "<font color=\"blue\"><b>锁定技，</b></font>你的手牌上限+2。",
	
	["suwako_hegemony"] = "洩矢诹访子",
	["qiankun_suwako"] = "乾坤",
	[":qiankun_suwako"] = "<font color=\"blue\"><b>锁定技，</b></font>你的手牌上限+2。",
	["chuancheng_hegemony"] = "传承",
	[":chuancheng_hegemony"] = "当你死亡时，你可以令一名同阵营的其他角色获得“乾坤”和“传承”，然后其获得你区域里的所有牌。",
	["@chuancheng_hegemony"] = "选择一名同阵营的其他角色，令其获得“乾坤”和“传承”以及你区域里的所有牌。",
	
	["sanae_hegemony"] = "东风谷早苗",
	["aya_hegemony"] = "射命丸文",
	["nitori_hegemony"] = "河城荷取",
	["hina_hegemony"] = "键山雏",
	["momizi_hegemony"] = "犬走椛",
	["minoriko_hegemony"] = "秋穰子",
	["shizuha_hegemony"] = "秋静叶",
	
	--国战改动
	["satori_hegemony"] = "古明地觉",
	["duxin_hegemony"] = "读心",
	[":duxin_hegemony"] = "<font color=\"blue\"><b>锁定技，</b></font>当你选择其他角色的手牌时，其手牌对你可见。你成为其他角色使用牌的唯一目标时，你查看其暗置的副人物牌。",
	
	--国战改动
	["koishi_hegemony"] = "古明地恋",
	["wunian_hegemony"] = "无念",
    [":wunian_hegemony"] = "<font color=\"blue\"><b>锁定技，</b></font>你即将造成伤害时，你令一名其他角色成为伤害来源；你成为其他角色使用锦囊牌的目标时，若你已受伤，你取消自己。",
	["@wunian_transfer"] = "无念：你即将对 %src 造成伤害，你须令一名其他角色成为此伤害来源",

	
	["utsuho_hegemony"] = "灵乌路空",
	["rin_hegemony"] = "火焰猫燐",
	["yugi_hegemony"] = "星熊勇仪",
	["parsee_hegemony"]= "水桥帕露西",
	
--***********************
--冬（雪）

	["yuyuko_hegemony"] = "西行寺幽幽子",
	["yukari_hegemony"]= "八云紫",
	["ran_hegemony"]= "八云蓝",
	["youmu_hegemony"] = "魂魄妖梦",
	--["prismriver"]= "普莉兹姆利巴三姐妹",
	
	["lunasa_hegemony"] =  "露娜萨",
	["!lunasa_hegemony"] =  "露娜萨·普莉兹姆利巴",
	["#lunasa_hegemony"] = "骚灵提琴手",
	["merlin_hegemony"] =  "梅露兰",
	["!merlin_hegemony"] =  "梅露兰·普莉兹姆利巴",
	["#merlin_hegemony"] = "骚灵小号手",
	["lyrica_hegemony"] =  "莉莉卡",
	["!lyrica_hegemony"] =  "莉莉卡·普莉兹姆利巴",
	["#lyrica_hegemony"] = "骚灵键盘手",
	
	["alice_hegemony"]="爱丽丝",
	["chen_hegemony"] = "橙",
	["letty_hegemony"]="蕾蒂",
	["cirno_hegemony"] = "琪露诺",--国战修改
	["dongjie_hegemony"] = "冻结",
	[":dongjie_hegemony"] = "当你对一名角色造成伤害时，你可令其选择： 弃置一张手牌； 或者摸一张牌，其翻面并防止此伤害。",
	["bingpo_hegemony"]= "冰魄",
	[":bingpo_hegemony"]= "<font color=\"blue\"><b>锁定技，</b></font>当你因火焰伤害外的原因而进入濒死状态时，你回复1点体力。",
	["#bingpo_hegemony_log"] = "%from的“%arg”被触发, %from 回复了 %arg2点体力.",
	
	["daiyousei_hegemony"]= "大妖精",--国战修改
	["banyue_hegemony"]= "半月",
	[":banyue_hegemony"]= "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以选择两名阵营不同的角色并失去1点体力，视为其中一名对另一名角色使用【远交近攻】。",
	["juxian_hegemony"]= "具现",
	[":juxian_hegemony"]= "<font color=\"red\"><b>限定技，</b></font>当你进入濒死状态时，你可亮出牌堆顶的三张牌并获得之，然后你回复x点体力（x为亮出的花色数）。",
	["$juxianAnimate"]= "skill=daiyousei:juxian",
	
	
	["lilywhite_hegemony"] = "莉莉霍瓦特",
	
	--上海国战削弱
	["shanghai_hegemony"] = "上海人形" ,
	["zhancao_hegemony"] = "战操" ,
	[":zhancao_hegemony"] = "当你或你攻击范围内的一名角色成为【杀】的目标后，你可以弃置一张装备区的牌，令此【杀】对其无效。",
	["@zhancao_hegemony-discard"] = "<font color=\"#00FF00\"><b>%src </b></font>使用【杀】指定了<font color=\"#FF8000\"><b>%dest </b></font>为目标，你可以发动“战操”，弃置一张装备区的牌，使此【杀】对<font color=\"#FF8000\"><b>%dest </b></font>无效" ,
	["#zhancaoTarget"] = "%from 使用 %arg 的目标是 %to。" ,
	["mocao_hegemony"] = "魔操" ,
	[":mocao_hegemony"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以获得一名其他角色装备区里的一张牌，令其摸X张牌（X为其已损失的体力值且至少为1）。",

	
	
	["youki_hegemony"] = "魂魄妖忌" ,

	
}
