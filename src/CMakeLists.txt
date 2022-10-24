cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS map_renderer.proto transport_catalogue.proto graph.proto transport_router.proto)

add_executable(
transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} 
main.cpp 
domain.cpp domain.h
geo.cpp geo.h 
graph.h
json_builder.cpp json_builder.h 
json_reader.cpp json_reader.h 
json.cpp json.h 
map_renderer.cpp map_renderer.h 
ranges.h 
request_handler.cpp request_handler.h
router.h
serialization.cpp serialization.h
svg.cpp svg.h 
transport_catalogue.cpp transport_catalogue.h 
transport_router.cpp transport_router.h)

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
 
target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads)