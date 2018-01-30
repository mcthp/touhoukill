return {
	["gzz"] = "绀",
	["th15"] = "绀珠传",
	
--***********************************
	--15001 纯狐 4hp
	["junko"] = "纯狐" ,
	["#junko"] = "无名之恨" ,
	["designer:junko"] = "三国有单",
	["xiahui"] = "瑕秽",
	[":xiahui"] = "你造成或受到伤害后，你可明置来源或受伤者的一张手牌。",
	["@xiahui"] = "你可以发动瑕秽明置 受伤者或来源 的一张手牌。",
	["chunhua"] = "纯化",
	[":chunhua"] = "一名角色使用明置的基本牌或普通锦囊指定目标角色后，你可根据其明置手牌的颜色（含此牌）令之效果改为：黑色“造成1点伤害”，红色“回复1点体力”。",
	["#Chunhua"] = "【%arg】 对 目标 %from 产生 纯化 效果",
	["#CancelChunhua"] = "【%arg】 的 纯化 效果 对 目标 %from 无效",
    ["chunhua:black"] = "黑色“造成1点伤害”" ,
	["chunhua:red"] = "红色“回复1点体力”" ,
	["$ChunhuaRed"] = "%from 对 %to 使用的【%arg】的效果改为 “回复1点体力”",
	["$ChunhuaBlack"] = "%from 对 %to 使用的【%arg】的效果改为 “造成1点伤害”",
	
	["shayi"] = "杀意",
	[":shayi"] = "<font color=\"orange\"><b>主公技，</b></font>其他绀势力角色的弃牌阶段结束时，你可以使用一张此阶段置入弃牌堆的【杀】。",
	["shayi_change"] = "杀意（主公技）",
    ["$Shayi"] = "%from 发动 %arg 成为了 %card 的使用者",
	["#shayi_temp"] = "杀意",
	["@shayi-use"] = "你可以发动“杀意”， 使用一张本阶段置入弃牌堆的【杀】",
	["~shayiuse"] = "选择目标->确定",


--***********************************	
	--15002 赫卡提亚·拉碧斯拉祖利 4hp
	["hecatia"] = "赫卡提亚" ,
	["!hecatia"] = "赫卡提亚·拉碧斯拉祖利" ,
	["#hecatia"] = "地狱的女神" ,
	["designer:hecatia"] = "辰焰天明",
	["santi"] = "三体",
	[":santi"] = "<font color=\"blue\"><b>锁定技，</b></font>你的非额外回合开始时，令你本回合依次执行摸牌/出牌/弃牌阶段各三次。你于回合内的第一/二/三个出牌阶段，只能使用基本牌/装备牌/锦囊牌。",
	["#santi"] = "三体",	
	
--***********************************		
	--15003 克劳恩皮斯 3hp
	["clownpiece"] = "克劳恩皮斯" ,
	["#clownpiece"] = "地狱的妖精" ,
	["designer:clownpiece"] = "三国有单",
	["kuangluan"] = "狂乱",
	["#kuangluan1"] = "狂乱",
	["#kuangluan2"] = "狂乱",
	["kuangluan1"] = "狂乱(明置手牌)",
	["kuangluan2"] = "狂乱(技能无效)",
    --[":kuangluan"] = "<font color=\"blue\"><b>锁定技，</b></font>一名角色于其摸牌阶段外获得牌后，若其没有明置手牌，其须明置这些牌。",
	[":kuangluan"] = "没有明置手牌的其他角色于其摸牌阶段外获得牌后，你可令其明置之。其他角色失去最后的明置手牌后，你可令其技能（永久技除外）失效，直到此回合结束。",
	["#kuangluan_invalidity"]= " %from的 所有技能 因“%arg”效果于此回合失效（永久技除外）。",
	
	["yuyi"] = "狱意",
	--[":yuyi"] = "出牌阶段开始时，你可指定所有有明置手牌的角色，这些角色各摸一张牌，然后这些角色的技能（永久技除外）于此回合内无效。其他角色出牌阶段开始时，若其没有明置手牌，你可令其摸一张牌，其技能（永久技除外）于此回合内无效。 ",
	[":yuyi"] = "你受到【杀】造成的伤害时，你可弃置你的一张牌，再弃置来源的一张牌，若两张牌类别不同，此伤害-1。",
	--[[["yuyi"] = "狱意",
	[":yuyi"] = "一名角色于其出牌阶段首次使用【杀】或普通锦囊牌指定目标后，你可以选择一项：明置一张手牌，令目标抵消此牌的抵消方式改为目标发起对使用者的拼点且获胜； 或明置其一张手牌。",
    ["yuyi:show"] = "明置使用人",
	["yuyi:changeProcess"] = "明置自己，改变抵消方式",
	["yuyiProcess"] = "狱意抵消",
	["yuyiProcess:cancel"]= "你可以以拼点的方式抵消  <font color=\"#FF8000\"><b>%src </b></font> 对你使用的【%dest】。",
	["#YuyiChanged"] = "%from 发动  %arg2 改变了目标 %to 对 【%arg】的抵消方式",
	["#YuyiCancel"] = "%to 以 %arg2 的拼点方式 抵消了 %to 对其使用的 【%arg】",
	["kuangluan"] = "狂乱",
    [":kuangluan"] = "与你拼点的角色需要选择拼点牌时，你可以代替其进行选择。一名角色因拼点而失去明置手牌后，你摸一张牌。 ",]]

--***********************************	
	--15004 稀神探女 4hp
	["sagume"] = "稀神探女" ,
	["#sagume"] = "招来口舌之祸的女神" ,
	["designer:sagume"] = "三国有单",
	["shehuo"] = "舌祸",
	[":shehuo"] = "<font color=\"blue\"><b>锁定技，</b></font>你指定或成为【杀】或普通锦囊牌的唯一目标时，若使用者和目标角色不是同一角色，后者选择一项： 此牌不能被响应； 或对前者使用一张类别不同的牌，取消原目标并令此技能于本回合内无效。",
    ["@shehuo_use"] = "舌祸: <font color=\"#00FF00\"><b>%src </b></font>对你使用了【%dest】, 你是否对<font color=\"#00FF00\"><b>%src </b></font>使用一张类别不同的牌，并取消你。",
	["#shehuo"] = "舌祸",
	["shenyan"] = "慎言",
	[":shenyan"] = "其他角色的弃牌阶段开始时，你可以视为使用【以逸待劳】（不在其攻击范围内的角色，本回合使用牌或成为过牌的目标的角色均不是此牌的合法目标）。",
	["@shenyan"] = "你可以发动“慎言”，视为使用【以逸待劳】。",
	["~shenyan"] = "选择目标->确定",


--***********************************	

--15005 哆来咪·苏伊特 3hp
	["doremy"] = "哆来咪·苏伊特" ,
	["#doremy"] = "梦之支配者" ,
	["&doremy"] = "哆来咪",
	["designer:doremy"] = "三国有单",
	["meimeng"] = "美梦",
	[":meimeng"] = "其他角色的牌因被其弃置而进入弃牌堆后，你可令其选择并获得其中一张牌并明置之，然后你可以获得其中一张并明置之。<font color=\"green\"><b>每阶段限一次，</b></font>",
	["emeng"] = "恶梦",
	[":emeng"] = "<font color=\"blue\"><b>锁定技，</b></font>你使用明置手牌指定有明置手牌的唯一目标后，你令其摸x张牌并翻面。（x为其明置手牌数且至多为3）",


--***********************************	
     --15006 铃瑚
	["ringo"] = "铃瑚" ,
	["#ringo"] = "橘色的月兔" ,
	["designer:ringo"] = "三国有单",
	["yuejian"] = "月见";
	[":yuejian"] = "你使用非转化的牌结算完毕后，若你为此牌目标，你可以与一名角色拼点。若你赢/没赢，你可视为使用”当【知己知彼】/【以逸待劳】。";
	["dango"] = "团子";
	["@yuejian-pindian"] = "你可以发动“月见”，和一名角色拼点";
	["@yuejian1"] = "你可以发动“月见”视为使用【知己知彼】";
	["@yuejian2"] = "你可以发动“月见”视为使用【以逸待劳】";
	["~yuejian"] = "选择目标，然后确定";

	
--***********************************	
	--15007 清兰 4hp
	["seiran"] = "清兰" ,
	["#seiran"] = "浅葱色的月兔",
	["designer:seiran"] = "辰焰天明",
	["yidan"] = "异弹",
	[":yidan"] = "你可以将与一名其他角色的一张明牌花色相同的一张手牌当【杀】对其使用（无距离和使用次数限制）;你不是其他角色使用与你的一张明牌花色相同的【杀】的合法目标。",
    ["#yidan"] = "异弹",
	
}
