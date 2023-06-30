#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "svg.h"

namespace transportCatalogue {

struct Stop {
    std::string name;
    double latitude;
    double longitude;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    bool roundtrip;
};

namespace requestsToFill {

struct StopInfo {
    std::string name;
    double latitude;
    double longitude;
    std::unordered_map<std::string, int> distances;
};

struct BusInfo {
    std::string name;
    std::vector<std::string> stops_names;
    bool roundtrip;
};

}

namespace requestsToSearch {

struct StopInfo {
    int id;
    std::string name;
};

struct BusInfo {
    int id;
    std::string name;
};

struct RouteInfo {
    int id;
    std::string from;
    std::string to;

};

}

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

namespace answersFromBase {

struct BusInfo {
    bool no_bus;
    std::string name;
    int stops_count;
    int unique_stops_count;
    double distance;
    double curvature;
    int id;
};

struct StopInfo {
    bool no_stop;
    bool no_buses;
    std::string name;
    std::set<std::string> buses;
    int id;
};

struct RouteInfo {
    int id;
    bool no_route;
    double total_time;

    struct Wait {
        std::string stop_name;
        double time;
    };

    struct Bus {
        std::string bus_name;
        int span_count;
        double time;
    };

    std::vector<std::variant<Wait, Bus>> items; 

};

}

}

namespace mapRenderer {

    struct RenderSettings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        int bus_label_font_size;
        std::pair<double, double> bus_label_offset;
        int stop_label_font_size;
        std::pair<double, double> stop_label_offset;
        svg::Color underlayer_color;
        double underlayer_width;
        std::vector<svg::Color> color_palette;
    };

    struct requestToMapRenderer {
        int id;
    };

    struct MapRendererResponse {
        int id;
        std::string map;
    };
}