#include "api_handler.h"
#include "json_loader.h"

namespace http_handler {

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status,
                                      std::string_view body,
                                      unsigned http_version,
                                      bool keep_alive,
                                      std::string_view content_type,
                                      size_t length,
                                      std::string allowed_methods) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.set(http::field::cache_control, "no-cache");
        if (status == http::status::method_not_allowed) {
            response.set("Allow"sv, allowed_methods);
        }
        response.body() = body;
        if (length == 0) {
            length = response.body().size();
        }
        response.content_length(length);
        response.keep_alive(keep_alive);
        return response;
    }

    std::optional<players::Token> TryToExtractToken(std::string_view auth_header) {
        constexpr size_t bearer_token_length = 32;

        if (auth_header.empty() || (auth_header.substr(0, 7) != "Bearer ")) {
            return std::nullopt;
        }
        std::string_view bearer_token = auth_header.substr(7);
        if (bearer_token.length() != bearer_token_length) {
            return std::nullopt;
        }
        return players::Token(std::move(std::string(bearer_token)));
    }

    /* ------------------------------------ Обработчик запросов к API ------------------------------------ */

    StringResponse APIHandler::ReturnAPIResponse(const StringRequest&& req, std::string req_str) {
        const std::string command_maps1_str             = "/api/v1/maps";
        const std::string command_map2_str              = "/api/v1/maps/";
        const size_t command_map2_symbols = command_map2_str.length();

        const std::string command_join_str              = "/api/v1/game/join";
        const std::string command_session_players_str   = "/api/v1/game/players";

        const std::string command_get_game_state_str    = "/api/v1/game/state";
        const std::string command_action_str            = "/api/v1/game/player/action";
        const std::string command_tick_str              = "/api/v1/game/tick";

        unsigned int version = req.version();
        bool keep_alive = req.keep_alive();

        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0,
                                                         std::string allowed_methods = "GET, HEAD, POST"s) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length, allowed_methods);
        };
        /* ------------------------------------------- вход в игру ------------------------------------------- */
        if (req_str.starts_with(command_join_str)) {
            if (ContentType::JSON.compare(req[http::field::content_type]) != 0) {
                return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument",
                                                                                        "Invalid content type"));
            }
            if (req.method() != http::verb::post) {
                return text_response(http::status::method_not_allowed,
                                    json_loader::MakeErrorString("invalidMethod", "Only POST method is expected"),
                                    0, "POST");
            }
            return HandleJoining(req.body(), version, keep_alive);

        /* ----------------------------------- запрашивается карта с заданным id ----------------------------------- */
        } else if (req_str.starts_with(command_map2_str)) {
            std::string_view map_id = req_str.substr(command_map2_symbols);
            std::optional<std::string> map_result = json_loader::GetMap(model::Map::Id{std::string(map_id)}, app_);
            if (map_result.has_value()) {
                return text_response(http::status::ok, map_result.value());
            } else { // Запрашиваемый id карты не найден
                return text_response(http::status::not_found, json_loader::MakeErrorString("mapNotFound", "Map not found"));
            }
        /* ----------------------------------- запрашивается список карт ----------------------------------- */
        } else if (req_str.compare(command_maps1_str) == 0) {
            return text_response(http::status::ok, json_loader::GetListOfMaps(app_));

        /* ----------------------------------- запрашивается список игроков в сессии ----------------------------------- */
        } else if (req_str.starts_with(command_session_players_str)) {
            if ((req.method() == http::verb::get) || (req.method() == http::verb::head)) {
                return ExecuteAuthorized(req, [this](std::shared_ptr<players::Player> found_player,
                                                     unsigned int version,
                                                     bool keep_alive,
                                                     bool head_only) {
                    return this->HandlePlayersList(found_player, version, keep_alive, head_only);
                });
            } else {
                return text_response(http::status::method_not_allowed,
                                     json_loader::MakeErrorString("invalidMethod", "Only GET, HEAD methods is expected"),
                                     0, "GET, HEAD");
            }

        /* ----------------------------------- запрос игрового состояния ----------------------------------- */
        } else if (req_str.starts_with(command_get_game_state_str)) {
            if ((req.method() == http::verb::get) || (req.method() == http::verb::head)) {
                return ExecuteAuthorized(req, [this](std::shared_ptr<players::Player> found_player,
                                                     unsigned int version,
                                                     bool keep_alive,
                                                     bool head_only) {
                    return this->HandleGameState(found_player, version, keep_alive, head_only);
                });
            } else {
                return text_response(http::status::method_not_allowed,
                                     json_loader::MakeErrorString("invalidMethod", "Only GET, HEAD methods is expected"),
                                     0, "GET, HEAD");
            }

        /* ----------------------------------- запрос на управление персонажем ----------------------------------- */
        } else if (req_str.starts_with(command_action_str)) {
            if (req.method() == http::verb::post) {
                if (ContentType::JSON.compare(req[http::field::content_type]) != 0) {
                    return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument",
                                                                                                 "Invalid content type"));
                }
                return ExecuteAuthorized(req, [this, &req](std::shared_ptr<players::Player> found_player,
                                                           unsigned int version, bool keep_alive,
                                                           [[maybe_unused]] bool head_only) {
                    return HandleAction(found_player, req.body(),
                                        version, keep_alive);
                });
            } else {
                return text_response(http::status::method_not_allowed,
                                     json_loader::MakeErrorString("invalidMethod", "Only POST method is expected"),
                                     0, "POST");
            }

        /* ----------------------------------- запрос на управление временем на карте ----------------------------------- */
        } else if (app_.IsTestMode() && req_str.starts_with(command_tick_str)) {
            if (req.method() == http::verb::post) {
                if (ContentType::JSON.compare(req[http::field::content_type]) != 0) {
                    return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument",
                                                                                                 "Invalid content type"));
                }
                return HandleTick(req.body(), version, keep_alive);
            } else {
                return text_response(http::status::method_not_allowed,
                                     json_loader::MakeErrorString("invalidMethod", "Only POST method is expected"),
                                     0, "POST");
            }
        } else {
            // Неправильный запрос
            return text_response(http::status::bad_request, json_loader::MakeErrorString("badRequest", "Invalid endpoint"));
        }
    }

    /*
     * Обработка запроса на подключение к игре
     */
    StringResponse APIHandler::HandleJoining(std::string_view body,
                                            unsigned int version,
                                            bool keep_alive) {
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length);
        };

        auto join_data = json_loader::LoadJSONJoinGame(body);

        if (join_data.error) {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Join game request parse error"));
        }

        if (join_data.user_name.empty()) {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Invalid name"));
        }

        if (join_data.map_id.empty()) {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument", "Invalid map"));
        }

        players::JoinGameResult result = app_.JoinPlayerToGame(model::Map::Id{join_data.map_id}, join_data.user_name);

        if (result.error != players::JoinGameErrorCode::NONE) {
            switch (result.error) {
            case players::JoinGameErrorCode::MAP_NOT_FOUND :
                return text_response(http::status::not_found, json_loader::MakeErrorString("mapNotFound", "Map not found"));
            case players::JoinGameErrorCode::SESSION_NOT_FOUND :
                return text_response(http::status::not_found, json_loader::MakeErrorString("mapNotFound", "Session not found"));
            case players::JoinGameErrorCode::INVALID_NAME :
                return text_response(http::status::not_found, json_loader::MakeErrorString("invalidArgument", "Invalid name"));
            }
        }

        return text_response(http::status::ok, json_loader::GetPlayerAddedAnswer(**(result.player_token), result.dog_id));
    }

    /*
     * Обработка запроса на получение списка игроков в сессии игрока (кто делает запрос)
     */
    StringResponse APIHandler::HandlePlayersList(std::shared_ptr<players::Player> found_player,
                                                 unsigned int version,
                                                 bool keep_alive,
                                                 bool head_only) {
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0,
                                                         std::string allowed_methods = "GET, HEAD"s) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length, allowed_methods);
        };
        model::GameSession::Dogs dogs = app_.GetDogsInSession(found_player);
        if (head_only) {
            return text_response(http::status::ok, "", json_loader::GetSessionPlayers(dogs).length());
        }
        return text_response(http::status::ok, json_loader::GetSessionPlayers(dogs));
    }

    /*
     * Обработка запроса на получение игрового состояния:
     * 1) получаем список всех игроков в сессии
     * 2) формируем данные (вектор состояний собак) для ответа
     */
    StringResponse APIHandler::HandleGameState(std::shared_ptr<players::Player> found_player,
                                               unsigned int version,
                                               bool keep_alive,
                                               bool head_only) {
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0,
                                                         std::string allowed_methods = "GET, HEAD"s) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length, allowed_methods);
        };
        std::vector<players::GameState> game_state;

        for (const auto next_player : app_.GetPlayersInSession(found_player)) {
            game_state.push_back(app_.GetPlayerGameState(next_player));
        }
        if (head_only) {
            return text_response(http::status::ok, "", json_loader::MakeGameStateAnswer(std::move(game_state)).length());
        }
        return text_response(http::status::ok, json_loader::MakeGameStateAnswer(std::move(game_state)));
    }

    StringResponse APIHandler::HandleAction(std::shared_ptr<players::Player> found_player,
                                            std::string_view body,
                                            unsigned int version,
                                            bool keep_alive) {
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0,
                                                         std::string allowed_methods = "POST"s) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length, allowed_methods);
        };

        auto action_data = json_loader::LoadActionMove(body);
        if (action_data) {
            if (*action_data == "L") {
                app_.SetDogAction(found_player, players::ActionMove::LEFT);
            } else if (*action_data == "R") {
                app_.SetDogAction(found_player, players::ActionMove::RIGHT);
            } else if (*action_data == "U") {
                app_.SetDogAction(found_player, players::ActionMove::UP);
            } else if (*action_data == "D") {
                app_.SetDogAction(found_player, players::ActionMove::DOWN);
            } else {
                app_.SetDogAction(found_player, players::ActionMove::STOP);
            }
        } else {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument",
                                                                                        "Failed to parse action"));
        }
        return text_response(http::status::ok, "{}");
    }

    StringResponse APIHandler::HandleTick(std::string_view body,
                                            unsigned int version,
                                            bool keep_alive) {
        const auto text_response = [version, keep_alive](http::status status, std::string_view text, size_t length = 0,
                                                         std::string allowed_methods = "POST"s) {
            return MakeStringResponse(status, text, version, keep_alive, ContentType::JSON, length, allowed_methods);
        };

        auto action_data = json_loader::LoadTimeDelta(body);
        if (action_data) {
            /* все персонажи должны переместиться по правилам перемещения персонажей.
             * Последующие запросы игрового состояния должны возвращать новые координаты персонажей. */
            app_.MoveDogs(*action_data);
        } else {
            return text_response(http::status::bad_request, json_loader::MakeErrorString("invalidArgument",
                                                                            "Failed to parse tick request JSON"));
        }
        return text_response(http::status::ok, "{}");
    }

} // namespace http_handler
