#ifndef ORDERPATH_H
#define ORDERPATH_H

#include<list>
#include "Globals.h"
#include <vector>
#include "Hex.h"
#include <string>

class OrderPath
{
	std::list<SOrder> path;

public:

	OrderPath() = default;

	void AddOrder(SOrder order);
	SOrder NextOrder();
	std::string toString();
};

#endif
