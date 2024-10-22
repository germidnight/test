#include "json_loader.h"

#include <fstream>
#include <memory>

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    const std::string maps_str  = "maps";
    const std::string id_str    = "id";
    const std::string name_str  = "name";
    const std::string roads_str = "roads";
    const std::string buildings_str = "buildings";
    const std::string offices_str   = "offices";

    // Загрузить содержимое файла json_path, в виде строки
    std::ifstream json_file(json_path, std::ios::in);
    if (!json_file.is_open()) {
        throw std::runtime_error("Can't open file:" + json_path.string());
    }
    std::string json_str{std::istreambuf_iterator<char>(json_file), std::istreambuf_iterator<char>()};

    // Распарсить строку как JSON, используя boost::json::parse
    auto config_data = boost::json::parse(std::forward<std::string>(json_str));

    /* Загрузить модель игры из файла:
     * 1) получить массив карт
     * 2) в цикле массива карт:
     *  2.1) создать карту model::Map
     *  2.2) заполнить карту объектами: дорогами, офисами, зданиями
     *  2.3) добавить карту в игру */
    model::Game game;
    auto maps_value = config_data.at(maps_str).as_array();

    for (auto it_map = maps_value.begin(); it_map != maps_value.end(); ++it_map) {
        model::Map map(model::Map::Id(std::string{it_map->at(id_str).as_string()}),
                                    std::string{it_map->at(name_str).as_string()});

        auto road_value = it_map->at(roads_str).as_array();
        LoadAndAddRoads(road_value, map);

        auto office_value = it_map->at(offices_str).as_array();
        LoadAndAddOffices(office_value, map);

        auto building_value = it_map->at(buildings_str).as_array();
        LoadAndAddBuildings(building_value, map);

        game.AddMap(map);
    }

    return game;
}

void LoadAndAddRoads(const boost::json::array& road_value, model::Map& map) {
    const std::string road_x0_str = "x0";
    const std::string road_y0_str = "y0";
    const std::string road_x1_str = "x1";
    const std::string road_y1_str = "y1";

    for (auto it_road = road_value.begin(); it_road != road_value.end(); ++it_road) {
        model::Point start_road = {static_cast<int>(it_road->at(road_x0_str).as_int64()),
                                   static_cast<int>(it_road->at(road_y0_str).as_int64())};

        try {
            map.AddRoad({model::Road::HORIZONTAL, start_road, static_cast<int>(it_road->at(road_x1_str).as_int64())});
        } catch (const std::out_of_range& e) {
            map.AddRoad({model::Road::VERTICAL, start_road, static_cast<int>(it_road->at(road_y1_str).as_int64())});
        }
    }
}

void LoadAndAddBuildings(const boost::json::array& building_value, model::Map& map) {
    const std::string building_x_str = "x";
    const std::string building_y_str = "y";
    const std::string building_w_str = "w";
    const std::string building_h_str = "h";

    for (auto it_building = building_value.begin(); it_building != building_value.end(); ++it_building) {
        model::Point point{static_cast<int>(it_building->at(building_x_str).as_int64()),
                            static_cast<int>(it_building->at(building_y_str).as_int64())};
        model::Size size{static_cast<int>(it_building->at(building_w_str).as_int64()),
                          static_cast<int>(it_building->at(building_h_str).as_int64())};
        map.AddBuilding(model::Building{model::Rectangle{point, size}});
    }
}

void LoadAndAddOffices(const boost::json::array& office_value, model::Map& map) {
    const std::string office_id_str = "id";
    const std::string office_x_str = "x";
    const std::string office_y_str = "y";
    const std::string office_offsetx_str = "offsetX";
    const std::string office_offsety_str = "offsetY";

    for (auto it_office = office_value.begin(); it_office != office_value.end(); ++it_office) {
        model::Point position{static_cast<int>(it_office->at(office_x_str).as_int64()),
                                static_cast<int>(it_office->at(office_y_str).as_int64())};
        model::Offset office_offset{static_cast<int>(it_office->at(office_offsetx_str).as_int64()),
                                static_cast<int>(it_office->at(office_offsety_str).as_int64())};
        map.AddOffice({model::Office::Id(std::string{it_office->at(office_id_str).as_string()}),
                        position,
                        office_offset});
    }
}

// Функции для формирования ответа (request_handler)
std::string GetListOfMaps(const model::Game &game) {
    const std::string id_str = "id";
    const std::string name_str = "name";

    boost::json::array arr_json;
    for (auto it = game.GetMaps().begin(); it != game.GetMaps().end(); ++it) {
        boost::json::object temp_obj;
        temp_obj[id_str] = *(it->GetId());
        temp_obj[name_str] = it->GetName();
        arr_json.emplace_back(temp_obj);
    }
    boost::json::value val_json(arr_json);
    return {boost::json::serialize(val_json)};
}

std::optional<std::string> GetMap(const model::Map::Id& map_id, const model::Game &game) {
    const std::string id_str = "id";
    const std::string name_str = "name";
    const std::string roads_str = "roads";
    const std::string buildings_str = "buildings";
    const std::string offices_str = "offices";

    const model::Map* map_ptr = game.FindMap(map_id);
    if (map_ptr == nullptr) {
        return std::nullopt;
    }

    boost::json::object res_json;
    res_json[id_str]        = *(map_ptr->GetId());
    res_json[name_str]      = map_ptr->GetName();
    res_json[roads_str]     = GetRoadsArray(*map_ptr);
    res_json[buildings_str] = GetBuildingsArray(*map_ptr);
    res_json[offices_str]   = GetOfficesArray(*map_ptr);

    boost::json::value val_json(res_json);
    return {boost::json::serialize(val_json)};
}

boost::json::array GetRoadsArray(const model::Map& map) {
    const std::string road_x0_str = "x0";
    const std::string road_y0_str = "y0";
    const std::string road_x1_str = "x1";
    const std::string road_y1_str = "y1";

    boost::json::array roads_arr;
    for (auto it_road = map.GetRoads().begin(); it_road != map.GetRoads().end(); ++it_road) {
        boost::json::object obj_road;
        obj_road[road_x0_str]       = it_road->GetStart().x;
        obj_road[road_y0_str]       = it_road->GetStart().y;
        if (it_road->IsHorizontal()) {
            obj_road[road_x1_str]   = it_road->GetEnd().x;
        } else {
            obj_road[road_y1_str]   = it_road->GetEnd().y;
        }
        roads_arr.emplace_back(obj_road);
    }
    return roads_arr;
}

boost::json::array GetBuildingsArray(const model::Map &map) {
    const std::string building_x_str = "x";
    const std::string building_y_str = "y";
    const std::string building_w_str = "w";
    const std::string building_h_str = "h";

    boost::json::array buildings_arr;
    for (auto it_building = map.GetBuildings().begin(); it_building != map.GetBuildings().end(); ++it_building) {
        boost::json::object obj_building;
        obj_building[building_x_str] = it_building->GetBounds().position.x;
        obj_building[building_y_str] = it_building->GetBounds().position.y;
        obj_building[building_w_str] = it_building->GetBounds().size.width;
        obj_building[building_h_str] = it_building->GetBounds().size.height;
        buildings_arr.emplace_back(obj_building);
    }
    return buildings_arr;
}

boost::json::array GetOfficesArray(const model::Map &map) {
    const std::string office_id_str = "id";
    const std::string office_x_str = "x";
    const std::string office_y_str = "y";
    const std::string office_offsetx_str = "offsetX";
    const std::string office_offsety_str = "offsetY";

    boost::json::array offices_arr;
    for (auto it_office = map.GetOffices().begin(); it_office != map.GetOffices().end(); ++it_office) {
        boost::json::object obj_office;
        obj_office[office_id_str]   = *(it_office->GetId());
        obj_office[office_x_str]    = it_office->GetPosition().x;
        obj_office[office_y_str]    = it_office->GetPosition().y;
        obj_office[office_offsetx_str] = it_office->GetOffset().dx;
        obj_office[office_offsety_str] = it_office->GetOffset().dy;
        offices_arr.emplace_back(obj_office);
    }
    return offices_arr;
}

std::string GetMapNotFoundString() {
    boost::json::object res_obj;
    res_obj["code"] = "mapNotFound";
    res_obj["message"] = "Map not found";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

std::string GetBadRequestString() {
    boost::json::object res_obj;
    res_obj["code"] = "badRequest";
    res_obj["message"] = "Bad request";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

// Функции для логирования - возвращают соответствующие строки для логирования (в формате JSON)
std::string GetLogServerStart(const std::string timestamp,
                              const std::string srv_address,
                              const int port) {
    boost::json::object res_obj;
    res_obj["timestamp"] = timestamp;

    boost::json::object data_obj;
    data_obj["port"] = port;
    data_obj["address"] = srv_address;

    res_obj["data"] = data_obj;
    res_obj["message"] = "server started";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

std::string GetLogServerStop(const std::string timestamp,
                             const int return_code,
                             const std::string exception_what) {
    boost::json::object res_obj;
    res_obj["timestamp"] = timestamp;

    boost::json::object data_obj;
    data_obj["code"] = return_code;
    if (!exception_what.empty()) {
        data_obj["exception"] = exception_what;
    }

    res_obj["data"] = data_obj;
    res_obj["message"] = "server exited";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}
std::string GetLogRequest(const std::string timestamp,
                          const std::string client_address,
                          const std::string uri,
                          const std::string http_method) {
    boost::json::object res_obj;
    res_obj["timestamp"] = timestamp;

    boost::json::object data_obj;
    data_obj["ip"] = client_address;
    data_obj["URI"] = uri;
    data_obj["method"] = http_method;

    res_obj["data"] = data_obj;
    res_obj["message"] = "request received";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

std::string GetLogResponse(const std::string timestamp,
                           const std::string client_address,
                           const int response_time_msec,
                           const int response_code,
                           const std::string content_type) {
    boost::json::object res_obj;
    res_obj["timestamp"] = timestamp;

    boost::json::object data_obj;
    data_obj["ip"] = client_address;
    data_obj["response_time"] = response_time_msec;
    data_obj["code"] = response_code;
    data_obj["content_type"] = content_type;

    res_obj["data"] = data_obj;
    res_obj["message"] = "response sent";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

std::string GetLogError(const std::string timestamp,
                        const int error_code,
                        const std::string error_text,
                        const std::string where) {
    boost::json::object res_obj;
    res_obj["timestamp"] = timestamp;

    boost::json::object data_obj;
    data_obj["code"] = error_code;
    data_obj["text"] = error_text;
    data_obj["where"] = where;

    res_obj["data"] = data_obj;
    res_obj["message"] = "error";
    boost::json::value val_json(res_obj);
    return {boost::json::serialize(val_json)};
}

}  // namespace json_loader
