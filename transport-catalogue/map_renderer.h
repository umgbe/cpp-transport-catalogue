#pragma once

#include <deque>
#include <map>

#include "domain.h"

namespace mapRenderer {

using namespace transportCatalogue;

class MapRenderer {
public:

    MapRenderer() = default;

    void AddStop(const requestsToFill::StopInfo& r);
    void AddBus(const requestsToFill::BusInfo& r);
    void SetRenderSettings(const RenderSettings& rs);

    MapRendererResponse GetMap(const requestToMapRenderer& r);

private:

    class SphereProjector;

    svg::Document BuildMap();
    SphereProjector BuildSphereProjector();
    void AddLines(svg::Document& document, const SphereProjector& proj);
    void AddLineNames(svg::Document& document, const SphereProjector& proj);
    void AddStationCircles(svg::Document& document, const SphereProjector& proj);
    void AddStationNames(svg::Document& document, const SphereProjector& proj);

    std::deque<Stop> stops;
    std::map<std::string_view, Stop*> stopname_to_stop;
    std::deque<Bus> buses;
    std::map<std::string_view, Bus*> busname_to_bus;
    std::unordered_map<Stop*, std::vector<Bus*>> stops_to_buses;
    RenderSettings render_settings;

};

}