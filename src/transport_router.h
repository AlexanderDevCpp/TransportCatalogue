#pragma once

#include <map>
#include <memory>
#include <vector>

#include "graph.h"
#include "router.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "transport_router.pb.h"
namespace router {

	const int SECONDS_IN_MINUTE = 60;

	struct RouterSettings
	{
		int bus_wait_time = 0;
		double bus_velocity = 0;
	};

	class TransportRouter {
	public:
		TransportRouter() = default;

		TransportRouter(RouterSettings settings, const catalogue::TransportCatalogue* catalogue)
			:settings_(settings), catalogue_(catalogue)
		{

		}

		TransportRouter(const transport_router_serialize::TransportRouterDataBase&, catalogue::TransportCatalogue& catalogue_);
		void AddRoute(const domain::Bus*);
		void BuildRouter();
		std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to);
		const graph::Edge<double>& GetEdge(graph::EdgeId) const;
		const std::map<graph::VertexId, const domain::Stop*>& GetIdsToStops() const;

		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		const graph::Router<double>& GetRouter() const;
		const router::RouterSettings& GetRouterSettings() const;

	private:
		RouterSettings settings_;

		std::map<graph::VertexId, const domain::Stop*> id_to_stop_;
		std::map<const domain::Stop*, graph::VertexId> stop_to_id_;

		const catalogue::TransportCatalogue* catalogue_ = nullptr;
		std::unique_ptr <graph::DirectedWeightedGraph<double>> graph_ = nullptr;
		std::unique_ptr<graph::Router<double>> router_ = nullptr;

		void InsertSettings(const transport_router_serialize::TransportRouterDataBase&);
		void InsertIdsAndStops(const transport_router_serialize::TransportRouterDataBase&);
		void InsertGraph(const transport_router_serialize::TransportRouterDataBase&, catalogue::TransportCatalogue&);
		void InsertRouter(const transport_router_serialize::TransportRouterDataBase&);
	};
}