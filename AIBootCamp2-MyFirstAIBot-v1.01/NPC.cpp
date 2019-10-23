#include "NPC.h"
#include "Manager.h"

NPC::NPC(Manager_t *manager, const SNPCInfo& npcInfo, Graph& graph) : manager{ manager }, uid { npcInfo.uid }, position{ Hex{npcInfo.q, npcInfo.r} }, visionRange{ npcInfo.visionRange }, graph{ graph }, blackboard{new Blackboard<Task::BlackboardKey>}
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

void NPC::SetPosition(const Hex& hex)
{
	position = hex;
}

void NPC::SetGoal(const Hex& hex)
{
	goal = hex;
}

Hex NPC::GetGoal()
{
	return goal;
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

void NPC::SetUpBlackboard()
{
	blackboard->clean();
	blackboard->write("npc", this);
	blackboard->write("path", &path);
	blackboard->write("manager", manager);
	blackboard->write("graph", &graph);
	blackboard->write("currentPos", position);
	blackboard->write("goal", goal);
}

void NPC::RunBehaviorTree()
{
	behaviorTree->run(blackboard);
}

bool NPC::operator<(const NPC& npc) const
{
	return uid < npc.uid;
}



Task::ReturnValue NPC::TaskNextBlocked::run(BlackboardPtr blackboard)
{
	auto e = blackboard->getValue<Path_t*>("path")->front();
	auto m = blackboard->getValue<Manager_t*>("manager");
	return m->isOccuped(e.getTo()) ? SUCCESS : FAILLURE;
}
