#pragma once

#include "request_handler.h"

namespace transportCatalogue {

namespace requestsProcessing {

class JsonReader : public InterfaceIn {

public:

    JsonReader() = default;

    void ReadBaseRequests(std::istream& in);
    void ReadStatRequests(std::istream& in);

    int GetFillRequestsCount() const override;
    fillInfo GetNextFillRequest() override;

    int GetSearchRequestsCount() const override;
    searchInfo GetNextSearchRequest() override;

    mapRenderer::RenderSettings GetRenderSettings() override;

    transportRouter::RoutingSettings GetRoutingSettings() override;

    serialization::SerializationSettings GetSerializationSettings() override;

private:

    std::deque<fillInfo> fill_requests;
    std::deque<searchInfo> search_requests;
    mapRenderer::RenderSettings render_settings;
    transportRouter::RoutingSettings routing_settings;
    serialization::SerializationSettings serialization_settings; 

};

class JsonWriter : public InterfaceOut {

public:

    JsonWriter() = default;

    void AddRequestAnswer(const answerInfo& answer) override;
    void PrintAllAnswers(std::ostream& out);

private:

    std::deque<answerInfo> answers;

};

}

}