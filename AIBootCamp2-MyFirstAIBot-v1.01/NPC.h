#ifndef NPC_H
#define NPC_H

#include"Hex.h"
#include"OrderPath.h"
#include "Globals.h"
#include "Task.h"
#include "Graph.h"

class Manager;

class NPC
{
	int uid, visionRange;
	Hex position;

	Hex goal;
	std::vector<Hex> pastGoals;
	std::vector<Hex> reachableGoals;

	OrderPath orderPath;
	BehaviorTree behaviorTree;

	Graph &graph;

public:
	using Manager_t = Manager;
	using Path_t = std::vector<Edge>;

private:

	Path_t path;

	bool moved;

	Task::BlackboardPtr blackboard;


public:

	
	Manager_t* manager;
	

	NPC(Manager_t *manager, const SNPCInfo &npcInfo, Graph &graph);

	bool Update(const SNPCInfo& npcInfo);

	int GetUid() const;
	int GetVisionRange() const;
	Hex GetPosition() const;
	void SetPosition(const Hex& hex);

	Hex GetIntention() const;
	bool IsOnGoal() const;
	void updateReachableGoals(Hex h);
	bool isReachableGoal(Hex h);

	void SetGoal(const Hex& hex);
	Hex GetGoal() const;
	void AskNewGoal();

	std::vector<Hex> GetPastGoals() const;

	void BuildOrder(const std::vector<Hex> path);
	void AddOrder(const SOrder& order);
	SOrder NextOrder();

	void SetBehaviorTree(const std::string &treeName);
	void SetUpBlackboard();
	void RunBehaviorTree();

	bool operator< (const NPC& npc) const;




	struct TaskPathEmpty : TaskCondition {
		ReturnValue run(BlackboardPtr blackboard) override {
			return blackboard->getValue<Path_t*>("path")->size() ? FAILLURE : SUCCESS;
		}

		ClonePtr clone() override {
			return new TaskPathEmpty();
		}
	};

	struct TaskGetPath : TaskAction 
	{
		ReturnValue run(BlackboardPtr blackboard) override {
			auto p = blackboard->getValue<Path_t*>("path");
			auto g = blackboard->getValue<Graph*>("graph");
			*p = g->aStar(blackboard->getValue<Hex>("currentPos"), blackboard->getValue<Hex>("goal"));
			return SUCCESS;
		}

		ClonePtr clone() override {
			return new TaskGetPath;
		}
	};

	struct TaskNextBlocked : TaskCondition
	{
		ReturnValue run(BlackboardPtr blackboard) override;

		ClonePtr clone() override {
			return new TaskNextBlocked;
		}
	};


	struct TaskWait : TaskAction
	{
		ReturnValue run(BlackboardPtr blackboard) override {
			auto npc = blackboard->getValue<NPC*>("npc");
			SOrder order;
			order.direction = CENTER;
			order.orderType = EOrderType::Move;
			order.npcUID = npc->GetUid();
			npc->AddOrder(order);
			npc->moved = true;
			return SUCCESS;
		}

		ClonePtr clone() override {
			return new TaskWait;
		}
	};

	struct TaskMove : TaskAction
	{
		ReturnValue run(BlackboardPtr blackboard) override {
			auto npc = blackboard->getValue<NPC*>("npc");
			auto p = blackboard->getValue<Path_t*>("path");
			auto e = (*p)[0];
			p->erase(std::begin(*p));
			SOrder order;
			order.direction = (e.getTo() - e.getFrom()).ToDirection();
			order.orderType = EOrderType::Move;
			order.npcUID = npc->GetUid();
			npc->AddOrder(order);
			npc->SetPosition(e.getTo());
			npc->moved = true;
			return SUCCESS;
		}

		ClonePtr clone() override {
			return new TaskMove;
		}
	};
};

#endif // !NPC_H



