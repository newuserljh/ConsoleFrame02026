::开始::
print("程序启动")

triggerSystem:add(
    function() return 血量 > 90 end,
    function() goto "结束" end
)

for i = 1, 100 do
    print("当前血量:", 血量)
    print("当前坐标",坐标X,坐标Y)
    sleep(10)
end

::结束::
print("任务完成")