#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (input.fail()) {
        throw ParsingError("");
    }

    return Node(move(result));
}

Node LoadString(istream& input) {
    std::string text;
    char c;
    c = input.get();
    while ((c != '"')) {
        if (c == '\\') {
            char next;
            next = input.get();
            if (next == '\\') {
                c = '\\';
            }
            if (next == 'r') {
                c = '\r';
            }
            if (next == 'n') {
                c = '\n';
            }
            if (next == 't') {
                c = '\t';
            }
            if (next == '"') {
                c = '"';
            }
        }
        text += c;
        c = input.get();
        if (c == -1) {
            throw ParsingError("");
        }
    }
    return Node(std::move(text));
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    bool end_of_dict_found = false;
    while (!end_of_dict_found) {
        c = input.get();
        while (((c == ' ') || (c == '\n') || (c == '\t') || (c == '\r') || (c == ','))) {
            c = input.get();
        }
        if (c == '}') {
            end_of_dict_found = true;
            continue;
        }
        if (c != '"') {
            throw ParsingError("начало ключа не найдено");
        }
        std::string key = LoadString(input).AsString();
        c = input.get();
        while (((c == ' ') || (c == '\n') || (c == '\t') || (c == '\r') || (c == ','))) {
            c = input.get();
        }
        if ((c != ':')) {
            throw ParsingError("разделитель ключа и значения не найден");
        }
        result.insert({std::move(key),LoadNode(input)});
        continue;
    }
    
    return Node(move(result));
}

Node LoadBool(istream& input) {
    std::string bool_s;
    char c;
    c = input.get();
    while ((c != ' ') && (c != '\n') && (c != '\t') && (c != '\r') && (c != ',') && (c != ']') && (c != '}') && (!input.fail())) {
        bool_s += c;
        c = input.get();
    }
    if (!input.fail()) {
        input.putback(c);
    }
    bool result;
    if (bool_s == "true"s) {
        result = true;
    } else if (bool_s == "false"s) {
        result = false;
    } else {
        throw ParsingError("попытка прочитать bool");
    }
    return Node(result);
}

Node LoadNull(istream& input) {
    std::string null_s;
    char c;
    c = input.get();
    while ((c != ' ') && (c != '\n') && (c != '\t') && (c != '\r') && (c != ',') && (c != ']') && (c != '}') && (!input.fail())) {
        null_s += c;
        c = input.get();
    }
    if (!input.fail()) {
        input.putback(c);
    }
    if (null_s != "null"s) {
        throw ParsingError("попытка прочитать null");
    }
    return Node(nullptr);
}
using Number = std::variant<int, double>;

Number LoadNum(std::istream& input) {
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
    } else {
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
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNumber(istream& input) {
    Number result = LoadNum(input);
    if (std::holds_alternative<int>(result)) {
        return Node(std::get<int>(result));
    } else {
        return Node(std::get<double>(result));
    }
}

Node LoadNode(istream& input) {
    char c;
    c = input.get();
    while (((c == ' ') || (c == '\n') || (c == '\t') || (c == '\r')) && (!input.fail())) {
        c = input.get();
    }
    if (input.fail()) {
        throw ParsingError("");
    }

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if ((c == 't') || (c == 'f')) {
        input.putback(c);
        return LoadBool(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if ((c == '-') || (isdigit(c))) {
        input.putback(c);
        return LoadNumber(input);
    } else {
        throw ParsingError("");
    }
}

}  // namespace

Node::Node(Array array)
    : data_(move(array)) {
}

Node::Node(Dict map)
    : data_(move(map)) {
}

Node::Node(int value)
    : data_(value) {
}

Node::Node(string value)
    : data_(move(value)) {
}

Node::Node(double value)
    : data_(value) {
}

Node::Node(bool value)
    : data_(value) {
}
    
Node::Node(std::nullptr_t value)
    : data_(value) {
}

Node::Node()
    : data_(nullptr) {
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(data_);
}

bool Node::IsDouble() const { //Возвращает true, если в Node хранится int либо double.
    return std::holds_alternative<double>(data_) || IsInt();
}

bool Node::IsPureDouble() const { //Возвращает true, если в Node хранится double.
    return std::holds_alternative<double>(data_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(data_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(data_);
}
    
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(data_);
}
    
bool Node::IsArray() const {
    return std::holds_alternative<Array>(data_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(data_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("");
    } else {
        return std::get<Array>(data_);
    }
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("");
    } else {
        return std::get<Dict>(data_);
    }
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("");
    } else {
        return std::get<int>(data_);
    }
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("");
    } else {
        return std::get<std::string>(data_);
    }
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("");
    } else {
        return std::get<bool>(data_);
    }
}
    
double Node::AsDouble() const { //Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    if (!IsDouble()) {
        throw std::logic_error("");
    } else {
        if (IsPureDouble()) {
            return std::get<double>(data_);
        } else {
            return static_cast<double>(std::get<int>(data_));
        }
    }
}

bool operator==(const Node& lhs, const Node& rhs) {
    if (lhs.IsArray() && rhs.IsArray()) {
        return lhs.AsArray() == rhs.AsArray();
    }
    if (lhs.IsBool() && rhs.IsBool()) {
        return lhs.AsBool() == rhs.AsBool();
    }
    if (lhs.IsPureDouble() && rhs.IsPureDouble()) {
        return lhs.AsDouble() == rhs.AsDouble();
    }
    if (lhs.IsInt() && rhs.IsInt()) {
        return lhs.AsInt() == rhs.AsInt();
    }
    if (lhs.IsMap() && rhs.IsMap()) {
        return lhs.AsMap() == rhs.AsMap();
    }
    if (lhs.IsNull() && rhs.IsNull()) {
        return true;
    }
    if (lhs.IsString() && rhs.IsString()) {
        return lhs.AsString() == rhs.AsString();
    }
    return false; 
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}


Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}

void PrintNode(const Node& node, std::ostream& output);

void PrintDict(const Node& node, std::ostream& output) {
    output << "{"sv;
    if (!node.AsMap().empty()) {
        for (auto it = node.AsMap().begin(); std::next(it) != node.AsMap().end(); ++it) {
            output << "\""sv << it->first << "\" : "sv;
            PrintNode(it->second, output);
            output << ", "sv;
        }
        output << "\""sv << node.AsMap().rbegin()->first << "\" : "sv;
        PrintNode(node.AsMap().rbegin()->second, output);
    }
    output << "}"sv;
}

void PrintArray(const Node& node, std::ostream& output) {
    output << "["sv;
    if (!node.AsArray().empty()) {
        for (size_t i = 0; i < node.AsArray().size() - 1; ++i) {
            PrintNode(node.AsArray()[i], output);
            output << ", "sv;
        }
        PrintNode(node.AsArray()[node.AsArray().size() - 1], output);
    }
    output << "]"sv;
}

void PrintInt(const Node& node, std::ostream& output) {
    output << node.AsInt();
}

void PrintString(const Node& node, std::ostream& output) {
    output << "\""sv;
    std::string_view s = node.AsString();
    while (!s.empty()) {
        if (s[0] == '\r') {
                output << '\\';
                output << 'r';
                s = s.substr(1, s.size());
                continue;
            } else if (s[0] == '\n') {
                output << '\\';
                output << 'n';
                s = s.substr(1, s.size());
                continue;
            } else if (s[0] == '\t') {
                output << '\t';
                s = s.substr(1, s.size());
                continue;
            } else  if (s[0] == '\\') {
                output << '\\';
                output << '\\';
                s = s.substr(1, s.size());
                continue;
            } else if (s[0] == '"') {
                output << '\\';
                output << '"';
                s = s.substr(1, s.size());
                continue;
            }
        output << s[0];
        s = s.substr(1, s.size());
    }
    output << "\""sv;
}

void PrintDouble(const Node& node, std::ostream& output) {
    output << node.AsDouble();
}

void PrintBool(const Node& node, std::ostream& output) {
    if (node.AsBool()) {
        output << "true"sv;
    } else {
        output << "false"sv;
    }
}

void PrintNull(std::ostream& output) {
    output << "null"sv;
}

void PrintNode(const Node& node, std::ostream& output) {
    if (node.IsArray()) {
        PrintArray(node, output);
    }
    if (node.IsMap()) {
        PrintDict(node, output);
    }
    if (node.IsInt()) {
        PrintInt(node, output);
    }
    if (node.IsString()) {
        PrintString(node, output);
    }
    if (node.IsPureDouble()) {
        PrintDouble(node, output);
    }
    if (node.IsBool()) {
        PrintBool(node, output);
    }
    if (node.IsNull()) {
        PrintNull(output);
    }
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json