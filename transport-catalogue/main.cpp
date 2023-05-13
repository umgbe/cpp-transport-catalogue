#include <iostream>
#include <sstream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transportCatalogue::fill::TransportReader tr(std::cin);
    transportCatalogue::search::TransportWriter tw(std::cin, std::cout);
    transportCatalogue::base::TransportCatalogue tc;
    tc.Fill(tr);
    tc.Search(tw);
}