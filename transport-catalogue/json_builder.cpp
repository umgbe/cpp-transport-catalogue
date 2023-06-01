#include "json_builder.h"

#include <algorithm>

using namespace json;
using namespace std::string_literals;

KeyItemContext Builder::Key(std::string s) {
    if (finished) {
        throw std::logic_error("вызов Key при готовом объекте"s);
    }
    if (nodes_stack_.empty() || (!std::holds_alternative<Dict>(nodes_stack_.back()))) {
        throw std::logic_error("вызов Key снаружи словаря"s);
    }
    if (last_command == Command::KEY) {
        throw std::logic_error("вызов Key после другого Key"s);
    }
    keys_stack_.push_back(std::move(s));
    last_command = Command::KEY;
    KeyItemContext k(*this);
    return k;
}

Builder& Builder::Value(Node::Value val) {
    if (finished) {
        throw std::logic_error("вызов Value при готовом объекте"s);
    }
    if (last_command == Command::STARTDICT) {
        throw std::logic_error("ожидался Key или EndDict после StartDict"s);
    }
    if (!(  last_command == Command::NOCOMMAND || 
            last_command == Command::KEY || 
            last_command == Command::STARTARRAY || 
            (last_command == Command::ENDDICT && std::holds_alternative<Array>(nodes_stack_.back())) ||
            (last_command == Command::ENDARRAY && std::holds_alternative<Array>(nodes_stack_.back())) ||
            (last_command == Command::VALUE && std::holds_alternative<Array>(nodes_stack_.back())))) {
        throw std::logic_error("Value вызвана не после конструктора, Key или предыдущего элемента массива.");
    }
    
    PutValue(val);
    
    last_command = Command::VALUE;
    return *this;
}

void Builder::PutValue(const Node::Value& val) {
    Node new_node;

    if (std::holds_alternative<nullptr_t>(val)) {
        new_node = Node(nullptr);
    } else if (std::holds_alternative<json::Array>(val)) {
        new_node = Node(std::get<json::Array>(val));
    } else if (std::holds_alternative<json::Dict>(val)) {
        new_node = Node(std::get<json::Dict>(val));
    } else if (std::holds_alternative<bool>(val)) {
        new_node = Node(std::get<bool>(val));
    } else if (std::holds_alternative<int>(val)) {
        new_node = Node(std::get<int>(val));
    } else if (std::holds_alternative<double>(val)) {
        new_node = Node(std::get<double>(val));
    } else if (std::holds_alternative<std::string>(val)) {
        new_node = Node(std::get<std::string>(val));
    }

    if (nodes_stack_.empty()) {
        root_ = std::move(new_node);
        finished = true;
    } else {
        if (std::holds_alternative<Array>(nodes_stack_.back())) {
            std::get<Array>(nodes_stack_.back()).push_back(std::move(new_node));
        } else if (std::holds_alternative<Dict>(nodes_stack_.back())) {
            std::get<Dict>(nodes_stack_.back()).insert({std::move(keys_stack_.back()), std::move(new_node)});
            keys_stack_.pop_back();
        }
    }
}



DictItemContext Builder::StartDict() {
    if (finished) {
        throw std::logic_error("вызов StartDict при готовом объекте"s);
    }
    if (last_command == Command::STARTDICT) {
        throw std::logic_error("ожидался Key или EndDict после StartDict"s);
    }
    if (!(  last_command == Command::NOCOMMAND || 
            last_command == Command::KEY || 
            last_command == Command::STARTARRAY || 
            (last_command == Command::ENDDICT && std::holds_alternative<Array>(nodes_stack_.back())) ||
            (last_command == Command::ENDARRAY && std::holds_alternative<Array>(nodes_stack_.back())) ||
            (last_command == Command::VALUE && std::holds_alternative<Array>(nodes_stack_.back())))) {
        throw std::logic_error("StartDict вызвана не после конструктора, Key или предыдущего элемента массива.");
    }
    nodes_stack_.push_back(Dict());
    last_command = Command::STARTDICT;
    DictItemContext d(*this);
    return d;
}

ArrayItemContext Builder::StartArray() {
    if (finished) {
        throw std::logic_error("вызов StartArray при готовом объекте"s);
    }
    if (last_command == Command::STARTDICT) {
        throw std::logic_error("ожидался Key или EndDict после StartDict"s);
    }
    if (!(  last_command == Command::NOCOMMAND || 
            last_command == Command::KEY || 
            last_command == Command::STARTARRAY || 
            (last_command == Command::ENDDICT && std::holds_alternative<Array>(nodes_stack_.back())) ||
            (last_command == Command::ENDARRAY && std::holds_alternative<Array>(nodes_stack_.back())) ||
            (last_command == Command::VALUE && std::holds_alternative<Array>(nodes_stack_.back())))) {
        throw std::logic_error("StartArray вызвана не после конструктора, Key или предыдущего элемента массива.");
    }
    nodes_stack_.push_back(Array());
    last_command = Command::STARTARRAY;
    ArrayItemContext a(*this);
    return a;
}

Builder& Builder::EndDict() {
    if (finished) {
        throw std::logic_error("вызов EndDict при готовом объекте"s);
    }
    if (nodes_stack_.empty() || (!std::holds_alternative<Dict>(nodes_stack_.back()))) {
        throw std::logic_error("окончание словаря без объявления начала"s);
    }
    //если ключей на стеке больше, чем незакрытых словарей
    if (keys_stack_.size() >= static_cast<size_t>(std::count_if(nodes_stack_.begin(), nodes_stack_.end(), 
    [] (const std::variant<Array, Dict>& element) { return std::holds_alternative<Dict>(element); }))) {
        throw std::logic_error("задан ключ, не было задано значение для словаря"s);
    }
    Dict temp = std::move(std::get<Dict>(nodes_stack_.back()));
    nodes_stack_.pop_back();
    PutValue(std::move(temp));
    last_command = Command::ENDDICT;
    return *this;
}

Builder& Builder::EndArray() {
    if (finished) {
        throw std::logic_error("вызов EndArray при готовом объекте"s);
    }
    if (nodes_stack_.empty() || (!std::holds_alternative<Array>(nodes_stack_.back()))) {
        throw std::logic_error("окончание массива без объявления начала"s);
    }
    Array temp = std::move(std::get<Array>(nodes_stack_.back()));    
    nodes_stack_.pop_back();
    PutValue(std::move(temp));
    last_command = Command::ENDARRAY;
    return *this;
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("незаконченный массив или словарь"s);
    }
    if (!finished) {
        throw std::logic_error("сборка пустого объекта"s);
    }
    last_command = Command::BUILD;
    return root_;
}

KeyItemContext ItemContext::Key(std::string s) {
    return b.Key(s);
}
Builder& ItemContext::Value(Node::Value val) {
    return b.Value(val);
}
DictItemContext ItemContext::StartDict() {
    return b.StartDict();
}
ArrayItemContext ItemContext::StartArray() {
    return b.StartArray();
}
Builder& ItemContext::EndDict() {
    return b.EndDict();
}
Builder& ItemContext::EndArray() {
    return b.EndArray();
}
Node ItemContext::Build() {
    return b.Build();
}

ValueAfterKeyContext KeyItemContext::Value(Node::Value val) {
    b.Value(val);
    ValueAfterKeyContext v(b);
    return v;
}

ValueAfterArrayContext ArrayItemContext::Value(Node::Value val) {
    b.Value(val);
    ValueAfterArrayContext v(b);
    return v;
}

ValueAfterArrayContext ValueAfterArrayContext::Value(Node::Value val) {
    b.Value(val);
    ValueAfterArrayContext v(b);
    return v;
}

