#include "serialization.h"

#include "map_renderer.h"
#include "transport_router.h"

#include "graph.h"

#include <fstream>

#include <iostream>

using namespace transportCatalogue::serialization;

Serializer::Serializer(const SerializationSettings& ss) 
    : serialization_settings(ss) {
}

void Serializer::SerializeTransportCatalogue(const base::TransportCatalogue& tc, transportCatalogueSerialized::TransportCatalogue& output_package) {
    int index = 0;
    for (const Stop& stop : tc.stops) {
        transportCatalogueSerialized::Stop s_stop;
        stops_unique_indexes.insert({&stop, index});
        s_stop.set_unique_index(index);
        s_stop.set_name(stop.name);
        s_stop.set_latitude(stop.latitude);
        s_stop.set_longitude(stop.longitude);

        output_package.add_stops();
        *output_package.mutable_stops(index) = std::move(s_stop);
        ++index;
    }

    index = 0;
    for (const Bus& bus : tc.buses) {
        transportCatalogueSerialized::Bus s_bus;
        buses_unique_indexes.insert({&bus, index});
        s_bus.set_unique_index(index);
        s_bus.set_name(bus.name);
        for (const Stop* stop : bus.stops) {
            s_bus.add_stops(stops_unique_indexes[stop]);
        }
        s_bus.set_roundtrip(bus.roundtrip);

        output_package.add_buses();
        *output_package.mutable_buses(index) = (s_bus);
        ++index;
    }

    index = 0;
    for (const auto [stops_pair, distance] : tc.distances) {
        transportCatalogueSerialized::Distance s_distance;
        s_distance.set_start_stop(stops_unique_indexes[stops_pair.first]);
        s_distance.set_finish_stop(stops_unique_indexes[stops_pair.second]);
        s_distance.set_distance(distance);

        output_package.add_distances();
        *output_package.mutable_distances(index) = std::move(s_distance);
        ++index;
    }
}

void Serializer::SerializeMapRenderer(const mapRenderer::MapRenderer& mr, transportCatalogueSerialized::TransportCatalogue& output_package) {
    mapRenderer::RenderSettings render_settings = mr.GetRenderSettings();

    transportCatalogueSerialized::RenderSettings s_render_settings;
    s_render_settings.set_width(render_settings.width);
    s_render_settings.set_height(render_settings.height);
    s_render_settings.set_padding(render_settings.padding);
    s_render_settings.set_line_width(render_settings.line_width);
    s_render_settings.set_stop_radius(render_settings.stop_radius);
    s_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
    s_render_settings.set_bus_label_offset_first(render_settings.bus_label_offset.first);
    s_render_settings.set_bus_label_offset_second(render_settings.bus_label_offset.second);
    s_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
    s_render_settings.set_stop_label_offset_first(render_settings.stop_label_offset.first);
    s_render_settings.set_stop_label_offset_second(render_settings.stop_label_offset.second);
    s_render_settings.set_underlayer_width(render_settings.underlayer_width);

    int string_count = 0;
    int rgb_count = 0;
    int rgba_count = 0;
    int sum = 0;

    if (std::holds_alternative<std::monostate>(render_settings.underlayer_color)) {
        s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::MONOSTATE);
    } else if (std::holds_alternative<std::string>(render_settings.underlayer_color)) {
        s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::STRING);
        s_render_settings.add_color_palette_string();
        *s_render_settings.mutable_color_palette_string(string_count) = std::get<std::string>(render_settings.underlayer_color);
        ++string_count;
    } else if (std::holds_alternative<svg::Rgb>(render_settings.underlayer_color)) {
        s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::RGB);
        s_render_settings.add_color_palette_rgb();
        svg::Rgb temp_color = std::get<svg::Rgb>(render_settings.underlayer_color);
        transportCatalogueSerialized::Rgb s_rgb;
        s_rgb.set_red(temp_color.red);
        s_rgb.set_green(temp_color.green);
        s_rgb.set_blue(temp_color.blue);
        *s_render_settings.mutable_color_palette_rgb(rgb_count) = std::move(s_rgb);
        ++rgb_count;
    } else {
        s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::RGBA);
        s_render_settings.add_color_palette_rgba();
        svg::Rgba temp_color = std::get<svg::Rgba>(render_settings.underlayer_color);
        transportCatalogueSerialized::Rgba s_rgba;
        s_rgba.set_red(temp_color.red);
        s_rgba.set_green(temp_color.green);
        s_rgba.set_blue(temp_color.blue);
        s_rgba.set_opacity(temp_color.opacity);
        *s_render_settings.mutable_color_palette_rgba(rgba_count) = std::move(s_rgba);
        ++rgba_count;
    }
    ++sum;

    for (const svg::Color& color : render_settings.color_palette) {
        if (std::holds_alternative<std::monostate>(color)) {
            s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::MONOSTATE);
        } else if (std::holds_alternative<std::string>(color)) {
            s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::STRING);
            s_render_settings.add_color_palette_string();
            *s_render_settings.mutable_color_palette_string(string_count) = std::get<std::string>(color);
            ++string_count;
        } else if (std::holds_alternative<svg::Rgb>(color)) {
            s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::RGB);
            s_render_settings.add_color_palette_rgb();
            svg::Rgb temp_color = std::get<svg::Rgb>(color);
            transportCatalogueSerialized::Rgb s_rgb;
            s_rgb.set_red(temp_color.red);
            s_rgb.set_green(temp_color.green);
            s_rgb.set_blue(temp_color.blue);
            *s_render_settings.mutable_color_palette_rgb(rgb_count) = std::move(s_rgb);
            ++rgb_count;
        } else {
            s_render_settings.add_color_types(transportCatalogueSerialized::ColorTypes::RGBA);
            s_render_settings.add_color_palette_rgba();
            svg::Rgba temp_color = std::get<svg::Rgba>(color);
            transportCatalogueSerialized::Rgba s_rgba;
            s_rgba.set_red(temp_color.red);
            s_rgba.set_green(temp_color.green);
            s_rgba.set_blue(temp_color.blue);
            s_rgba.set_opacity(temp_color.opacity);
            *s_render_settings.mutable_color_palette_rgba(rgba_count) = std::move(s_rgba);
            ++rgba_count;
        }
        ++sum;
    }

    *output_package.mutable_render_settings() = std::move(s_render_settings);
}

void Serializer::SerializeTransportRouter(const transportRouter::TransportRouter& tr, transportCatalogueSerialized::TransportCatalogue& output_package) {
    
    transportCatalogueSerialized::TransportRouter s_transport_router;

    transportCatalogueSerialized::RoutingSettings s_routing_settings;
    s_routing_settings.set_bus_wait_time(tr.routing_settings.bus_wait_time);
    s_routing_settings.set_bus_velocity(tr.routing_settings.bus_velocity);

    *s_transport_router.mutable_routing_settings() = std::move(s_routing_settings);

    int index = 0;
    transportCatalogueSerialized::StopsInGraphMap s_stops_in_graph;
    for (const auto [stop_ptr, stops_pair] : tr.stops_in_graph) {

        s_stops_in_graph.add_unique_id(stops_unique_indexes[stop_ptr]);        
        
        transportCatalogueSerialized::StopInGraph s_first;
        transportCatalogueSerialized::StopInGraph s_second;
        s_first.set_real_stop(stops_unique_indexes[stops_pair.first.real_stop]);
        s_second.set_real_stop(stops_unique_indexes[stops_pair.second.real_stop]);
        s_first.set_id(stops_pair.first.id);
        s_second.set_id(stops_pair.second.id);

        s_stops_in_graph.add_first();
        *s_stops_in_graph.mutable_first(index) = std::move(s_first);
        s_stops_in_graph.add_second();
        *s_stops_in_graph.mutable_second(index) = std::move(s_second);

        ++index;
    }

    *s_transport_router.mutable_stops_in_graph() = std::move(s_stops_in_graph);

    index = 0;
    transportCatalogueSerialized::EdgesToRoutesMap s_edges_to_routes;
    for (const auto [edge_id, route_in_graph] : tr.edges_to_routes) {

        s_edges_to_routes.add_id(edge_id);

        transportCatalogueSerialized::RouteInGraph s_route_in_graph;

        if (route_in_graph.type == transportRouter::TransportRouter::RouteInGraph::RouteType::WAIT) {
            s_route_in_graph.set_type(true);
        } else {
            s_route_in_graph.set_type(false);
        }
        s_route_in_graph.set_time(route_in_graph.time);
        s_route_in_graph.set_real_stop(stops_unique_indexes[route_in_graph.real_stop]);
        s_route_in_graph.set_real_bus(buses_unique_indexes[route_in_graph.real_bus]);
        s_route_in_graph.set_stops_count(route_in_graph.stops_count);
        s_route_in_graph.set_id(route_in_graph.id);

        s_edges_to_routes.add_routes();
        *s_edges_to_routes.mutable_routes(index) = std::move(s_route_in_graph);
        ++index;
    }

    *s_transport_router.mutable_edges_to_routes() = std::move(s_edges_to_routes);
    
    transportCatalogueSerialized::Graph s_graph;

    index = 0;
    for (const graph::Edge<double>& edge : tr.routing_graph.edges_) {

        transportCatalogueSerialized::Edge s_edge;
        s_edge.set_from(edge.from);
        s_edge.set_to(edge.to);
        s_edge.set_weight(edge.weight);

        s_graph.add_edges_();
        *s_graph.mutable_edges_(index) = std::move(s_edge);
        ++index;
    }

    index = 0;
    for (const std::vector<graph::EdgeId>& incidence_list : tr.routing_graph.incidence_lists_) {
        transportCatalogueSerialized::IncidenceList s_incidence_list;
        for (const graph::EdgeId& edge_id : incidence_list) {
            s_incidence_list.add_id(edge_id);
        }
        s_graph.add_incidence_lists_();
        *s_graph.mutable_incidence_lists_(index) = std::move(s_incidence_list);
        ++index;
    }

    *s_transport_router.mutable_routing_graph() = std::move(s_graph);

    index = 0;
    transportCatalogueSerialized::Router s_router;
    for (const std::vector<std::optional<graph::Router<double>::RouteInternalData>>& vector_routes : tr.router->routes_internal_data_) {
        transportCatalogueSerialized::RouteInternalDataVector s_vector_routes;
        int internal_index = 0;
        for (const std::optional<graph::Router<double>::RouteInternalData>& route_internal_data : vector_routes) {
            transportCatalogueSerialized::RouteInternalData s_route_internal_data;
            if (route_internal_data.has_value()) {
                s_vector_routes.add_route_internal_data_exists(true);

                s_route_internal_data.set_weight(route_internal_data.value().weight);
                if (route_internal_data.value().prev_edge.has_value()) {
                    s_route_internal_data.set_prev_edge_exists(true);
                    s_route_internal_data.set_prev_edge(route_internal_data.value().prev_edge.value());
                } else {
                    s_route_internal_data.set_prev_edge_exists(false);
                }  

            } else {
                s_vector_routes.add_route_internal_data_exists(false);
            }

            s_vector_routes.add_route_internal_data();
            *s_vector_routes.mutable_route_internal_data(internal_index) = std::move(s_route_internal_data);
            ++internal_index;
        }

        s_router.add_routes_internal_data_();
        *s_router.mutable_routes_internal_data_(index) = std::move(s_vector_routes);
        ++index;
    }

    *s_transport_router.mutable_router() = std::move(s_router);

    *output_package.mutable_transport_router() = std::move(s_transport_router);
}

void Serializer::DeserializeTransportCatalogue(base::TransportCatalogue& tc, transportCatalogueSerialized::TransportCatalogue& input_package) {
    
    for (int i = 0; i < input_package.stops_size(); ++i) {
        transportCatalogueSerialized::Stop& s_stop = *input_package.mutable_stops(i);
        tc.stops.push_back({s_stop.name(), s_stop.latitude(), s_stop.longitude()});
        Stop* ptr = &tc.stops.back();
        tc.stopname_to_stop.insert({tc.stops.back().name, ptr});
        unique_indexes_stops.insert({s_stop.unique_index(), ptr});
        tc.stops_to_buses.insert({ptr, std::vector<Bus*>()});
    }

    for (int i = 0; i < input_package.buses_size(); ++i) {
        transportCatalogueSerialized::Bus& s_bus = *input_package.mutable_buses(i);
        Bus new_bus;
        new_bus.name = s_bus.name();
        for (int k = 0; k < s_bus.stops_size(); ++k) {
            int stop_index = s_bus.stops(k);
            new_bus.stops.push_back(unique_indexes_stops[stop_index]);
        }
        new_bus.roundtrip = s_bus.roundtrip();
        tc.buses.push_back(std::move(new_bus));
        Bus* ptr = &tc.buses.back();
        for (int k = 0; k < s_bus.stops_size(); ++k) {
            int stop_index = s_bus.stops(k);
            tc.stops_to_buses[unique_indexes_stops[stop_index]].push_back(ptr);
        }
        unique_indexes_buses.insert({s_bus.unique_index(), ptr});
        tc.busname_to_bus.insert({tc.buses.back().name, ptr});
    }

    for (int i = 0; i < input_package.distances_size(); ++i) {
        transportCatalogueSerialized::Distance& s_distance = *input_package.mutable_distances(i);
        Stop* ptr = unique_indexes_stops[s_distance.start_stop()];
        Stop* other_ptr = unique_indexes_stops[s_distance.finish_stop()];
        tc.distances[{ptr, other_ptr}] = s_distance.distance();
    }
}

void Serializer::DeserializeMapRenderer(mapRenderer::MapRenderer& mr, transportCatalogueSerialized::TransportCatalogue& input_package) {
    
    mapRenderer::RenderSettings render_settings;
    transportCatalogueSerialized::RenderSettings& s_render_settings = *input_package.mutable_render_settings();

    render_settings.width = s_render_settings.width();
    render_settings.height = s_render_settings.height();
    render_settings.padding = s_render_settings.padding();
    render_settings.line_width = s_render_settings.line_width();
    render_settings.stop_radius = s_render_settings.stop_radius();
    render_settings.bus_label_font_size = s_render_settings.bus_label_font_size();
    render_settings.bus_label_offset = {s_render_settings.bus_label_offset_first(), s_render_settings.bus_label_offset_second()};
    render_settings.stop_label_font_size = s_render_settings.stop_label_font_size();
    render_settings.stop_label_offset = {s_render_settings.stop_label_offset_first(), s_render_settings.stop_label_offset_second()};    
    render_settings.underlayer_width = s_render_settings.underlayer_width();

    int string_count = 0;
    int rgb_count = 0;
    int rgba_count = 0;
    for (int i = 0; i < s_render_settings.color_types_size(); ++i) {
        transportCatalogueSerialized::ColorTypes type = s_render_settings.color_types(i);
        svg::Color color;
        if (type == transportCatalogueSerialized::ColorTypes::MONOSTATE) {
            color = std::monostate{};
        } else if (type == transportCatalogueSerialized::ColorTypes::STRING) {
            color = *s_render_settings.mutable_color_palette_string(string_count);
            ++string_count;
        } else if (type == transportCatalogueSerialized::ColorTypes::RGB) {   
            transportCatalogueSerialized::Rgb& s_temp_color = *s_render_settings.mutable_color_palette_rgb(rgb_count);
            svg::Rgb temp_color;
            temp_color.red = s_temp_color.red();
            temp_color.green = s_temp_color.green();
            temp_color.blue = s_temp_color.blue();
            color = std::move(temp_color);
            ++rgb_count;
        } else {
            transportCatalogueSerialized::Rgba& s_temp_color = *s_render_settings.mutable_color_palette_rgba(rgba_count);
            svg::Rgba temp_color;
            temp_color.red = s_temp_color.red();
            temp_color.green = s_temp_color.green();
            temp_color.blue = s_temp_color.blue();
            temp_color.opacity = s_temp_color.opacity();
            color = std::move(temp_color);
            ++rgba_count;
        }
        if (i == 0) {
            render_settings.underlayer_color = std::move(color);
        } else {
            render_settings.color_palette.push_back(std::move(color));
        }
    }

    mr.SetRenderSettings(render_settings);
}

void Serializer::DeserializeTransportRouter(transportRouter::TransportRouter& tr, transportCatalogueSerialized::TransportCatalogue& input_package) {
    
    transportCatalogueSerialized::TransportRouter& s_transport_router = *input_package.mutable_transport_router();

    transportCatalogueSerialized::RoutingSettings& s_routing_settings = *s_transport_router.mutable_routing_settings();
    transportRouter::RoutingSettings routing_settings;
    routing_settings.bus_wait_time = s_routing_settings.bus_wait_time();
    routing_settings.bus_velocity = s_routing_settings.bus_velocity();
    tr.SetRoutingSettings(routing_settings);

    transportCatalogueSerialized::StopsInGraphMap& s_stops_in_graph = *s_transport_router.mutable_stops_in_graph();
    std::unordered_map<const Stop*, std::pair<transportRouter::TransportRouter::StopInGraph, transportRouter::TransportRouter::StopInGraph>> stops_in_graph;
    for (int i = 0; i < s_stops_in_graph.unique_id_size(); ++i) {
        
        const Stop* ptr = unique_indexes_stops[s_stops_in_graph.unique_id(i)];

        transportCatalogueSerialized::StopInGraph& s_first = *s_stops_in_graph.mutable_first(i);
        transportCatalogueSerialized::StopInGraph& s_second = *s_stops_in_graph.mutable_second(i);

        transportRouter::TransportRouter::StopInGraph first;
        transportRouter::TransportRouter::StopInGraph second;

        first.real_stop = unique_indexes_stops[s_first.real_stop()];
        second.real_stop = unique_indexes_stops[s_second.real_stop()];

        first.id = s_first.id();
        second.id = s_second.id();

        stops_in_graph.insert({ptr, {std::move(first), std::move(second)}});        
    }

    tr.stops_in_graph.clear();
    tr.stops_in_graph = std::move(stops_in_graph);

    transportCatalogueSerialized::EdgesToRoutesMap& s_edges_to_routes = *s_transport_router.mutable_edges_to_routes();
    std::unordered_map<graph::EdgeId, transportRouter::TransportRouter::RouteInGraph> edges_to_routes;
    for (int i = 0; i < s_edges_to_routes.id_size(); ++i) {
        graph::EdgeId id = s_edges_to_routes.id(i);

        transportCatalogueSerialized::RouteInGraph& s_route = *s_edges_to_routes.mutable_routes(i);
        transportRouter::TransportRouter::RouteInGraph route;

        if (s_route.type() == true) {
            route.type = transportRouter::TransportRouter::RouteInGraph::RouteType::WAIT;
        } else {
            route.type = transportRouter::TransportRouter::RouteInGraph::RouteType::BUS;
        }
        route.time = s_route.time();
        route.real_stop = unique_indexes_stops[s_route.real_stop()];
        route.real_bus = unique_indexes_buses[s_route.real_bus()];
        route.stops_count = s_route.stops_count();
        route.id = s_route.id();

        edges_to_routes.insert({id, std::move(route)});
    }

    tr.edges_to_routes.clear();
    tr.edges_to_routes = std::move(edges_to_routes);

    transportCatalogueSerialized::Graph& s_graph = *s_transport_router.mutable_routing_graph();

    std::vector<graph::Edge<double>> edges_;
    edges_.reserve(s_graph.edges__size());
    for (int i = 0; i < s_graph.edges__size(); ++i) {
        
        transportCatalogueSerialized::Edge& s_edge = *s_graph.mutable_edges_(i);
        graph::Edge<double> edge;
        edge.from = s_edge.from();
        edge.to = s_edge.to();
        edge.weight = s_edge.weight();

        edges_.push_back(std::move(edge));
    }
    tr.routing_graph.edges_.clear();
    tr.routing_graph.edges_ = std::move(edges_);

    std::vector<std::vector<graph::EdgeId>> incidence_lists_;
    incidence_lists_.reserve(s_graph.incidence_lists__size());
    for (int i = 0; i < s_graph.incidence_lists__size(); ++i) {
        
        transportCatalogueSerialized::IncidenceList& s_incidence_list = *s_graph.mutable_incidence_lists_(i);
        std::vector<graph::EdgeId> incidence_list;
        incidence_list.reserve(s_incidence_list.id_size());
        for (int k = 0; k < s_incidence_list.id_size(); ++k) {
            incidence_list.push_back(s_incidence_list.id(k));
        }
        incidence_lists_.push_back(std::move(incidence_list));

    }
    tr.routing_graph.incidence_lists_.clear();
    tr.routing_graph.incidence_lists_ = std::move(incidence_lists_);
    
    transportCatalogueSerialized::Router& s_router = *s_transport_router.mutable_router();

    std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> routes_internal_data_;
    routes_internal_data_.reserve(s_router.routes_internal_data__size());

    for (int i = 0; i < s_router.routes_internal_data__size(); ++i) {
        transportCatalogueSerialized::RouteInternalDataVector& s_vector_routes = *s_router.mutable_routes_internal_data_(i);
        std::vector<std::optional<graph::Router<double>::RouteInternalData>> vector_routes;
        vector_routes.resize(s_vector_routes.route_internal_data_size());
        for (int k = 0; k < s_vector_routes.route_internal_data_size(); ++k) {
            if (s_vector_routes.route_internal_data_exists(k)) {
                transportCatalogueSerialized::RouteInternalData& s_route = *s_vector_routes.mutable_route_internal_data(k);
                graph::Router<double>::RouteInternalData route;
                route.weight = s_route.weight();
                if (s_route.prev_edge_exists()) {
                    route.prev_edge = s_route.prev_edge();
                }
                vector_routes[k] = std::move(route);
            }
        }
        routes_internal_data_.push_back(std::move(vector_routes));
    }

    tr.router.reset();
    tr.router = std::make_unique<graph::Router<double>>(graph::Router<double>(tr.routing_graph, routes_internal_data_));
}

bool Serializer::Serialize(const base::TransportCatalogue& tc, const mapRenderer::MapRenderer& mr, const transportRouter::TransportRouter& tr) {

    std::ofstream out;

    try {
        out.open(serialization_settings.file, std::ios::binary);
    } catch (...) {
        return false;
    }

    transportCatalogueSerialized::TransportCatalogue output_package;

    SerializeTransportCatalogue(tc, output_package);
    SerializeMapRenderer(mr, output_package);
    SerializeTransportRouter(tr, output_package);
    
    try {
        output_package.SerializeToOstream(&out);
    } catch (...) {
        return false;
    }

    return true;    

}

bool Serializer::Deserialize(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr) {

    std::ifstream in;

    try {
        in.open(serialization_settings.file, std::ios::binary);
    } catch (...) {
        return false;
    }

    transportCatalogueSerialized::TransportCatalogue input_package;
    input_package.ParseFromIstream(&in);

    DeserializeTransportCatalogue(tc, input_package);
    DeserializeMapRenderer(mr, input_package);
    DeserializeTransportRouter(tr, input_package);  
    
    return true;
}