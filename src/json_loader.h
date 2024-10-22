#pragma once

#include <boost/json.hpp>

#include <filesystem>
#include <optional>
#include <string>

#include "model.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);

void LoadAndAddRoads(const boost::json::array &road_value, model::Map &map);
void LoadAndAddBuildings(const boost::json::array &building_value, model::Map &map);
void LoadAndAddOffices(const boost::json::array& office_value, model::Map &map);

// Функции для формирования ответа (request_handler)
std::string GetListOfMaps(const model::Game& game);
std::optional<std::string> GetMap(const model::Map::Id& map_id, const model::Game& game);
std::string GetMapNotFoundString();
std::string GetBadRequestString();

boost::json::array GetRoadsArray(const model::Map &map);
boost::json::array GetBuildingsArray(const model::Map &map);
boost::json::array GetOfficesArray(const model::Map &map);

// Функции для логирования - возвращают соответствующие строки для логирования (в формате JSON)
std::string GetLogServerStart(const std::string timestamp,
                            const std::string srv_address,
                            const int port);
std::string GetLogServerStop(const std::string timestamp,
                            const int return_code,
                            const std::string exception_what);
std::string GetLogRequest(const std::string timestamp,
                            const std::string client_address,
                            const std::string uri,
                            const std::string http_method);
std::string GetLogResponse(const std::string timestamp,
                            const std::string client_address,
                            const int response_time_msec,
                            const int response_code,
                            const std::string content_type);
std::string GetLogError(const std::string timestamp,
                            const int error_code,
                            const std::string error_text,
                            const std::string where);

}  // namespace json_loader
