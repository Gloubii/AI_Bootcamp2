#include "MyBotLogic.h"
#include <algorithm>
#include <vector>
#include <string>
#include "Globals.h"
#include "ConfigData.h"
#include "InitData.h"
#include "TurnData.h"
#include "Graph.h"

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

	BehaviorTree::initBehaviorTree();

	//Create graph
	g = Graph(_initData);

	for (int i = 0; i < _initData.nbNPCs; ++i) {
		manager.npcs.push_back(NPC{&manager,_initData.npcInfoArray[i], g});
	}

	manager.createBasicbehaviorTree();
	BOT_LOGIC_LOG(mLogger, "BehaviorTree created", true);

	if (_initData.omniscient || true) {
		BOT_LOGIC_LOG(mLogger, "omniscient", true);
		//manager.goals = g.GetGoals();
		manager.goals = { Hex{ 0, 4 } };
		manager.assignGoals();
	}

	BOT_LOGIC_LOG(mLogger, g.toString(), true);
}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);
	g.Update(_turnData);
	manager.updateNpc(_turnData);
	BOT_LOGIC_LOG(mLogger, "Updated graph", true);
	for (NPC& npc : manager.npcs) {
		npc.SetUpBlackboard();
		BOT_LOGIC_LOG(mLogger, "Setup Blackboard", true);
		npc.RunBehaviorTree();
		BOT_LOGIC_LOG(mLogger, "Run behavior Tree", true);
		_orders.push_back(npc.NextOrder());
		BOT_LOGIC_LOG(mLogger, "push back order", true);
	}
}


