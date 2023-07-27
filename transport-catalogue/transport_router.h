#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"

namespace transportRouter {

using namespace transportCatalogue;

class TransportRouter {

public:

    explicit TransportRouter(const base::TransportCatalogue& transport_catalogue);

    void SetRoutingSettings(const RoutingSettings& rs);
    void BuildGraph();

    RouterResponce GetRoute(const requestToRouter& r) const;

    friend class serialization::Serializer;

private:

    const base::TransportCatalogue& tc;

    /*  Каждая остановка в графе хранится в виде пары из двух подостановок.
        Первая представляет "остановку старта", которая требует ожидания автобуса.
        Вторая представляет "остановку маршрута", она подразумевает, что мы
        уже находимся в автобусе и можем ехать к следующей остановке немедленно.     */

    RoutingSettings routing_settings;

    struct StopInGraph {
        const Stop* real_stop;
        graph::VertexId id;
    };

    struct RouteInGraph {
        enum class RouteType { WAIT, BUS };
        RouteType type;
        double time;
        const Stop* real_stop;
        const Bus* real_bus;
        int stops_count;
        graph::EdgeId id;
    };

    std::unordered_map<const Stop*, std::pair<StopInGraph, StopInGraph>> stops_in_graph;
    std::unordered_map<graph::EdgeId, RouteInGraph> edges_to_routes;
    

    graph::DirectedWeightedGraph<double> routing_graph;
    
    std::unique_ptr<graph::Router<double>> router;

};

}