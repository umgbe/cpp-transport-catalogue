#include "transport_catalogue.h"
#include "geo.h"

#include <sstream>
#include <cassert>
#include <algorithm>

using namespace transportCatalogue;
using namespace transportCatalogue::base;
using namespace std::string_literals;

void TransportCatalogue::AddStop(const requestsToFill::StopInfo& r) {
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

void TransportCatalogue::AddBus(const requestsToFill::BusInfo& r) {
    Bus new_bus;
    new_bus.name = r.name;
    for (const std::string& stop_name : r.stops_names) {
        new_bus.stops.push_back(stopname_to_stop[stop_name]);
    }
    new_bus.roundtrip = r.roundtrip;
    buses.push_back(std::move(new_bus));
    Bus* ptr = &buses.back();
    for (const std::string& stop_name : r.stops_names) {
        stops_to_buses[stopname_to_stop[stop_name]].push_back(ptr);
    }
    busname_to_bus.insert({buses.back().name, ptr});
}

answersFromBase::BusInfo TransportCatalogue::GetBus(const requestsToSearch::BusInfo& r) {
    answersFromBase::BusInfo result;
    result.id = r.id;
    result.name = r.name;
    if (!busname_to_bus.count(r.name)) {
        result.no_bus = true;
        return result;
    }
    result.no_bus = false;
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
        geo_distance += geo::ComputeDistance({ptr->stops[i-1]->latitude, ptr->stops[i-1]->longitude}, {ptr->stops[i]->latitude, ptr->stops[i]->longitude});
    }

    result.curvature = result.distance / geo_distance;

    return result;
}

answersFromBase::StopInfo TransportCatalogue::GetStop(const requestsToSearch::StopInfo& r) {
    answersFromBase::StopInfo result;
    result.id = r.id;
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