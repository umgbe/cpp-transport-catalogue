#pragma once

#include "request_handler.h"

namespace transportCatalogue {

namespace requestsProcessing {

class JsonReader : public InterfaceIn {

public:

    JsonReader(std::istream& in);

    int GetFillRequestsCount() const override;
    fillInfo GetNextFillRequest() override;

    int GetSearchRequestsCount() const override;
    searchInfo GetNextSearchRequest() override;

    mapRenderer::RenderSettings GetRenderSettings() override;

private:

    std::deque<fillInfo> fill_requests;
    std::deque<searchInfo> search_requests;
    mapRenderer::RenderSettings render_settings; 

};

class JsonWriter : public InterfaceOut {

public:

    JsonWriter(std::ostream& out);

    void AddRequestAnswer(const answerInfo& answer) override;
    void PrintAllAnswers() override;

private:

    std::ostream& out_;
    std::deque<answerInfo> answers;

};

}

}