#pragma once

#include "json.h"

namespace json {

class ItemContext;
class KeyItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {

public:

    Builder() = default;

    KeyItemContext Key(std::string s);
    Builder& Value(Node::Value val);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

    Node Build();

private:

    Node root_;

    std::vector<std::variant<Array, Dict>> nodes_stack_;
    std::vector<std::string> keys_stack_;

    bool finished = false;

    enum Command {
        KEY, VALUE, STARTDICT, STARTARRAY, ENDDICT, ENDARRAY, BUILD, NOCOMMAND
    };

    Command last_command = Command::NOCOMMAND;

    void PutValue(const Node::Value& val);

};

class ItemContext {

public:

    ItemContext(Builder& builder)
    : b(builder) {}

    KeyItemContext Key(std::string s);
    Builder& Value(Node::Value val);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

protected:

    Builder& b;

};

class DictItemContext : public ItemContext {

public:

    DictItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& Value(Node::Value val) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;

};

class KeyItemContext : public ItemContext {

public:

    KeyItemContext(Builder& builder) : ItemContext(builder) {}

    KeyItemContext Key(std::string s) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;

    DictItemContext Value(Node::Value val);

};

class ArrayItemContext : public ItemContext {

public :

    ArrayItemContext(Builder& builder) : ItemContext(builder) {}

    KeyItemContext Key(std::string s) = delete;
    Builder& EndDict() = delete;
    Node Build() = delete;

    ArrayItemContext Value(Node::Value val);

};

}