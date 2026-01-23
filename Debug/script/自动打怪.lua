-- 全局 lua_interface 对象 game
local game=_G.game
-- 打印分隔线
local function printSeparator()
    print("--------------------------------------------------")
end

-- 回城
local function goHome()
    if luaStopFlag then 
     return
    end
    printSeparator()
    print("回城整理背包...")
    game:使用物品("永久回城神石")
    sleep(2000);
	if (game:当前地图名()=="中州") and (game:计算距离(468, 226)<20) then
		game:回城整理背包()
	else
		goHome()
	end
end

-- 存仓库
local function storeGoods()
    local maxAttempts = 5
    local attempt = 1

    while attempt <= maxAttempts do
        print("正在存仓库... (尝试次数: " .. attempt .. ")")

        if game:存仓库() then
            print("存仓库成功")
            game:回城整理背包()

            -- 检查是否有剩余物品需要存储
            local remainingStoreCount = game:待存物品数量()
            if remainingStoreCount == 0 then
                print("所有物品已成功存入仓库")
                break
            else
                print("仍有 " .. remainingStoreCount .. " 件物品需要存储")
            end
        else
            print("存仓库失败")
        end

        attempt = attempt + 1
        if attempt > maxAttempts then
            print("已达到最大尝试次数 (" .. maxAttempts .. ")，仍未能成功存入仓库")
            break
        end
    end
end

-- 卖物品的通用函数
local function sellItems(itemType, maxAttempts, npcName, npcLocation)
    local attempt = 1
    while attempt <= maxAttempts do
        print("正在卖" .. itemType .. "... (尝试次数: " .. attempt .. ")")
        if game["卖" .. itemType]() then
            print("卖" .. itemType .. "成功")
            break
        else
            print("卖" .. itemType .. "失败")
        end
        attempt = attempt + 1
        if attempt > maxAttempts then
            print("已达到最大尝试次数 (" .. maxAttempts .. ")，仍未能成功卖出" .. itemType)
            break
        end
    end
end

-- 卖药
local function sellMedicine()
    print("正在卖药...")
    if game:卖药() then
	    sleep(1000)
		game:卖药()
		sleep(1000)
		game:卖药()
		sleep(1000)
		game:卖药()
        print("卖药成功")
    else
        print("卖药失败")
    end
end

-- 购买药品
local function buyMedicines(name, count)
    local maxAttempts = 5
    local attempt = 1

    -- 买药
    local bagCount = game:计算物品数量(name)
    if bagCount < count then
        while attempt <= maxAttempts do
            print(name.."数量不足，正在购买,尝试次数: ("..attempt..")")

            -- 计算需要购买的数量
            local needToBuy = count - bagCount
            if game:买药(name, needToBuy) then
                print("购买"..name.."成功")
                bagCount = game:计算物品数量(name)  -- 更新当前数量
                if bagCount >= count then
                    print(name.."数量已满足需求: " .. count)
                    break  -- 成功后退出循环
                end
            else
                print("买药失败")
            end

            attempt = attempt + 1
            if attempt > maxAttempts then
                print("已达到最大尝试次数 (" .. maxAttempts .. ")，仍未能购买足够的药")
                break
            end
        end
    else
        print(name.."数量充足: " .. count)
    end

end

-- 移动到目标点（异步检测）
local function moveToTargetAsync(x, y)
    printSeparator()
    print("正在移动到目标点 (" .. x .. ", " .. y .. ")...")

    -- 计算初始距离
    local distance = game:计算距离(x, y)
    print("与目标点的距离: " .. distance)
    -- 寻路到目标点
	local threshold = 5 -- 允许的误差范围
    if distance > threshold  then
        if game:寻路到(x, y) then
            print("寻路到目标点成功")

            -- 异步检测是否到达目标点

            while  not luaStopFlag do
                sleep(500) -- 每 500 毫秒检测一次
                local currentX = game:当前坐标X()
                local currentY = game:当前坐标Y()
                local currentDistance = game:计算距离(x, y)
                print("当前位置: (" .. currentX .. ", " .. currentY .. "), 剩余距离: " .. currentDistance)

                if currentDistance <= threshold then
                    print("到达目标点")
                    break
                end
            end
        else
            print("寻路到目标点失败")
        end
    else
        print("已经在目标点")
    end
end


-- 过图
local function passMap(currentMapName,targetMapName,x,y,x1,y1)
    local maxAttempts = 5
    local attempt = 1
	 -- 设置默认值
	x1 = x1 or 0
    y1 = y1 or 0
          moveToTargetAsync(x, y)
    while attempt <= maxAttempts do
        print("正在过图... (尝试次数: " .. attempt .. ")")
		game:走到目标点(x,y)
		sleep(1000)

        if game:当前地图名()==targetMapName then
            print("过图成功")
            break  -- 成功后退出循环
        else
			if(x1~=0 and y1~=0) then
				game:走到目标点(x,y)
				sleep(1000)
			end
        end

        attempt = attempt + 1
        if attempt > maxAttempts then
            print("已达到最大尝试次数 (" .. maxAttempts .. ")，仍未能成功过图")
            break
        end
    end
end

-- 战斗
function startCombat()
        game:开始战斗()
    while not luaStopFlag do
	-- 检查是否需要暂停或终止
        if luaStopFlag then
		game:结束战斗() 
        end
        sleep(5000)  -- 每 5000 毫秒检查一次停止标志
    end
	game:结束战斗() 
end

local function 出发()
	if game:当前地图名()=="中州" and game:计算距离(470,221)<20 then
	 	moveToTargetAsync(470,223)
		game:对话NPC选择命令("老兵","@main1")
		sleep(500)
		game:选择命令("@west")
		sleep(500)
		game:选择命令("@west1")
		sleep(1000)
	 end

	 if game:当前地图名()=="西域奇境" and game:计算距离(463,110)<20 then
	 	moveToTargetAsync(463,110)
		game:对话NPC选择命令("老兵","@main1")
		sleep(500)
		game:选择命令("@ttta")
		sleep(500)
		game:选择命令("@ta")
		sleep(1000)
	 end

	if game:当前地图名()=="西域奇境" and game:计算距离(140, 107)<20 then
	 	passMap("西域奇境","一手遮天", 137, 102 , 137, 103)
	 end

	if luaStopFlag then
	   return
	end

	if game:当前地图名()=="一手遮天" then
    -- 开始战斗
        print("到达战斗地图")
	else
		goHome()
	end
end


-- 补给
local function 卖物补给()
        -- 回城整理背包
        goHome()
        -- 获取待存和待卖物品数量
        local storeCount = game:待存物品数量()
        local sellJewelryCount = game:待卖首饰数量()
        local sellClothesCount = game:待卖衣服数量()
        local sellWeaponCount = game:待卖武器数量()

        print("待存物品数量: " .. storeCount)
        print("待卖首饰数量: " .. sellJewelryCount)
        print("待卖衣服数量: " .. sellClothesCount)
        print("待卖武器数量: " .. sellWeaponCount)

        -- 存仓库
        if storeCount > 0 then
            moveToTargetAsync(455, 228)
            game:对话NPC选择命令("便捷传送门", "@Shop_GO&0&378&217")
            sleep(1000)
            moveToTargetAsync(375, 211)
            storeGoods()
            game:使用物品("永久回城神石")
            sleep(1000)
        end

        -- 卖物品
    if sellJewelryCount > 0 then
        moveToTargetAsync(438, 249)
        sellItems("首饰", 5, "首饰店老板", {438, 249})
        game:使用物品("永久回城神石")
        sleep(1000)
    end

    if sellClothesCount > 0 then
        moveToTargetAsync(417, 206)
        passMap("中州", "服装店", 418, 205)
        sellItems("衣服", 5, "服装店老板", {418, 205})
        game:使用物品("永久回城神石")
        sleep(1000)
    end

    if sellWeaponCount > 0 then
        moveToTargetAsync(419, 246)
        sellItems("武器", 5, "武器店老板", {419, 246})
        game:使用物品("永久回城神石")
        sleep(1000)
    end

        -- 购买药品
        moveToTargetAsync(455, 228)
        game:对话NPC选择命令("便捷传送门", "@Shop_GO&0&502&266")
        sleep(1000)
        moveToTargetAsync(498, 268)
        sellMedicine()
        sleep(1000)
        buyMedicines("特级金创药包", 6)
        buyMedicines("特级魔法药包", 6)
        game:使用物品("永久回城神石")
        sleep(1000)

end




local function main()
-- 运行主循环
    print("Running...")
   卖物补给()
    -- 移动到战斗点
    出发()
end


main()
startCombat()
