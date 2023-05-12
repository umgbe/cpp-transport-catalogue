#pragma once

#include <iostream>

#include "transport_catalogue.h"

using namespace std::string_literals;

namespace transportCatalogue {

namespace fill {

class TransportReader {

public:

    TransportReader(std::istream& in);

    size_t GetStopsCount() const;
    size_t GetBusesCount() const;
    size_t GetStopDistancesCount() const;

    requestsToBase::AddStop GetNextStop();
    requestsToBase::AddBus GetNextBus();
    requestsToBase::AddStopDistance GetNextStopDistance();

private:

    void ParseStopRequest(std::string_view s);
    void ParseBusRequest(std::string_view s);

    std::vector<requestsToBase::AddStop> stops_requests;
    std::vector<requestsToBase::AddBus> buses_requests;
    std::vector<requestsToBase::AddStopDistance> stops_distances_requests;

};

}

namespace tests {

void TestTransportReader();

}

}