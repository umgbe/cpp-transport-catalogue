#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <set>

namespace transportCatalogue {

namespace requestsToBase {

struct StopInfo {
    std::string name;
    double latitude;
    double longitude;
    std::unordered_map<std::string, int> distances;
};

struct BusInfo {
    std::string name;
    std::vector<std::string> stops_names;
};

}

namespace requestsFromBase {

struct BusInfo {
    bool bus_found;
    std::string name;
    int stops_count;
    int unique_stops_count;
    double distance;
    double curvature;
};

struct StopInfo {
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

    void AddStop(const requestsToBase::StopInfo& r);
    void AddBus(const requestsToBase::BusInfo& r);

    requestsFromBase::BusInfo GetBus(const requestsToBase::BusInfo& r);
    requestsFromBase::StopInfo GetStop(const requestsToBase::StopInfo& r);

private:

    struct Stop {
        std::string name;
        double latitude;
        double longitude;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
    };

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