#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses,
};

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};

istream &operator>>(istream &is, Query &q) {
    // Реализуйте эту функцию
    string query_string, query_part;

    /* Предварительная очиска мусора от предыдущей итерации */
    q.stops.clear();

    getline(is, query_string);
    if (query_string.empty()) {
        getline(is, query_string);
    }
    istringstream query(query_string);

    query >> query_part;
    if (query_part == "NEW_BUS"s) {
        int len;
        string str;

        q.type = QueryType::NewBus;
        query >> q.bus;
        query >> len;
        for (int i = 0; i < len; ++i) {
            query >> str;
            q.stops.push_back(str);
        }
    } else if (query_part == "BUSES_FOR_STOP"s) {
        q.type = QueryType::BusesForStop;
        query >> q.stop;
    } else if (query_part == "STOPS_FOR_BUS"s) {
        q.type = QueryType::StopsForBus;
        query >> q.bus;
    } else if (query_part == "ALL_BUSES"s) {
        q.type = QueryType::AllBuses;
    }
    return is;
}

struct BusesForStopResponse {
    // Наполните полями эту структуру
    string stop;
    vector<string> buses;
};

ostream &operator<<(ostream &os, const BusesForStopResponse &r) {
    // Реализуйте эту функцию
    if (!r.stop.empty()) {
        os << "Stop "s << r.stop << ":"s;
        for (const string &str : r.buses) {
            os << " "s << str;
        }
    } else {
        os << "No stop"s;
    }
    return os;
}

struct StopsForBusResponse {
    // Наполните полями эту структуру
    string bus;
    vector<BusesForStopResponse> stop_buses;
};

ostream &operator<<(ostream &os, const StopsForBusResponse &r) {
    // Реализуйте эту функцию
    bool first_time = true;

    if (!r.bus.empty()) {
        for (const auto &stop_interchanges : r.stop_buses) {
            if (!first_time) {
                os << endl;
            } else {
                first_time = false;
            }

            if (!stop_interchanges.buses.empty()) { // выводим маршруты, проходящие через остановку
                os << stop_interchanges;
            } else { // через остановку проходит только один маршрут
                os << "Stop "s << stop_interchanges.stop << ": no interchange"s;
            }
        }
    } else {
        os << "No bus"s;
    }
    return os;
}

struct AllBusesResponse {
    // Наполните полями эту структуру
    vector<string> buses;
    map<string, vector<string>> stops;
};

ostream &operator<<(ostream &os, const AllBusesResponse &r) {
    // Реализуйте эту функцию
    bool first_time = true;

    if (!r.buses.empty()) {
        for (const string &bus : r.buses) {
            if (!first_time) {
                os << endl;
            } else {
                first_time = false;
            }

            os << "Bus "s << bus << ":"s;
            for (const string &stop : r.stops.at(bus)) {
                os << " "s << stop;
            }
        }
    } else {
        os << "No buses"s;
    }
    return os;
}

class BusManager {
public:
    void AddBus(const string &bus, const vector<string> &stops) {
        // Реализуйте этот метод
        buses_.push_back(bus);
        stops_[bus] = stops;
        for (const string &stop : stops) {
            buses_for_stop_[stop].emplace(bus);
        }
    }

    BusesForStopResponse GetBusesForStop(const string &stop) const {
        // Реализуйте этот метод
        BusesForStopResponse bfsr;

        if (buses_for_stop_.count(stop) > 0) {
            bfsr.stop = stop;
            copy_if(buses_.begin(), buses_.end(), back_inserter(bfsr.buses), [this, &stop](const string &bus) {
                return buses_for_stop_.at(stop).count(bus) > 0;
            });
        }
        return bfsr;
    }

    StopsForBusResponse GetStopsForBus(const string &bus) const {
        // Реализуйте этот метод
        StopsForBusResponse sfbr;

        if (stops_.count(bus) > 0) {
            sfbr.bus = bus;
            for (const string &stop : stops_.at(bus)) {
                sfbr.stop_buses.push_back({stop, {}});
                copy_if(buses_.begin(), buses_.end(), back_inserter(sfbr.stop_buses.back().buses), [this, &stop, &sfbr](const string &bus_for_stop) {
                    return (buses_for_stop_.at(stop).count(bus_for_stop) > 0) && (bus_for_stop != sfbr.bus);
                });
            }
        }
        return sfbr;
    }

    AllBusesResponse GetAllBuses() const {
        // Реализуйте этот метод
        AllBusesResponse abr;

        abr.buses = buses_;
        sort(abr.buses.begin(), abr.buses.end());
        abr.stops = stops_;
        return abr;
    }

private:
    vector<string> buses_;
    map<string, vector<string>> stops_;
    map<string, set<string>> buses_for_stop_;
};

// Реализуйте функции и классы, объявленные выше, чтобы эта функция main
// решала задачу "Автобусные остановки"
void TestReadingQuery() {
    Query test_query;
    istringstream input_str;

    input_str.str("NEW_BUS 32 3 Tolstopaltsevo Marushkino Vnukovo"s);
    input_str >> test_query;
    assert((test_query.type == QueryType::NewBus) && (test_query.bus == "32"s) && (test_query.stops.size() == 3) && (test_query.stops[0] == "Tolstopaltsevo"s) && (test_query.stops[1] == "Marushkino"s) && (test_query.stops[2] == "Vnukovo"s));

    /* Проверка наложения содержимого Query q.stops */
    input_str.str(""s);
    input_str.clear();
    input_str.str("NEW_BUS 32 3 Smolnaya Marushkino Vnukovo"s);
    input_str >> test_query;
    assert((test_query.type == QueryType::NewBus) && (test_query.bus == "32"s) && (test_query.stops.size() == 3) && (test_query.stops[0] == "Smolnaya"s) && (test_query.stops[1] == "Marushkino"s) && (test_query.stops[2] == "Vnukovo"s));

    input_str.str(""s);
    input_str.clear();
    input_str.str("BUSES_FOR_STOP Vnukovo"s);
    input_str >> test_query;
    assert((test_query.type == QueryType::BusesForStop) && (test_query.stop == "Vnukovo"s));

    input_str.str(""s);
    input_str.clear();
    input_str.str("STOPS_FOR_BUS 272"s);
    input_str >> test_query;
    assert((test_query.type == QueryType::StopsForBus) && (test_query.bus == "272"s));

    input_str.str(""s);
    input_str.clear();
    input_str.str("ALL_BUSES"s);
    input_str >> test_query;
    assert(test_query.type == QueryType::AllBuses);
}

void TestOutputBusesForStop() {
    BusesForStopResponse test_bfsr;
    ostringstream test_out;

    test_out << test_bfsr;
    assert(test_out.str() == "No stop"s);

    test_out.str(""s);
    test_out.clear();
    test_bfsr.stop = "Vnukovo"s;
    test_bfsr.buses = {"32"s, "first"s};
    test_out << test_bfsr;
    assert(test_out.str() == "Stop Vnukovo: 32 first"s);
}

void TestOutputStopsForBus() {
    StopsForBusResponse test_sfbr;
    ostringstream test_out;

    test_out << test_sfbr;
    assert(test_out.str() == "No bus"s);

    test_out.str(""s);
    test_out.clear();
    test_sfbr.bus = "one"s;
    test_sfbr.stop_buses = {{"Marushkino"s, {"32k"s, "first"s, "55"s}},
                            {"Liman"s, {"zero"s}}};
    test_out << test_sfbr;
    assert(test_out.str() == "Stop Marushkino: 32k first 55\nStop Liman: zero"s);

    test_out.str(""s);
    test_out.clear();
    test_sfbr.bus = "one"s;
    test_sfbr.stop_buses = {{"Vityazevo"s, {}}};
    test_out << test_sfbr;
    assert(test_out.str() == "Stop Vityazevo: no interchange"s);
}

void TestOutputAllBusses() {
    AllBusesResponse test_abr;
    ostringstream test_out;

    test_out << test_abr;
    assert(test_out.str() == "No buses"s);

    test_out.str(""s);
    test_out.clear();
    test_abr.buses = {"272"s, "32k"s, "first"s};
    test_abr.stops = {{"272"s, {"Vnukovo"s, "Marushkino"s, "Liman"s}},
                      {"32k"s, {"Vnukovo"s, "zero"s}},
                      {"first"s, {"Liman"s, "zero"s}}};
    test_out << test_abr;
    assert(test_out.str() == "Bus 272: Vnukovo Marushkino Liman\nBus 32k: Vnukovo zero\nBus first: Liman zero"s);
}
void TestGetAllBuses() {
    BusManager test_bm;
    AllBusesResponse test_abr;

    test_abr = test_bm.GetAllBuses();
    assert(test_abr.buses.empty() && test_abr.stops.empty());

    vector<string> vec_str_buses = {"32"s, "32K"s};
    vector<string> vec_str_stops0 = {"Tolstopaltsevo"s, "Marushkino"s, "Vnukovo"s};
    vector<string> vec_str_stops1 = {"Tolstopaltsevo"s, "Marushkino"s, "Vnukovo"s, "Peredelkino"s, "Solntsevo"s, "Skolkovo"s};
    test_bm.AddBus(vec_str_buses[1], vec_str_stops1);
    test_bm.AddBus(vec_str_buses[0], vec_str_stops0);
    test_abr = test_bm.GetAllBuses();
    assert((test_abr.buses == vec_str_buses) && (test_abr.stops.at(vec_str_buses[0]) == vec_str_stops0) && (test_abr.stops.at(vec_str_buses[1]) == vec_str_stops1));
}
void TestGetBusesForStop() {
    BusManager test_bm;
    BusesForStopResponse test_bfsr;

    test_bfsr = test_bm.GetBusesForStop("stop"s);
    assert(test_bfsr.stop.empty() && test_bfsr.buses.empty());

    vector<string> vec_str_buses = {"32"s, "32K"s};
    vector<string> vec_str_stops0 = {"Tolstopaltsevo"s, "Marushkino"s, "Vnukovo"s};
    vector<string> vec_str_stops1 = {"Tolstopaltsevo"s, "Marushkino"s, "Vnukovo"s, "Peredelkino"s, "Solntsevo"s, "Skolkovo"s};
    test_bm.AddBus(vec_str_buses[1], vec_str_stops1);
    test_bfsr = test_bm.GetBusesForStop("Tolstopaltsevo"s);
    vector<string> vec_str_buses1 = {"32K"s};
    assert(test_bfsr.stop == "Tolstopaltsevo"s && test_bfsr.buses == vec_str_buses1);

    test_bm.AddBus(vec_str_buses[0], vec_str_stops0);
    vec_str_buses1 = {"32K"s, "32"s};
    test_bfsr = test_bm.GetBusesForStop("Tolstopaltsevo"s);
    assert(test_bfsr.stop == "Tolstopaltsevo"s && test_bfsr.buses == vec_str_buses1);
}
void TestGetStopsForBus() {
    BusManager test_bm;
    StopsForBusResponse test_sfbr;

    test_sfbr = test_bm.GetStopsForBus("bus"s);
    assert(test_sfbr.bus.empty() && test_sfbr.stop_buses.empty());

    vector<string> vec_str_buses = {"32"s, "32K"s};
    vector<string> vec_str_stops0 = {"Tolstopaltsevo"s, "Vnukovo"s};
    vector<string> vec_str_stops1 = {"Tolstopaltsevo"s, "Vnukovo"s, "Peredelkino"s};
    test_bm.AddBus(vec_str_buses[1], vec_str_stops1);
    test_bm.AddBus(vec_str_buses[0], vec_str_stops0);
    test_sfbr = test_bm.GetStopsForBus(vec_str_buses[1]);
    assert(test_sfbr.bus == vec_str_buses[1] && test_sfbr.stop_buses.at(0).stop == vec_str_stops1[0] && test_sfbr.stop_buses.at(1).stop == vec_str_stops1[1]);
}

// Реализуйте функции и классы, объявленные выше, чтобы эта функция main
// решала задачу "Автобусные остановки"

int main() {
    int query_count;
    Query q;

    cin >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        cin >> q;
        switch (q.type) {
        case QueryType::NewBus:
            bm.AddBus(q.bus, q.stops);
            break;
        case QueryType::BusesForStop:
            cout << bm.GetBusesForStop(q.stop) << endl;
            break;
        case QueryType::StopsForBus:
            cout << bm.GetStopsForBus(q.bus) << endl;
            break;
        case QueryType::AllBuses:
            cout << bm.GetAllBuses() << endl;
            break;
        }
    }
}