--[[ Script Luau para testar as funções da sua biblioteca C.
print("--- Iniciando os testes da biblioteca Luau customizada ---")

----------------------------------------------------
-- Testes de criação e tipos de closures
----------------------------------------------------

print("\n--- Testando a criação de closures e verificações de tipo ---")

-- Cria e testa um novo C closure
local myCClosure = newcclosure(function(arg)
    print("Chamado do C closure! Recebido:", arg)
    return "sucesso"
end)

print("C closure criado com sucesso.")

-- Verifica os tipos de closures
print("O myCClosure é um C closure?", iscclosure(myCClosure))
print("O myCClosure é um C closure 'novo'?", isnewcclosure(myCClosure))

-- Verifica um closure Luau padrão
local myLClosure = function() end
print("Uma função normal é um Luau closure?", islclosure(myLClosure))

-- Verifica os aliases de executor
print("O myCClosure é um closure executor?", isexecutorclosure(myCClosure))
print("O myCClosure é o nosso closure?", isourclosure(myCClosure))
print("O checkclosure funciona no myCClosure?", checkclosure(myCClosure))

-- Chama o C closure para confirmar o funcionamento
local result = myCClosure("Olá do Luau!")
print("Valor de retorno do C closure:", result)

----------------------------------------------------
-- Testes de hooking e clonagem
----------------------------------------------------

print("\n--- Testando hooking e clonagem ---")

-- Teste de clonefunction
local originalFunc = function(a, b)
    local upvalue = "original"
    print("Função original chamada. Upvalue:", upvalue)
    return a + b
end

local clonedFunc = clonefunction(originalFunc)
print("Função clonada criada.")
print("Chamando a função clonada com (5, 3):", clonedFunc(5, 3))
print("Chamando a função original com (10, 20):", originalFunc(10, 20))

-- Teste de hookfunction e seu alias replaceclosure
local hookFunc = function(arg)
    print("Enganchado! Novo comportamento para o C closure. Arg:", arg)
    return "enganchado!"
end

local originalCClosure = hookfunction(myCClosure, hookFunc)
print("myCClosure enganchado com sucesso. A função original foi retornada.")

-- Chama o closure 'enganchado' para ver o novo comportamento
local hookedResult = myCClosure("Isso vai acionar o gancho!")
print("Resultado do closure enganchado:", hookedResult)

-- Chama o closure original para provar que ele ainda funciona
print("Chamando a função original (não enganchada):")
local originalResult = originalCClosure("Re-chamando a função original.")
print("Resultado da função original:", originalResult)

-- Teste do alias 'replaceclosure'
local oldFunc = newcclosure(function() print("Eu sou a função antiga.") end)
local newFunc = newcclosure(function() print("Eu sou a função nova e substituída.") end)
replaceclosure(oldFunc, newFunc)
print("Chamando o closure substituído:")
oldFunc()

----------------------------------------------------
-- Testes de metamethods e outros
----------------------------------------------------

print("\n--- Testando metamethods e outras funções ---")

-- Teste de lookmetamethod
local myTable = {}
local mt = {}
setmetatable(myTable, mt)

lookmetamethod(myTable, "__index", function(self, key)
    print("Metamethod enganchado! Tentativa de acesso à chave:", key)
    return "valor_enganchado"
end)

-- Acessa uma chave inexistente para acionar o hook
print("Acessando myTable.nome:", myTable.nome)

-- Teste de getrawmetatable e setrawmetatable
local rawTbl = {}
local rawMt = {__index = {a = 1}}
setmetatable(rawTbl, rawMt)
print("Valor 'a' do raw metatable:", getrawmetatable(rawTbl).__index.a)

local newRawMt = {__index = {b = 2}}
setrawmetatable(rawTbl, newRawMt)
print("Valor 'b' do novo raw metatable:", getrawmetatable(rawTbl).__index.b)

-- Teste de setreadonly e isreadonly
local readOnlyTbl = { value = "imutável" }
setreadonly(readOnlyTbl, true)
print("A tabela é somente leitura?", isreadonly(readOnlyTbl))
local success, err = pcall(function()
    readOnlyTbl.value = "mudar valor"
end)
print("Tentativa de mudar tabela somente leitura. Sucesso?", success)
print("Erro:", err)

----------------------------------------------------
-- Manipulação de upvalues e constantes
----------------------------------------------------

print("\n--- Testando a manipulação de upvalues e constantes ---")

-- Teste de getupvalue e getupvalues
local upval = "string_upvalue"
local funcWithUpval = function() print(upval) end
local name, val = getupvalue(funcWithUpval, 1)
print("Nome do upvalue:", name, "Valor:", val)

local upvalsTable = getupvalues(funcWithUpval)
print("Todos os upvalues:", upvalsTable[1].name, "=", upvalsTable[1].value)

-- Teste de setupvalue
setupvalue(funcWithUpval, 1, "nova_string_upvalue")
print("Novo upvalue definido. Chamando a função para verificar...")
funcWithUpval()

-- Teste de getconstant e setconstant
local funcWithConsts = function()
    local x = "valor_constante"
    local y = 123
    return x, y
end

local const1 = getconstant(funcWithConsts, 1)
local const2 = getconstant(funcWithConsts, 2)
print("Constante 1:", const1, "Constante 2:", const2)

-- Define uma nova constante
setconstant(funcWithConsts, 1, "constante_alterada")
local newConst1 = getconstant(funcWithConsts, 1)
print("Constante 1 atualizada:", newConst1)
]]


print("--- Realistic hookmetamethod Test ---")

local remoteEvent = {}
local remoteFunction = {}

-- Create a dummy RemoteEvent metatable
local event_mt = {}
function event_mt:FireServer(...)
    print("Original FireServer called with:", ...)
    return "Event fired successfully!"
end
setmetatable(remoteEvent, {__index = event_mt})

-- Create a dummy RemoteFunction metatable
local func_mt = {}
function func_mt:InvokeServer(...)
    print("Original InvokeServer called with:", ...)
    return "Function invoked successfully!"
end
setmetatable(remoteFunction, {__index = func_mt})

-- Now, use hookmetamethod to spy on these dummy objects

-- Hook the RemoteEvent's 'FireServer'
local originalFireServer
hookmetamethod(remoteEvent, "__index", function(self, key)
    if key == "FireServer" then
        print("Hooked FireServer! Intercepting the call.")
        return function(...)
            print("Intercepted! FireServer was called with:", ...)
            return originalFireServer(...)
        end
    end
    return originalFireServer(self, key)
end)
-- Store the original function returned by the hook
-- The hook returns a function that will be called for all __index lookups.
-- So we can't capture the original `FireServer` this way.
-- A better way is to do the hook inside the hook.
-- Let's re-write the test to be more direct.

---

-- Let's try a better approach: Hook the metamethod and check for a specific key
local game = {}
local game_mt = {
    __index = function(self, key)
        if key == "Players" then
            print("Original __index called for 'Players'")
            return "Original Players Table"
        end
        return nil
    end
}
setmetatable(game, game_mt)


local original_index
pcall(function()
    original_index = hookmetamethod(game, "__index", function(self, key)
        print("Hooked __index method called.")
        if key == "HookedValue" then
            return "This value was intercepted!"
        end
        return original_index(self, key)
    end)
end)

print("\n--- Testing hookmetamethod ---")
print("Accessing a hooked property:", game.HookedValue)
print("Accessing an unhooked property:", game.Players)
print("Accessing a non-existent property:", game.NonExistent)

-- The original function should now be callable separately
print("\n--- Calling the original function directly ---")
local game_instance = game
local unhooked_players = original_index(game_instance, "Players")
print("Result from original_index:", unhooked_players)

