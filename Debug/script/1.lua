-- 初始化全局环境
local ffi = require("ffi")
ffi.cdef[[void Sleep(int ms);]]

-- 标签系统
_G.__GOTO_TABLE__ = {}

function _G.scan_labels(code)
    local labels = {}
    local line_number = 1
    for line in code:gmatch("([^\n]*)\n?") do
        local label = line:match("^%s*::%s*(.-)%s*::%s*$")
        if label and label ~= "" then
            labels[label] = line_number
        end
        line_number = line_number + 1
    end
    return labels
end

function _G.goto(label)
    local target = __GOTO_TABLE__[label]
    if not target then
        error("未定义标签: "..label)
    end
    error("__GOTO__"..target, 2)
end

-- 触发器系统
_G.triggerSystem = {
    triggers = {},
    add = function(self, condition_func, action_func)
        table.insert(self.triggers, {
            condition = condition_func,
            action = action_func
        })
    end
}

function _G.sleep(ms)
    ffi.C.Sleep(ms)
    for _, t in ipairs(triggerSystem.triggers) do
        if t.condition() then
            t.action()
            break
        end
    end
end

-- 主执行器
function _G.execute(code)
    _G.__GOTO_TABLE__ = scan_labels(code)

    local chunk, err = load(code, nil, "t", _G)
    if not chunk then return print("加载错误:", err) end

    while true do
        local success, msg = xpcall(chunk, debug.traceback)
        if success then break end

        if msg:find("__GOTO__") then
            local target_line = tonumber(msg:match("__GOTO__(%d+)"))
            local new_code = {}
            local current_line = 1
            for line in code:gmatch("([^\n]*)\n?") do
                if current_line >= target_line then
                    table.insert(new_code, line)
                end
                current_line = current_line + 1
            end
            chunk = load(table.concat(new_code, "\n"), nil, "t", _G)
        else
            return print("运行时错误:", msg)
        end
    end
end

-- 主执行器（支持文件路径输入）
function _G.executefile(filepath)
    -- 读取文件内容
    local f = io.open(filepath, "r")
    if not f then
        error("无法打开文件: "..filepath)
    end
    local code = f:read("*a")
    f:close()

    -- 扫描标签并初始化
    _G.__GOTO_TABLE__ = scan_labels(code)

    -- 创建代码块加载器闭包
    local function create_chunk(code_segment)
        return load(code_segment, "@"..filepath, "t", _G)
    end

    -- 初始代码块
    local chunk, err = create_chunk(code)
    if not chunk then
        error("加载错误: "..tostring(err))
    end

    -- 执行循环
    while true do
        local success, msg = xpcall(chunk, debug.traceback)
        if success then break end

        -- 处理跳转信号
        if msg:find("__GOTO__") then
            local target_line = tonumber(msg:match("__GOTO__(%d+)"))

            -- 重建跳转后的代码段
            local new_code = {}
            local current_line = 1
            for line in code:gmatch("([^\n]*)\n?") do
                if current_line >= target_line then
                    table.insert(new_code, line)
                end
                current_line = current_line + 1
            end

            chunk = create_chunk(table.concat(new_code, "\n"))
        else
            error("运行时错误: "..msg)
        end
    end
end

-- 使用示例
executefile("D:\\LJH\\VS_PROJECT\\ConsoleFrame\\Debug\\script\\0307.lua")  -- 直接执行外部脚本文件



