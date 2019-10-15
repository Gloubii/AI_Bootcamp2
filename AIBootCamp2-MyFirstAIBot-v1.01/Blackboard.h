#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <unordered_map>
#include <string>
#include <memory>


template<class Key>
class Blackboard
{
	Blackboard* parent;

	using ValuePtr = std::shared_ptr<void>;

	std::unordered_map<Key, ValuePtr> enumMap;

public:
	Blackboard(Blackboard* parent = nullptr);

	void write(const Key& key, ValuePtr value);
	void write(const Key& key, int value);
	void write(const Key& key, std::string value);
	void write(const Key& key, float value);
	void write(const Key& key, bool value);

	//Return nullptr if not in the blackboard
	ValuePtr get(const Key& key);

	class ValueNotFound {};
	//Throw ValuenotFound if the value is not in the blackboard
	bool getBool(const Key& key);
	int getInt(const Key& key);
	float getFloat(const Key& key);
	std::string getString(const Key& key);

	//Return true if erase sucessfully
	bool erase(const Key& key);
};

#endif