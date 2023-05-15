#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"

#include <sstream>
#include <cassert>
#include <algorithm>

using namespace transportCatalogue;
using namespace transportCatalogue::base;

void TransportCatalogue::AddStop(const requestsToBase::StopInfo& r) {
    Stop* ptr;
    if (!stopname_to_stop.count(r.name)) {
        stops.push_back({r.name, r.latitude, r.longitude});
        ptr = &stops.back();
        stopname_to_stop.insert({stops.back().name, ptr});
    } else {
        ptr = stopname_to_stop[r.name];
        ptr->latitude = r.latitude;
        ptr->longitude = r.longitude;
    }
    stops_to_buses.insert({ptr, std::vector<Bus*>()});
    for (const auto [other_stop_name, distance] : r.distances) {
        Stop* other_ptr;
        if (!stopname_to_stop.count(other_stop_name)) {            
            stops.push_back({other_stop_name, 1000.0, 1000.0});
            other_ptr = &stops.back();
            stopname_to_stop.insert({stops.back().name, other_ptr});
        } else {
            other_ptr = stopname_to_stop[other_stop_name];
        }
        distances[{ptr, other_ptr}] = distance;
        if (!distances.count({other_ptr, ptr})) {
            distances[{other_ptr, ptr}] = distance;
        }
    }
}

void TransportCatalogue::AddBus(const requestsToBase::BusInfo& r) {
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

requestsFromBase::BusInfo TransportCatalogue::GetBus(const requestsToBase::BusInfo& r) {
    requestsFromBase::BusInfo result;
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

requestsFromBase::StopInfo TransportCatalogue::GetStop(const requestsToBase::StopInfo& r) {
    requestsFromBase::StopInfo result;
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