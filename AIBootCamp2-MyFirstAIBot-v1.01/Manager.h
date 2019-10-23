#pragma once

#include "NPC.h"
#include "Globals.h"

class Manager
{
public:
	std::vector<NPC> npcs;
	std::vector<Hex> goals;

	void createBasicbehaviorTree();
	void updateNpc(const STurnData& _turnData);
	bool isOccuped(const Hex& hex);
	//Return nullptr if the hex is empty
	NPC* getOccupant(const Hex& hex);

	void assignGoals();
	void getNewGoal(NPC* npc);
};

