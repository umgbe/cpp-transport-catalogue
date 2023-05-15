#pragma once

#include <iostream>

#include "transport_catalogue.h"

using namespace std::string_literals;

namespace transportCatalogue {

namespace fill {

class TransportReader {

public:

    TransportReader(std::istream& in);

    void Fill(base::TransportCatalogue& tc);

private:

    size_t GetStopsCount() const;
    size_t GetBusesCount() const;

    requestsToBase::StopInfo GetNextStop();
    requestsToBase::BusInfo GetNextBus();

    void ParseStopRequest(std::string_view s);
    void ParseBusRequest(std::string_view s);

    std::vector<requestsToBase::StopInfo> stops_requests;
    std::vector<requestsToBase::BusInfo> buses_requests;

};

}

namespace tests {

void TestTransportReader();

}

}