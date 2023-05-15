#pragma once

#include <iostream>
#include <deque>

#include "transport_catalogue.h"

namespace transportCatalogue {

namespace search {

class TransportWriter {

public:

    TransportWriter(std::istream& in, std::ostream& o);

    void Search(base::TransportCatalogue& tc);

private:

    enum class RequestType {
        GETBUS,
        GETSTOP,
    };

    size_t GetAllRequestsCount() const;
    size_t GetBusRequestsCount() const;
    size_t GetStopRequestsCount() const;

    RequestType GetNextRequestType() const;

    requestsToBase::BusInfo GetNextBusRequest();
    requestsToBase::StopInfo GetNextStopRequest();

    void PrintBusAnswer(const requestsFromBase::BusInfo& a) const;
    void PrintStopAnswer(const requestsFromBase::StopInfo& a) const;

    void ParseBusRequest(std::string_view s);
    void ParseStopRequest(std::string_view s);

    std::ostream& out;

    std::deque<RequestType> all_requests;
    std::deque<requestsToBase::BusInfo> bus_requests;
    std::deque<requestsToBase::StopInfo> stops_requests;

};

}

namespace tests {

void TestTransportWriter();

}


}