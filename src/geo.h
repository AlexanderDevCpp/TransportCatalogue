#pragma once

namespace geo {

struct Coordinates {

    bool operator<(const Coordinates& other) const {
        return (lat < other.lat);
    }
    bool operator==(const Coordinates& other) const {
        return ((lat == other.lat) && (lng == other.lng));
    }
    double lat; // Широта
    double lng; // Долгота
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo