#pragma once

#include <optional>

#include "transport_catalogue.h"

class RequestHandler {
public:

    RequestHandler(const catalogue::TransportCatalogue& db) 
        :db_(db) {
    }

    std::optional<catalogue::detail::RouteInfo> GetBusStat(const std::string_view& bus_name) const;

    const std::vector<std::string> GetBusesByStop(const std::string_view& stop_name) const;

private:
    const catalogue::TransportCatalogue& db_;
};
