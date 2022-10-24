#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"
#include <transport_catalogue.pb.h>
namespace catalogue {

	namespace detail {

		struct RouteStats {
			double route_length = 0;
			double curvature = 0.0;
		};

		struct RouteInfo
		{
			const domain::Bus* bus;
			catalogue::detail::RouteStats stats;
		};

		struct StopInfo {
			const domain::Stop* stop;
			std::set<std::string_view> buses{};
		};

		struct StopPairHasher {
			const int NUMBER = 31;
			size_t operator() (std::pair<const domain::Stop*, const domain::Stop*> pair) const {
				return ptr_hasher(pair.first) * NUMBER + ptr_hasher(pair.second) * (NUMBER * NUMBER);
			}
			std::hash<const void*> ptr_hasher;
		};
	}

	class TransportCatalogue {
	public:

		TransportCatalogue() = default;
		TransportCatalogue(transport_catalogue_serialize::TransportCatalogue& db);

		void AddStop(std::string stop, geo::Coordinates coords);
		void AddBusRoute(std::string bus, std::vector<std::string_view> stops, bool);
		void SetStopDistance(const domain::Stop* from, const domain::Stop* to, int diststance);

		const domain::Stop* FindStop(std::string_view stop_name) const;
		const domain::Bus* FindBusRoute(std::string_view bus_name) const;

		catalogue::detail::RouteInfo GetRouteInfo(const domain::Bus* bus) const;
		catalogue::detail::StopInfo GetStopInfo(const domain::Stop* stop) const;
		const std::vector<const domain::Bus*>& GetRoutes() const;
		const std::vector<const domain::Stop*>& GetStops() const;
		const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, catalogue::detail::StopPairHasher>& GetStopDistances() const;
		const std::unordered_map<const domain::Stop*, std::unordered_set <std::string_view>>& GetStopToBuses() const;
		int GetStopDistance(const domain::Stop* from, const domain::Stop* to) const;
		catalogue::detail::RouteStats ComputeRouteStats(const domain::Bus* temp_bus) const;
	private:
		std::deque<domain::Stop> bus_stops_;
		std::deque<domain::Bus> buses_;

		std::vector<const domain::Bus*> bus_ptrs_;
		std::vector<const domain::Stop*> stop_ptrs_;

		std::unordered_map<const domain::Stop*, std::unordered_set <std::string_view>> stop_to_buses_;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, catalogue::detail::StopPairHasher> stop_distance_;

		std::unordered_map<std::string_view, const domain::Stop*> view_to_stop_;
		std::unordered_map<std::string_view, const domain::Bus*> view_to_bus_;

		void InsertBuses(transport_catalogue_serialize::TransportCatalogue& db);
		void InsertStops(transport_catalogue_serialize::TransportCatalogue& db);
		void InsertStopToBuses(transport_catalogue_serialize::TransportCatalogue& db);
		void InsertDistances(transport_catalogue_serialize::TransportCatalogue& db);
	};
}