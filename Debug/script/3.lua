local manager=scriptMgr
manager:add_trigger("血量>90", "回城")
print(血量)
print("12311111")
--game:使用物品("永久回城神石")

-- 4. 非阻塞循环：模拟游戏主循环
for i = 0, 99 do
    -- 5. 打印当前血量（通过 scriptMgr 获取）
    print("当前血量:".. 血量)
    sleep(100)
    -- 6. 主动触发条件检测
   -- manager:tick()  -- 假设 scriptMgr 有此方法
    
    -- 7. 模拟血量减少（每10次循环减少一次）
    if i % 10 == 0 then
        print(i)
    end
end

function 回城()
   print("回城")
   manager:clear_triggers()
end
