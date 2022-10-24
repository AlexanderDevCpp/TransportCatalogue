#include "svg.h"

namespace svg {

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap)
    {
        if (line_cap == StrokeLineCap::BUTT)
        {
            out << "butt";
        }
        if (line_cap == StrokeLineCap::ROUND)
        {
            out << "round";
        }
        if (line_cap == StrokeLineCap::SQUARE)
        {
            out << "square";
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join)
    {

        if (line_join == StrokeLineJoin::ARCS)
        {
            out << "arcs";
        }
        if (line_join == StrokeLineJoin::BEVEL)
        {
            out << "bevel";
        }
        if (line_join == StrokeLineJoin::MITER)
        {
            out << "miter";
        }
        if (line_join == StrokeLineJoin::MITER_CLIP)
        {
            out << "miter-clip";
        }
        if (line_join == StrokeLineJoin::ROUND)
        {
            out << "round";
        }
        return out;
    }

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Document ------------------
    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        RenderContext p(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (auto& object : objects_)
        {
            object.get()->Render(p);
        }
        out << "</svg>"sv;
    }
    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  " << "<circle ";
        out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::PrintPoints(std::ostream& out) const {
        for (size_t i = 0; i < points_.size(); i++)
        {
            out << points_[i].x << ',';

            if (i == points_.size() - 1)
            {
                out << points_[i].y;
            }
            else
            {
                out << points_[i].y << ' ';
            }
        }
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  " << "<polyline "sv;
        out << "points=\"";
        PrintPoints(out);
        out << '\"';
        RenderAttrs(out);
        out << " />"sv;
    }
    // ---------- Text ------------------
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = ParseData(data);
        return *this;
    }

    std::string Text::ParseData(std::string data) {
        std::string data_result{};
        data_result.reserve(data.size() * 10);
        for (size_t i = 0; i < data.size(); i++)
        {
            if (data[i] == '\"')
            {
                data_result.insert(data_result.size(), "&quot;");
            }
            else if (data[i] == '\'')
            {
                data_result.insert(data_result.size(), "&apos;"s);
            }
            else if (data[i] == '<')
            {
                data_result.insert(data_result.size(), "&lt;"s);
            }
            else if (data[i] == '>')
            {
                data_result.insert(data_result.size(), "&gt;"s);
            }
            else if (data[i] == '&')
            {
                data_result.insert(data_result.size(), "&amp;"s);
            }
            else
            {
                data_result.push_back(data[i]);
            }
        }
        data_result.shrink_to_fit();
        return data_result;
    }
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  " << "<text"sv;
        RenderAttrs(out);
        out << ' ' << "x=" << '\"' << pos_.x << '\"' << ' ' << "y=" << '\"' << pos_.y << '\"' << ' ';
        out << "dx=" << '\"' << offset_.x << '\"' << ' ' << "dy=" << '\"' << offset_.y << '\"' << ' ';
        out << "font-size="sv << '\"' << font_size_ << '\"' << ' ';
        if (!font_family_.empty())
        {
            out << "font-family="sv << '\"' << font_family_ << '\"' << ' ';
        }
        if (!font_weight_.empty())
        {
            out << "font-weight=" << '\"' << font_weight_ << '\"';
        }
        out << '>';
        out << data_;
        out << "</text>"sv;
    }

    // Прочие данные и методы, необходимые для реализации элемента <text>


    // ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

}  // namespace svg