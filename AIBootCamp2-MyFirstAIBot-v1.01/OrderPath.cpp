#include "OrderPath.h"
#include <exception>


void OrderPath::AddOrder(SOrder order)
{
	path.push_back(order);
}

SOrder OrderPath::NextOrder()
{
	if (path.empty()) {
		throw std::exception("No next order");
	}
	auto tmp = path.front();
	path.pop_front();
	//DebugHelper::Log("NPC id -> " + std::to_string(tmp.npcUID) + " Order -> Type " + std::to_string(tmp.orderType) + " Dir : " + std::to_string(tmp.direction));
	return tmp;
}

std::string OrderPath::toString()
{
	std::string res;
	for (auto order : path) {
		res += "-> " + std::to_string(order.direction);
	}
	return res;
}
