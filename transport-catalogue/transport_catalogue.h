#pragma once

#include <string>
#include <vector>
#include <map>
#include <deque>
#include <set>
#include <memory>

#include "domain.h"

namespace transportCatalogue {

namespace base {

class TransportCatalogue {

public:

    TransportCatalogue() = default;

    void AddStop(const requestsToFill::StopInfo& r);
    void AddBus(const requestsToFill::BusInfo& r);

    answersFromBase::BusInfo GetBus(const requestsToSearch::BusInfo& r) const;
    answersFromBase::StopInfo GetStop(const requestsToSearch::StopInfo& r) const;

    friend class mapRenderer::MapRenderer;
    friend class transportRouter::TransportRouter;

    friend class serialization::Serializer;

private:

    std::deque<Stop> stops;
    std::map<std::string_view, Stop*> stopname_to_stop;

    std::deque<Bus> buses;
    std::map<std::string_view, Bus*> busname_to_bus;

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