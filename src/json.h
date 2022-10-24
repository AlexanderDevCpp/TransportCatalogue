#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map < std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::nullptr_t, Array, Dict, int, std::string, bool, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, int, std::string, bool, double>
    {
    public:
        using variant::variant;

        bool operator==(json::Node rhs) const {
            return *this == rhs;
        }

        bool operator!=(json::Node rhs) const {
            return *this != rhs;
        }

        //AsValue

        const Array& AsArray() const;
        const Dict& AsDict() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        //IsValue

        bool IsNull() const;
        bool IsArray()const;
        bool IsDict() const;
        bool IsInt() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        const Value& GetValue() const {
            return *this;
        }
        Value& GetValue() {
            return *this;
        }
    private:
    };

    Node Load(std::istream& input);
    void Print(const Node& node, std::ostream& output);


    class Document {
    public:
        explicit Document(Node root)
            : root_(std::move(root)) {
        }

        const Node& GetRoot() const {
            return root_;
        }

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    void Print(const Document& doc, std::ostream& output);


}  // namespace json