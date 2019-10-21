#ifndef NPC_H
#define NPC_H

#include"Hex.h"
#include"OrderPath.h"
#include "Globals.h"

class NPC
{
	int uid, visionRange;
	Hex position;
	OrderPath orderPath;

public:
	

	NPC() = default;
	NPC(const SNPCInfo &npcInfo);

	bool Update(const SNPCInfo& npcInfo);

	int GetUid() const;
	int GetVisionRange() const;
	Hex GetPosition() const;

	void BuildOrder(const std::vector<Hex> path);
	void AddOrder(const SOrder& order);
	SOrder NextOrder();

	bool operator< (const NPC& npc) const;
};

#endif // !NPC_H



