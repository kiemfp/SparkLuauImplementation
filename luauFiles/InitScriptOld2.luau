
-- Cria uma corrotina
local coroutine_to_test = coroutine.create(function()
    -- Este é o ambiente da corrotina.
    -- 'a' e 'b' são variáveis locais.
    local a = 10
    b = 20

    -- Ponto de suspensão
    coroutine.yield()

    -- Após o 'yield', a execução da corrotina continua aqui.
    print("Corrotina retomada.")
end)

-- Inicia a corrotina. Ela irá suspender no `yield()`.
coroutine.resume(coroutine_to_test)
print("Corrotina suspensa.")

-- Obtém o ambiente da corrotina usando 'gettenv'
-- A função 'gettenv' do C é chamada aqui.
local coroutine_env = gettenv(coroutine_to_test)

-- Verifica o ambiente da corrotina
-- 'coroutine_env' é uma tabela.
print("O ambiente da corrotina é uma tabela:", type(coroutine_env) == "table")

-- A variável 'b' (sem 'local') foi criada no ambiente global da corrotina.
-- Portanto, ela está presente na tabela 'coroutine_env'.
print("Variável 'b' no ambiente da corrotina:", coroutine_env.b)

-- Adiciona uma nova variável no ambiente da corrotina
coroutine_env.new_var = 30
print("Adicionada 'new_var' no ambiente da corrotina.")

-- Agora, resume a corrotina. A variável `new_var` pode ser acessada dentro dela.
coroutine.resume(coroutine_to_test, "novo_valor")
print("Corrotina retomada.")

-- Tenta acessar a variável 'new_var' dentro da corrotina
-- O código da corrotina pode ser modificado para imprimir `new_var`.
-- Por exemplo: `print("Nova variável:", new_var)`



--[[
-- Instance class
local Instance = {}
Instance.__index = Instance

function Instance.new(ClassName, Parent)
    local self = setmetatable({
        ClassName = ClassName,
        Name = "(unnamed)",
        __Children = {},
        __Parent = nil
    }, Instance)

    if Parent then
        self.Parent = Parent
    end

    return self
end

function Instance:SetParent(Parent)
    if self.__Parent then
        for i, Child in ipairs(self.__Parent.__Children) do
            if Child == self then
                table.remove(self.__Parent.__Children, i)
                break
            end
        end
    end
    self.__Parent = Parent
    if Parent then
        table.insert(Parent.__Children, self)
    end
end

function Instance:GetChildren()
    return self.__Children
end

function Instance:FindFirstChild(Name)
    for _, Child in ipairs(self.__Children) do
        if Child.Name == Name then
            return Child
        end
    end
    return nil
end

function Instance:__tostring()
    return string.format("%s: %s", self.ClassName, self.Name)
end

Instance.__newindex = function(self, Key, Value)
    if Key == "Parent" then
        self:SetParent(Value)
    else
        rawset(self, Key, Value)
    end
end

-- DataModel (game)
local game = Instance.new("DataModel")
game.Name = "GAME_NAME"

function game:GetService(ServiceName)
    local service = self:FindFirstChild(ServiceName)
    if service then
        return service
    end
    error(("Service '%s' not found."):format(ServiceName))
end

-- Example Players service
local Players = Instance.new("Players", game)
Players.Name = "Players"
Players.LocalPlayer = { Name = "Kiemfp", UserId = 12345 }
Players.Parent = game

-- Example usage
print(game) -- DataModel: RobloxGameMock
local pServ = game:GetService("Players")
print(pServ.LocalPlayer.Name) -- Kiemfp

local Workspace = Instance.new("Workspace", game)
Workspace.Name = "Workspace"

local Part = Instance.new("Part", Workspace)
Part.Name = "BasePart"

for _, Child in ipairs(game:GetChildren()) do
    print("Child of game:", Child)
end
getgenv().workspace = Workspace
--return game, Instance, workspace
getgenv().game = game

getgenv().Game = getgenv().game
getgenv().Workspace = getgenv().workspace


]]