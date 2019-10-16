#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <unordered_map>
#include <string>
#include <memory>

	//The key must be hashable
template<class Key>
class Blackboard
{
	Blackboard<Key>* parent;

	using Any = void;
	using ValuePtr = std::unique_ptr<Any>;

	std::unordered_map<Key, ValuePtr> map;

public:
	Blackboard(Blackboard<Key>* parent = nullptr) : parent{ parent } {};

	template<class ValueType>
	void write(const Key& key, const ValueType &value) {
		map[key] = std::make_unique<ValueType>(value);
	}

	class ValueNotFound {};
	//Throw ValuenotFound if the value is not in the blackboard
	template<class ValueType>
	ValueType getValue(const Key& key) {
		if (auto res = get(key)) {
			return *static_cast<ValueType*>(res);
		}
		if (parent) {
			return parent->getValue<ValueType>(key);
		}
		throw ValueNotFound{};
	}

	//Return true if erase sucessfully
	bool erase(const Key& key) {
		return map.erase(key);
	}

	void clean() {
		map.clear();
	}

private:
	//Return nullptr if not in the blackboard
	Any* get(const Key& key) {
		return map[key].get();
	}

};

#endif
