--【死蝶】ai--1修改函数isPriorFriendOfSlash --2 function SmartAI:slashProhibit(card, enemy, from)--进而影响函数useCardSlash sgs.ai_skill_use.slash  --3 sgs.ai_card_intention.Slash 技能者本人的杀仇恨为0 死蝶杀的仇恨为0 --4 sgs.ai_skill_invoke.EightDiagram--different from sgs.ai_skill_playerchosen.zero_card_as_slashsgs.ai_skill_playerchosen.sidie = function(self, targets)	local sidie =self.player:getTag("sidie_target"):toPlayer()	local victim,sidieType = getSidieVictim(self, targets)	if victim then		return victim	else		if self:isFriend(sidie) then			for _,p in pairs(self.enemies) do				if sidie:canSlash(p,nil,false) then					return p				end			end		end		return nil	end			--[[local use_sidie=true	local target_table = self:getEnemies(self.player)	self:sort(target_table, "hp")	local bad_targets={}	local normal_targets={}	local good_targets={}	local slash = sgs.cloneCard("slash", sgs.Card_NoSuit, 0)    --没考虑闪的情况 默认命中后的效果			for _,p in pairs(target_table) do		if self:slashProhibit(slash, p, sidie) or not self:slashIsEffective(slash, p, sidie)		then 			table.insert(bad_targets,p)			continue		end		local dummy_damage=sgs.DamageStruct(slash, sidie, p, 1, sgs.DamageStruct_Normal)		local final_damage=self:touhouDamage(dummy_damage,sidie, p)		if final_damage.damage>0 then			if self:isWeak(p) then				table.insert(good_targets,p)			elseif final_damage.damage>1 then				table.insert(good_targets,p)			else				table.insert(normal_targets,p)			end		end	end			if #good_targets>0 then 		return good_targets[1]	elseif #normal_targets>0 then		return normal_targets[1]	end	--没有合适目标	if self:isFriend(sidie) then		if #bad_targets>0 then			return bad_targets[1]		end	end]]	return nilendsgs.ai_playerchosen_intention.sidie = 30function getSidieVictim(self, targets)	local sidie =self.player:getTag("sidie_target"):toPlayer()	local slash = sgs.cloneCard("slash")	local targetlist = sgs.QList2Table(targets)	local arrBestHp, canAvoidSlash, forbidden = {}, {}, {}	self:sort(targetlist, "defenseSlash")	for _, target in ipairs(targetlist) do		if self:isEnemy(target) and not self:slashProhibit(slash,target,sidie) and sgs.isGoodTarget(target, targetlist, self) then			if self:slashIsEffective(slash, target,sidie) then				if self:getDamagedEffects(target, sidie, true)  then					--or self:needLeiji(target, self.player)					table.insert(forbidden, target)				elseif self:needToLoseHp(target, sidie, true, true) then					table.insert(arrBestHp, target)				else					return target, "good" 				end			else				table.insert(canAvoidSlash, target)			end		end	end	--[[for i=#targetlist, 1, -1 do		local target = targetlist[i]		if not self:slashProhibit(slash, target,sidie) then			if self:slashIsEffective(slash, target,sidie) then				if self:isFriend(target) and (self:needToLoseHp(target, sidie, true, true)					or self:getDamagedEffects(target, sidie, true) ) then --or self:needLeiji(target, self.player)						--return target,"goodfriend"				end			else				table.insert(canAvoidSlash, target)			end		end	end]]	if #canAvoidSlash > 0 then return canAvoidSlash[1], "canAvoid" end	if #arrBestHp > 0 then return arrBestHp[1], "bestHp" end	self:sort(targetlist, "defenseSlash")	targetlist = sgs.reverse(targetlist)	for _, target in ipairs(targetlist) do		if target:objectName() ~= sidie:objectName()  and not self:isFriend(target) and not table.contains(forbidden, target) then			return target, "other"		end	end	return nilendfunction SmartAI:sidieEffect(from)	if from:hasSkill("sidie") and from:getPhase() ==sgs.Player_Play and not from:hasFlag("sidie_used") then 		return true	end	return falseend	sgs.ai_cardneed.sidie = function(to, card, self)	if not self:willSkipPlayPhase(to) then		return getCardsNum("Slash", to, self.player) <1		and card:isKindOf("Slash")	endendsgs.sidie_keep_value = {	Slash = 7}--【返魂】aisgs.ai_skill_playerchosen.wangxiang = function(self, targets)	if not self:invokeTouhouJudge() then return nil end	target_table =sgs.QList2Table(targets)	if #target_table==0 then return false end	for _,target in pairs(target_table) do			if  self:isFriend(target) then			return target		end	end	return nilendsgs.ai_playerchosen_intention.wangxiang = -80--【境界】aisgs.ai_skill_invoke.jingjie = truesgs.ai_skill_discard.jingjie = function(self)	local to_discard = {}	local cards = self.player:getHandcards()	cards = sgs.QList2Table(cards)	self:sortByKeepValue(cards)	table.insert(to_discard, cards[1]:getEffectiveId())	return to_discardend--【死生】ai--sgs.ai_choicemade_filter.cardChosen.sisheng = -50sgs.ai_skill_invoke.sisheng = function(self)	local who= self.room:getCurrentDyingPlayer()		if self:isFriend(who) then 		if getCardsNum("Peach", who, self.player) >= 1 or getCardsNum("Analeptic", who, self.player) >= 1 then			if self:hasWeiya(who)  then				return true			elseif who:getHandcardNum()>=3 then 				return true			end			return false		end		return true 	end	return false endsgs.ai_skill_askforag.sisheng = function(self, card_ids)	return card_ids[1]endsgs.ai_choicemade_filter.skillInvoke.sisheng = function(self, player, promptlist)	local who= player:getRoom():getCurrentDyingPlayer()		if promptlist[#promptlist] == "yes" then			sgs.updateIntention(player, who, -70)		--else  --明桃不救的情况暂时不好排除 		--	if player:getPile("jingjie"):length()>=2 then		--		sgs.updateIntention(player, who, 60)		--	end		endendsgs.ai_skill_cardchosen.sisheng = function(self, who, flags)	if flags == "he" then		local who= self.player:getRoom():getCurrentDyingPlayer()		if who:getEquip(1) and who:getEquip(1):isKindOf("SilverLion") then			return who:getEquip(1)		end		local hcards = who:getCards("h")		if hcards:length()>0 then			return hcards:first()		end		local ecards = who:getCards("e")		if ecards:length()>0 then			return ecards:first()		end	endend--【静动】aisgs.ai_skill_invoke.jingdong = function(self)	local target=self.player:getTag("jingdong_target"):toPlayer()	if  target:hasSkill("huanmeng") or  target:hasSkill("zaozu") or target:hasSkill("yongheng")then		return false 	end	num=target:getHandcardNum()-target:getMaxCards()	if num==0 then return false end	cards=self.player:getPile("jingjie")	if cards:isEmpty() then return false end	if not self:isFriend(target) then		return false	else		if self:isWeak(target) and num>0 then			return true		end		if num>1   then			if cards:length()>3 then				return true			else				if num>3 then					return true				end			end		end	end	return falseendsgs.ai_skill_askforag.jingdong = function(self, card_ids)	--考虑神隐八云紫作为敌友的情况下，还要给牌排序。	return card_ids[1]endsgs.ai_choicemade_filter.skillInvoke.jingdong = function(self, player, promptlist)		local to=self.room:getCurrent()	if not (to:hasSkill("huanmeng") or  to:hasSkill("zaozu") or to:hasSkill("yongheng"))then	num=to:getHandcardNum()-to:getMaxCards()	if promptlist[#promptlist] == "yes" then			sgs.updateIntention(player, to, -60)	else		if num>=3 then			sgs.updateIntention(player, to, 30)		end	end	endend--【照料】aisgs.ai_skill_cardask["@zhaoliao"] = function(self, data)	local a=data:toDamage().to	if not self:isFriend(a) then return "." end	--目前只尝试给出一张	--给出装备优先。。。	local ecards=self.player:getCards("e")	if ecards:length()>0 then return "$" .. ecards:first():getId() end	local cards = self.player:getCards("h")	cards = sgs.QList2Table(cards)	if #cards==0 then return "." end	self:sortByUseValue(cards)	return "$" .. cards[1]:getId()endsgs.ai_skill_choice.zhaoliao=function(self)	if self.player:isKongcheng() then return "zhaoliao1" end	return "zhaoliao2"endsgs.ai_choicemade_filter.cardResponded["@zhaoliao"] = function(self, player, promptlist)	if promptlist[#promptlist] ~= "_nil_" then		local target =player:getTag("zhaoliao_target"):toPlayer()		if not target then return end			sgs.updateIntention(player, target, -80)	endend--[[sgs.ai_choicemade_filter.skillChoice.zhaoliao = function(self, player, promptlist)	local choice = promptlist[#promptlist]	local target =player:getTag("zhaoliao_target"):toPlayer()	if not target then return end		if choice== "zhaoliao1" then  		sgs.updateIntention(player, target, -40)	else		sgs.updateIntention(player, target, -80)	endend]]--无中生有 askForExchange使用默认ai--【狡黠】aisgs.ai_skill_invoke.jiaoxia = function(self)	return self:invokeTouhouJudge()endsgs.ai_need_bear.jiaoxia = function(self, card,from,tos) 	from =from or self.player	if not card:isKindOf("EquipCard") then return false end	if (from:getHp()==1 and from:getCards("e"):length()==0) then		return true	end	if not self:isWeak(from) then		if card:isKindOf("Armor") and not from:getArmor() then			return false		elseif  card:isKindOf("DefensiveHorse") and not from:getDefensiveHorse() then			return false		end		end	if self:getOverflow(from) <= 0 then		return true	end	return falseend--【剑术】ai sgs.ai_cardneed.jianshu = function(to, card, self)	if not to:getOffensiveHorse() and getCardsNum("OffensiveHorse", to, self.player) < 1 then		return  card:isKindOf("OffensiveHorse")	endend--【楼观】ai 锁定技不需要--【白楼】ai--part1：是否发动白楼sgs.ai_skill_invoke.bailou =function(self,data)	--对敌人默认发动	local target=data:toPlayer()	if self:isEnemy(target) then		return true	endendsgs.ai_cardneed.bailou = function(to, card, self)	if not self:willSkipPlayPhase(to) then		return card:isKindOf("Slash") and card:isRed()	endend--【协奏】aisgs.ai_skill_use["@@xiezou"] = function(self, prompt)	local dummy_use = { isDummy = true, to = sgs.SPlayerList() }	local cardname=self.player:property("xiezou_card"):toString()	local card=sgs.cloneCard(cardname, sgs.Card_NoSuit, 0)	card:setSkillName("xiezou")	local target	if card:isKindOf("TrickCard") then		self:useTrickCard(card, dummy_use)	else		self:useBasicCard(card, dummy_use)	end		if not dummy_use.card then return false end	if dummy_use.to:isEmpty() then --targetfix的？		if card:isKindOf("IronChain") then			return "."		end		return dummy_use.card:toString()	else		local target_objectname = {}		if card:isKindOf("IronChain") then			for _, p in sgs.qlist(dummy_use.to) do				if (self:isEnemy(p) and not p:isChained()) 				or (self:isFriend(p) and p:isChained())then					table.insert(target_objectname, p:objectName())				end				if #target_objectname==2 then break end			end		else			for _, p in sgs.qlist(dummy_use.to) do				if self:isEnemy(p) then					table.insert(target_objectname, p:objectName())					target=p					break				end			end		end				if card:isKindOf("Collateral") then			local victim			for _,p in sgs.qlist(self.room:getOtherPlayers(target))do				if self:isEnemy(p) and target:canSlash(p,nil,true) then					table.insert(target_objectname, p:objectName())					victim=p					break				end			end			if not victim then				return "."			end					end		if #target_objectname>0 then			return dummy_use.card:toString() .. "->" .. table.concat(target_objectname, "+")		end	end	return "."end--【和声】ai --part1：禁止对和声出杀sgs.ai_slash_prohibit.hesheng = function(self, from, to, card)	--part1.1 基本条件检测，为队友可以杀（比如借刀）	local prevent=false	if not to:hasSkill("hesheng") then return false end	if self:isFriend(from,to) then return false end	--part1.2 检测和声条件	for _,p in sgs.qlist(self.room:getAlivePlayers())do		if (p:getCards("j"):length()>0 ) then			prevent=true			break		end	end	--part1.3 达成和声条件时，检测自身是否有优先于取消伤害的技能（如千年紫）	if prevent then		local damage=sgs.DamageStruct(card, from, to,self:touhouDamageNature(card,from,to))		local slash_effect=self:touhouDamageEffect(damage,from,to)		if slash_effect then return false end	end	return preventend--part2:伤害相关估值 关于和声sgs.ai_damageInflicted.hesheng =function(self, damage)	local can =false	for _,p in sgs.qlist(self.room:getAlivePlayers()) do		if p:getCards("j"):length()>0 then			can=true			break		end	end	if can then		damage.damage=0	end	return damageend--【人偶】ai --默认发动sgs.ai_skill_invoke.renou = true--【军势】ai sgs.ai_view_as.junshi = function(card, player, card_place)	local pattern=sgs.Sanguosha:getCurrentCardUsePattern()	if not card:isKindOf("EquipCard") then return false end	local suit = card:getSuitString()	local number = card:getNumberString()	local card_id = card:getEffectiveId()	if card_place ~= sgs.Player_PlaceSpecial then		if pattern=="jink" then			return ("jink:junshi[%s:%s]=%d"):format(suit, number, card_id)		elseif pattern=="slash" then			return ("slash:junshi[%s:%s]=%d"):format(suit, number, card_id)		end	endendlocal junshi_skill = {}junshi_skill.name = "junshi"table.insert(sgs.ai_skills, junshi_skill)junshi_skill.getTurnUseCard = function(self, inclusive)        if not sgs.Slash_IsAvailable(self.player)  then return false end		local ecards={}		local cards=self.player:getCards("he")		cards=self:touhouAppendExpandPileToList(self.player,cards)		for _,c in sgs.qlist(cards) do			if c:isKindOf("EquipCard") then				table.insert(ecards,c)			end		end        if #ecards==0 then return false end		self:sortByUseValue(ecards, true)        local suit = ecards[1]:getSuitString()        local number = ecards[1]:getNumberString()        local card_id = ecards[1]:getEffectiveId()        local slash_str = ("slash:junshi[%s:%s]=%d"):format(suit, number, card_id)        local slash = sgs.Card_Parse(slash_str)                        assert(slash)        return slash	endsgs.ai_cardneed.junshi = function(to, card, self)	return card:isKindOf("EquipCard")end--【式神】aisgs.ai_skill_choice.shishen=function(self)	if self.player:getPhase() == sgs.Player_Start then		return "shishen1"	end	if self.player:hasFlag("shishen_choice") then--受伤的情况		ran = self.room:findPlayerBySkillName("zhaoliao")		if ran and ran:getCards("he")>=2 and self:isFriend(ran) then			return "cancel"		else			return "shishen1"		end	end	if self.player:getPhase() == sgs.Player_Play  and self.player:getMark("@shi")==0    then		for _,card in sgs.qlist(self.player:getCards("h")) do			if card:isNDTrick() and not card:isKindOf("Nullification") then				return "shishen2"			end		end	end	return "cancel"end--【野性】 aisgs.ai_slash_prohibit.yexing = function(self, from, to, card)	if to:hasSkill("yexing") and to:getMark("@shi") ==0 then		if card:isKindOf("NatureSlash")  then 			return false		else			if from:hasSkill("here")then				return false			end		end	end	return trueend--【妖术】ai   --问题不少 --目前只会用，不会考虑情况再用。。。--操作上仅仅区分了借刀 和铁锁sgs.ai_skill_use["@@yaoshu"] = function(self, prompt)		local dummy_use = { isDummy = true, to = sgs.SPlayerList() }	local cardname=self.player:property("yaoshu_card"):toString()	local card=sgs.cloneCard(cardname, sgs.Card_NoSuit, 0)	card:setSkillName("yaoshu")	local target		self:useTrickCard(card, dummy_use)	if not dummy_use.card then return false end		if dummy_use.to:isEmpty() then		if card:isKindOf("IronChain") then			return "."		end		return dummy_use.card:toString()	else		local target_objectname = {}		if card:isKindOf("IronChain") then			for _, p in sgs.qlist(dummy_use.to) do				if (self:isEnemy(p) and not p:isChained()) 				or (self:isFriend(p) and p:isChained())then					table.insert(target_objectname, p:objectName())				end				if #target_objectname==2 then break end			end		else			for _, p in sgs.qlist(dummy_use.to) do				if self:isEnemy(p) then					table.insert(target_objectname, p:objectName())					target=p					break				end			end		end				if card:isKindOf("Collateral") then			local victim			for _,p in sgs.qlist(self.room:getOtherPlayers(target))do				if self:isEnemy(p) and target:canSlash(p,nil,true) then					table.insert(target_objectname, p:objectName())					victim=p					break				end			end			if not victim then				return "."			end					end		if #target_objectname>0 then			return dummy_use.card:toString() .. "->" .. table.concat(target_objectname, "+")		end	end	return "."endsgs.ai_cardneed.yaoshu = function(to, card, self)	return card:isNDTrick()end--【记忆】ai  sgs.ai_skill_invoke.jiyi = true--sgs.ai_skill_askforyiji.jiyi = sgs.ai_skill_askforyiji.yiji--几乎照搬秘计sgs.ai_skill_askforyiji.jiyi = function(self, card_ids)	local available_friends = {}	if #self.friends_noself==0 then return nil, -1 end	for _, friend in ipairs(self.friends_noself) do		--if not friend:hasSkill("manjuan") and not self:isLihunTarget(friend) then 		table.insert(available_friends, friend) 		--end	end    if self.player:getHandcardNum()<=2 then  return nil, -1 end	local toGive, allcards = {}, {}	local keep	for _, id in ipairs(card_ids) do		local card = sgs.Sanguosha:getCard(id)		if not keep and (isCard("Jink", card, self.player) or isCard("Analeptic", card, self.player)) then			keep = true		else			table.insert(toGive, card)		end		table.insert(allcards, card)	end	local cards = #toGive > 0 and toGive or allcards	self:sortByKeepValue(cards, true)	local id = cards[1]:getId()	local card, friend = self:getCardNeedPlayer(cards)	if card and friend and table.contains(available_friends, friend) then return friend, card:getId() end	if #available_friends > 0 then		self:sort(available_friends, "handcard")		for _, afriend in ipairs(available_friends) do			if not self:needKongcheng(afriend, true) then				return afriend, id			end		end		self:sort(available_friends, "defense")		return available_friends[1], id	end	return nil, -1endsgs.ai_Yiji_intention.jiyi=-30--【春眠】ai锁定技 不需要--【报春】ai--part1：报春对象的选择sgs.ai_skill_playerchosen.baochun = function(self, targets)	--[[target_table =sgs.QList2Table(targets)	if #target_table==0 then return false end	self:sort(target_table, "handcard")	for _,target in pairs(target_table) do			if  self:isFriend(target) then			return target			--break		end	end]]	--寻找一个合适的摸排对象	local target =self:touhouFindPlayerToDraw(true, self.player:getLostHp())	if target then return target end	if #self.friends>0 then return self.friends[1] end	return nilend--part2：发动报春选择角色，对该角色的仇恨sgs.ai_playerchosen_intention.baochun = -80--part3：报春需要卖血 --包含1自己由伤害机会主动卖--2 队友主动杀sgs.ai_need_damaged.baochun = function(self, attacker, player)	--卖血条件：体力值大于1，且能补3张以上	local x= player:getLostHp()+1 	if x>=3 and player:getHp()>1 then		return true	end	return falseend--【春意】ai  锁定技 不需要--【战操】ai--part1：发动战操的判断sgs.ai_skill_invoke.zhancao = function(self,data)	--part1.1:基本判断	--特别是 该杀已经无效，或友方的【理智】出杀时，不用战操	local use=self.player:getTag("zhancao_carduse"):toCardUse()	local target =self.player:getTag("zhancao_target"):toPlayer()	if not self:isFriend(target) then return false end	if self:touhouCardEffectNullify(use.card,target) then return false end --此杀已经无效	if self:isFriend(use.from) and use.from:hasSkills("shenyin|lizhi") then return false end		--part1.2:检测装备	local hasEquip=false	cards =self.player:getCards("he") 	for _,card in sgs.qlist(cards) do		if card:isKindOf("EquipCard") then			hasEquip=true			break		end	end	--part1.3:目标虚弱时，只要自己不虚弱或者有装备，无条件发动	if self:isWeak(target)		and ((not self:isWeak(self.player)) or hasEquip) then		return true	end	--part1.4:检测闪的明牌情况	if getCardsNum("Jink", target, self.player) < 1 or sgs.card_lack[target:objectName()]["Jink"] >0 then		--自己是目标时，果断发动		if target:objectName()==self.player:objectName() then			return true		else			--其他人是目标时，考虑酒杀一类高伤害和铁锁伤害			if  (hasEquip or self.player:getHp()>1) then				if self:hasHeavySlashDamage(use.from, use.card, target) then					return true				end				if target:isChained() and (use.card:isKindOf("NatureSlash") ) then					for _,p in pairs (self.friends) do						if self:isWeak(p) and p:isChained() then							return true						end					end				end			end		end	end	return falseend--part2：发动战操弃置装备的执行sgs.ai_skill_cardask["@zhancao-discard"] = function(self, data)		cards =self.player:getCards("he") 	cards=sgs.QList2Table(cards)	ecards={}	for _,card in pairs(cards) do		if card:isKindOf("EquipCard") then			table.insert(ecards,card)		end	end		if #ecards==0 then return "." end	self:sortByCardNeed(ecards)	return "$" .. ecards[1]:getId()end	--part3：发动战操后的仇恨sgs.ai_choicemade_filter.skillInvoke.zhancao = function(self, player, promptlist)	--获得战操保护的角色	local target =player:getTag("zhancao_target"):toPlayer()	--更新对该角色的仇恨	if target then		if promptlist[#promptlist] == "yes" then			sgs.updateIntention(player, target, -50)		end		endendsgs.ai_cardneed.zhancao = function(to, card, self)	return card:isKindOf("EquipCard")endsgs.zhaocao_keep_value = { 	EquipCard = 7}sgs.ai_need_bear.zhaocao = function(self, card,from,tos) 	from =from or self.player	if not card:isKindOf("EquipCard") then return false end	if self:getSameEquip(card,from) then		if card:isKindOf("Weapon") then			if self:getOverflow(from) >= 0 then				local old_range=sgs.weapon_range[from:getWeapon():getClassName()] or 0				local new_range = sgs.weapon_range[card:getClassName()] or 0				if new_range<=old_range then					return true				end			end		else			return true		end	end	return falseend--【魔操】ailocal mocao_skill = {}mocao_skill.name = "mocao"table.insert(sgs.ai_skills, mocao_skill)mocao_skill.getTurnUseCard = function(self)	--if self.player:hasUsed("#mocao") then return nil end	if self.player:hasUsed("mocaoCard") then return nil end	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do		if  p:getCards("e"):length()>0 then			t=true			break		end	end	if t then	--return sgs.Card_Parse("#mocao:.:")		return sgs.Card_Parse("@mocaoCard=.")	end	return nilendsgs.ai_skill_use_func.mocaoCard = function(card, use, self)--sgs.ai_skill_use_func["#mocao"]=function(card,use,self)	local targets={}	for _,p in sgs.qlist(self.room:getOtherPlayers(self.player)) do		if  p:getCards("e"):length()>0 then			table.insert(targets,p)		end	end		self:sort(targets,"value")	for _, p in ipairs(targets) do        if (p:getLostHp()<2 and self:isEnemy(p)) or (p:getLostHp()>1 and self:isFriend(p))  then			use.card = card			if use.to then				use.to:append(p)				if use.to:length() >= 1 then return end			end		end    endendsgs.ai_use_value.mocaoCard = 9sgs.ai_use_priority.mocaoCard = 6--sgs.ai_card_intention.mocao = function(self, card, from, tos)sgs.ai_card_intention.mocaoCard = function(self, card, from, tos)	if tos[1]:getLostHp()>=2  then		sgs.updateIntention(from, tos[1], -30)	else		sgs.updateIntention(from, tos[1], 30)	endend--【神隐】ai--function SmartAI:getDamagedEffects(to, from, slash)--绝情  旧马岱 寒冰的效果判断都在这里  返回bool值sgs.ai_skill_invoke.shenyin = function(self,data)	local target=data:toPlayer()	if  self:isEnemy(target) then		if not self:canAttack(target) then			return true		end		if self:isWeak(target) then			return false		end		return true	end	if  self:isFriend(target) then		return true	endend--【隙间】aisgs.ai_skill_playerchosen.xijian = function(self, targets)	local target_willobtain = self.player:getTag("xijian_target"):toPlayer()	if not self:isFriend(target_willobtain) then return  nil end	if self:isFriend(target_willobtain) then		target_table =sgs.QList2Table(targets)		local xijian_target=target_table[1]		sgs.updateIntention(self.player, target_willobtain, -50)		return xijian_target	end	return nil	--其实是没想选择人的策略	--for _,target in pairs(target_table) do		--	if  self:isEnemy(target) and  then	--		return target	--	end	--endendsgs.ai_skill_askforag.xijian = function(self, card_ids)	local cards={}	local target = self.player:getTag("xijian_target"):toPlayer()	--为什么是table?	for _,card_id in pairs(card_ids) do		local card=sgs.Sanguosha:getCard(card_id)		table.insert(cards,card)	end	self:sortByCardNeed(cards, true)	if self:isFriend(target)  then		return cards[1]:getId()	endendfunction SmartAI:enemyXijian()	local yukari=self.room:findPlayerBySkillName("xijian")	if yukari and not self:isFriend(yukari) then		return true	end	return falseend--【幽曲】aisgs.ai_skill_choice.youqu=function(self)	local yukari=self.player:getRoom():findPlayerBySkillName("xijian")		if yukari then		if self:isFriend(yukari) then			return "siling3"		else			return "siling1"		end	end	local peach_num = self:getCardsNum("Peach")	if not self.player:isWounded() and peach_num>0  then		return "siling3"	end	if self.player:isLord() and sgs.current_mode_players["rebel"]==0 then		return "siling1"	end	local enemy_num =0	if self.player:isLord() or self.player:getRole() == "loyalist" then		enemy_num = sgs.current_mode_players["rebel"]	elseif self.player:getRole() == "rebel" then		enemy_num = sgs.current_mode_players["loyalist"]	else		enemy_num = #self.enemies	end	local cards=self.player:getPile("siling")	local good_count = 1 	for i=1, 3, 1 do		if cards:length()+i >= enemy_num then			good_count = i			break		end	end	if good_count == 1 then		return "siling1"	elseif good_count == 2 then		return "siling2"	elseif good_count == 3 then		return "siling3"	end	return "siling1"endsgs.ai_choicemade_filter.skillChoice.youqu = function(self, player, promptlist)	sgs.siling_lack[player:objectName()]["Red"] = 0	sgs.siling_lack[player:objectName()]["Black"] = 0end--【亡舞】aisgs.ai_skill_invoke.wangwu = function(self,data)	local target=self.player:getTag("wangwu_use"):toCardUse().from	if not target then return false end	--还应该考虑卖血？	if self:isEnemy(target) then		return true	end	return falseendsgs.ai_skill_askforag.wangwu = function(self, card_ids)	local wangwu_card=self.player:getTag("wangwu_card"):toCard()	local cards={}	for _,card_id in pairs(card_ids) do		local card=sgs.Sanguosha:getCard(card_id)		table.insert(cards,card)	end	self:sortByCardNeed(cards)		for _,card in pairs(cards) do        if card:sameColorWith(wangwu_card) then            return cards[1]:getId()        end    endendsgs.ai_choicemade_filter.skillInvoke.wangwu = function(self, player, promptlist)	local use = player:getTag("wangwu_use"):toCardUse()		if use.card and (use.card:isRed() or  use.card:isBlack()) then		local str 		if use.card:isRed() then			str = "Red"		end		if use.card:isBlack() then			str = "Black"		end		if use.from and self:isEnemy(use.from,player) then			if promptlist[#promptlist] == "yes" then				local findSame =false				for _,id in sgs.qlist(player:getPile("siling")) do					if sgs.Sanguosha:getCard(id):sameColorWith(use.card) then						findSame =true						break					end				end				if not findSame then					sgs.siling_lack[player:objectName()][str] = 1				end			else				sgs.siling_lack[player:objectName()][str] = 1			end			end	endendsgs.ai_slash_prohibit.wangwu = function(self, from, to, card)	if self:isFriend(from,to) then		return false	end	if to:getPile("siling"):length()==0 then return false end	if card:isRed() then		if sgs.siling_lack[to:objectName()]["Red"] > 0 then			return false		end	elseif card:isBlack() then		if sgs.siling_lack[to:objectName()]["Black"] > 0 then			return false		end	else		return false	end	local fakeDamage=sgs.DamageStruct()	fakeDamage.nature= sgs.DamageStruct_Normal	fakeDamage.damage=1	fakeDamage.from=to	fakeDamage.to=from	local damageEffect = self:touhouNeedAvoidAttack(fakeDamage,to,from) 	return damageEffectendsgs.ai_trick_prohibit.wangwu = function(self, from, to, card)	if self:isFriend(from,to) then		return false	end	if to:getPile("siling"):length()==0 then return false end	if card:isRed() then		if sgs.siling_lack[to:objectName()]["Red"] > 0 then			return false		end	elseif card:isBlack() then		if sgs.siling_lack[to:objectName()]["Black"] > 0 then			return false		end	else		return false	end		local fakeDamage=sgs.DamageStruct()	fakeDamage.nature= sgs.DamageStruct_Normal	fakeDamage.damage=1	fakeDamage.from=to	fakeDamage.to=from	local damageEffect = self:touhouNeedAvoidAttack(fakeDamage,to,from) 	return damageEffectend--【死欲】aisgs.ai_slash_prohibit.hpymsiyu = function(self, from, to, card)	if self:isFriend(from,to) then		return false	end	local callback=sgs.ai_damage_prohibit["hpymsiyu"]	return callback(self, from, to, card)endsgs.ai_trick_prohibit.hpymsiyu = function(self, from, to, card)	if self:isFriend(from,to) then return false end	local isDamage=false	if ( card:isKindOf("Duel") or card:isKindOf("AOE") or card:isKindOf("FireAttack") 			or sgs.dynamic_value.damage_card[card:getClassName()]) then		isDamage=true	end	if isDamage then		local callback=sgs.ai_damage_prohibit["hpymsiyu"]		return callback(self, from, to, card)	end	return falseendsgs.ai_damage_prohibit.hpymsiyu = function(self, from, to, card)	if not to:hasSkills("hpymsiyu+juhe") then return false end	if to:getPhase() ~=sgs.Player_NotActive then return false end	if self:isFriend(from,to) then return false end	local fakeDamage=sgs.DamageStruct()	fakeDamage.card=card	fakeDamage.nature= self:touhouDamageNature(card,from,to)	fakeDamage.damage=1	fakeDamage.from=from	fakeDamage.to=to	if self:touhouDamage(fakeDamage,from,to).damage < to:getHp() then 		return false	end	if to:containsTrick("indulgence") or to:containsTrick("supply_shortage") then		return true	end	if not to:faceUp() then		return true	end	local recoverNum = self:getAllPeachNum(to) + getCardsNum("Analeptic", to, self.player)	return recoverNum>0 end--【居合】ai--只考虑了妖梦本身，如果双将或者凭依的话，还要考虑血量sgs.ai_skill_invoke.juhe = true--sgs.ai_skill_discard.juhe = sgs.ai_skill_discard.gamerule--死欲+居合 应该保持攻击性 需要随时保留距离-- sgs.ai_skill_discard.gamerule 不符合要求sgs.ai_skill_discard.juhe = function(self,discard_num)	local Weapon=self.player:getWeapon()  	local Distance=self.player:getOffensiveHorse()	if  Distance and Weapon then		local gamerule = sgs.QList2Table(self.player:getCards("h"))		self:sortByKeepValue(gamerule,false)		local gamerule_discard={}		for var=1, discard_num ,1 do			table.insert(gamerule_discard,gamerule[var]:getId())		end		return gamerule_discard	end	local cards = {}	local tmp_dis	local weapons={}	for _,c in sgs.qlist(self.player:getCards("h")) do		if not Distance and c:isKindOf("OffensiveHorse") and not tmp_dis then			tmp_dis=c		elseif not Weapon and c:isKindOf("Weapon") then				table.insert(weapons,c)				 --local equip = tmp_weapon:getRealCard():toWeapon()				 --local equip1 = c:getRealCard():toWeapon()				 --if equip1:getRange()>tmp_weapon:getRange()		else			table.insert(cards,c)		end	end	if #weapons>1 then		self:sortByUseValue(weapons,false)		for var=2, #weapons ,1 do			table.insert(cards,weapons[var])		end		--table.removeOne(cards, weapons[1])	end		self:sortByKeepValue(cards)	local to_discard = {}	for var=1, discard_num ,1 do		table.insert(to_discard,cards[var]:getId())	end	return to_discardendfunction SmartAI:touhouBreakDamage(damage,to)	if to:hasSkill("hpymsiyu") and to:getPhase()==sgs.Player_NotActive then		if to:getHp()>0   then			return damage.damage>= to:getHp()		else			return damage.damage>0		end	end	return falseend--嘲讽值设定--[[sgs.ai_chaofeng.yym001 = 0sgs.ai_chaofeng.yym002 = 1sgs.ai_chaofeng.yym003 = 3sgs.ai_chaofeng.yym004 = 1sgs.ai_chaofeng.yym005 = 0sgs.ai_chaofeng.yym006 = 0sgs.ai_chaofeng.yym007 = 1sgs.ai_chaofeng.yym008 = 1sgs.ai_chaofeng.yym009 = 3sgs.ai_chaofeng.yym010 = 1sgs.ai_chaofeng.yym011 = 0sgs.ai_chaofeng.yym012 = -1sgs.ai_chaofeng.yym013 = -2]]