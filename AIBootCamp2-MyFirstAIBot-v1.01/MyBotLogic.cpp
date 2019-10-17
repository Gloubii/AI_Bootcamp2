#include "MyBotLogic.h"
#include <algorithm>
#include <vector>
#include <string>
#include "Globals.h"
#include "ConfigData.h"
#include "InitData.h"
#include "TurnData.h"
#include "Graph.h"

#include "OutilDebug/IOGraphe.h"
#include "OutilDebug/TestGraphe.h"


MyBotLogic::MyBotLogic()
{
	//Write Code Here
}

MyBotLogic::~MyBotLogic()
{
	//Write Code Here
}

void MyBotLogic::Configure(const SConfigData& _configData)
{
#ifdef BOT_LOGIC_DEBUG
	mLogger.Init(_configData.logpath, "MyBotLogic.log");
#endif

	BOT_LOGIC_LOG(mLogger, "Configure", true);


	//Write Code Here
}

Graph g;

void MyBotLogic::Init(const SInitData& _initData)
{
	BOT_LOGIC_LOG(mLogger, "Init", true);
	BOT_LOGIC_LOG(mLogger, std::to_string(_initData.objectInfoArraySize), true);

	//Create graph
	g = Graph(_initData);
	//CheckNeighbours(g);
	Graph2JsonFile(g);
	
	
	BOT_LOGIC_LOG(mLogger, g.toString(), true);
}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);

	//Write Code Here
}