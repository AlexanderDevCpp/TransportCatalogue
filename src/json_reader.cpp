#include "json_reader.h"

using namespace std;
geo::Coordinates GetCoords(const json::Node& request) {
    return { request.AsDict().at("latitude").AsDouble(),request.AsDict().at("longitude").AsDouble() };
}

std::vector<std::string_view> GetStops(const json::Node& request) {
    std::vector<std::string_view> result;

    for (auto& element : request.AsDict().at("stops").AsArray()) {
        result.push_back(element.AsString());
    }
    if (!request.AsDict().at("is_roundtrip").AsBool()) {
        result.insert(result.end(), result.rbegin() + 1, result.rend());
    }

    return result;
}

svg::Color SetColor(const json::Node& color_node) {
    if (color_node.IsString()) {
        return color_node.AsString();
    }
    else {
        if (color_node.AsArray().size() == 3) {
            svg::Rgb result(color_node.AsArray()[0].AsInt(), color_node.AsArray()[1].AsInt(), color_node.AsArray()[2].AsInt());
            return result;
        }
        else {
            svg::Rgba result(color_node.AsArray()[0].AsInt(), color_node.AsArray()[1].AsInt(), color_node.AsArray()[2].AsInt(), color_node.AsArray()[3].AsDouble());
            return result;
        }
    }
}

std::vector<svg::Color> SetColorVector(const json::Array& arr) {
    std::vector<svg::Color> result;

    for (auto& color : arr) {
        result.push_back(SetColor(color));
    }

    return result;
}

const std::vector<std::string> GetBusesByStop(catalogue::TransportCatalogue& catalogue, const json::Node& stop_request) {
    std::vector<std::string> result;
    catalogue::detail::StopInfo stop_info = catalogue.GetStopInfo(catalogue.FindStop(stop_request.AsDict().at("name").AsString()));

    for (auto& stop : stop_info.buses)
    {
        result.push_back(std::string(stop));
    }
    return result;
}

renderer::RendererSettings SetRenderSettings(const json::Dict& settings) {
    renderer::RendererSettings result;

    auto width_ptr = settings.find("width");
    if (width_ptr != settings.end()) {
        result.width_ = width_ptr->second.AsDouble();
    }

    auto height_ptr = settings.find("height");
    if (height_ptr != settings.end()) {
        result.height_ = height_ptr->second.AsDouble();
    }

    auto padding_ptr = settings.find("padding");
    if (padding_ptr != settings.end()) {
        result.padding_ = padding_ptr->second.AsDouble();
    }

    auto stop_radius_ptr = settings.find("stop_radius");
    if (stop_radius_ptr != settings.end()) {
        result.stop_radius_ = stop_radius_ptr->second.AsDouble();
    }

    auto line_width_ptr = settings.find("line_width");
    if (line_width_ptr != settings.end()) {
        result.line_width_ = line_width_ptr->second.AsDouble();
    }

    auto bus_label_font_size_ptr = settings.find("bus_label_font_size");
    if (bus_label_font_size_ptr != settings.end()) {
        result.bus_label_font_size_ = bus_label_font_size_ptr->second.AsInt();
    }

    auto bus_label_offset_ptr = settings.find("bus_label_offset");
    if (bus_label_offset_ptr != settings.end()) {
        result.bus_label_offset_ = { bus_label_offset_ptr->second.AsArray()[0].AsDouble(), bus_label_offset_ptr->second.AsArray()[1].AsDouble() };
    }

    auto stop_label_font_size_ptr = settings.find("stop_label_font_size");
    if (stop_label_font_size_ptr != settings.end()) {
        result.stop_label_font_size_ = stop_label_font_size_ptr->second.AsInt();
    }

    auto stop_label_offset_ptr = settings.find("stop_label_offset");
    if (stop_label_offset_ptr != settings.end()) {
        result.stop_label_offset_ = { stop_label_offset_ptr->second.AsArray()[0].AsDouble(), stop_label_offset_ptr->second.AsArray()[1].AsDouble() };
    }

    auto underlayer_width_ptr = settings.find("underlayer_width");
    if (underlayer_width_ptr != settings.end()) {
        result.underlayer_width_ = underlayer_width_ptr->second.AsDouble();
    }

    auto underlayer_color_ptr = settings.find("underlayer_color");
    if (underlayer_color_ptr != settings.end()) {
        result.underlayer_color_ = SetColor(underlayer_color_ptr->second);
    }

    auto color_palette_ptr = settings.find("color_palette");
    if (color_palette_ptr != settings.end()) {
        result.color_palette_ = SetColorVector(color_palette_ptr->second.AsArray());
    }

    return result;
}

router::RouterSettings SetRouterSettings(const json::Dict& settings) {
    router::RouterSettings result;

    auto bus_wait_time_ptr = settings.find("bus_wait_time");
    if (bus_wait_time_ptr != settings.end()) {
        result.bus_wait_time = bus_wait_time_ptr->second.AsInt();
    }

    auto bus_velocity_ptr = settings.find("bus_velocity");
    if (bus_velocity_ptr != settings.end()) {
        result.bus_velocity = bus_velocity_ptr->second.AsDouble() / 3.6;
    }

    return result;
}
void StopRequestsProcessing(catalogue::TransportCatalogue& catalogue, const json::Array& stop_requests) {
    for (auto& stop_request : stop_requests) {
        catalogue.AddStop(stop_request.AsDict().at("name").AsString(), GetCoords(stop_request));
    }
    for (auto& stop_request : stop_requests) {
        if (stop_request.AsDict().count("road_distances")) {
            for (auto& [stop_name, distance] : stop_request.AsDict().at("road_distances").AsDict()) {
                catalogue.SetStopDistance(catalogue.FindStop(stop_request.AsDict().at("name").AsString()), catalogue.FindStop(stop_name), distance.AsInt());
            }
        }
    }
}

void BusRequestsProcessing(catalogue::TransportCatalogue& catalogue, const json::Array& bus_requests) {
    for (auto& bus_request : bus_requests) {
        catalogue.AddBusRoute(bus_request.AsDict().at("name").AsString(), GetStops(bus_request), bus_request.AsDict().at("is_roundtrip").AsBool());
    }
}

void BaseRequestsProcessing(catalogue::TransportCatalogue& catalogue, const json::Array& base_requests) {
    json::Array bus_requests;
    json::Array stop_requests;

    for (auto& request : base_requests) {
        if (request.AsDict().at("type").AsString() == "Bus") {
            bus_requests.push_back(request);
        }
        if (request.AsDict().at("type").AsString() == "Stop") {
            stop_requests.push_back(request);
        }
    }

    StopRequestsProcessing(catalogue, stop_requests);
    BusRequestsProcessing(catalogue, bus_requests);
}

json::Dict MapResponseProcessing(catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer, const json::Node& map_request) {
    std::ostringstream s;

    s << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
    s << '\n';
    s << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;

    svg::RenderContext content(s);
    renderer.RenderMap(content, catalogue.GetRoutes());

    s << "</svg> ";

    return json::Builder().StartDict().Key("request_id"s).Value(map_request.AsDict().at("id")).Key("map"s).Value(s.str()).EndDict().Build().AsDict();
}

json::Dict BusResponseProcessing(catalogue::TransportCatalogue& catalogue, const json::Node& bus_request) {
    auto bus_ptr = catalogue.FindBusRoute(bus_request.AsDict().at("name").AsString());

    if (bus_ptr != nullptr) {
        catalogue::detail::RouteInfo route_info = catalogue.GetRouteInfo(bus_ptr);
        return json::Builder().
            StartDict()
            .Key("request_id"s).Value(bus_request.AsDict().at("id"))
            .Key("curvature"s).Value(route_info.stats.curvature)
            .Key("route_length"s).Value(route_info.stats.route_length)
            .Key("stop_count"s).Value(static_cast<int>(route_info.bus->route.size()))
            .Key("unique_stop_count"s).Value(static_cast<int>(route_info.bus->unique_stops.size()))
            .EndDict().Build().AsDict();
    }
    else {
        return json::Builder().StartDict()
            .Key("request_id"s).Value(bus_request.AsDict().at("id"))
            .Key("error_message"s).Value("not found"s)
            .EndDict().Build().AsDict();
    }
}
json::Dict StopResponseProcessing(catalogue::TransportCatalogue& catalogue, const json::Node& stop_request) {
    auto stop_ptr = catalogue.FindStop(stop_request.AsDict().at("name").AsString());

    if (stop_ptr != nullptr) {
        auto buses = GetBusesByStop(catalogue, stop_request);
        return json::Builder().StartDict()
            .Key("request_id"s).Value(stop_request.AsDict().at("id"))
            .Key("buses"s).Value(json::Array{ buses.begin(),buses.end() })
            .EndDict().Build().AsDict();
    }
    else {
        return json::Builder().StartDict()
            .Key("request_id"s).Value(stop_request.AsDict().at("id"))
            .Key("error_message"s).Value("not found"s)
            .EndDict().Build().AsDict();
    }

}

json::Node ParseRoute(std::optional<graph::Router<double>::RouteInfo> route, const router::TransportRouter& router) {
    json::Builder result;

    if (route.has_value()) {
        result.StartArray();
        for (auto& edgeid : route->edges) {
            auto& element = router.GetEdge(edgeid);

            if (element.bus_name == "") {
                result.StartDict()
                    .Key("stop_name"s).Value(router.GetIdsToStops().at(element.to)->Stop_name)
                    .Key("time"s).Value(element.weight)
                    .Key("type"s).Value("Wait"s)
                    .EndDict();
            }
            else {
                result.StartDict()
                    .Key("bus"s).Value(std::string(element.bus_name))
                    .Key("span_count"s).Value(element.span_count)
                    .Key("time"s).Value(element.weight)
                    .Key("type"s).Value("Bus"s)
                    .EndDict();
            }
        }
        return result.EndArray().Build();
    }

    return 	{};
}

json::Dict RouteResponseProcessing(router::TransportRouter& router, const json::Node& route_request) {
    json::Builder result;
    json::Node route = ParseRoute(router.BuildRoute(route_request.AsDict().at("from"s).AsString(), route_request.AsDict().at("to"s).AsString()),router);

    if (route.IsNull()) {
        result.StartDict().Key("error_message").Value("not found"s);
        result.Key("request_id"s).Value(route_request.AsDict().at("id").AsInt());
        return result.EndDict().Build().AsDict();
    }

    double total_time = 0;
    for (auto& element : route.AsArray()) {
        total_time += element.AsDict().at("time").AsDouble();
    }

    return result.StartDict().Key("items").Value(route).Key("request_id"s).Value(route_request.AsDict().at("id").AsInt()).Key("total_time").Value(total_time).EndDict().Build().AsDict();
}

json::Node StatRequestsProcessing(catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer, router::TransportRouter& router, const json::Array& stat_requests) {
    json::Builder responses;
    responses.StartArray();
    for (auto& request : stat_requests) {
        if (request.AsDict().at("type").AsString() == "Bus") {
            responses.Value(BusResponseProcessing(catalogue, request));
        }
        if (request.AsDict().at("type").AsString() == "Stop") {
            responses.Value(StopResponseProcessing(catalogue, request));
        }
        if (request.AsDict().at("type").AsString() == "Map") {
            responses.Value(MapResponseProcessing(catalogue, renderer, request));
        }
        if (request.AsDict().at("type").AsString() == "Route") {
            responses.Value(RouteResponseProcessing(router, request));
        }
    }
    return responses.EndArray().Build();
}

void PrintResponses(const json::Node& responses) {
    json::Print(responses, std::cout);
}

void InitializeAndSerializeDataBase() {
    catalogue::TransportCatalogue catalogue; 
    renderer::MapRenderer renderer;
    json::Node requests = json::Load(std::cin);

    BaseRequestsProcessing(catalogue, requests.AsDict().at("base_requests").AsArray());

    renderer.InsertSettings(SetRenderSettings(requests.AsDict().at("render_settings").AsDict()));

    router::TransportRouter transport_router(SetRouterSettings(requests.AsDict().at("routing_settings").AsDict()), &catalogue);
    transport_router.BuildRouter();

    SerializeDataBase(catalogue, renderer, transport_router, requests.AsDict().at("serialization_settings").AsDict().at("file").AsString());
}

void RequestsProcessing() {

    json::Node requests = json::Load(std::cin);

    transport_catalogue_serialize::DataBase database;

    std::ifstream in;
    in.open(requests.AsDict().at("serialization_settings").AsDict().at("file").AsString(), std::ios::binary);
    database.ParseFromIstream(&in);
    catalogue::TransportCatalogue catalogue(*database.mutable_catalogue_base());

    renderer::MapRenderer renderer;
    renderer.InsertSettings(*database.mutable_render_settings());
    router::TransportRouter transport_router(database.transport_router_base(), catalogue);

    PrintResponses(json::Builder().Value(StatRequestsProcessing(catalogue, renderer, transport_router, requests.AsDict().at("stat_requests").AsArray())).Build());
}
