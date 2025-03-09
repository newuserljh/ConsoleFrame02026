function _G.get_parent_directory()
    -- 获取调用栈信息
    local info = debug.getinfo(1, "S")
    if not info or not info.source then
        return nil, "cannot get source path"
    end

    -- 处理路径
    local path = info.source:sub(2)
    if path:find("^%a+://") then  -- 过滤特殊路径
        return nil, "unsupported URI scheme"
    end

    -- 路径规范化处理
    path = path:gsub("\\", "/")
            :gsub("/+$", "")

    -- 层级提取
    local dir_path = path:match("^(.*)/") or ""
    local parent_dir = dir_path:match("^(.*)/") or dir_path

    -- 空路径检查
    if parent_dir == "" then
        return nil, "already at root directory"
    end

    -- 恢复原生路径格式
    if package.config:sub(1,1) == "\\" then
        parent_dir = parent_dir:gsub("/", "\\")
    end

    return parent_dir
end

-- 获取指定目录下的所有子目录（适用于 Windows）
function get_subdirectories(directory)
    local subdirs = {}
    local handle = io.popen('dir /AD /B /S "' .. directory .. '"')
    for dir in handle:lines() do
        table.insert(subdirs, dir)
    end
    handle:close()
    return subdirs
end

-- 添加目录及其子目录到 package.path 和 package.cpath
function add_directories_to_package_paths(directory)
    local sep = package.config:sub(1, 1) -- 获取路径分隔符
    local subdirs = get_subdirectories(directory)   -- 获取所有子目录
    -- 添加到 package.path
    for _, subdir in ipairs(subdirs) do

        local custom_path = subdir .. sep .. "?.lua"
        if not package.path:find(custom_path, 1, true) then
            package.path = package.path .. ";" .. custom_path
        end
    end

-- 添加到 package.cpath
    for _, subdir in ipairs(subdirs) do
        local custom_dll_path = subdir .. sep .. "?" .. (sep == "\\" and ".dll" or ".so")
        if not package.cpath:find(custom_dll_path, 1, true) then
            package.cpath = package.cpath .. ";" .. custom_dll_path
        end
    end
end

local cdir=_G.get_parent_directory()
print(cdir)
add_directories_to_package_paths(cdir) --初始化目录

-- 运行初始化
print(package.path)
print("\n")
print(package.cpath)

-- 加载地图数据
_G.map = require("map_data")
-- 定义广度优先搜索函数
function _G.bfs(startRegion, endRegion, transitions)
    local queue = {{region = startRegion, path = {region = startRegion}}}
    local visited = {}

    while #queue > 0 do
        local current = table.remove(queue, 1)
        local region = current.region
        local path = current.path

        if not visited[region] then
            visited[region] = true

            if region == endRegion then
                return path -- 返回找到的路径
            end

            if transitions[region] then
                for nextRegion, coords in pairs(transitions[region]) do
                    if not visited[nextRegion] then
                        local newPath = {}
                        for _, v in ipairs(path) do
                            table.insert(newPath, v)
                        end
                        table.insert(newPath, {region = nextRegion, coords = coords})
                        table.insert(queue, {region = nextRegion, path = newPath})
                    end
                end
            end
        end
    end

    return nil -- 如果没有找到路径，则返回nil
end

-- 打印路径和过图点（含坐标）
function _G.printPathAndNodesWithCoordinates(path)
    if path then
        print("找到路径:")
        for i, node in ipairs(path) do
            print(string.format("%d. %s", i, node.region))
            if node.coords then
                for j, coord in ipairs(node.coords) do
                    print(string.format("   - 坐标 %d: (%d, %d)", j, coord.x, coord.y))
                end
            end
        end
    else
        print("未找到路径")
    end
end

-- 使用示例
local startRegion = "1"
local endRegion = "L004"
local path = bfs(startRegion, endRegion, map.transitions)

printPathAndNodesWithCoordinates(path)

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
    end,
    -- 新增清空方法
    clear = function(self)
        self.triggers = {}  -- 重新初始化触发器列表
	_G.__GOTO_TABLE__ = {}
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
	code = code .. [[
    triggerSystem:add(
      function() return luaStopFlag end,
      function() goto "endScript" end
     )
    ::endScript::
     print("脚本已停止!!!")
]]
    -- 扫描标签并初始化
	--_G.triggerSystem:clear()
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
