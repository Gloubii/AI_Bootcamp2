#include "NPC.h"

NPC::NPC(const SNPCInfo& npcInfo) : uid{npcInfo.uid}, position{Hex{npcInfo.q, npcInfo.r}}, visionRange{npcInfo.visionRange}
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

void NPC::SetBehaviorTree(const std::string& treeName)
{
	behaviorTree = BehaviorTree::createBehaviorTree(treeName);
}

void NPC::RunBehaviorTree(Task::BlackboardPtr blackboard)
{
	behaviorTree->run(blackboard);
}

bool NPC::operator<(const NPC& npc) const
{
	return uid < npc.uid;
}

