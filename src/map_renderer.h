#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <sstream>

#include "svg.h"
#include "geo.h"
#include "domain.h"
#include "map_renderer.pb.h"


class SphereProjector;

namespace renderer {
    struct RendererSettings
    {
        double width_ = 0;
        double height_ = 0;
        double padding_ = 0;
        double line_width_ = 0;

        double stop_radius_ = 0;
        svg::Point stop_label_offset_ = {};
        int stop_label_font_size_ = 0;

        int bus_label_font_size_ = 0;
        svg::Point bus_label_offset_ = {};
        double underlayer_width_ = 0;

        svg::Color underlayer_color_;
        std::vector<svg::Color> color_palette_;
    };

    class MapRenderer {
    public:

        MapRenderer() = default;

        MapRenderer(RendererSettings settings)
            :settings_(settings)
        {

        }

        void InsertSettings(map_renderer_serialize::RenderSettings& settings);
        void InsertSettings(const renderer::RendererSettings& settings);

        svg::Text RenderStopNameUnderlayer(svg::Color&, SphereProjector&, const domain::Stop*);
        svg::Text RenderStopName(svg::Color&, SphereProjector&, const domain::Stop*);
        svg::Circle RenderStopCircle(SphereProjector&, const domain::Stop*);

        svg::Text RenderRouteNameUnderlayer(svg::Color&, SphereProjector&, const domain::Bus*, const domain::Stop*);
        svg::Text RenderRouteName(svg::Color&, SphereProjector&, const domain::Bus*, const domain::Stop*);
        svg::Polyline RenderRoutePath(svg::Color&, SphereProjector&, const domain::Bus*);

        void RenderMap(svg::RenderContext& context, const std::vector<const domain::Bus*> routes);

        const RendererSettings& GetSettings() const;
    private:
        RendererSettings settings_;
    };

}
