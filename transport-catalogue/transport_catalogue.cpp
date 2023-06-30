#include "transport_catalogue.h"
#include "geo.h"

#include <sstream>
#include <cassert>
#include <algorithm>

using namespace transportCatalogue;
using namespace transportCatalogue::base;
using namespace std::string_literals;

void TransportCatalogue::AddRoutingSettings(const RoutingSettings& rs) {
    routing_settings = rs;
}

void TransportCatalogue::AddStop(const requestsToFill::StopInfo& r) {
    Stop* ptr;
    if (!stopname_to_stop.count(r.name)) {
        stops.push_back({r.name, r.latitude, r.longitude});
        ptr = &stops.back();
        stopname_to_stop.insert({stops.back().name, ptr});
    } else {
        ptr = stopname_to_stop[r.name];
        ptr->latitude = r.latitude;
        ptr->longitude = r.longitude;
    }
    stops_to_buses.insert({ptr, std::vector<Bus*>()});
    for (const auto [other_stop_name, distance] : r.distances) {
        Stop* other_ptr;
        if (!stopname_to_stop.count(other_stop_name)) {            
            stops.push_back({other_stop_name, 1000.0, 1000.0});
            other_ptr = &stops.back();
            stopname_to_stop.insert({stops.back().name, other_ptr});
        } else {
            other_ptr = stopname_to_stop[other_stop_name];
        }
        distances[{ptr, other_ptr}] = distance;
        if (!distances.count({other_ptr, ptr})) {
            distances[{other_ptr, ptr}] = distance;
        }
    }
}

void TransportCatalogue::AddBus(const requestsToFill::BusInfo& r) {
    Bus new_bus;
    new_bus.name = r.name;
    for (const std::string& stop_name : r.stops_names) {
        new_bus.stops.push_back(stopname_to_stop[stop_name]);
    }
    new_bus.roundtrip = r.roundtrip;
    buses.push_back(std::move(new_bus));
    Bus* ptr = &buses.back();
    for (const std::string& stop_name : r.stops_names) {
        stops_to_buses[stopname_to_stop[stop_name]].push_back(ptr);
    }
    busname_to_bus.insert({buses.back().name, ptr});
}

answersFromBase::BusInfo TransportCatalogue::GetBus(const requestsToSearch::BusInfo& r) {
    answersFromBase::BusInfo result;
    result.id = r.id;
    result.name = r.name;
    if (!busname_to_bus.count(r.name)) {
        result.no_bus = true;
        return result;
    }
    result.no_bus = false;
    Bus* ptr = busname_to_bus[r.name];
    result.stops_count = ptr->stops.size();

    std::vector<Stop*> stops_without_duplicates(ptr->stops.begin(), ptr->stops.end());
    std::sort(stops_without_duplicates.begin(), stops_without_duplicates.end());
    auto last = std::unique(stops_without_duplicates.begin(), stops_without_duplicates.end());
    stops_without_duplicates.erase(last, stops_without_duplicates.end());
    result.unique_stops_count = stops_without_duplicates.size();

    result.distance = 0.0;
    double geo_distance = 0.0;

    for (size_t i = 1; i < ptr->stops.size(); ++i) {
        result.distance += distances[{ptr->stops[i-1], ptr->stops[i]}];
        geo_distance += geo::ComputeDistance({ptr->stops[i-1]->latitude, ptr->stops[i-1]->longitude}, {ptr->stops[i]->latitude, ptr->stops[i]->longitude});
    }

    result.curvature = result.distance / geo_distance;

    return result;
}

answersFromBase::StopInfo TransportCatalogue::GetStop(const requestsToSearch::StopInfo& r) {
    answersFromBase::StopInfo result;
    result.id = r.id;
    result.name = r.name;
    if (!stopname_to_stop.count(r.name)) {
        result.no_stop = true;
        return result;
    }
    result.no_stop = false;
    for (const Bus* bus : stops_to_buses[stopname_to_stop[r.name]]) {
        result.buses.insert(bus->name);
    }
    if (result.buses.empty()) {
        result.no_buses = true;
        return result;
    }
    result.no_buses = false;
    return result;
}

void TransportCatalogue::BuildGraph() {

    graph::VertexId vertex_count = stops.size() * 2;
    routing_graph = graph::DirectedWeightedGraph<double>(vertex_count);
    vertex_count = 0;

    stops_in_graph.clear();
    vertexes_to_stops.clear();
    edges_to_routes.clear();
    router.reset();

    for (Stop& stop : stops) {
        Stop* stop_ptr = &stop;
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

    for (Bus& bus : buses) {
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
                int distance_in_meters = (distances.at({bus.stops[k - 1], bus.stops[k]}));
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

answersFromBase::RouteInfo TransportCatalogue::GetRoute(const requestsToSearch::RouteInfo& r) {

    graph::VertexId start_point = stops_in_graph[stopname_to_stop[r.from]].first.id;
    graph::VertexId finish_point = stops_in_graph[stopname_to_stop[r.to]].first.id;

    std::optional<graph::Router<double>::RouteInfo> route = router->BuildRoute(start_point, finish_point);

    answersFromBase::RouteInfo result;

    result.id = r.id;
    if (!route) {
        result.no_route = true;
        return result;
    }

    result.no_route = false;
    result.total_time = route.value().weight;
    result.items.reserve(route.value().edges.size());

    for (const graph::EdgeId edge : route.value().edges) {
        RouteInGraph route_element = edges_to_routes[edge];
        if (route_element.type == RouteInGraph::RouteType::WAIT) {
            answersFromBase::RouteInfo::Wait wait;
            wait.time = route_element.time;
            wait.stop_name = route_element.real_stop->name;
            result.items.push_back(std::move(wait));
        } else if (route_element.type == RouteInGraph::RouteType::BUS) {
            answersFromBase::RouteInfo::Bus bus;
            bus.time = route_element.time;
            bus.bus_name = route_element.real_bus->name;
            bus.span_count = route_element.stops_count;
            result.items.push_back(std::move(bus));
        }
    }

    return result;

}