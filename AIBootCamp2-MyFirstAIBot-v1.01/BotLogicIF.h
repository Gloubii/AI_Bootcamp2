#pragma once

#include <list>

struct SConfigData;
struct SInitData;
struct STurnData;
struct SOrder;

class BotLogicIF
{
public:
	virtual ~BotLogicIF() = default;

	virtual void Configure(const SConfigData& _configData) = 0;
	virtual void Init(const SInitData& _initData) = 0;
	virtual void GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders) = 0; //calculate orders for a single turn
	//virtual void Exit() = 0;
};
