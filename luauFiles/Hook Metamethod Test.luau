print("--- Realistic hookmetamethod Test ---")

local game = {}
local game_mt = {
    -- Agora, o __index está definido.
    __index = function(self, key)
        print("Original __index chamado para chave:", key)
        if key == "Players" then
            return "Tabela Players Original"
        end
        return nil
    end
}
setrawmetatable(game, game_mt)

local original_index
pcall(function()
    -- Hookando o método __index que já existe.
    original_index = lookmetamethod(game, "__index", function(self, key)
        print("Hooked __index method called.")
        if key == "HookedValue" then
            return "Esse valor foi interceptado!"
        end
        -- Fallback para a função original que foi retornada pelo hook.
        return original_index(self, key)
    end)
end)

print("\n--- Testando o hookmetamethod ---")
print("Acessando uma propriedade hookada:", game.HookedValue)
print("Acessando uma propriedade não hookada:", game.Players)
print("Acessando uma propriedade inexistente:", game.NonExistent)

print("\n--- Chamando a função original diretamente ---")
local game_instance = game
local unhooked_players = original_index(game_instance, "Players")
print("Resultado do original_index:", unhooked_players)

