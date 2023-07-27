#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <variant>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

namespace transportCatalogue {

namespace requestsProcessing {

using fillInfo = std::variant<  requestsToFill::StopInfo, 
                                requestsToFill::BusInfo>;
using searchInfo = std::variant<requestsToSearch::StopInfo, 
                                requestsToSearch::BusInfo, 
                                mapRenderer::requestToMapRenderer, 
                                transportRouter::requestToRouter>;
using answerInfo = std::variant<answersFromBase::StopInfo, 
                                answersFromBase::BusInfo, 
                                mapRenderer::MapRendererResponse,
                                transportRouter::RouterResponce>;

class InterfaceIn {

public:
   
    virtual int GetFillRequestsCount() const = 0;
    virtual fillInfo GetNextFillRequest() = 0;

    virtual int GetSearchRequestsCount() const = 0;
    virtual searchInfo GetNextSearchRequest() = 0;

    virtual mapRenderer::RenderSettings GetRenderSettings() = 0;

    virtual transportRouter::RoutingSettings GetRoutingSettings() = 0;

    virtual serialization::SerializationSettings GetSerializationSettings() = 0;

};

class InterfaceOut {

public:
   
    virtual void AddRequestAnswer(const answerInfo& answer) = 0;

};

class RequestHandler {

public:

    RequestHandler() = default;

    void GetBaseRequests(InterfaceIn& in);
    void GetStatRequests(InterfaceIn& in);

    void Fill(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr);
    void Search(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr, InterfaceOut& out);

    bool MakeBase(const base::TransportCatalogue& tc, const mapRenderer::MapRenderer& mr, const transportRouter::TransportRouter& tr);
    bool RestoreBase(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr);

private:

    std::deque<fillInfo> fill_requests;
    std::deque<searchInfo> search_requests;
    mapRenderer::RenderSettings render_settings;
    transportRouter::RoutingSettings routing_settings;
    serialization::SerializationSettings serialization_settings;

};

}

}