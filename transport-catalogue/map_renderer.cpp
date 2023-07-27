#include "map_renderer.h"

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <sstream>

using namespace mapRenderer;
using namespace std::string_literals;

MapRenderer::MapRenderer(const transportCatalogue::base::TransportCatalogue& transport_catalogue)
    : tc(transport_catalogue) {}

void MapRenderer::SetRenderSettings(const RenderSettings& rs) {
    render_settings = rs;
}

RenderSettings MapRenderer::GetRenderSettings() const {
    return render_settings;
}

MapRendererResponse MapRenderer::GetMap(const requestToMapRenderer& r) {
    svg::Document document = BuildMap();
    std::ostringstream result;
    document.Render(result);
    return {r.id, result.str()};    
}

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class MapRenderer::SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

svg::Document MapRenderer::BuildMap() {
    svg::Document document;
    const MapRenderer::SphereProjector proj = BuildSphereProjector();   
    AddLines(document, proj);
    AddLineNames(document, proj);
    AddStationCircles(document, proj);
    AddStationNames(document, proj);
    return document;
}

MapRenderer::SphereProjector MapRenderer::BuildSphereProjector() {
    std::vector<geo::Coordinates> all_coordinates;
    all_coordinates.reserve(tc.stops.size());
    for (const auto [stop, buses] : tc.stops_to_buses) {
        if (!buses.empty()) {
            all_coordinates.push_back({stop->latitude, stop->longitude});
        }
    }
    SphereProjector proj{all_coordinates.begin(), all_coordinates.end(), render_settings.width, render_settings.height, render_settings.padding};
    return proj;
}

void MapRenderer::AddLines(svg::Document& document, const SphereProjector& proj) {
    size_t color_index = 0;
    for (const auto [busname, bus] : tc.busname_to_bus) {        
        if (bus->stops.empty()) {
            continue;
        }
        svg::Polyline line;
        line.SetFillColor(svg::NoneColor).SetStrokeColor(render_settings.color_palette[color_index]).SetStrokeWidth(render_settings.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        line.SetStrokeColor(render_settings.color_palette[color_index]);
        if ((color_index + 1) == render_settings.color_palette.size()) {
            color_index = 0;
        } else {
            ++color_index;
        }
        for (const Stop* stop : bus->stops) {
            line.AddPoint(proj({stop->latitude, stop->longitude}));
        }
        document.Add(std::move(line));
    }
}

void MapRenderer::AddLineNames(svg::Document& document, const SphereProjector& proj) {
    size_t color_index = 0;
    for (const auto [busname, bus] : tc.busname_to_bus) {
        if (bus->stops.empty()) {
            continue;
        }
        svg::Text text;
        svg::Text underline_text;
        svg::Point coordinates = proj({bus->stops[0]->latitude, bus->stops[0]->longitude});
        svg::Point offset = {render_settings.bus_label_offset.first, render_settings.bus_label_offset.second};
        text.SetPosition(coordinates).SetOffset(offset).SetFontSize(render_settings.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(bus->name);
        underline_text.SetPosition(coordinates).SetOffset(offset).SetFontSize(render_settings.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(bus->name);
        underline_text.SetFillColor(render_settings.underlayer_color).SetStrokeColor(render_settings.underlayer_color);
        underline_text.SetStrokeWidth(render_settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetFillColor(render_settings.color_palette[color_index]);        
        document.Add(underline_text);
        document.Add(text);
        if (!bus->roundtrip) {
            int final_stop_index = (bus->stops.size() - 1) / 2;
            if (bus->stops[0] != bus->stops[final_stop_index]) {
                coordinates = proj({bus->stops[final_stop_index]->latitude, bus->stops[final_stop_index]->longitude});
                text.SetPosition(coordinates);
                underline_text.SetPosition(coordinates);            
                document.Add(underline_text);
                document.Add(text);
            }
        }
        if ((color_index + 1) == render_settings.color_palette.size()) {
            color_index = 0;
        } else {
            ++color_index;
        }
    }
}

void MapRenderer::AddStationCircles(svg::Document& document, const SphereProjector& proj) {
    for (const auto [stopname, stop] : tc.stopname_to_stop) {
        if (tc.stops_to_buses.at(stop).empty()) {
            continue;
        }
        svg::Circle circle;
        circle.SetCenter(proj({stop->latitude, stop->longitude})).SetRadius(render_settings.stop_radius).SetFillColor("white"s);
        document.Add(circle);
    }
}

void MapRenderer::AddStationNames(svg::Document& document, const SphereProjector& proj) {
    for (const auto [stopname, stop] : tc.stopname_to_stop) {
        if (tc.stops_to_buses.at(stop).empty()) {
            continue;
        }
        svg::Text text;
        svg::Text underline_text;
        svg::Point coordinates = proj({stop->latitude, stop->longitude});
        svg::Point offset = {render_settings.stop_label_offset.first, render_settings.stop_label_offset.second};
        text.SetPosition(coordinates).SetOffset(offset).SetFontSize(render_settings.stop_label_font_size).SetFontFamily("Verdana"s).SetData(stop->name);
        underline_text.SetPosition(coordinates).SetOffset(offset).SetFontSize(render_settings.stop_label_font_size).SetFontFamily("Verdana"s).SetData(stop->name);
        underline_text.SetFillColor(render_settings.underlayer_color).SetStrokeColor(render_settings.underlayer_color);
        underline_text.SetStrokeWidth(render_settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetFillColor("black"s);        
        document.Add(std::move(underline_text));
        document.Add(std::move(text));
    }
}