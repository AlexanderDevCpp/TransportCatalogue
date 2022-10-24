#include "json_builder.h"

using namespace std::literals;
namespace json {

void Builder::IsReady() {
	if (ready_ == true) {
		throw std::logic_error("json already finished");
	}
}

Builder& Builder::Key(std::string key) {
	IsReady();

	if (!nodes_stack_.back()->IsDict()) {
		throw std::logic_error("last element is not Dict");
	}

	nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetValue())[key]);
	return *this;
}

Builder& Builder::Value(Node value) {
	IsReady();

	if (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsNull()) {
		throw std::logic_error("Value");
	}

	if (nodes_stack_.back()->IsArray()) {
		std::get<Array>(nodes_stack_.back()->GetValue()).push_back(value);
	}
	else {
		if (nodes_stack_.size() == 1 && nodes_stack_.back()->IsNull()) {
			ready_ = 1;
		}
		*nodes_stack_.back() = value;
		nodes_stack_.pop_back();
	}
	return *this;
}


Builder::DictItemContext Builder::StartDict() {
	IsReady();

	if (nodes_stack_.back()->IsDict() && !nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()) {
		throw std::logic_error("StartDict Error");
	}

	if (nodes_stack_.back()->IsNull()) {
		*nodes_stack_.back() = Node(Dict());
	}
	else if (nodes_stack_.back()->IsArray()) {
		std::get<Array>(nodes_stack_.back()->GetValue()).push_back(Node(Dict()));
		nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
	}
	return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
	IsReady();

	if (nodes_stack_.back()->IsDict() && !nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()) {
		throw std::logic_error("StartArray Error");
	}

	if (nodes_stack_.back()->IsNull()) {
		*nodes_stack_.back() = Node(Array());
	}
	else if (nodes_stack_.back()->IsArray()) {
		std::get<Array>(nodes_stack_.back()->GetValue()).push_back(Node(Array()));
		nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
	}
	return *this;
}

Builder& Builder::EndDict() {
	IsReady();

	if (!nodes_stack_.back()->IsDict()) {
		throw std::logic_error("last container is not an Dict");
	}

	nodes_stack_.pop_back();
	return *this;
}

Builder& Builder::EndArray() {
	IsReady();

	if (!nodes_stack_.back()->IsArray()) {
		throw std::logic_error("last container is not an Array");
	}

	nodes_stack_.pop_back();
	return *this;
}

Node Builder::Build() {
	if (!nodes_stack_.empty()) {
		throw std::logic_error("json not finished or empty");
	}
	else {
		ready_ = true;
		return root_;
	}
}

// DictItemContext

Builder::DictElementItemContext Builder::DictItemContext::Key(std::string key) {
	builder_.Key(key);
	return builder_;
}

Builder& Builder::DictItemContext::EndDict() {
	builder_.EndDict();
	return builder_;
}
// DictElementItemContext

Builder::DictItemContext Builder::DictElementItemContext::Value(Node value) {
	builder_.Value(value);
	return builder_;
}
Builder::DictItemContext Builder::DictElementItemContext::StartDict() {
	builder_.StartDict();
	return builder_;
}
Builder::ArrayItemContext Builder::DictElementItemContext::StartArray() {
	builder_.StartArray();
	return builder_;
}


//ArrayItemContext
Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
	builder_.StartDict();
	return builder_;
}
Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
	builder_.StartArray();
	return builder_;
}
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) {
	builder_.Value(value);
	return builder_;
}
Builder& Builder::ArrayItemContext::EndArray() {
	builder_.EndArray();
	return builder_;
}

}
