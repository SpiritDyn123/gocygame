-- key:1~8 regin_key; 9:info_key
-- arg:1 userId; 2:begin; 3;end
math.randomseed(redis.call('TIME')[2])

local regionId = redis.call('hget', KEYS[9], ARGV[1])
local inArena = true
if not regionId then
	inArena = false
	regionId = 1
else 
	regionId = tonumber(regionId)
end
local score = redis.call('zscore', KEYS[regionId], ARGV[1]) or 0
local rank = redis.call('zrevrank', KEYS[regionId], ARGV[1]) or 0

local region = {
	200,
	500,
	800,
	1200,
	1600,
	2000,
	2500,
	3000,
}
local begin = tonumber(score) + ARGV[2]
local send = tonumber(score) + ARGV[3]
local selectCnt = tonumber(ARGV[4])
local inc = 5

local rMini = 1
local rMax = #region

local RBase = 1
local RCnt = 2
local RDIR = 3
local RRegion = 4
local RUseCnt = 5
local RUseList = 6

local testCnt = 0
local cntMini = 5
local testRegion = {}
testRegion[#testRegion+1] = regionId
for i = rMini, rMax do
	if regionId + i <= rMax then
		testRegion[#testRegion+1] = regionId + i
	end
	if regionId - i >= rMini then
		testRegion[#testRegion+1] = regionId - i
	end
end
local maxRange = 50
local idx = 0
while true do
	local rbegin = begin
	if rbegin < 0 then
		rbegin = 0
	end
	local rend = send
	-- if rend >= region[#region] then
		-- rend = 2147483647
	-- end
	local acnt = {}
	local total = 0
	--local iat = {}
	for k,i in ipairs(testRegion) do
		local v = region[i]
		local vEnd = v
		if i == rMax then
			vEnd = rend
		end
		if rbegin < vEnd then
			if rend < v then
				vEnd = rend
			end
			if regionId == i then
				if testCnt == 0 then
					local cnt = redis.call('zcount', KEYS[i], '-inf', '+inf')
					if cnt < cntMini then
						rbegin = 0
						vEnd = '+inf'
					end
				end
				local cnt = redis.call('zcount', KEYS[i], rbegin, '(' .. score)
				if cnt > 0 then
					acnt[#acnt + 1] = { [RCnt] = cnt, [RBase] = rank, [RDIR] = 1, [RRegion] = i}
					total = total + cnt
				end 
				cnt = redis.call('zcount', KEYS[i], '(' .. score, vEnd)
				if cnt > 0 then
					acnt[#acnt + 1] = { [RCnt] = cnt, [RBase] = rank, [RDIR] = -1, [RRegion] = i}
					total = total + cnt
				end
				--iat[#iat+1] = string.format("in %d %d %d %d",i, cnt, rbegin, vEnd)
			else
				local cnt = redis.call('zcount', KEYS[i], rbegin, vEnd)
				if cnt > 0 then
					if i > regionId  then
						acnt[#acnt + 1] = { [RCnt] = cnt, [RBase] = 0, [RDIR] = -1, [RRegion] = i}
					else
						acnt[#acnt + 1] = { [RCnt] = cnt, [RBase] = -1, [RDIR] = 1, [RRegion] = i}
					end
					total = total + cnt
				end
				--iat[#iat+1] = string.format("out %d %d %d %d",i, cnt, rbegin, vEnd)
			end
		end
		if rend < v then
			break
		end
	end
	local ch = 0
	if begin > 0 then
		ch = 1
	end
	if send < region[rMax] then
		ch = 1
	end
	idx = idx + 1
	--local selectCnt = 3
	if total >= selectCnt or (ch == 0 and total > 0) or idx > maxRange then
		local sCnt = total >= selectCnt and selectCnt or total
		local selected = {}
		for i = 1,sCnt do
			local rand = math.random(1, total)
			local test = 0
			for k,v in pairs(acnt) do
				local uCnt = v[RUseCnt] or 0
				test = test + v[RCnt] - uCnt
				if rand <= test then
					-- use this region
					local rId = rand - ( test - v[RCnt] + uCnt)
					local uList = v[RUseList] or {}
					v[RUseList] = uList
					table.sort(uList)
					for _,k in ipairs(uList) do
						if rId >= k then
							rId = rId + 1
						end
					end
					uList[#uList+1] = rId
					local tarRank = v[RBase] + v[RDIR] * rId
					local items = redis.call('zrevrange', KEYS[v[RRegion]], tarRank, tarRank)
					if #items == 0 then
						selected[#selected + 1] = 0
					else
						selected[#selected + 1] = tonumber(items[1])
					end
					v[RUseCnt] = uCnt + 1
					total = total - 1
					break
				end
			end
		end
		return selected
	end
	if ch == 0 then
		return {}
	end
	
	begin = begin - inc * idx
	send = send + inc * idx
	if begin < 0 then
		begin = 0
	end
	testCnt = testCnt + 1
end
