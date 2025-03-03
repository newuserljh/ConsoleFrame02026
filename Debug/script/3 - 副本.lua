-- 全局跳转表，存储标签及其对应的 PC 值
__GOTO_TABLE__ = {}

-- 栈结构，用于处理嵌套块的标签
__GOTO_STACK__ = {}

-- 原始的 goto 函数
local original_goto = _G.goto

-- 自定义的 goto 函数
function _G.goto(label)
    -- 查找标签在全局表中的 PC 值
    local target_pc = __GOTO_TABLE__[label]
    if not target_pc then
        error(string.format("标签 '%s' 未定义", label), 2)
    end

    -- 修改程序计数器以实现跳转
    debug.setlocal(1, 0, target_pc)
end

-- 钩子函数，用于记录标签及其 PC 值
local function hook(event, line)
    if event == "call" then
        -- 进入函数时，记录当前块的标签
        local info = debug.getinfo(2, "n")
        if info.name == "goto" then
            local current_label = debug.getlocal(2, 1)
            table.insert(__GOTO_STACK__, current_label)
        end
    elseif event == "return" then
        -- 退出函数时，移除当前块的标签
        if #__GOTO_STACK__ > 0 then
            table.remove(__GOTO_STACK__, #__GOTO_STACK__)
        end
    elseif event == "line" then
        -- 记录标签及其 PC 值
        local info = debug.getinfo(2, "S")
        if info.source ~= nil then
            local current_label = debug.getlocal(2, 1)
            if current_label ~= nil then
                __GOTO_TABLE__[current_label] = debug.getpc()
            end
        end
    end
end

-- 设置调试钩子
debug.sethook(hook, "call,return,line")

-- 自定义触发器系统
local triggerSystem = {
    triggers = {},
    checkInterval = 500
}

function triggerSystem:add(condition, action)
    table.insert(self.triggers, {
        condition = condition,
        action = action
    })
end

function triggerSystem:check()
    for _, t in ipairs(self.triggers) do
        if load("return "..t.condition)() then
            load(t.action)()
        end
    end
end

-- 修改后的sleep函数
local originalSleep = sleep
function sleep(ms)
    originalSleep(ms)
    triggerSystem:check()
end


-- 使用示例
triggerSystem:add("血量 < 30", [[
    game:useItem("红药")
    print("自动使用红药")
]])

triggerSystem:add("血量>90", [[
  goto 结束
]])

function 回城()
   print("回城")
end


print(血量)
print("12311111")

for i = 0, 99 do
    print("当前血量:".. 血量)
    sleep(100)
    if i % 10 == 0 then
        print(i)
    end
end

::结束::
print("end！")
