#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace svg {

    class Rgb
    {
    public:
        Rgb() {
            red = 0;
            green = 0;
            blue = 0;

        }
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            :red(r), green(g), blue(b)
        {

        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    class Rgba
    {
    public:
        Rgba() {
            red = 0;
            green = 0;
            blue = 0;
            opacity = 1.0;
        }
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o)
            :red(r), green(g), blue(b), opacity(o)
        {

        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    struct PrintColor {
        std::ostream& out;
        void operator()(std::monostate) {
            out << "none";
        }
        void operator()(std::string color) {
            out << color;
        }
        void operator()(Rgb colors) {
            out << "rgb(" << static_cast<int>(colors.red) << ',' << static_cast<int>(colors.green) << ',' << static_cast<int>(colors.blue) << ")";
        }
        void operator()(Rgba colors) {
            out << "rgba(" << static_cast<int>(colors.red) << ',' << static_cast<int>(colors.green) << ',' << static_cast<int>(colors.blue) << ',' << colors.opacity << ")";
        }
    };

    using Color = std::variant <std::monostate, svg::Rgb, svg::Rgba, std::string >;

    inline const Color NoneColor;

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = std::move(line_join);
            return AsOwner();
        }
    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                std::visit(PrintColor{ out }, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(PrintColor{ out }, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;

    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0.0;
        double y = 0.0;
    };
    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };


    class ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container)const = 0;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    class Polyline : public Object, public PathProps<Polyline> {
    public:

        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void PrintPoints(std::ostream& out) const;
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:

        bool operator<(const svg::Text& rhs) const{
            return this->data_ < rhs.data_;
        }

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);
    private:

        std::string ParseData(std::string data);
        void RenderObject(const RenderContext& context) const override;

        std::string data_;
        std::string font_;
        std::string font_family_;
        uint32_t font_size_ = 1;
        std::string font_weight_;
        Point pos_ = { 0.0,0.0 };
        Point offset_ = { 0.0,0.0 };

    };

    class Document :public ObjectContainer {
    public:
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::deque<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg



