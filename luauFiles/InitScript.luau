
getgenv().wait = newcclosure(function(seconds) -- make it in C side 
    local initialTime = os.clock()
    while (os.clock() - initialTime) < seconds do
        coroutine.yield()
    end
end) 

getgenv().ClassDescryptors = {
	DataModel = {
		ClassName = "DataModel"
	}
}
getgenv().PropertyDescryptors = {
	DataModel = {
	}
}
getgenv().Hierarchy = { -- should be local for less bypasses.
	
}

getgenv().Instances = {
	
}

local steal_a_roblox = loadstring([[
	getgenv().game = newproxy(true)
	GameProxy = getrawmetatable(game)
	GameProxy.__index = newcclosure(function(self, index)
		
	end)
]])()