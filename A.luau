game = loadstring([[

getgenv().game = newproxy(true);
local GameMetatable = {}

function GameMetatable.__add(game1, game2)
    local newGame = {
        name = game1.name .. " & " .. game2.name,
        players = game1.players + game2.players,
        status = game1.status .. ", " .. game2.status
    }
    setmeta(newGame, GameMetatable)
    return newGame
end

function GameMetatable.__sub(game1, game2)
    return nil
end

function GameMetatable.__mul(game1, game2)
    return nil
end

function GameMetatable.__div(game1, game2)
    return nil
end

function GameMetatable.__mod(game1, game2)
    return nil
end

function GameMetatable.__pow(game1, game2)
    return nil
end

function GameMetatable.__unm(game)
    return nil
end

function GameMetatable.__concat(game1, game2)
    local newGame = {
        name = game1.name .. " & " .. game2.name,
        players = game1.players + game2.players,
        status = game1.status .. ", " .. game2.status
    }
    setmeta(newGame, GameMetatable)
    return newGame
end

function GameMetatable.__len(game)
    return nil
end

function GameMetatable.__eq(game1, game2)
    return nil
end

function GameMetatable.__lt(game1, game2)
    return nil
end

function GameMetatable.__le(game1, game2)
    return nil
end

function GameMetatable.__index(table, key)
    return nil
end

function GameMetatable.__newindex(table, key, value)
end

function GameMetatable.__call(table, ...)
    return nil
end

function GameMetatable.__tostring(game)
    return "Um objeto Game"
end

function GameMetatable.__gc(game)
end

function createGame(name, players, status)
    local game = {name = name, players = players, status = status}
    setmeta(game, GameMetatable)
    return game
end

local g1 = createGame("smt1", 2, "Em andamento")
local g2 = createGame("smth2", 3, "Pausado")

print(g1.name, g1.players, g1.status)
print(g2.name, g2.players, g2.status)

local g3 = g1 + g2
local g4 = g1 .. g2

print(g3.name, g3.players, g3.status)
print(g4.name, g4.players, g4.status)

print(tostring(g1))

setmeta(getgenv().game, GameMetatable)

getgenv().workspace = getmeta(getgenv().game)

print(getgenv().workspace == GameMetatable)

local AnotherMetatable = {
    __tostring = function() return "Outra Metatable!" end
}
print(getmeta(getgenv().game) == GameMetatable)
setmeta(getgenv().game, AnotherMetatable)
print(getmeta(getgenv().game) == AnotherMetatable)
print(tostring(getgenv().game))

setmeta(getgenv().game, GameMetatable)

	return {20,20,20,20}
]])()

print(game[1] + game[2] + game[3] + game[4]) --80
if not getmeta then
    print(getmetatable(getgenv().game))
    print(getmetatable(getgenv().workspace))
else
    print(getmeta(getgenv().game))
    print(getmeta(getgenv().workspace))
end