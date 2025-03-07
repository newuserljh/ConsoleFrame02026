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



