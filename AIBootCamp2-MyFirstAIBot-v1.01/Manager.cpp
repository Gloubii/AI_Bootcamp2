#include "Manager.h"
#include <algorithm>
void Manager::createBasicbehaviorTree()
{
	for (auto& npc : npcs) {
		npc.SetBehaviorTree("NpcABitLessBasicTree");
	}
}

void Manager::updateNpc(const STurnData& _turnData)
{
	for (int i = 0; i < _turnData.npcInfoArraySize; ++i) {
		npcs[i].Update(_turnData.npcInfoArray[i]);
	}
}

bool Manager::isOccuped(const Hex& hex){
	return std::find_if(begin(npcs), end(npcs), [hex](auto &npc) {return npc.GetPosition() == hex; }) != end(npcs);
}

NPC* Manager::getOccupant(const Hex& hex)
{
	auto p = std::find_if(begin(npcs), end(npcs), [hex](auto& npc) {return npc.GetPosition() == hex; });
	return p != end(npcs) ? &*p : nullptr;
}

void Manager::assignGoals()
{
	for (NPC& npc : npcs) {
		
		auto nearest = std::min_element(goals.cbegin(), goals.cend(), 
			[start = npc.GetPosition()](const Hex& g1, const Hex& g2) 
			{
				return g1.DistanceTo(start) < g2.DistanceTo(start); 
			}
		);
		npc.SetGoal(*nearest);
		goals.erase(nearest);
	}
}

void Manager::getNewGoal(NPC* npc)
{
	if (!goals.size()) throw std::exception("No other goals to give");

	std::vector<Hex> remainingGoals;
	std::copy_if(begin(goals), end(goals), std::back_inserter(remainingGoals), [&lastGoals = npc->GetPastGoals()](auto hex) 
		{
		return std::find(begin(lastGoals), end(lastGoals), hex) == end(lastGoals);
		}
	);

	if (!remainingGoals.size()) throw std::exception("No other goals to give");

	auto nearest = std::min_element(remainingGoals.cbegin(), remainingGoals.cend(),
		[start = npc->GetPosition()](const Hex& g1, const Hex& g2)
		{
			return g1.DistanceTo(start) < g2.DistanceTo(start);
		}
	);

	goals.push_back(npc->GetGoal());
	npc->SetGoal(*nearest);
	goals.erase(std::find(begin(goals), end(goals), *nearest));
}
