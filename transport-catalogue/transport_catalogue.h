#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <set>

namespace transportCatalogue {

namespace requestsToBase {

struct AddStop {
    std::string name;
    double latitude;
    double longitude;

    //для тестов
    bool operator==(const AddStop& other) {
        return (name == other.name) && (latitude == other.latitude) && (longitude == other.longitude);
    }

};

struct AddStopDistance {
    std::string name_first;
    std::string name_second;
    int distance;

    //для тестов
    bool operator==(const AddStopDistance& other) {
        return (name_first == other.name_first) && (name_second == other.name_second) && (distance == other.distance);
    }
};

struct AddBus {
    std::string name;
    std::vector<std::string> stops_names;

    //для тестов
    bool operator==(const AddBus& other) {
        return (name == other.name) && (stops_names == other.stops_names);
    }
};

struct GetBus {
    std::string name;

    //для тестов
    bool operator==(const GetBus& other) {
        return (name == other.name);
    }
};

struct GetStop {
    std::string name;

    //для тестов
    bool operator==(const GetStop& other) {
        return (name == other.name);
    }
};

}

namespace requestsFromBase {

struct GetBus {
    bool bus_found;
    std::string name;
    int stops_count;
    int unique_stops_count;
    double distance;
    double curvature;
};

struct GetStop {
    bool no_stop;
    bool no_buses;
    std::string name;
    std::set<std::string> buses;
};

}

namespace base {

class TransportCatalogue {

public:

    TransportCatalogue() = default;

    void AddStop(const requestsToBase::AddStop& r);

    void AddBus(const requestsToBase::AddBus& r);

    void AddStopDistance(const requestsToBase::AddStopDistance& r);

    requestsFromBase::GetBus GetBus(const requestsToBase::GetBus& r);

    requestsFromBase::GetStop GetStop(const requestsToBase::GetStop& r);

    struct Stop {
        std::string name;
        double latitude;
        double longitude;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
    };

private:

    std::deque<Stop> stops;

    std::unordered_map<std::string_view, Stop*> stopname_to_stop;

    std::deque<Bus> buses;

    std::unordered_map<std::string_view, Bus*> busname_to_bus;

    std::unordered_map<Stop*, std::vector<Bus*>> stops_to_buses;

    struct DoublePointerHasher {
        size_t operator()(const std::pair<Stop*, Stop*>& p) const {
            return std::hash<Stop*>{}(p.first) + std::hash<Stop*>{}(p.second) * 37;
        }
    };

    std::unordered_map<std::pair<Stop*, Stop*>, int, DoublePointerHasher> distances;

};

}

namespace tests {

void TestTransportCatalogue();

}

}