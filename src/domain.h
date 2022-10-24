#pragma once

#include <string>
#include <set>
#include <vector>
#include <unordered_set>

#include "geo.h"

namespace domain {
	struct Stop {

		bool operator<(const Stop* other) const {
			return Stop_name < other->Stop_name;
		}

		bool operator<(const Stop& other) const {
			return Stop_name < other.Stop_name;
		}

		bool operator==(const Stop& other) const {
			return Stop_name == other.Stop_name;
		}

		std::string Stop_name;
		geo::Coordinates coordinates;
	};


	struct Bus {

		bool operator==(const Bus& other) const {
			return bus_name == other.bus_name;
		}

		std::string bus_name;

		std::set <const Stop*> unique_stops;
		std::vector <const Stop*> route;

		double route_length = 0;
		double curvature = 0.0;
		bool is_roundtrip = false;
	};

}
