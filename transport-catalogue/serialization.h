#pragma once

#include "domain.h"
#include "transport_catalogue.h"

#include <transport_catalogue.pb.h>

namespace transportCatalogue {

namespace serialization {

class Serializer {

public:

    Serializer(const SerializationSettings& ss);

    bool Serialize(const base::TransportCatalogue& tc, const mapRenderer::MapRenderer& mr, const transportRouter::TransportRouter& tr);
    bool Deserialize(base::TransportCatalogue& tc, mapRenderer::MapRenderer& mr, transportRouter::TransportRouter& tr);

private:

    SerializationSettings serialization_settings;

    std::unordered_map<const Stop*, int> stops_unique_indexes;
    std::unordered_map<const Bus*, int> buses_unique_indexes;

    std::unordered_map<int, Stop*> unique_indexes_stops;
    std::unordered_map<int, Bus*> unique_indexes_buses;

    void SerializeTransportCatalogue(const base::TransportCatalogue& tc, transportCatalogueSerialized::TransportCatalogue& output_package);
    void SerializeMapRenderer(const mapRenderer::MapRenderer& mr, transportCatalogueSerialized::TransportCatalogue& output_package);
    void SerializeTransportRouter(const transportRouter::TransportRouter& tr, transportCatalogueSerialized::TransportCatalogue& output_package);

    void DeserializeTransportCatalogue(base::TransportCatalogue& tc, transportCatalogueSerialized::TransportCatalogue& input_package);
    void DeserializeMapRenderer(mapRenderer::MapRenderer& mr, transportCatalogueSerialized::TransportCatalogue& input_package);
    void DeserializeTransportRouter(transportRouter::TransportRouter& tr, transportCatalogueSerialized::TransportCatalogue& input_package);

};

}

}