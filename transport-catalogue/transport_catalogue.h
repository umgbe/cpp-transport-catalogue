#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <set>

#include "domain.h"

namespace transportCatalogue {

namespace base {

class TransportCatalogue {

public:

    TransportCatalogue() = default;

    void AddStop(const requestsToFill::StopInfo& r);
    void AddBus(const requestsToFill::BusInfo& r);

    answersFromBase::BusInfo GetBus(const requestsToSearch::BusInfo& r);
    answersFromBase::StopInfo GetStop(const requestsToSearch::StopInfo& r);

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

}