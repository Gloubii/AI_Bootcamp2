#pragma once

#ifdef DEBUG_FRAMEWORK
#define BOT_DEBUG
#endif

#ifdef BOT_DEBUG
#define BOT_LOG(logger, text, autoEndLine) logger.Log(text, autoEndLine)
#else
#define BOT_LOG(logger, text, autoEndLine) 0
#endif

#include <list>

struct SConfigData;
struct SInitData;
struct STurnData;
struct SOrder;

class BotLogicIF;

class AIBot
{
public:
	AIBot() = default;
	virtual ~AIBot();

	void Configure(const SConfigData& _configData);
	void Init(const SInitData& _initData);
	void GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders);

	void SetBotLogic(BotLogicIF* _pBotLogic);

protected:
	BotLogicIF* mpBotLogic = nullptr;

#ifdef BOT_DEBUG
	Logger mLogger;
#endif
};