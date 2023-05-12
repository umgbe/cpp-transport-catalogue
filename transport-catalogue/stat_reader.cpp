#include "stat_reader.h"

#include <sstream>
#include <cassert>

using namespace std::string_literals;

using namespace transportCatalogue;
using namespace transportCatalogue::search;

TransportWriter::TransportWriter(std::istream& in, std::ostream& o) 
: out(o) {
    std::string s;
    std::getline(in, s);
    int requests_count = std::stoi(s);
    for (int i = 0; i < requests_count; ++i) {
        std::getline(in, s);
        if (s[0] == 'B') {
            ParseBusRequest(s);
            continue;
        }
        if (s[0] == 'S') {
            ParseStopRequest(s);
            continue;
        }
        throw std::logic_error("неизвестный запрос");
    }
}

void TransportWriter::ParseBusRequest(std::string_view s) {
    requestsToBase::GetBus result;
    s = s.substr(3, s.size());                                              //отрезаем слово Bus
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы перед названием
    s = s.substr(0, s.find_last_not_of(' ') + 1);                           //отрезаем пробелы в конце названия
    result.name = std::string(s);
    bus_requests.push_back(std::move(result));
    all_requests.push_back(RequestType::GETBUS);
}

void TransportWriter::ParseStopRequest(std::string_view s) {
    requestsToBase::GetStop result;
    s = s.substr(4, s.size());                                              //отрезаем слово Stop
    s = s.substr(s.find_first_not_of(' '), s.size());                       //отрезаем пробелы перед названием
    s = s.substr(0, s.find_last_not_of(' ') + 1);                           //отрезаем пробелы в конце названия
    result.name = std::string(s);
    stops_requests.push_back(std::move(result));
    all_requests.push_back(RequestType::GETSTOP);
}

size_t TransportWriter::GetAllRequestsCount() const {
    return all_requests.size();
}

TransportWriter::RequestType TransportWriter::GetNextRequestType() const {
    return all_requests.front();
}

size_t TransportWriter::GetBusRequestsCount() const {
    return bus_requests.size();
}

size_t TransportWriter::GetStopRequestsCount() const {
    return stops_requests.size();
}

requestsToBase::GetBus TransportWriter::GetNextBusRequest() {
    if (GetBusRequestsCount() < 1) {
        throw std::logic_error("запросов автобусов больше нет"s);
    }
    if (all_requests.front() != RequestType::GETBUS) {
        throw std::logic_error("запросы предоставляются в неожиданном порядке"s);
    }
    requestsToBase::GetBus result = std::move(bus_requests.front());
    bus_requests.pop_front();
    all_requests.pop_front();
    return result;
}

requestsToBase::GetStop TransportWriter::GetNextStopRequest() {
    if (GetStopRequestsCount() < 1) {
        throw std::logic_error("запросов остановок больше нет"s);
    }
    if (all_requests.front() != RequestType::GETSTOP) {
        throw std::logic_error("запросы предоставляются в неожиданном порядке"s);
    }
    requestsToBase::GetStop result = std::move(stops_requests.front());
    stops_requests.pop_front();
    all_requests.pop_front();
    return result;
}

void TransportWriter::PrintBusAnswer(const requestsFromBase::GetBus& a) {
    if (a.bus_found) {
        out << "Bus "s << a.name << ": "s << a.stops_count << " stops on route, "s << a.unique_stops_count << " unique stops, "s << a.distance << " route length, "s << a.curvature << " curvature\n";
    } else {
        out << "Bus "s << a.name << ": not found\n"s;
    }
}

void TransportWriter::PrintStopAnswer(const requestsFromBase::GetStop& a) {
    if (a.no_stop) {
        out << "Stop "s << a.name << ": not found\n";
        return;
    }
    if (a.no_buses) {
        out << "Stop "s << a.name << ": no buses\n";
        return;
    }
    out << "Stop "s << a.name << ": buses";
    for (const std::string& bus : a.buses) {
        out << " "s << bus;
    }
    out << "\n"s;
}

void tests::TestTransportWriter() {

    std::istringstream testIn {
        "6\n"
        "Bus 256\n"
        "Bus 750\n"
        "Bus 751\n"
        "Stop Samara\n"
        "Stop Prazhskaya\n"
        "Stop Biryulyovo Zapadnoye\n"};

    std::ostringstream testOut;

    TransportWriter tw(testIn, testOut);

    assert(tw.GetAllRequestsCount() == 6);
    assert(tw.GetNextRequestType() == TransportWriter::RequestType::GETBUS);

    assert(tw.GetBusRequestsCount() == 3);
    assert(tw.GetNextBusRequest() == (requestsToBase::GetBus{"256"}));
    assert(tw.GetBusRequestsCount() == 2);
    assert(tw.GetNextBusRequest() == (requestsToBase::GetBus{"750"}));
    assert(tw.GetBusRequestsCount() == 1);
    assert(tw.GetNextBusRequest() == (requestsToBase::GetBus{"751"}));
    assert(tw.GetBusRequestsCount() == 0);

    assert(tw.GetAllRequestsCount() == 3);
    assert(tw.GetNextRequestType() == TransportWriter::RequestType::GETSTOP);

    assert(tw.GetStopRequestsCount() == 3);
    assert(tw.GetNextStopRequest() == (requestsToBase::GetStop{"Samara"}));
    assert(tw.GetStopRequestsCount() == 2);
    assert(tw.GetNextStopRequest() == (requestsToBase::GetStop{"Prazhskaya"}));
    assert(tw.GetStopRequestsCount() == 1);
    assert(tw.GetNextStopRequest() == (requestsToBase::GetStop{"Biryulyovo Zapadnoye"}));
    assert(tw.GetStopRequestsCount() == 0);

    requestsFromBase::GetBus a = {true, "256", 6, 5, 5950, 1.36124};
    requestsFromBase::GetBus b = {true, "750", 7, 3, 27400, 1.30853};
    requestsFromBase::GetBus c = {false, "751", 0, 0, 0, 0};

    tw.PrintBusAnswer(a);
    tw.PrintBusAnswer(b);
    tw.PrintBusAnswer(c);

}