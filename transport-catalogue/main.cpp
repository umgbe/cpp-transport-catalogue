#include <iostream>
#include <sstream>

#include "json_reader.h"

int main() {
    transportCatalogue::base::TransportCatalogue tc;
    transportCatalogue::requestsProcessing::RequestHandler rh;
    mapRenderer::MapRenderer mr;
    transportCatalogue::requestsProcessing::JsonReader jr(std::cin);
    transportCatalogue::requestsProcessing::JsonWriter jw(std::cout);
    rh.GetRequests(jr);
    rh.Fill(tc, mr);
    rh.Search(tc, mr, jw);
}