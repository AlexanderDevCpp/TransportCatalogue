#pragma once
#include <fstream>
#include <iostream>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include "transport_router.pb.h"
#include "transport_catalogue.pb.h"


struct IndexBook
{
	std::map<std::string_view, int> stop_to_index;
	std::map<std::string_view, int> bus_to_index;
};

void SetStops(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book);
void SetBuses(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book);
void SetStopToBuses(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book);
void SetDistances(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book);
void SerializeTransportCatalogue(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::DataBase& db, IndexBook& book);
void SerializeMapRenderSettings(const renderer::MapRenderer& renderer, transport_catalogue_serialize::DataBase& db);
void SerializeDataBase(const catalogue::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const router::TransportRouter& router, std::string filename);