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
	using ValuePtr = Any*;

	std::unordered_map<Key, ValuePtr> map;

public:
	Blackboard(Blackboard<Key>* parent = nullptr) : parent{ parent } {};

	//Write the value in the current blackboard
	//Overwrite the value if it already exist
	template<class ValueType>
	void write(const Key& key, const ValueType &value) {
		if (contain(key)) delete map[key]; 
		map[key] = new ValueType(value);
	}

	class KeyNotFound {};
	//Overwrite the value in the first blackboard containing the key
	//Throw KeyNotFound if the key is not in the blackboard
	template<class ValueType>
	void overwrite(const Key& key, const ValueType& value) {
		if (contain(key)) {
			delete map[key];
			map[key] = new ValueType(value);
			return;
		}
		if (parent) {
			parent->overwrite(key, value);
			return;
		}
		throw KeyNotFound{};
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

	bool contain(const Key& key) const {
		return map.count(key);
	}

	//Return true if erased sucessfully
	bool erase(const Key& key) {
		delete map[key];
		return map.erase(key);
	}

	void clean() {
		for_each(begin(map), end(map), [](auto& pair) {delete pair.second; pair.second = nullptr; });
		map.clear();
	}

	~Blackboard() {
		clean();
	}

private:
	//Return nullptr if not in the blackboard
	Any* get(const Key& key) {
		return map[key];
	}

};

#endif
