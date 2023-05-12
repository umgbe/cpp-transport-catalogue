#include <iostream>
#include <sstream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transportCatalogue::fill::TransportReader tr(std::cin);
    transportCatalogue::search::TransportWriter tw(std::cin, std::cout);
    transportCatalogue::base::TransportCatalogue tc;
    while (tr.GetStopsCount() != 0) {
        tc.AddStop(tr.GetNextStop());
    }
    while (tr.GetStopDistancesCount() != 0) {
        tc.AddStopDistance(tr.GetNextStopDistance());
    }
    while (tr.GetBusesCount() != 0) {
        tc.AddBus(tr.GetNextBus());
    }
    while (tw.GetAllRequestsCount() != 0) {
        if (tw.GetNextRequestType() == transportCatalogue::search::TransportWriter::RequestType::GETBUS) {
            tw.PrintBusAnswer(tc.GetBus(tw.GetNextBusRequest()));
            continue;
        }
        if (tw.GetNextRequestType() == transportCatalogue::search::TransportWriter::RequestType::GETSTOP) {
            tw.PrintStopAnswer(tc.GetStop(tw.GetNextStopRequest()));
            continue;
        }
        throw std::logic_error("неизвестный тип запроса в очереди"s);
    }

}