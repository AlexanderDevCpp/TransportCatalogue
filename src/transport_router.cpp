#include "transport_router.h"

using namespace std::literals;

router::TransportRouter::TransportRouter(const transport_router_serialize::TransportRouterDataBase& db, catalogue::TransportCatalogue& catalogue) {
	catalogue_ = &catalogue;
	InsertSettings(db);
	InsertIdsAndStops(db);
	InsertGraph(db, catalogue);
	InsertRouter(db);
}

void router::TransportRouter::AddRoute(const domain::Bus* bus) {
	for (size_t i = 0; i < bus->route.size(); i++) {
		for (size_t c = i; c < bus->route.size(); c++) {
			double distance = 0;
			int span_count = 0;

			if (i == c) {
				continue;
			}

			for (size_t b = i; b < bus->route.size(); b++) {
				if (bus->route[c] == bus->route[b]) {
					break;
				}

				distance += catalogue_->GetStopDistance(bus->route[b], bus->route[b+1]);
				span_count++;
			}

			graph_->AddEdge({ stop_to_id_.at(bus->route[i]),stop_to_id_.at(bus->route[c])-1,(distance / settings_.bus_velocity) / SECONDS_IN_MINUTE,bus->bus_name,span_count });

		}

	}
}
void router::TransportRouter::BuildRouter() {
	graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(graph::DirectedWeightedGraph<double>(catalogue_->GetStops().size() * 2));
	size_t id = 1;

	for (const auto& stop : catalogue_->GetStops()) {
		id_to_stop_.emplace(id, stop);
		stop_to_id_.emplace(stop, id);

		id++;
		id++;
	}

	for (auto stop : catalogue_->GetStops()) {
		graph_->AddEdge({ stop_to_id_.at(stop) - 1,stop_to_id_.at(stop),static_cast<double>(settings_.bus_wait_time) });
	}

	for (auto& bus : catalogue_->GetRoutes()) {
		AddRoute(bus);
	}

	router_ = std::make_unique<graph::Router<double>>(graph::Router<double>(*graph_));
}

std::optional<graph::Router<double>::RouteInfo> router::TransportRouter::BuildRoute(std::string_view from, std::string_view to) {
	return router_->BuildRoute(stop_to_id_.at(catalogue_->FindStop(from)) - 1, stop_to_id_.at(catalogue_->FindStop(to)) - 1);
}

const graph::Edge<double>& router::TransportRouter::GetEdge(graph::EdgeId id) const {
	return graph_->GetEdge(id);
}
const std::map<graph::VertexId, const domain::Stop*>& router::TransportRouter::GetIdsToStops() const {
	return id_to_stop_;
}

const graph::DirectedWeightedGraph<double>& router::TransportRouter::GetGraph() const {
	return *graph_;
}

const graph::Router<double>& router::TransportRouter::GetRouter() const {
	return *router_;
}

const router::RouterSettings& router::TransportRouter::GetRouterSettings() const {
	return settings_;
}

void router::TransportRouter::InsertSettings(const transport_router_serialize::TransportRouterDataBase& db) {
	settings_.bus_velocity = db.settings().bus_velocity();
	settings_.bus_wait_time = db.settings().bus_wait_time();
}

void router::TransportRouter::InsertIdsAndStops(const transport_router_serialize::TransportRouterDataBase& db) {
	size_t id = 1;
	for (const auto& stop : catalogue_->GetStops()) {
		id_to_stop_.emplace(id, stop);
		stop_to_id_.emplace(stop, id);

		id++;
		id++;
	}
}

void router::TransportRouter::InsertGraph(const transport_router_serialize::TransportRouterDataBase& db, catalogue::TransportCatalogue& catalogue) {
	std::vector<std::vector<size_t>> incidence_lists;
	for (auto& list : db.graph().incidence_lists_()) {
		std::vector<size_t> temp_data;
		for (auto data : list.list()) {
			temp_data.push_back(data);
		}
		incidence_lists.push_back(temp_data);
	}

	std::vector<graph::Edge<double>> edges;
	for (auto& edge : db.graph().edges_()) {
		graph::Edge<double> temp_edge;

		temp_edge.from = edge.from();
		temp_edge.to = edge.to();
		temp_edge.weight = edge.weight();
		if (edge.bus_name_case() == 5) {
			temp_edge.bus_name = catalogue.GetRoutes()[edge.data()]->bus_name;
		}

		temp_edge.span_count = edge.span_count();

		edges.push_back(temp_edge);
	}

	graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(graph::DirectedWeightedGraph<double>(std::move(edges), std::move(incidence_lists)));
}

void router::TransportRouter::InsertRouter(const transport_router_serialize::TransportRouterDataBase& db) {
	router_ = std::make_unique<graph::Router<double>>(graph::Router<double>(db.router(), *graph_));
}
