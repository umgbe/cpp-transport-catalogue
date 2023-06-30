#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <set>
#include <memory>

#include "domain.h"

#include "router.h"

namespace transportCatalogue {

namespace base {

class TransportCatalogue {

public:

    TransportCatalogue() = default;

    void AddRoutingSettings(const RoutingSettings& rs);
    void AddStop(const requestsToFill::StopInfo& r);
    void AddBus(const requestsToFill::BusInfo& r);

    answersFromBase::BusInfo GetBus(const requestsToSearch::BusInfo& r);
    answersFromBase::StopInfo GetStop(const requestsToSearch::StopInfo& r);
    answersFromBase::RouteInfo GetRoute(const requestsToSearch::RouteInfo& r);

    void BuildGraph();

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

    /*  Каждая остановка в графе хранится в виде пары из двух подостановок.
        Первая представляет "остановку старта", которая требует ожидания автобуса.
        Вторая представляет "остановку маршрута", она подразумевает, что мы
        уже находимся в автобусе и можем ехать к следующей остановке немедленно.     */

    RoutingSettings routing_settings;

    struct StopInGraph {
        Stop* real_stop;
        graph::VertexId id;
    };

    struct RouteInGraph {
        enum class RouteType { WAIT, BUS };
        RouteType type;
        double time;
        Stop* real_stop;
        Bus* real_bus;
        int stops_count;
        graph::EdgeId id;
    };

    std::unordered_map<Stop*, std::pair<StopInGraph, StopInGraph>> stops_in_graph;
    std::unordered_map<graph::VertexId, StopInGraph> vertexes_to_stops;
    std::unordered_map<graph::EdgeId, RouteInGraph> edges_to_routes;
    

    graph::DirectedWeightedGraph<double> routing_graph;
    
    std::unique_ptr<graph::Router<double>> router;

};

}

}