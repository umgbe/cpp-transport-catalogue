#include <iostream>
#include <fstream>

#include "json_reader.h"

void FileTest() {
    std::ifstream in("input.txt");
    std::ofstream out("output.txt");
    transportCatalogue::base::TransportCatalogue tc;
    transportCatalogue::requestsProcessing::RequestHandler rh;
    mapRenderer::MapRenderer mr;
    transportCatalogue::requestsProcessing::JsonReader jr(in);
    transportCatalogue::requestsProcessing::JsonWriter jw(out);
    rh.GetRequests(jr);
    rh.Fill(tc, mr);
    rh.Search(tc, mr, jw);
}

int main() {
    //FileTest();
    transportCatalogue::base::TransportCatalogue tc;
    transportCatalogue::requestsProcessing::RequestHandler rh;
    mapRenderer::MapRenderer mr;
    transportCatalogue::requestsProcessing::JsonReader jr(std::cin);
    transportCatalogue::requestsProcessing::JsonWriter jw(std::cout);
    rh.GetRequests(jr);
    rh.Fill(tc, mr);
    rh.Search(tc, mr, jw);
}