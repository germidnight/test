#include "players.h"

#include <iomanip>
#include <sstream>

namespace players {

namespace detail {
    // длина строки всегда 32 hex цифры (128 бит)
    std::string TokenTag::Serialize() {
        const size_t len_of_64bit_word_in_hex = 16;
        std::ostringstream os;
        for (int i = 0; i < 2; ++i) {
            os << std::hex << std::setfill('0') << std::setw(len_of_64bit_word_in_hex) << tag[i];
        }
        return os.str();
    }

} // namespace detail

    Token* PlayerTokens::AddPlayer(Player* player) {
        Token token(detail::TokenTag{generator1_(), generator2_()}.Serialize());
        while (token_to_player_.count(token) > 0) {
            token = Token(detail::TokenTag{generator1_(), generator2_()}.Serialize());
        }
        token_to_player_[token] = player;
        return &(tokens_.emplace_back(std::move(token)));
    }

    Player* PlayerTokens::FindPlayerByToken(const Token& token) const noexcept {
        if (token_to_player_.count(token) > 0) {
            return token_to_player_.at(token);
        }
        return nullptr;
    }

    /* Создание и добавление пользователя:
     * 1) создаём и добавляем игрока с собакой в перечень игроков
     * 2) в выбранной игровой сессии добавляем собаку нового игрока
     */
    Player* Players::Add(std::string player_name, model::GameSession* game_session) {
        model::Map* game_map = game_session->GetMap();
        ;
        /*Player &player = players_.emplace_back(model::Dog(++next_dog_id_, player_name,
                                                            game_session->GetMap()->GetRandomPositionOnRoads()),
                                                     game_session);*/
        Player& player = players_.emplace_back(model::Dog(++next_dog_id_, player_name,
                                                            game_session->GetMap()->GetTestPositionOnRoads()),
                                                    game_session);
        game_session->AddDog(player.GetDog());
        map_id_to_index_.emplace(player.GetDog()->GetDogId(), players_.size() - 1);
        return &player;
    }

    Player* Players::FindPlayerByDogId(size_t dog_id) {
        if (map_id_to_index_.count(dog_id) > 0) {
            return &(players_[map_id_to_index_.at(dog_id)]);
        }
        return nullptr;
    }

    /* Добавляем нового пользователя в игру
     * 1) находим карту
     * 2) получаем игровую сессию
     * 3) добавляем пользователя
     * 4) получаем токен пользователя */
    JoinGameResult Application::JoinPlayerToGame(model::Map::Id map_id, std::string_view player_name) {
        const model::Map* map = game_.FindMap(map_id);
        if (map == nullptr) {
            return JoinGameResult(nullptr, 0, JoinGameErrorCode::MAP_NOT_FOUND);
        }

        model::GameSession* game_session = game_.PlacePlayerOnMap(map->GetId());
        if (game_session == nullptr) {
            return JoinGameResult(nullptr, 0, JoinGameErrorCode::SESSION_NOT_FOUND);
        }

        players::Player* player = players_.Add(std::string(player_name), game_session);
        Token *token = player_tokens_.AddPlayer(player);
        return JoinGameResult(token, player->GetDog()->GetDogId(), JoinGameErrorCode::NONE);
    }

    std::vector<Player*> Application::GetPlayersInSession(const Player* player) {
        std::vector<Player *> res_players;

        const model::GameSession *session = player->GetGameSession();
        for (const model::Dog *next_dog : session->GetDogs()) {
            Player *next_player = players_.FindPlayerByDogId(next_dog->GetDogId());
            if (next_player != nullptr) {
                res_players.push_back(next_player);
            }
        }
        return std::move(res_players);
    }

    void Application::SetDogAction(Player* player, ActionMove action_move) {
        auto dog_speed = player->GetGameSession()->GetMap()->GetSpeed();
        auto dog = player->GetDog();
        switch (action_move) {
        case ActionMove::LEFT : {
            dog->SetVelocity({-dog_speed, 0.});
            dog->SetDirection(model::Direction::WEST);
            break;
        }
        case ActionMove::RIGHT : {
            dog->SetVelocity({dog_speed, 0.});
            dog->SetDirection(model::Direction::EAST);
            break;
        }
        case ActionMove::UP : {
            dog->SetVelocity({0., -dog_speed});
            dog->SetDirection(model::Direction::NORTH);
            break;
        }
        case ActionMove::DOWN : {
            dog->SetVelocity({0., dog_speed});
            dog->SetDirection(model::Direction::SOUTH);
            break;
        }
        default: {
            dog->SetVelocity({0., 0.});
            break;
        }
        }
    }

    void Application::MoveDogs(double time_period) {
        for (auto &player : players_.GetPlayers()) {
            player.GetDog()->SetState(player.GetGameSession()->GetMap()->MoveDog(player.GetDog(), time_period));
        }
    }

} // namespace players
