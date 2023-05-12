#include "input_reader.h"

#include <algorithm>
#include <sstream>
#include <cassert>

using namespace std::string_literals;

using namespace transportCatalogue;
using namespace transportCatalogue::fill;

TransportReader::TransportReader(std::istream& in) {
        std::string s;
        std::getline(in, s);
        int requests_count = std::stoi(s);
        stops_requests.reserve(requests_count);
        buses_requests.reserve(requests_count);
        for (int i = 0; i < requests_count; ++i) {
            std::getline(in, s);
            if (s[0] == 'S') {
                ParseStopRequest(s);
                continue;
            }
            if (s[0] == 'B') {
                ParseBusRequest(s);
                continue;
            }
            throw std::logic_error("неизвестный запрос");
        }
}

size_t TransportReader::GetStopsCount() const {
    return stops_requests.size();
}

size_t TransportReader::GetBusesCount() const {
    return buses_requests.size();
}

size_t TransportReader::GetStopDistancesCount() const {
    return stops_distances_requests.size();
}

requestsToBase::AddStop TransportReader::GetNextStop() {
    if (GetStopsCount() < 1) {
        throw std::logic_error("в запросе больше нет остановок"s);
    }
    requestsToBase::AddStop result = std::move(stops_requests.back());
    stops_requests.pop_back();
    return result;
}

requestsToBase::AddBus TransportReader::GetNextBus() {
    if (GetBusesCount() < 1) {
        throw std::logic_error("в запросе больше нет автобусов"s);
    }
    requestsToBase::AddBus result = std::move(buses_requests.back());
    buses_requests.pop_back();
    return result;
}

requestsToBase::AddStopDistance TransportReader::GetNextStopDistance() {
    if (GetStopDistancesCount() < 1) {
        throw std::logic_error("в запросе больше нет дистанций между остановками"s);
    }
    requestsToBase::AddStopDistance result = std::move(stops_distances_requests.back());
    stops_distances_requests.pop_back();
    return result;
}

void TransportReader::ParseStopRequest(std::string_view s) {
    requestsToBase::AddStop new_stop;
    s = s.substr(4, s.size());                                              //отрезаем слово Stop
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы перед названием
    std::string_view stop_name = s.substr(0, s.find_first_of(':'));         //строка до двоеточия
    stop_name = stop_name.substr(0, stop_name.find_last_not_of(' ') + 1);   //отрезаем пробелы в конце названия
    new_stop.name = std::string(stop_name);
    s = s.substr(s.find_first_of(':') + 1, s.size());                       //отрезаем всё до двоеточия включительно
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы после двоеточия
    std::string_view stop_latitude = s.substr(0, s.find_first_of(','));     //строка до запятой
    new_stop.latitude = std::stod(std::string(stop_latitude));
    s = s.substr(s.find_first_of(',') + 1, s.size());                       //отрезаем всё до запятой включительно
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы после запятой
    std::string_view stop_longitude = s.substr(0, s.find_first_of(','));    //строка до запятой
    stop_longitude = stop_longitude.substr(0, s.find_last_not_of(' ') + 1); //отрезаем пробелы в конце
    new_stop.longitude = std::stod(std::string(s));
    if (s.find_first_of(',') == std::string::npos) {
        stops_requests.push_back(std::move(new_stop));
        return;
    }
    s = s.substr(s.find_first_of(',') + 1, s.size());                       //отрезаем всё до запятой включительно
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы после запятой
    int stops_count = std::count(s.begin(), s.end(), ',') + 1;
    for (int i = 0; i < stops_count; ++i) {
        std::string_view other_stop = s.substr(0, s.find_first_of(','));                            //строка до разделителя
        std::string_view other_stop_distance = other_stop.substr(0, other_stop.find_first_of('m')); //строка до буквы 'm'  
        double distance_number = std::stod(std::string(other_stop_distance));
        other_stop = other_stop.substr(other_stop.find_first_of('m') + 1, other_stop.size());       //отрезаем всё до буквы 'm' включительно
        other_stop = other_stop.substr(other_stop.find_first_not_of(' '), other_stop.size());       //отрезаем пробелы перед to
        other_stop = other_stop.substr(2, other_stop.size());                                       //отрезаем to
        other_stop = other_stop.substr(other_stop.find_first_not_of(' '), other_stop.size());       //отрезаем пробелы перед названием
        other_stop = other_stop.substr(0, other_stop.find_last_not_of(' ') + 1);                    //отрезаем пробелы в конце названия
        requestsToBase::AddStopDistance new_distance;
        new_distance.name_first = std::string(stop_name);
        new_distance.name_second = std::string(other_stop);
        new_distance.distance = distance_number;
        stops_distances_requests.push_back(std::move(new_distance));
        s = s.substr(s.find_first_of(',') + 1, s.size());                                           //отрезаем всё до запятой включительно
    }
    stops_requests.push_back(std::move(new_stop));
}

void TransportReader::ParseBusRequest(std::string_view s) {
    requestsToBase::AddBus new_bus;
    s = s.substr(3, s.size());                                              //отрезаем слово Bus
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы перед названием
    std::string_view bus_name = s.substr(0, s.find_first_of(':'));          //строка до двоеточия
    bus_name = bus_name.substr(0, bus_name.find_last_not_of(' ') + 1);      //отрезаем пробелы в конце названия
    new_bus.name = std::string(bus_name);
    s = s.substr(s.find_first_of(':') + 1, s.size());                       //отрезаем всё до двоеточия включительно
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы после двоеточия
    char separator;
    if (s.find_first_of('>') != std::string::npos) {
        separator = '>';
    } else {
        separator = '-';
    }
    int stops_count = std::count(s.begin(), s.end(), separator) + 1;
    for (int i = 0; i < stops_count; ++i) {
        std::string_view stop_name = s.substr(0, s.find_first_of(separator));//строка до разделителя
        stop_name = stop_name.substr(0, stop_name.find_last_not_of(' ') + 1);//отрезаем пробелы в конце названия
        new_bus.stops_names.push_back(std::string(stop_name));
        s = s.substr(s.find_first_of(separator) + 1, s.size());              //отрезаем всё до разделителя включительно
        s = s.substr(s.find_first_not_of(' '), s.size());                    //отрезаем пробелы после разделителя
    }
    if (separator == '-') {
        std::vector<std::string> all_stations_copied = new_bus.stops_names;
        for (auto i = (new_bus.stops_names.rbegin() + 1); i < new_bus.stops_names.rend(); ++i) {
            all_stations_copied.push_back(std::move(*i));
        }
        std::swap(all_stations_copied, new_bus.stops_names);
    }
    buses_requests.push_back(std::move(new_bus));
}

void tests::TestTransportReader() {
    std::istringstream test {
        "13\n"
        "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
        "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
        "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"
        "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
        "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
        "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
        "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
        "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
        "Stop Prazhskaya: 55.611678, 37.603831\n"};

    TransportReader tr(test);

    assert(tr.GetBusesCount() == 3);
    assert(tr.GetStopsCount() == 10);
    assert(tr.GetStopDistancesCount() == 12);

    assert(tr.GetNextStop() == (requestsToBase::AddStop{"Prazhskaya"s, 55.611678, 37.603831}));
    assert(tr.GetStopsCount() == 9);
    assert(tr.GetNextStop() == (requestsToBase::AddStop{"Rossoshanskaya ulitsa"s, 55.595579, 37.605757}));
    assert(tr.GetStopsCount() == 8);
    assert(tr.GetNextStop() == (requestsToBase::AddStop{"Biryulyovo Passazhirskaya"s, 55.580999, 37.659164}));
    assert(tr.GetStopsCount() == 7);

    assert(tr.GetNextStopDistance() == (requestsToBase::AddStopDistance{"Biryulyovo Passazhirskaya"s, "Biryulyovo Zapadnoye"s, 1200}));
    assert(tr.GetStopDistancesCount() == 11);
    assert(tr.GetNextStopDistance() == (requestsToBase::AddStopDistance{"Biryulyovo Tovarnaya"s, "Biryulyovo Passazhirskaya"s, 1300}));
    assert(tr.GetStopDistancesCount() == 10);

    assert(tr.GetNextBus() == (requestsToBase::AddBus{"828"s, {"Biryulyovo Zapadnoye"s, "Universam"s, "Rossoshanskaya ulitsa"s, "Biryulyovo Zapadnoye"s}}));
    assert(tr.GetBusesCount() == 2);
    assert(tr.GetNextBus() == (requestsToBase::AddBus{"750"s, {"Tolstopaltsevo"s, "Marushkino"s, "Marushkino"s, "Rasskazovka"s, "Marushkino"s, "Marushkino"s, "Tolstopaltsevo"s}}));
    assert(tr.GetBusesCount() == 1);
    assert(tr.GetNextBus() == (requestsToBase::AddBus{"256"s, {"Biryulyovo Zapadnoye"s, "Biryusinka"s, "Universam"s, "Biryulyovo Tovarnaya"s, "Biryulyovo Passazhirskaya"s, "Biryulyovo Zapadnoye"s}}));
    assert(tr.GetBusesCount() == 0);

}