#include "json_reader.h"
#include "json.h"

using namespace transportCatalogue;
using namespace transportCatalogue::requestsProcessing;
using namespace json;
using namespace std::string_literals;

JsonReader::JsonReader(std::istream& in) {
    const Document data = json::Load(in);
    const Dict& all_requests = data.GetRoot().AsMap();
    const Array& base_requests = all_requests.at("base_requests"s).AsArray();
    const Array& stat_requests = all_requests.at("stat_requests"s).AsArray();
    const Dict& render = all_requests.at("render_settings"s).AsMap();
    for (const Node& node : base_requests) {
        const Dict& request = node.AsMap();
        if (request.at("type"s).AsString() == "Stop"s) {
            requestsToFill::StopInfo result;
            result.name = request.at("name"s).AsString();
            result.latitude = request.at("latitude"s).AsDouble();
            result.longitude = request.at("longitude"s).AsDouble();
            const Dict& road_distances = request.at("road_distances").AsMap();
            for (const auto [stopname, dist] : road_distances) {
                result.distances.insert({stopname, dist.AsInt()});
            }
            fill_requests.push_back(std::move(result));
        } else if (request.at("type"s).AsString() == "Bus"s) {
            requestsToFill::BusInfo result;
            result.name = request.at("name"s).AsString();
            const Array& stops = request.at("stops"s).AsArray();
            for (const Node& stop : stops) {
                result.stops_names.push_back(stop.AsString());
            }
            if (!request.at("is_roundtrip"s).AsBool()) {
                std::vector<std::string> all_stations_copied = result.stops_names;
                for (auto i = (result.stops_names.rbegin() + 1); i < result.stops_names.rend(); ++i) {
                    all_stations_copied.push_back(std::move(*i));
                }
                std::swap(all_stations_copied, result.stops_names);
                result.roundtrip = false;
            } else {
                result.roundtrip = true;
            }
            fill_requests.push_back(std::move(result));
        } else {
            throw std::logic_error("неизвестный тип запроса"s);
        }
    }
    for (const Node& node : stat_requests) {
        const Dict& request = node.AsMap();
        if (request.at("type").AsString() == "Stop"s) {
            requestsToSearch::StopInfo result;
            result.name = request.at("name"s).AsString();
            result.id = request.at("id"s).AsInt();
            search_requests.push_back(std::move(result));
        } else if (request.at("type").AsString() == "Bus"s) {
            requestsToSearch::BusInfo result;
            result.name = request.at("name"s).AsString();
            result.id = request.at("id"s).AsInt();
            search_requests.push_back(std::move(result));
        } else if (request.at("type").AsString() == "Map"s) {
            mapRenderer::requestToMapRenderer result;
            result.id = request.at("id"s).AsInt();
            search_requests.push_back(std::move(result));
        } else {
            throw std::logic_error("неизвестный тип запроса"s);
        }
    }
    render_settings.width = render.at("width"s).AsDouble();
    render_settings.height = render.at("height"s).AsDouble();
    render_settings.padding = render.at("padding"s).AsDouble();
    render_settings.stop_radius = render.at("stop_radius"s).AsDouble();
    render_settings.line_width = render.at("line_width"s).AsDouble();
    render_settings.bus_label_font_size = render.at("bus_label_font_size"s).AsInt();
    render_settings.bus_label_offset.first = render.at("bus_label_offset"s).AsArray()[0].AsDouble();
    render_settings.bus_label_offset.second = render.at("bus_label_offset"s).AsArray()[1].AsDouble();
    render_settings.stop_label_font_size = render.at("stop_label_font_size"s).AsInt();
    render_settings.stop_label_offset.first = render.at("stop_label_offset"s).AsArray()[0].AsDouble();
    render_settings.stop_label_offset.second = render.at("stop_label_offset"s).AsArray()[1].AsDouble();
    render_settings.underlayer_width = render.at("underlayer_width"s).AsDouble();
    if (render.at("underlayer_color"s).IsString()) {
        render_settings.underlayer_color = render.at("underlayer_color"s).AsString();
    } else {
        if (render.at("underlayer_color"s).AsArray().size() == 3) {
            render_settings.underlayer_color = svg::Rgb(render.at("underlayer_color"s).AsArray()[0].AsInt(),
                                                        render.at("underlayer_color"s).AsArray()[1].AsInt(),
                                                        render.at("underlayer_color"s).AsArray()[2].AsInt());
        } else {
            render_settings.underlayer_color = svg::Rgba(render.at("underlayer_color"s).AsArray()[0].AsInt(),
                                                         render.at("underlayer_color"s).AsArray()[1].AsInt(),
                                                         render.at("underlayer_color"s).AsArray()[2].AsInt(),
                                                         render.at("underlayer_color"s).AsArray()[3].AsDouble());
        }
    }
    const Array& color_palette = render.at("color_palette"s).AsArray();
    for (const Node& node : color_palette) {
        if (node.IsString()) {
            render_settings.color_palette.push_back(node.AsString());
        } else {
            if (node.AsArray().size() == 3) {
                render_settings.color_palette.push_back(svg::Rgb(node.AsArray()[0].AsInt(),
                                                                 node.AsArray()[1].AsInt(),
                                                                 node.AsArray()[2].AsInt()));
            } else {
                render_settings.color_palette.push_back(svg::Rgba(node.AsArray()[0].AsInt(),
                                                                  node.AsArray()[1].AsInt(),
                                                                  node.AsArray()[2].AsInt(),
                                                                  node.AsArray()[3].AsDouble()));
            }
        }
    }
}

int JsonReader::GetFillRequestsCount() const {
    return fill_requests.size();
}

fillInfo JsonReader::GetNextFillRequest() {
    fillInfo result = std::move(fill_requests.front());
    fill_requests.pop_front();
    return result;
}

int JsonReader::GetSearchRequestsCount() const {
    return search_requests.size();
}

searchInfo JsonReader::GetNextSearchRequest() {
    searchInfo result = std::move(search_requests.front());
    search_requests.pop_front();
    return result;
}

mapRenderer::RenderSettings JsonReader::GetRenderSettings() {
    return std::move(render_settings);
}

JsonWriter::JsonWriter(std::ostream& out) 
: out_(out) {}

void JsonWriter::AddRequestAnswer(const answerInfo& answer) {
    answers.push_back(answer);
}

void JsonWriter::PrintAllAnswers() {
    Array all_answers;
    all_answers.reserve(answers.size());
    while (!answers.empty()) {
        Dict answer;
        if (std::holds_alternative<answersFromBase::StopInfo>(answers.front())) {
            answersFromBase::StopInfo data = std::get<answersFromBase::StopInfo>(std::move(answers.front()));
            answers.pop_front();
            answer.insert({"request_id"s, Node(data.id)});
            if (data.no_stop) {
                answer.insert({"error_message"s, Node("not found"s)});
            } else {
                Array buses;
                buses.reserve(data.buses.size());
                for (std::string s : data.buses) {
                    buses.push_back(Node(std::move(s)));
                }
                answer.insert({"buses"s, std::move(buses)});
            }
        } else if (std::holds_alternative<answersFromBase::BusInfo>(answers.front())) {
            answersFromBase::BusInfo data = std::get<answersFromBase::BusInfo>(std::move(answers.front()));
            answers.pop_front();
            answer.insert({"request_id"s, Node(data.id)});
            if (data.no_bus) {
                answer.insert({"error_message"s, Node("not found"s)});
            } else {
                answer.insert({"curvature"s, data.curvature});
                answer.insert({"route_length"s, data.distance});
                answer.insert({"stop_count"s, data.stops_count});
                answer.insert({"unique_stop_count"s, data.unique_stops_count});
            }
        } else if (std::holds_alternative<mapRenderer::answerFromMapRenderer>(answers.front())) {
            mapRenderer::answerFromMapRenderer data = std::get<mapRenderer::answerFromMapRenderer>(std::move(answers.front()));
            answers.pop_front();
            answer.insert({"request_id"s, Node(data.id)});
            answer.insert({"map"s, std::move(data.map)});
        } else {
            throw std::logic_error("неизвестный тип ответа"s);
        }
        all_answers.push_back(std::move(answer));
    }
    Document d(std::move(all_answers));
    json::Print(d, out_);
}