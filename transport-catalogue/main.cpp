#include <fstream>
#include <iostream>
#include <string_view>

#include "json_reader.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    
    if (mode == "make_base"sv) {

        // make base here
        transportCatalogue::base::TransportCatalogue tc;
        transportCatalogue::requestsProcessing::RequestHandler rh;
        mapRenderer::MapRenderer mr(tc);
        transportRouter::TransportRouter tr(tc);
        transportCatalogue::requestsProcessing::JsonReader jr;
        jr.ReadBaseRequests(std::cin);
        rh.GetBaseRequests(jr);
        rh.Fill(tc, mr, tr);
        rh.MakeBase(tc, mr, tr);

    } else if (mode == "process_requests"sv) {

        // process requests here

        transportCatalogue::base::TransportCatalogue tc;
        transportCatalogue::requestsProcessing::RequestHandler rh;
        mapRenderer::MapRenderer mr(tc);
        transportRouter::TransportRouter tr(tc);
        transportCatalogue::requestsProcessing::JsonReader jr;
        transportCatalogue::requestsProcessing::JsonWriter jw;
        jr.ReadStatRequests(std::cin);
        rh.GetStatRequests(jr);
        rh.RestoreBase(tc, mr, tr);
        rh.Search(tc, mr, tr, jw);        
        jw.PrintAllAnswers(std::cout);


    } else {
        PrintUsage();
        return 1;
    }

}