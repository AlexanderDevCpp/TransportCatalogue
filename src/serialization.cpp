#include "serialization.h"

map_renderer_serialize::Color GetColor(const svg::Color& color) {
	map_renderer_serialize::Color result;

	if (color.index() == 1) {
		map_renderer_serialize::Rgb rgb;
		rgb.set_red(std::get<svg::Rgb>(color).red);
		rgb.set_green(std::get<svg::Rgb>(color).green);
		rgb.set_blue(std::get<svg::Rgb>(color).blue);

		*result.mutable_rgb() = rgb;
	}
	else if (color.index() == 2) {
		map_renderer_serialize::Rgba rgba;
		rgba.set_red(std::get<svg::Rgba>(color).red);
		rgba.set_green(std::get<svg::Rgba>(color).green);
		rgba.set_blue(std::get<svg::Rgba>(color).blue);
		rgba.set_opacity(std::get<svg::Rgba>(color).opacity);

		*result.mutable_rgba() = rgba;
	}
	else if (color.index() == 3) {
		*result.mutable_string_color() = std::get<std::string>(color);
	}

	return result;
}

void SetColor (const renderer::RendererSettings& settings, map_renderer_serialize::RenderSettings& serialize_settings) {
	*serialize_settings.mutable_underlayer_color_() = GetColor(settings.underlayer_color_);
}

void SetColorPalette(const renderer::RendererSettings& settings, map_renderer_serialize::RenderSettings& serialize_settings) {
	for (auto& color : settings.color_palette_) {
		*serialize_settings.mutable_color_palette_()->Add() = GetColor(color);
	}
}

void SetStops(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book) {
	int index = 0;
	for (auto& stop : catalogue.GetStops()) {
		transport_catalogue_serialize::Stop temp_stop;
		temp_stop.set_stop_name(stop->Stop_name);

		transport_catalogue_serialize::Coords coords;
		coords.set_lat(stop->coordinates.lat);
		coords.set_lng(stop->coordinates.lng);
		*temp_stop.mutable_coords() = coords;

		db.mutable_index_to_stop()->insert({ index, temp_stop });
		book.stop_to_index[stop->Stop_name] = index;

		index++;
	}

}

void SetBuses(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book) {
	int index = 0;

	for (auto bus : catalogue.GetRoutes()) {
		transport_catalogue_serialize::Bus temp_bus;
		temp_bus.set_bus_name(bus->bus_name);
		temp_bus.set_curvature(bus->curvature);
		temp_bus.set_route_length(bus->route_length);
		temp_bus.set_is_roundtrip(bus->is_roundtrip);

		for (auto stop : bus->route) {
			temp_bus.mutable_stops()->Add(book.stop_to_index.at(stop->Stop_name));
		}

		book.bus_to_index[bus->bus_name] = index;

		db.mutable_index_to_bus()->insert({ index, temp_bus });
		index++;
	}
}

void SetStopToBuses(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book) {
	for (auto [stop,buses] : catalogue.GetStopToBuses()) {
		transport_catalogue_serialize::BusVector temp_buses;
		for (auto bus : buses) {
			temp_buses.add_buses(book.bus_to_index.at(bus));
		}
		db.mutable_stop_to_buses()->insert({ book.stop_to_index.at(stop->Stop_name), temp_buses });
	}
}

void SetDistances(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& db, IndexBook& book) {
	for (auto distance : catalogue.GetStopDistances()) {
		transport_catalogue_serialize::Distance dist;
		dist.set_from(book.stop_to_index.at(distance.first.first->Stop_name));
		dist.set_to(book.stop_to_index.at(distance.first.second->Stop_name));
		dist.set_distance(distance.second);
		*db.mutable_distances()->Add() = dist;
	}
}

void SerializeTransportCatalogue(const catalogue::TransportCatalogue& catalogue, transport_catalogue_serialize::DataBase& db, IndexBook& book) {
	transport_catalogue_serialize::TransportCatalogue result;

	SetStops(catalogue, result, book);
	SetBuses(catalogue, result, book);
	SetStopToBuses(catalogue, result, book);
	SetDistances(catalogue, result, book);

	*db.mutable_catalogue_base() = result;

}

void SerializeMapRenderSettings(const renderer::MapRenderer& renderer, transport_catalogue_serialize::DataBase& db) {
	auto& settings = renderer.GetSettings();
	map_renderer_serialize::RenderSettings result;

	result.set_width_(settings.width_);
	result.set_height_(settings.height_);
	result.set_padding_(settings.padding_);
	result.set_line_width_(settings.line_width_);

	result.set_stop_radius_(settings.stop_radius_);

	map_renderer_serialize::Point stop_label_offset;

	stop_label_offset.set_x(settings.stop_label_offset_.x);
	stop_label_offset.set_y(settings.stop_label_offset_.y);
	*result.mutable_stop_label_offset_() = stop_label_offset;

	result.set_stop_label_font_size_(settings.stop_label_font_size_);
	result.set_bus_label_font_size_(settings.bus_label_font_size_);

	map_renderer_serialize::Point bus_label_offset;

	bus_label_offset.set_x(settings.bus_label_offset_.x);
	bus_label_offset.set_y(settings.bus_label_offset_.y);
	*result.mutable_bus_label_offset_() = bus_label_offset;

	result.set_underlayer_width_(settings.underlayer_width_);

	SetColor(settings, result);
	SetColorPalette(settings, result);

	*db.mutable_render_settings() = result;
}

void SerializeRouter(const graph::Router<double>& router, transport_catalogue_serialize::DataBase& db) {
	for (auto& vector : router.GetData()) {
		transport_router_serialize::RoutesInternalDataVector temp_data;
		for (auto& data : vector) {
			transport_router_serialize::RoutesInternalData internal_data;
			transport_router_serialize::RoutesInternalDataOptional optional_data;
			if (data.has_value()) {

				internal_data.set_weight(data->weight);
				if (data->prev_edge.has_value()) {
					internal_data.set_edgeid(*data->prev_edge);
				}
				else {
					internal_data.set_nullopt(true);
				}
				*optional_data.mutable_data() = internal_data;
			}
			else {
				optional_data.set_nullopt(true);
			}
			*temp_data.add_data() = optional_data;
		}
		*db.mutable_transport_router_base()->mutable_router()->mutable_routes_internal_data()->Add() = temp_data;
	}
}

void SerializeGraph(const graph::DirectedWeightedGraph<double>& graph, transport_catalogue_serialize::DataBase& db, IndexBook& book) {
	for (auto& edge : graph.GetEdges()) {
		graph_serialize::Edge temp_edge;
		temp_edge.set_from(edge.from);
		temp_edge.set_to(edge.to);
		temp_edge.set_weight(edge.weight);
		temp_edge.set_span_count(edge.span_count);

		if (edge.bus_name != "") {
			temp_edge.set_data(book.bus_to_index.at(edge.bus_name));
		}
		else {
			temp_edge.set_nullopt(true);
		}

		*db.mutable_transport_router_base()->mutable_graph()->add_edges_() = temp_edge;
	}

	for (auto& list : graph.GetIncidenceLists()) {
		graph_serialize::IncidenceList temp_list;
		for (auto edge_id : list) {
			temp_list.add_list(edge_id);
		}
		*db.mutable_transport_router_base()->mutable_graph()->mutable_incidence_lists_()->Add() = temp_list;
	}


}

void SerializeRouterSettings(const router::RouterSettings& settings, transport_catalogue_serialize::DataBase& db) {
	db.mutable_transport_router_base()->mutable_settings()->set_bus_velocity(settings.bus_velocity);
	db.mutable_transport_router_base()->mutable_settings()->set_bus_wait_time(settings.bus_wait_time);
}

void SerializeTransportRouter(const router::TransportRouter& router, transport_catalogue_serialize::DataBase& db, IndexBook& book) {

	SerializeRouterSettings(router.GetRouterSettings(), db);
	SerializeGraph(router.GetGraph(), db, book);
	SerializeRouter(router.GetRouter(), db);
}

void SerializeDataBase(const catalogue::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const router::TransportRouter& router, std::string filename) {
	transport_catalogue_serialize::DataBase result;
	IndexBook book;
	SerializeTransportCatalogue(catalogue, result, book);
	SerializeMapRenderSettings(renderer, result);
	SerializeTransportRouter(router, result, book);
	std::ofstream out;
	out.open(filename, std::ios::binary);
	result.SerializeToOstream(&out);
}