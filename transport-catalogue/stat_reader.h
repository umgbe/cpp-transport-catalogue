#pragma once

#include <iostream>
#include <deque>

#include "transport_catalogue.h"

namespace transportCatalogue {

namespace search {

class TransportWriter {

public:

    TransportWriter(std::istream& in, std::ostream& o);

    enum class RequestType {
        GETBUS,
        GETSTOP,
    };

    size_t GetAllRequestsCount() const;
    size_t GetBusRequestsCount() const;
    size_t GetStopRequestsCount() const;

    RequestType GetNextRequestType() const;

    requestsToBase::GetBus GetNextBusRequest();
    requestsToBase::GetStop GetNextStopRequest();

    void PrintBusAnswer(const requestsFromBase::GetBus& a);
    void PrintStopAnswer(const requestsFromBase::GetStop& a);


private:

    void ParseBusRequest(std::string_view s);
    void ParseStopRequest(std::string_view s);

    std::ostream& out;

    std::deque<RequestType> all_requests;
    std::deque<requestsToBase::GetBus> bus_requests;
    std::deque<requestsToBase::GetStop> stops_requests;

};

}

namespace tests {

void TestTransportWriter();

}


}