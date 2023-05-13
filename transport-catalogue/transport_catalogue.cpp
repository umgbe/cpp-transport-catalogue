#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"

#include <sstream>
#include <cassert>
#include <algorithm>

using namespace transportCatalogue;
using namespace transportCatalogue::base;

void TransportCatalogue::AddStop(const requestsToBase::AddStop& r) {
    stops.push_back({r.name, r.latitude, r.longitude});
    Stop* ptr = &stops.back();
    stopname_to_stop.insert({stops.back().name, ptr});
    stops_to_buses.insert({ptr, std::vector<Bus*>()});
}

void TransportCatalogue::AddBus(const requestsToBase::AddBus& r) {
    Bus new_bus;
    new_bus.name = r.name;
    for (const std::string& stop_name : r.stops_names) {
        new_bus.stops.push_back(stopname_to_stop[stop_name]);
    }
    buses.push_back(std::move(new_bus));
    Bus* ptr = &buses.back();
    for (const std::string& stop_name : r.stops_names) {
        stops_to_buses[stopname_to_stop[stop_name]].push_back(ptr);
    }
    busname_to_bus.insert({buses.back().name, ptr});
}

void TransportCatalogue::AddStopDistance(const requestsToBase::AddStopDistance& r) {
    Stop* first_ptr = stopname_to_stop[r.name_first];
    Stop* second_ptr = stopname_to_stop[r.name_second];
    distances[{first_ptr, second_ptr}] = r.distance;
    if (!distances.count({second_ptr, first_ptr})) {
        distances[{second_ptr, first_ptr}] = r.distance;
    }
}

void TransportCatalogue::Fill(fill::TransportReader& tr) {
    while (tr.GetStopsCount() != 0) {
        AddStop(tr.GetNextStop());
    }
    while (tr.GetStopDistancesCount() != 0) {
        AddStopDistance(tr.GetNextStopDistance());
    }
    while (tr.GetBusesCount() != 0) {
        AddBus(tr.GetNextBus());
    }
}

void TransportCatalogue::Search(search::TransportWriter& tw) {
    while (tw.GetAllRequestsCount() != 0) {
        if (tw.GetNextRequestType() == transportCatalogue::search::TransportWriter::RequestType::GETBUS) {
            tw.PrintBusAnswer(GetBus(tw.GetNextBusRequest()));
            continue;
        }
        if (tw.GetNextRequestType() == transportCatalogue::search::TransportWriter::RequestType::GETSTOP) {
            tw.PrintStopAnswer(GetStop(tw.GetNextStopRequest()));
            continue;
        }
        throw std::logic_error("неизвестный тип запроса в очереди"s);
    }
}

requestsFromBase::GetBus TransportCatalogue::GetBus(const requestsToBase::GetBus& r) {
    requestsFromBase::GetBus result;
    result.name = r.name;
    if (!busname_to_bus.count(r.name)) {
        result.bus_found = false;
        return result;
    }
    result.bus_found = true;
    Bus* ptr = busname_to_bus[r.name];
    result.stops_count = ptr->stops.size();

    std::vector<Stop*> stops_without_duplicates(ptr->stops.begin(), ptr->stops.end());
    std::sort(stops_without_duplicates.begin(), stops_without_duplicates.end());
    auto last = std::unique(stops_without_duplicates.begin(), stops_without_duplicates.end());
    stops_without_duplicates.erase(last, stops_without_duplicates.end());
    result.unique_stops_count = stops_without_duplicates.size();

    result.distance = 0.0;
    double geo_distance = 0.0;

    for (size_t i = 1; i < ptr->stops.size(); ++i) {
        result.distance += distances[{ptr->stops[i-1], ptr->stops[i]}];
        geo_distance += ComputeDistance({ptr->stops[i-1]->latitude, ptr->stops[i-1]->longitude}, {ptr->stops[i]->latitude, ptr->stops[i]->longitude});
    }

    result.curvature = result.distance / geo_distance;

    return result;
}

requestsFromBase::GetStop TransportCatalogue::GetStop(const requestsToBase::GetStop& r) {
    requestsFromBase::GetStop result;
    result.name = r.name;
    if (!stopname_to_stop.count(r.name)) {
        result.no_stop = true;
        return result;
    }
    result.no_stop = false;
    for (const Bus* bus : stops_to_buses[stopname_to_stop[r.name]]) {
        result.buses.insert(bus->name);
    }
    if (result.buses.empty()) {
        result.no_buses = true;
        return result;
    }
    result.no_buses = false;
    return result;
}

using namespace transportCatalogue::fill;

void tests::TestTransportCatalogue() {
    /*std::istringstream test {
        "13\n"
        "Stop Tolstopaltsevo: 55.611087, 37.208290\n"
        "Stop Marushkino: 55.595884, 37.209755\n"
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
        "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
        "Stop Rasskazovka: 55.632761, 37.333324\n"
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700\n"
        "Stop Biryusinka: 55.581065, 37.648390\n"
        "Stop Universam: 55.587655, 37.645687\n"
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
        "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
        "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
        "Stop Prazhskaya: 55.611678, 37.603831}\n"};

    TransportReader tr(test);
    TransportCatalogue tc;

    while (tr.GetStopsCount() > 0) {
        tc.AddStop(tr.GetNextStop());
    }
    while (tr.GetBusesCount() > 0) {
        tc.AddBus(tr.GetNextBus());
    }
    
    requestsFromBase::GetBus testbus = tc.GetBus({"256"});

    assert(testbus.bus_found == true);
    assert(testbus.name == "256");
    assert(testbus.stops_count == 6);
    assert(testbus.unique_stops_count == 5);

    testbus = tc.GetBus({"750"});

    assert(testbus.bus_found == true);
    assert(testbus.name == "750");
    assert(testbus.stops_count == 5);
    assert(testbus.unique_stops_count == 3);

    testbus = tc.GetBus({"751"});

    assert(testbus.bus_found == false);

    requestsFromBase::GetStop teststop = tc.GetStop({"Samara"});

    assert(teststop.no_stop == true);

    teststop = tc.GetStop({"Prazhskaya"});

    assert(teststop.no_stop == false);
    assert(teststop.no_buses == true);

    teststop = tc.GetStop({"Biryulyovo Zapadnoye"});

    assert(teststop.no_stop == false);
    assert(teststop.no_buses == false);
    assert(teststop.buses == std::set<std::string>({"256", "828"}));
    */

}