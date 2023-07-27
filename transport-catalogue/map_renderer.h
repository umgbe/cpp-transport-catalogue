#pragma once

#include "domain.h"
#include "transport_catalogue.h"

namespace mapRenderer {

using namespace transportCatalogue;

class MapRenderer {
public:

    explicit MapRenderer(const transportCatalogue::base::TransportCatalogue& transport_catalogue);

    void SetRenderSettings(const RenderSettings& rs);
    RenderSettings GetRenderSettings() const;

    MapRendererResponse GetMap(const requestToMapRenderer& r);

private:

    class SphereProjector;

    svg::Document BuildMap();
    SphereProjector BuildSphereProjector();
    void AddLines(svg::Document& document, const SphereProjector& proj);
    void AddLineNames(svg::Document& document, const SphereProjector& proj);
    void AddStationCircles(svg::Document& document, const SphereProjector& proj);
    void AddStationNames(svg::Document& document, const SphereProjector& proj);

    const transportCatalogue::base::TransportCatalogue& tc;

    RenderSettings render_settings;

};

}