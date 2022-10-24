#include "transport_catalogue.h"
//using namespace catalogue;
using namespace catalogue::detail;
using namespace geo;
using namespace domain;
namespace catalogue {

	TransportCatalogue::TransportCatalogue(transport_catalogue_serialize::TransportCatalogue& db) {
		InsertStops(db);
		InsertBuses(db);
		InsertStopToBuses(db);
		InsertDistances(db);
	}

	void TransportCatalogue::AddStop(std::string stop_name, Coordinates coords) {
		bus_stops_.push_back({ std::move(stop_name),std::move(coords) });
		view_to_stop_.insert({ bus_stops_.back().Stop_name, &bus_stops_.back() });
		stop_ptrs_.push_back(&bus_stops_.back());
	}
	void TransportCatalogue::AddBusRoute(std::string bus_name, std::vector<std::string_view> stops, bool is_roundtrip) {

		Bus temp_bus;
		temp_bus.bus_name = std::move(bus_name);
		temp_bus.is_roundtrip = is_roundtrip;
		for (size_t i = 0; i < stops.size(); i++)
		{
			const Stop* temp_stop = FindStop(stops[i]);
			temp_bus.route.push_back(temp_stop);
			temp_bus.unique_stops.insert(temp_stop);
		}

		buses_.push_back(std::move(temp_bus));
		bus_ptrs_.push_back(&buses_.back());
		view_to_bus_.insert({ buses_.back().bus_name, &buses_.back() });


		for (const Stop* stop : buses_.back().unique_stops)
		{
			stop_to_buses_[stop].insert(buses_.back().bus_name);
		}
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
		auto ptr = view_to_stop_.find(stop_name);
		if (ptr != view_to_stop_.end())
		{
			return ptr->second;
		}
		return nullptr;
	}

	const Bus* TransportCatalogue::FindBusRoute(std::string_view bus_name) const {
		auto ptr = view_to_bus_.find(bus_name);
		if (ptr != view_to_bus_.end())
		{
			return ptr->second;
		}
		return nullptr;
	}

	RouteStats TransportCatalogue::ComputeRouteStats(const Bus* temp_bus) const {

		RouteStats temp_stats;

		for (size_t i = 0; i < temp_bus->route.size() - 1; i++)
		{
			temp_stats.route_length += GetStopDistance(temp_bus->route[i], temp_bus->route[i + 1]);
			temp_stats.curvature += ComputeDistance(temp_bus->route[i]->coordinates, temp_bus->route[i + 1]->coordinates);
		}

		temp_stats.curvature = temp_stats.route_length / temp_stats.curvature;

		return temp_stats;
	}

	RouteInfo TransportCatalogue::GetRouteInfo(const Bus* bus) const {
		return { bus ,ComputeRouteStats(bus) };
	}

	StopInfo TransportCatalogue::GetStopInfo(const Stop* stop) const {
		if (stop_to_buses_.count(stop))
		{
			return { stop,{stop_to_buses_.at(stop).begin(),stop_to_buses_.at(stop).end()} };
		}
		return { stop,{} };
	}

	void TransportCatalogue::SetStopDistance(const Stop* from, const Stop* to, int distance) {
		stop_distance_[{from, to}] = distance;
	}

	int TransportCatalogue::GetStopDistance(const Stop* from, const Stop* to) const {
		if (stop_distance_.count({ from,to }))
		{
			return stop_distance_.at({ from,to });
		}
		else if (stop_distance_.count({ to,from })) {
			return stop_distance_.at({ to,from });
		}
		return 0;
	}

	const std::vector<const domain::Bus*>& TransportCatalogue::GetRoutes() const{
		return bus_ptrs_;
		
	}
	const std::vector<const domain::Stop*>& TransportCatalogue::GetStops() const {
		return stop_ptrs_;
	}

	const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, catalogue::detail::StopPairHasher>& TransportCatalogue::GetStopDistances() const {
		return stop_distance_;
	}

	const std::unordered_map<const domain::Stop*, std::unordered_set <std::string_view>>& TransportCatalogue::GetStopToBuses() const{
		return stop_to_buses_;
	}

//private:

void TransportCatalogue::InsertBuses(transport_catalogue_serialize::TransportCatalogue& db) {
	buses_.resize(db.index_to_bus().size());
	bus_ptrs_.resize(db.index_to_bus().size());
	for (auto& [index, bus] : db.index_to_bus()) {
		Bus temp_bus;
		temp_bus.bus_name = std::string{ bus.bus_name() };
		temp_bus.is_roundtrip = bus.is_roundtrip();
		temp_bus.curvature = bus.curvature();
		temp_bus.route_length = bus.route_length();

		for (auto stop : bus.stops()) {
			temp_bus.route.push_back(&bus_stops_[stop]);
			temp_bus.unique_stops.insert(&bus_stops_[stop]);
		}

		buses_[index] = std::move(temp_bus);
		bus_ptrs_[index] = &buses_[index];
		view_to_bus_.insert({ buses_[index].bus_name, &buses_[index] });
	}
}
void TransportCatalogue::InsertStops(transport_catalogue_serialize::TransportCatalogue& db) {
	bus_stops_.resize(db.index_to_stop().size());
	stop_ptrs_.resize(db.index_to_stop().size());
	for (auto& [index, stop] : db.index_to_stop()) {
		Stop temp_stop;
		temp_stop.coordinates = { stop.coords().lat(),stop.coords().lng() };
		temp_stop.Stop_name = std::string{ stop.stop_name() };
		bus_stops_[index] = std::move(temp_stop);

		view_to_stop_.insert({ bus_stops_[index].Stop_name, &bus_stops_[index] });
		stop_ptrs_[index] = &bus_stops_[index];
	}
}
void TransportCatalogue::InsertStopToBuses(transport_catalogue_serialize::TransportCatalogue& db) {
	for (auto [stop, buses] : db.stop_to_buses()) {
		stop_to_buses_[&bus_stops_[stop]];
		for (auto bus : buses.buses()) {
			stop_to_buses_[&bus_stops_[stop]].insert(buses_[bus].bus_name);
		}
	}
}
void TransportCatalogue::InsertDistances(transport_catalogue_serialize::TransportCatalogue& db) {
	for (auto distance : db.distances()) {
		stop_distance_[{&bus_stops_[distance.from()], & bus_stops_[distance.to()]}] = distance.distance();
	}
}

}