#include "request_handler.h"


std::optional<catalogue::detail::RouteInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return db_.GetRouteInfo(db_.FindBusRoute(bus_name));
}

const std::vector<std::string> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {

    std::vector<std::string> result;
    catalogue::detail::StopInfo stop_info = db_.GetStopInfo(db_.FindStop(stop_name));

    for (auto& stop : stop_info.buses)
    {
        result.push_back(std::string(stop));
    }
    return result;
}
