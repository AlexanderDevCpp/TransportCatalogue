#include "map_renderer.h"


inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:

    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
        double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

svg::Color GetColor(const map_renderer_serialize::Color& color) {
    if (color.ColorSet_case() == 1) {
        return {};
    }
    else if (color.ColorSet_case() == 2) {
        svg::Rgb result(color.rgb().red(), color.rgb().green(), color.rgb().blue());
        return result;
    }
    else if (color.ColorSet_case() == 3) {
        svg::Rgba result(color.rgba().red(), color.rgba().green(), color.rgba().blue(), color.rgba().opacity());
        return result;
    }
    else {
        return color.string_color();
    }
}

std::vector<svg::Color> GetColorVector(const map_renderer_serialize::RenderSettings& settings) {
    std::vector<svg::Color> result;

    for (auto& color : settings.color_palette_()) {
        result.push_back(GetColor(color));
    }

    return result;
}

void renderer::MapRenderer::InsertSettings(map_renderer_serialize::RenderSettings& settings) {
    settings_.width_ = settings.width_();
    settings_.height_ = settings.height_();
    settings_.padding_ = settings.padding_();
    settings_.line_width_ = settings.line_width_();

    settings_.stop_radius_ = settings.stop_radius_();
    settings_.stop_label_offset_ = { settings.stop_label_offset_().x(),settings.stop_label_offset_().y() };
    settings_.stop_label_font_size_ = settings.stop_label_font_size_();

    settings_.bus_label_font_size_ = settings.bus_label_font_size_();
    settings_.bus_label_offset_ = { settings.bus_label_offset_().x(),settings.bus_label_offset_().y() };
    settings_.underlayer_width_ = settings.underlayer_width_();

    settings_.underlayer_color_ = GetColor(settings.underlayer_color_());
    settings_.color_palette_ = GetColorVector(settings);
}

void renderer::MapRenderer::InsertSettings(const renderer::RendererSettings& settings) {
    settings_ = std::move(settings);
}

svg::Text renderer::MapRenderer::RenderStopNameUnderlayer(svg::Color&, SphereProjector& projector, const domain::Stop* stop) {
    svg::Text result;
    result.
        SetData(stop->Stop_name).
        SetFillColor(settings_.underlayer_color_).
        SetStrokeColor(settings_.underlayer_color_).
        SetPosition(projector(stop->coordinates)).
        SetOffset(settings_.stop_label_offset_).
        SetFontSize(settings_.stop_label_font_size_).
        SetFontFamily("Verdana").
        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
        SetStrokeWidth(settings_.underlayer_width_);
    return result;
}


svg::Text renderer::MapRenderer::RenderStopName(svg::Color&, SphereProjector& projector, const domain::Stop* stop) {
    svg::Text result;
    result.
        SetData(stop->Stop_name).
        SetFillColor("black").
        SetPosition(projector(stop->coordinates)).
        SetOffset(settings_.stop_label_offset_).
        SetFontSize(settings_.stop_label_font_size_).
        SetFontFamily("Verdana");
    return result;
}

svg::Circle renderer::MapRenderer::RenderStopCircle(SphereProjector& projector, const domain::Stop* stop) {
    svg::Circle result;
    result.
        SetCenter(projector(stop->coordinates)).
        SetRadius(settings_.stop_radius_).
        SetFillColor("white");
    return result;
}

svg::Text renderer::MapRenderer::RenderRouteNameUnderlayer(svg::Color& color, SphereProjector& projector, const domain::Bus* bus, const domain::Stop* stop) {
    svg::Text result;

    result.
        SetData(bus->bus_name).
        SetFillColor(color).
        SetStrokeColor(color).
        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
        SetStrokeWidth(settings_.underlayer_width_).
        SetPosition(projector(stop->coordinates)).
        SetOffset(settings_.bus_label_offset_).
        SetFontSize(settings_.bus_label_font_size_).
        SetFontFamily("Verdana").
        SetFontWeight("bold");

    return result;
}

svg::Text renderer::MapRenderer::RenderRouteName(svg::Color& color, SphereProjector& projector, const domain::Bus* bus, const domain::Stop* stop) {
    svg::Text result;

    result.
        SetData(bus->bus_name).
        SetFillColor(color).
        SetPosition(projector(stop->coordinates)).
        SetOffset(settings_.bus_label_offset_).
        SetFontSize(settings_.bus_label_font_size_).
        SetFontFamily("Verdana").
        SetFontWeight("bold");

    return result;
}

svg::Polyline renderer::MapRenderer::RenderRoutePath(svg::Color& color, SphereProjector& projector, const domain::Bus* bus) {
    svg::Polyline result;

    result.
        SetStrokeColor(color).
        SetStrokeWidth(settings_.line_width_).
        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
        SetFillColor(svg::Color());

    for (auto& stop : bus->route)
    {
        result.AddPoint(projector(stop->coordinates));
    }
    return result;
}

void renderer::MapRenderer::RenderMap(svg::RenderContext& context, std::vector<const domain::Bus*> routes) {
    std::sort(routes.begin(), routes.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {return lhs->bus_name < rhs->bus_name; });
    std::set <domain::Stop, std::less<>> stops;
    std::set <geo::Coordinates, std::less<>> coords;

    int color_index = 0;
    for (auto& route : routes)
    {
        for (auto& stop : route->unique_stops)
        {
            stops.insert(*stop);
            coords.insert({ stop->coordinates });
        }
    }

    SphereProjector projector(coords.begin(), coords.end(), this->settings_.width_, settings_.height_, settings_.padding_);
    std::vector<svg::Polyline> polylines;
    polylines.reserve(routes.size());
    std::vector<svg::Text> route_names;
    route_names.reserve(routes.size());
    std::vector<svg::Circle> stop_circles;
    stop_circles.reserve(stops.size());
    std::vector<svg::Text> stop_names;
    stop_names.reserve(stops.size());

    for (auto& route : routes)
    {
        if (!route->route.empty()) {

            if (color_index == static_cast<int>((settings_.color_palette_.size()))) {
                color_index = 0;
            }

            if (route->route.empty()) {
                continue;
            }

            polylines.push_back(std::move(RenderRoutePath(settings_.color_palette_[color_index], projector, route)));

            if (route->is_roundtrip == true) {
                route_names.push_back(std::move(RenderRouteNameUnderlayer(settings_.underlayer_color_, projector, route, route->route[0])));
                route_names.push_back(std::move(RenderRouteName(settings_.color_palette_[color_index], projector, route, route->route[0])));
            }
            else {

                route_names.push_back(std::move(RenderRouteNameUnderlayer(settings_.underlayer_color_, projector, route, route->route[0])));
                route_names.push_back(std::move(RenderRouteName(settings_.color_palette_[color_index], projector, route, route->route[0])));

                if (route->route[0] != route->route[route->route.size() / 2])
                {
                    route_names.push_back(std::move(RenderRouteNameUnderlayer(settings_.underlayer_color_, projector, route, route->route[route->route.size() / 2])));
                    route_names.push_back(std::move(RenderRouteName(settings_.color_palette_[color_index], projector, route, route->route[route->route.size() / 2])));
                }

            }
            color_index++;
        }
    }

    if (color_index == static_cast<int>((settings_.color_palette_.size()))) {
        color_index = 0;
    }

    for (auto& stop : stops)
    {
        stop_circles.push_back(std::move(RenderStopCircle(projector, &stop)));
        stop_names.push_back(std::move(RenderStopNameUnderlayer(settings_.underlayer_color_, projector, &stop)));
        stop_names.push_back(std::move(RenderStopName(settings_.color_palette_[color_index], projector, &stop)));
    }

    for (auto& line : polylines)
    {
        line.Render(context);
    }

    for (auto& text : route_names)
    {
        text.Render(context);
    }

    for (auto& circle : stop_circles)
    {
        circle.Render(context);
    }

    for (auto& text : stop_names)
    {
        text.Render(context);
    }
}


const renderer::RendererSettings& renderer::MapRenderer::GetSettings() const {
    return settings_;
}
