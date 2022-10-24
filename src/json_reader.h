#pragma once

#include <cstdio>
#include <fstream>

#include "json_builder.h"
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.pb.h"
#include "serialization.h"

geo::Coordinates GetCoords(const json::Node& request);
std::vector<std::string_view> GetStops(const json::Node& request);
svg::Color SetColor(const json::Node& node);
std::vector<svg::Color> SetColorVector(const json::Array& arr);

void PrintResponses(const json::Node& responses);

renderer::RendererSettings SetRenderSettings(const json::Dict& settings);
void StopRequestsProcessing(catalogue::TransportCatalogue& catalogue, const json::Array& stop_requests);
void BusRequestsProcessing(catalogue::TransportCatalogue& catalogue, const json::Array& bus_requests);
void BaseRequestsProcessing(catalogue::TransportCatalogue& catalogue, const json::Array& base_requests);

json::Dict MapResponseProcessing(catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer, const json::Node& bus_request);
json::Dict BusResponseProcessing(catalogue::TransportCatalogue& catalogue, const json::Node& bus_request);
json::Dict StopResponseProcessing(catalogue::TransportCatalogue& catalogue, const json::Node& stop_requests);
json::Node StatRequestsProcessing(catalogue::TransportCatalogue& catalogue,renderer::MapRenderer& renderer, router::TransportRouter& router, const json::Array& stat_requests);

void InitializeAndSerializeDataBase();

void RequestsProcessing(/*catalogue::TransportCatalogue& catalogue*/);
