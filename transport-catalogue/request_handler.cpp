#include "request_handler.h"

#include <stdexcept>

using namespace transportCatalogue;
using namespace transportCatalogue::requestsProcessing;
using namespace std::string_literals;

void RequestHandler::GetBaseRequests(InterfaceIn& in) {
    while (in.GetFillRequestsCount() > 0) {
        fill_requests.push_back(in.GetNextFillRequest());
    }
    render_settings = in.GetRenderSettings();
    routing_settings = in.GetRoutingSettings();
    serialization_settings = in.GetSerializationSettings();    
}

void RequestHandler::GetStatRequests(InterfaceIn& in) {
    while (in.GetSearchRequestsCount() > 0) {
        search_requests.push_back(in.GetNextSearchRequest());
    }
    serialization_settings = in.GetSerializationSettings(); 
}

void RequestHandler::Fill(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr) {
    std::deque<requestsToFill::StopInfo> stops;
    std::deque<requestsToFill::BusInfo> buses;
    while (!fill_requests.empty()) {
        if (std::holds_alternative<requestsToFill::StopInfo>(fill_requests.front())) {
            stops.push_back(std::get<requestsToFill::StopInfo>(std::move(fill_requests.front())));
            fill_requests.pop_front();
        } else if (std::holds_alternative<requestsToFill::BusInfo>(fill_requests.front())) {
            buses.push_back(std::get<requestsToFill::BusInfo>(std::move(fill_requests.front())));
            fill_requests.pop_front();
        } else {
            using namespace std::string_literals;
            throw std::logic_error("неизвестный тип заполняющего запроса"s);
        }
    }
    while (!stops.empty()) {
        tc.AddStop(stops.front());
        stops.pop_front();
    }
    while (!buses.empty()) {
        tc.AddBus(buses.front());
        buses.pop_front();
    }
    mr.SetRenderSettings(render_settings);
    tr.SetRoutingSettings(routing_settings);
    tr.BuildGraph();
}

void RequestHandler::Search(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr, InterfaceOut& out) {
    while (!search_requests.empty()) {
        if (std::holds_alternative<requestsToSearch::StopInfo>(search_requests.front())) {
            out.AddRequestAnswer(tc.GetStop(std::get<requestsToSearch::StopInfo>(std::move(search_requests.front()))));
            search_requests.pop_front();
        } else if (std::holds_alternative<requestsToSearch::BusInfo>(search_requests.front())) {
            out.AddRequestAnswer(tc.GetBus(std::get<requestsToSearch::BusInfo>(std::move(search_requests.front()))));
            search_requests.pop_front();
        } else if (std::holds_alternative<mapRenderer::requestToMapRenderer>(search_requests.front())) {
            out.AddRequestAnswer(mr.GetMap(std::get<mapRenderer::requestToMapRenderer>(std::move(search_requests.front()))));
            search_requests.pop_front();
        } else if (std::holds_alternative<transportRouter::requestToRouter>(search_requests.front())) {
            out.AddRequestAnswer(tr.GetRoute(std::get<transportRouter::requestToRouter>(std::move(search_requests.front()))));
            search_requests.pop_front();
        }
    }
}

bool RequestHandler::MakeBase(const base::TransportCatalogue& tc, const mapRenderer::MapRenderer& mr, const transportRouter::TransportRouter& tr) {
    serialization::Serializer sr(serialization_settings);
    return sr.Serialize(tc, mr, tr);
}

bool RequestHandler::RestoreBase(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr) {
    serialization::Serializer sr(serialization_settings);
    return sr.Deserialize(tc, mr, tr);
}