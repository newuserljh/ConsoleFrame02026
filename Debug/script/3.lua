local manager=scriptMgr
manager:add_trigger("血量>90", "回城")
print(血量)
print("12311111")
--game:使用物品("永久回城神石")
i=0
while (i<100) do
   print(i)
   i=i+1
end
manager:stop_all_triggers()

function 回城()
   print("回城")
   manager:stop_all_triggers()
end
