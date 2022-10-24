#include "json.h"

using namespace std;

namespace json {

    namespace {
        Node LoadNode(istream& input);

        //LoadValue
        Node LoadArray(istream& input) {
            Array result;
            std::string temp_string;
            char c = -1;
            while (input >> c)
            {
                if (c == ']')
                {
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c == -1)
            {
                throw ParsingError("Failed to read array from stream");
            }
            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            string temp_string;
            for (size_t i = 0; i < 4; i++)
            {
                temp_string.push_back(input.get());
            }
            if (temp_string == "true") {
                return Node(true);
            }
            temp_string.push_back(input.get());
            if (temp_string == "false") {
                return Node(false);
            }
            throw ParsingError("Failed to read bool value from stream");
        }


        Node LoadNull(istream& input) {
            string temp_string;
            for (size_t i = 0; i < 4; i++)
            {
                temp_string.push_back(input.get());
            }
            if (temp_string == "null") {
                return Node(nullptr);
            }
            throw ParsingError("Failed to read null value from stream");
        }

        using Number = std::variant<int, double>;

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            string line;
            char c = -1;
            noskipws(input);
            while (input >> c)
            {
                if (c == '"') {
                    break;
                }
                if (c == '\\') {
                    input >> c;
                    if (c == 'n') {
                        line.push_back('\n');
                    }
                    else if (c == 'r') {
                        line.push_back('\r');
                    }
                    else if (c == '"') {
                        line.push_back('\"');
                    }
                    else if (c == 't') {
                        line.push_back('\t');
                    }
                    else if (c == '\\') {
                        line.push_back('\\');
                    }
                }
                else
                {
                    line.push_back(c);
                }
            }
            if (c != '\"')
            {
                throw ParsingError("Failed to read string value from stream");
            }
            if (c == -1)
            {
                throw ParsingError("Failed to read string value from stream");
            }
            skipws(input);
            return Node(move(line));
        }
        // только string
        Node LoadDict(istream& input) {
            string key;
            Dict result;
            char c = -1;
            while (input >> c)
            {
                if (c == '}')
                {
                    break;
                }
                if (c == ',')
                {
                    continue;
                }
                key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c == -1) {
                throw ParsingError("Failed to read array from stream");
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            if (c == ' ')
            {
                input >> c;
                //c = input.get();
            }
            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    //AsValue
    const Array& Node::AsArray() const {
        if (!IsArray())
        {
            throw logic_error("value != array");
        }
        return get<Array>(*this);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict())
        {
            throw logic_error("value != Map");
        }
        return get<Dict>(*this);
    }

    int Node::AsInt() const {
        if (!IsInt())
        {
            throw logic_error("value != int");
        }
        return get<int>(*this);
    }

    const string& Node::AsString() const {
        if (!IsString())
        {
            throw logic_error("value != string");
        }
        return get<string>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool())
        {
            throw logic_error("value != bool");
        }
        return get<bool>(*this);
    }

    double Node::AsDouble() const {
        if (IsInt())
        {
            return static_cast<double>(get<int>(*this));
        }
        if (IsPureDouble())
        {
            return get<double>(*this);
        }
        throw logic_error("value != double / int");
    }

    //IsValue
    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }
    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }
    bool Node::IsDouble() const {
        return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this);
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    Node Load(istream& input) {
        return LoadNode(input);
    }
    // PrintValue
    void PrintMap(const Node& node, std::ostream& out);
    void PrintArray(const Node& node, std::ostream& out);

    void PrintNull(const Node&, std::ostream& out) {
        out << "null";
    }

    void PrintDouble(const Node& node, std::ostream& out) {
        out << node.AsDouble();
    }

    void PrintInt(const Node& node, std::ostream& out) {
        out << node.AsInt();
    }

    void PrintBool(const Node& node, std::ostream& out) {
        out << std::boolalpha << node.AsBool();
    }

    void PrintString(const Node& node, std::ostream& out) {
        std::string_view temp_string = node.AsString();
        out << '\"';
        for (char symbol : temp_string)
        {
            if (symbol == '\n') {
                out << '\\';
                out << 'n';
            }
            else if (symbol == '\r') {
                out << '\\';
                out << 'r';
            }
            else if (symbol == '\"') {
                out << '\\';
                out << '\"';
            }
            else if (symbol == '\t') {
                out << '\\';
                out << 't';
            }
            else if (symbol == '\\') {
                out << '\\';
                out << '\\';
            }
            else
            {
                out << symbol;
            }
        }
        out << '\"';
    }

    void PrintValue(const Node& node, std::ostream& out) {

        if (node.IsArray()) {
            PrintArray(node, out);
        }
        else if (node.IsDict()) {
            PrintMap(node, out);
        }
        else if (node.IsInt()) {
            PrintInt(node, out);
        }
        else if (node.IsString()) {
            PrintString(node, out);
        }
        else if (node.IsBool()) {
            PrintBool(node, out);
        }
        else if (node.IsPureDouble()) {
            PrintDouble(node, out);
        }
        else
        {
            PrintNull(node, out);
        }
    }

    void PrintArray(const Node& node, std::ostream& out) {
        auto& temp_array = node.AsArray();
        out << '[';
        for (auto& element : node.AsArray())
        {
            PrintValue(element, out);
            if (&element != &temp_array.back())
            {
                out << ',';
            }
        }
        out << ']';
    }

    void PrintMap(const Node& node, std::ostream& out) {
        out << '{';
        for (auto& element : node.AsDict())
        {
            PrintValue(Node(element.first), out);
            out << ": ";
            PrintValue(element.second, out);
            if (&element != &*node.AsDict().rbegin())
            {
                out << ',';
            }
        }
        out << '}';
    }

    void Print(const Node& node, std::ostream& output) {
        PrintValue(node, output);
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintValue(doc.GetRoot(), output);
    }
}  // namespace json

