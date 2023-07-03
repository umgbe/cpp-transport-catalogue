#include "transport_router.h"

using namespace transportRouter;

TransportRouter::TransportRouter(const base::TransportCatalogue& transport_catalogue) 
    : tc(transport_catalogue) { }

void TransportRouter::SetRoutingSettings(const RoutingSettings& rs) {
    routing_settings = rs;
}

void TransportRouter::BuildGraph() {

    graph::VertexId vertex_count = tc.stops.size() * 2;
    routing_graph = graph::DirectedWeightedGraph<double>(vertex_count);
    vertex_count = 0;

    stops_in_graph.clear();
    vertexes_to_stops.clear();
    edges_to_routes.clear();
    router.reset();

    for (const Stop& stop : tc.stops) {
        const Stop* stop_ptr = &stop;
        StopInGraph waiting_stop;
        StopInGraph routing_stop;

        waiting_stop.id = vertex_count;
        ++vertex_count;
        waiting_stop.real_stop = stop_ptr;

        routing_stop.id = vertex_count;
        ++vertex_count;
        routing_stop.real_stop = stop_ptr;

        vertexes_to_stops[waiting_stop.id] = waiting_stop;
        vertexes_to_stops[routing_stop.id] = routing_stop;
        stops_in_graph[stop_ptr] = { waiting_stop, routing_stop };

        graph::Edge<double> new_edge;
        new_edge.from = waiting_stop.id;
        new_edge.to = routing_stop.id;
        new_edge.weight = routing_settings.bus_wait_time;
        graph::EdgeId new_edge_id = routing_graph.AddEdge(new_edge);

        RouteInGraph new_route;
        new_route.type = RouteInGraph::RouteType::WAIT;
        new_route.real_stop = stop_ptr;
        new_route.time = routing_settings.bus_wait_time;
        new_route.id = new_edge_id;
        edges_to_routes[new_edge_id] = new_route;
    }

    for (const Bus& bus : tc.buses) {
        for (size_t i = 0; i < bus.stops.size(); ++i) {
            
            StopInGraph current_stop = stops_in_graph[bus.stops[i]].second;
            
            int final_stop_number;
            if (!bus.roundtrip && (i < (bus.stops.size() / 2))) {
                final_stop_number = bus.stops.size() / 2;
            } else {
                final_stop_number = bus.stops.size() - 1;
            }

            if (i == final_stop_number) {
                continue;
            }

            double total_time = 0.0;
            int stops_count = 0;

            for (int k = (i + 1); k <=final_stop_number; ++k) {
                int distance_in_meters = (tc.distances.at({bus.stops[k - 1], bus.stops[k]}));
                total_time += (distance_in_meters / 1000.0) / routing_settings.bus_velocity * 60;
                stops_count += 1;

                graph::Edge<double> new_edge;
                new_edge.from = current_stop.id;
                new_edge.to = stops_in_graph[bus.stops[k]].first.id;
                new_edge.weight = total_time;
                graph::EdgeId new_edge_id = routing_graph.AddEdge(new_edge);

                RouteInGraph new_route;
                new_route.type = RouteInGraph::RouteType::BUS;
                new_route.real_bus = &bus;
                new_route.time = total_time;
                new_route.stops_count = stops_count;
                new_route.id = new_edge_id;
                edges_to_routes[new_edge_id] = new_route;
            }
        }
    }

    router = std::make_unique<graph::Router<double>>(routing_graph);

}

RouterResponce TransportRouter::GetRoute(const requestToRouter& r) const {

    graph::VertexId start_point = stops_in_graph.at(tc.stopname_to_stop.at(r.from)).first.id;
    graph::VertexId finish_point = stops_in_graph.at(tc.stopname_to_stop.at(r.to)).first.id;

    std::optional<graph::Router<double>::RouteInfo> route = router->BuildRoute(start_point, finish_point);

    RouterResponce result;

    result.id = r.id;
    if (!route) {
        result.no_route = true;
        return result;
    }

    result.no_route = false;
    result.total_time = route.value().weight;
    result.items.reserve(route.value().edges.size());

    for (const graph::EdgeId edge : route.value().edges) {
        RouteInGraph route_element = edges_to_routes.at(edge);
        if (route_element.type == RouteInGraph::RouteType::WAIT) {
            RouterResponce::Wait wait;
            wait.time = route_element.time;
            wait.stop_name = route_element.real_stop->name;
            result.items.push_back(std::move(wait));
        } else if (route_element.type == RouteInGraph::RouteType::BUS) {
            RouterResponce::Bus bus;
            bus.time = route_element.time;
            bus.bus_name = route_element.real_bus->name;
            bus.span_count = route_element.stops_count;
            result.items.push_back(std::move(bus));
        }
    }

    return result;

}