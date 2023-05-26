#include "request_handler.h"

#include <stdexcept>

using namespace transportCatalogue;
using namespace transportCatalogue::requestsProcessing;
using namespace std::string_literals;

void RequestHandler::GetRequests(InterfaceIn& in) {
    while (in.GetFillRequestsCount() > 0) {
        fill_requests.push_back(in.GetNextFillRequest());
    }
    while (in.GetSearchRequestsCount() > 0) {
        search_requests.push_back(in.GetNextSearchRequest());
    }
    render_settings = in.GetRenderSettings();    
}

void RequestHandler::Fill(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr) {
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
        mr.AddStop(stops.front());
        stops.pop_front();
    }
    while (!buses.empty()) {
        tc.AddBus(buses.front());
        mr.AddBus(buses.front());
        buses.pop_front();
    }
    mr.SetRenderSettings(render_settings);
}

void RequestHandler::Search(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, InterfaceOut& out) {
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
        }
    }
    out.PrintAllAnswers();
}