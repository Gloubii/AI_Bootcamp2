#include "NPC.h"

NPC::NPC(const SNPCInfo& npcInfo) : uid{npcInfo.uid}, position{Hex{npcInfo.q, npcInfo.r}}, visionRange{npcInfo.visionRange}, currentGoal{}, hasGoal{false}, currentPathId{-1}
{

}

bool NPC::Update(const SNPCInfo& npcInfo)
{
	bool updated = false;

	if (visionRange != npcInfo.visionRange) {
		visionRange = npcInfo.visionRange;
		updated = true;
	}
	Hex h = Hex{ npcInfo.q, npcInfo.r };
	if (position == h ) {
		position = h;
		updated = true;
	}
	return updated;
}

int NPC::GetUid() const
{
	return uid;
}

int NPC::GetVisionRange() const
{
	return visionRange;
}

Hex NPC::GetPosition() const
{
	return position;
}

Hex NPC::GetCurrentGoal() const
{
	if (!hasGoal)
		throw std::exception("Ce NPC n'a pas de goal");
	return currentGoal;
}

bool NPC::HasCurrentGoal() const
{
	return hasGoal;
}

void NPC::SetCurrentPathId(const int& id)
{
	currentPathId = id;
}

int NPC::GetCurrentPathId() const
{
	if (!hasGoal) {
		throw ("Ce npc n'a pas de goal !");
	}
	return currentPathId;
}

void NPC::RemoveGoal()
{
	hasGoal = false;
}

void NPC::SetGoal(const Hex& hex)
{
	hasGoal = true;
	currentGoal = hex;
}

void NPC::BuildOrder(const std::vector<Hex> path)
{
	for (unsigned __int64 i = 0; i < path.size() - 1; ++i) {
		SOrder order;
		order.npcUID = GetUid();
		order.orderType = EOrderType::Move;
		order.direction = (path[i+1] - path[i]).ToDirection();
		AddOrder(order);
	}
	//DebugHelper::Log("NPC :" + orderPath.toString());
}

void NPC::AddOrder(const SOrder& order)
{
	orderPath.AddOrder(order);
}

SOrder NPC::NextOrder()
{
	try {
		return orderPath.NextOrder();
	}
	catch(std::exception e){
		SOrder order;
		order.npcUID = GetUid();
		order.orderType = EOrderType::Move;
		order.direction = EHexCellDirection::CENTER;
		return order;
	}
	
}

bool NPC::operator<(const NPC& npc) const
{
	return uid < npc.uid;
}

