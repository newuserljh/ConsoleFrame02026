_G.__GOTO_TABLE__ = {}

-- 工具函数：获取表格键列表
function table.keys(t)
    local keys = {}
    for k in pairs(t) do table.insert(keys, k) end
    return keys
end

-- 精准标签扫描器
function scan_labels(code)  -- 确保参数存在性检查
    if not code then
        error("scan_labels: 缺少代码内容")
    end

    local labels = {}
    local line_number = 1
    for line in code:gmatch("([^\r\n]*)\r?\n?") do
        local label = line:match("^%s*::%s*(.-)%s*::%s*$")
        if label and label ~= "" then
            labels[label] = line_number
        end
        line_number = line_number + 1
    end
    return labels
end

-- 可靠跳转函数
function _G.goto(label)
    local labels = _G.__GOTO_TABLE__

    if not labels[label] then
        error(string.format("标签 '%s' 不存在（可用标签: %s）",
             label, table.concat(table.keys(labels), ", ")))
    end

    local target_line = labels[label]
    local current_line = debug.getinfo(2, "l").currentline or 0

    if current_line >= target_line then
        error(string.format("禁止向后跳转 (当前行:%d → 目标行:%d)",
              current_line, target_line))
    end

    debug.sethook(function(event, line)
        if event == "line" then
            if line >= target_line then
                debug.sethook()
            end
        end
    end, "l")
end

-- 改进版触发器系统（修复nil条件显示）
triggerSystem = {
    triggers = {},
    priority = {}
}

function triggerSystem:add(condition, action, level)
    level = level or 0

    -- 显式存储原始条件表达式
    local raw_condition = condition

    -- 条件函数编译（带错误处理）
    local cond_func, err = load(
        "return function() return "..condition.." end",
        "@condition",
        "t",
        _G
    )
    if not cond_func then
        error("条件编译失败: "..err)
    end
    cond_func = cond_func()

    -- 动作函数编译
    local act_func, err = load(action, "@action", "t", _G)
    if not act_func then
        error("动作编译失败: "..err)
    end

    table.insert(self.triggers, {
        condition = cond_func,
        action = act_func,
        level = level,
        raw_condition = raw_condition  -- 新增原始条件存储
    })

    -- 按优先级降序排序
    table.sort(self.triggers, function(a,b) return a.level > b.level end)
end

function triggerSystem:check()
    print("\n[触发器检测开始]")
    for _, t in ipairs(self.triggers) do
        -- 使用存储的原始条件显示
        local cond_str = t.raw_condition or "未知条件"

        -- 带错误捕获的条件检测
        local ok, result = pcall(t.condition)
        local status = ok and (result and "满足" or "不满足") or "错误"

        print(string.format("条件检测: %-20s → %s", cond_str, status))

        if ok and result then
            print("★ 触发动作执行")
            pcall(t.action)
            return true
        end
    end
    print("[触发器检测结束]")
    return false
end

local function run_test_case(test_code, init_hp)
    _G.血量 = init_hp
    _G.__GOTO_TABLE__ = scan_labels(test_code)

    print("\n"..string.rep("=", 40))
    print("启动测试案例，初始血量 =", init_hp)

    local chunk, err = load(test_code, "@test", "t", _G)
    if not chunk then error("编译错误: "..err) end

    local ok, msg = pcall(chunk)
    if not ok then print("执行错误: "..msg) end
end
-- 测试代码更新（增加终止条件）
local test_code = [[
::开始::
print("当前血量:", 血量)

triggerSystem:add("血量>90", "goto '安全血量'", 2)
triggerSystem:add("血量>50 and 血量<=90", "goto '中等血量'", 1)

for i = 1, 3 do  -- 限制检测次数
    sleep(100)
    if 血量 == nil then break end  -- 安全保护
end

::安全血量::
print(">> 进入安全状态")
血量 = nil  -- 清除状态防止重复触发
goto 结束

::中等血量::
print(">> 进入警戒状态")
血量 = nil
goto 结束

::结束::
print("=== 程序正常终止 ===")
]]



-- 执行测试案例（显式传递代码）
run_test_case(test_code, 60)
run_test_case(test_code, 30)
