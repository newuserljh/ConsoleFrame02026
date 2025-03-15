print("程序启动")

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

startCombat()
