cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto transport_router.proto)

set(TRANSPORT_FILES domain.h 
                    geo.cpp 
                    geo.h
                    graph.h 
                    json_builder.cpp 
                    json_builder.h 
                    json_reader.cpp
                    json_reader.h
                    json.cpp 
                    json.h 
                    main.cpp 
                    map_renderer.cpp 
                    map_renderer.h 
                    ranges.h 
                    request_handler.cpp 
                    request_handler.h 
                    router.h 
                    serialization.cpp 
                    serialization.h
                    svg.cpp
                    svg.h 
                    transport_catalogue.cpp
                    transport_catalogue.h
                    transport_router.cpp
                    transport_router.h
                    transport_catalogue.proto)
add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(transport_catalogue ${Protobuf_LIBRARY})