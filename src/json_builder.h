#pragma once

#include <vector>
#include <sstream>

#include "json.h"

namespace json {

class Builder {
private:
	class DictItemContext;
	class DictElementItemContext;
	class ArrayItemContext;

public:

	Builder() {
		nodes_stack_.push_back(&root_);
	}
	~Builder() = default;
	Builder& Key(std::string);
	Builder& Value(Node);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndDict();
	Builder& EndArray();
	Node Build();

private:
	void IsReady();

	Node root_;
	bool ready_ = 0;
	std::vector<Node*> nodes_stack_;
	 
//utility classes
	class DictElementItemContext {
	public:
		DictElementItemContext(Builder& builder)
			:builder_(builder)
		{

		}
		Builder& Key(std::string) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
		Builder::DictItemContext Value(Node);
		Builder::DictItemContext StartDict();
		Builder::ArrayItemContext StartArray();

	private:
		Builder& builder_;
	};

	class DictItemContext {
	public:
		DictItemContext(Builder& builder)
			:builder_(builder)
		{
		}
		Builder& Value(Node) = delete;
		Builder& StartDict() = delete;
		Builder& StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
		Builder::DictElementItemContext Key(std::string);
		Builder& EndDict();

	private:
		Builder& builder_;
	};

	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder)
			:builder_(builder)
		{
		}
		Node Build() = delete;
		DictItemContext Key(std::string) = delete;
		Builder& EndDict() = delete;
		Builder::DictItemContext StartDict();
		Builder::ArrayItemContext StartArray();
		Builder::ArrayItemContext Value(Node);
		Builder& EndArray();
	private:
		Builder& builder_;
	};
};

}
