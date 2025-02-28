-- 全局变量初始化
health = 100
mana = 200

-- 触发器定义
-- Triggers:
health < 50 goto heal_phase
mana < 30 goto restore_phase

-- 首次执行跳转到主循环

::main_loop::
start_auto_combat(1.5)
while true do
    print(string.format("状态: HP=%d MP=%d", health, mana))
    sleep(1)
end

::heal_phase::
print("进入治疗阶段!")
health = 80
goto main_loop

::restore_phase::
print("进入恢复阶段!")
mana = 100
goto main_loop