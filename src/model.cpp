#include "model.h"

#include <cmath>
#include <random>
#include <stdexcept>
#include <optional>

namespace model {
using namespace std::literals;

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        offices_.pop_back();
        throw;
    }
}

void Map::AddRoad(const Road& road) {
    Point start = road.GetStart();
    Point end = road.GetEnd();

    if (start.x > end.x) {
        std::swap(start.x, end.x);
    }
    if (start.y > end.y) {
        std::swap(start.y, end.y);
    }
    if (road.IsHorizontal()) {
        normal_roads_.emplace_back(Road::HORIZONTAL, start, end.x);
    } else {
        normal_roads_.emplace_back(Road::VERTICAL, start, end.y);
    }
    roads_.emplace_back(road);
}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
            sessions_.push_back(Sessions{});
        } catch (...) {
            map_id_to_index_.erase(it);
            sessions_.pop_back();
            throw;
        }
    }
}

GameSession* Game::PlacePlayerOnMap(const Map::Id& map_id) {
    if (map_id_to_index_.count(map_id) == 0) {
        return nullptr;
    }
    const size_t map_index = map_id_to_index_.at(map_id);
    if (sessions_[map_index].empty() || (sessions_[map_index].back().CountDogsInSession() == MAX_DOGS_ON_MAP)) {
        sessions_[map_index].emplace_back(model::GameSession(&maps_[map_index]));
    }
    return &(sessions_[map_index].back());
}

Position Map::GetRandomPositionOnRoads() const {
    Position res_pos;
    // определить дорогу
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> road_gen(0, normal_roads_.size() - 1);
    const Road& rand_road = normal_roads_[road_gen(gen)];
    // определить позицию на дороге
    if (rand_road.IsHorizontal()) {
        res_pos.y = rand_road.GetStart().y;
        std::uniform_int_distribution<> coord_gen(rand_road.GetStart().x, rand_road.GetEnd().x);
        res_pos.x = coord_gen(gen);
    } else {
        res_pos.x = rand_road.GetStart().x;
        std::uniform_int_distribution<> coord_gen(rand_road.GetStart().y, rand_road.GetEnd().y);
        res_pos.y = coord_gen(gen);
    }
    return res_pos;
}

Position Map::GetTestPositionOnRoads() const noexcept {
    return {static_cast<double>(normal_roads_[0].GetStart().x),
            static_cast<double>(normal_roads_[0].GetStart().y)};
}

namespace detail {
    /* Округление позиции на дороге до координат */
    Coord RoundPosition(double pos) {
        const double round_delta = 0.5999; // 1 - 0.0001 - HALF_ROAD_WIDE
        if (pos >= 0) {
            return static_cast<Coord>(pos + round_delta);
        }
        return static_cast<Coord>(pos - round_delta);
    }

    /* Дороги общие для начальной точки пути и для конечной точки пути  */
    bool FoundRoad(const std::vector<size_t> &roads_now, const std::vector<size_t> &roads_future) {
        if (!roads_now.empty() && !roads_future.empty()) {
            for (const size_t road_n : roads_now) {
                for (const size_t road_f : roads_future) {
                    if (road_n == road_f) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /* Нашли ли мы дорогу длиннее */
    bool FoundBigger(double &max_length, size_t &long_road_idx, double length, size_t j) {
        if (max_length < length) {
            max_length = length;
            long_road_idx = j;
            return true;
        }
        return false;
    }

    bool DoubleIsZero(double val) {
        constexpr double DELTA = 0.000001;
        if ((val > -DELTA) && (val < DELTA)) {
            return true;
        }
        return false;
    }
} // namespace detail

/*
 * Расчёт передвижения собаки по дорогам (возвращает новое разрешённое состояние собаки):
 * 1) Находим перечень дорог на которых может находиться собака (сейчас)
 * 2) Расчитываем ожидаемое положение
 * 3) Находим перечень дорог где может находиться собака (после перемещения)
 * 4) Если есть общая дорога для шагов 1 и 3, значит перемещаем собаку
 * 5) Если общей дороги нет, то перемещаем собаку на границу дороги и останавливаем
 * 5.1) Поиск границы дороги вдоль осуществляется исходя из направления движения перебором всех текущих дорог
 * 5.2) Если собака находится в крайней точке дороги, то перемещаем её на 0.4 (по направлению движения) и останавливаем
 * 5.3) Если движемся поперёк дороги, то перемещаем собаку на 0.4 (по направлению движения) и останавливаем
 */
DogState Map::MoveDog(const Dog* dog, double time) {
    Position pos_now = dog->GetDogState().position;
    Velocity dog_speed = dog->GetDogState().velocity;
    DogState new_dog_state = dog->GetDogState();

    std::vector<size_t> roads_now = GetRoadByPosition(pos_now);

    Position pos_future = {pos_now.x + (time * dog_speed.x), pos_now.y + (time * dog_speed.y)};
    std::vector<size_t> roads_future = GetRoadByPosition(pos_future);

    bool calculated = false;
    if (detail::FoundRoad(roads_now, roads_future)) { // можно переместиться в конечную точку
        new_dog_state.position = pos_future;
    } else { // нельзя переместиться в конечную точку - нужно найти максимальную доступную крайнюю точку
        /* Ищем дорогу, которая позволяет уехать максимально далеко в нужном направлении */
        size_t long_road_idx = 0;
        double max_length = 0.;
        for (size_t i = 0; i < roads_now.size(); ++i) {
            switch (dog->GetDogState().direction) {
            case Direction::EAST: { // "R" — задаёт направление движения персонажа вправо (на восток) - скорость равна {s, 0}.
                size_t j = roads_now[i];
                if (normal_roads_[j].IsHorizontal()) {
                    calculated = detail::FoundBigger(max_length, long_road_idx, (normal_roads_[j].GetEnd().x - pos_now.x), j);
                }
                break;
            }
            case Direction::WEST: { // "L" — задаёт направление движения персонажа влево (на запад) - скорость равна {-s, 0}.
                size_t j = roads_now[i];
                if (normal_roads_[j].IsHorizontal()) {
                    calculated = detail::FoundBigger(max_length, long_road_idx, (pos_now.x - normal_roads_[j].GetStart().x), j);
                }
                break;
            }
            case Direction::NORTH: { // "U" — задаёт направление движения персонажа вверх (на север) - скорость равна {0, -s}.
                size_t j = roads_now[i];
                if (normal_roads_[j].IsVertical()) {
                    calculated = detail::FoundBigger(max_length, long_road_idx, (pos_now.y - normal_roads_[j].GetStart().y), j);
                }
                break;
            }
            case Direction::SOUTH: { // "D" — задаёт направление движения персонажа вниз (на юг) - скорость равна {0, s}.
                size_t j = roads_now[i];
                if (normal_roads_[j].IsVertical()) {
                    calculated = detail::FoundBigger(max_length, long_road_idx, (normal_roads_[j].GetEnd().y - pos_now.y), j);
                }
                break;
            }
            }
        }
        double speed_sign_x = 1.;
        double speed_sign_y = 1.;
        if (dog_speed.x != 0.) {
            speed_sign_x = dog_speed.x / std::fabs(dog_speed.x);
        }
        if (dog_speed.y != 0.) {
            speed_sign_y = dog_speed.y / std::fabs(dog_speed.y);
        }
        if (calculated) { /* Движемся вдоль дороги */
            if (normal_roads_[long_road_idx].IsHorizontal()) {
                if (!detail::DoubleIsZero(max_length)) {
                    new_dog_state.position.x += ((max_length + HALF_ROAD_WIDE) * speed_sign_x);
                } else {
                    new_dog_state.position.x += (HALF_ROAD_WIDE * speed_sign_x);
                }
            } else {
                if (!detail::DoubleIsZero(max_length)) {
                    new_dog_state.position.y += ((max_length + HALF_ROAD_WIDE) * speed_sign_y);
                } else {
                    new_dog_state.position.y += (HALF_ROAD_WIDE * speed_sign_y);
                }
            }
        } else { /* Движемся поперёк дороги */
            if (!detail::DoubleIsZero(dog_speed.x)) {
                new_dog_state.position.x = detail::RoundPosition(new_dog_state.position.x) + HALF_ROAD_WIDE * speed_sign_x;
            }
            if (!detail::DoubleIsZero(dog_speed.y)) {
                new_dog_state.position.y = detail::RoundPosition(new_dog_state.position.y) + HALF_ROAD_WIDE * speed_sign_y;
            }
        }
        new_dog_state.velocity.x = 0.;
        new_dog_state.velocity.y = 0.;
    }
    return new_dog_state;
}

std::vector<size_t> Map::GetRoadByPosition(const Position &pos) {
    Point curr_pos{static_cast<Coord>(detail::RoundPosition(pos.x)),
                   static_cast<Coord>(detail::RoundPosition(pos.y))};
    std::vector<size_t> found_road_idxs;

    for (size_t i = 0; i < normal_roads_.size(); ++i) {
        if (normal_roads_[i].IsHorizontal()) {
            if ((normal_roads_[i].GetStart().y == curr_pos.y) &&
                (curr_pos.x >= normal_roads_[i].GetStart().x) &&
                (curr_pos.x <= normal_roads_[i].GetEnd().x)) {
                found_road_idxs.push_back(i);
            }
        } else {
            if ((normal_roads_[i].GetStart().x == curr_pos.x) &&
                (curr_pos.y >= normal_roads_[i].GetStart().y) &&
                (curr_pos.y <= normal_roads_[i].GetEnd().y)) {
                found_road_idxs.push_back(i);
            }
        }
    }
    return std::move(found_road_idxs);
}

}  // namespace model
