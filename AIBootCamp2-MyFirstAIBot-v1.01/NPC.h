#ifndef NPC_H
#define NPC_H

#include "Hex.h"
#include "OrderPath.h"
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
	bool operator== (const NPC& npc) const;
	bool operator!= (const NPC& npc) const;



	struct TaskPathNonEmpty : TaskCondition {
		ReturnValue run(BlackboardPtr blackboard) override {
			return blackboard->getValue<Path_t*>("path")->size() ? SUCCESS : FAILLURE;
		}

		ClonePtr clone() override {
			return new TaskPathNonEmpty();
		}
	};

	struct TaskGetPath : TaskAction 
	{
		ReturnValue run(BlackboardPtr blackboard);

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

	struct TaskComputeIntermediateGoal : TaskAction
	{

		int lerp(float x, float y, float t) {
			return x + (y - x) * t;
		}

		ReturnValue run(BlackboardPtr blackboard) override {
			auto pos = blackboard->getValue<Hex>("currentPos");
			auto goal = blackboard->getValue<Hex>("goal");
			auto g = blackboard->getValue<Graph*>("graph");
			int distance = pos.DistanceTo(goal);
			float t = 1.0f / distance;
			float factor = 6 * t;
			for (int i = distance-1; i >= 0; --i) {
				Hex center = Hex{ lerp(pos.x, goal.x, i * t), lerp(pos.y, goal.y, i * t), lerp(pos.z, goal.z, i * t) };
				
				if (g->Contains(center) && g->getNode(center).getValue() >= factor * i + 1 && g->atteignable(pos, center) && center != pos) {
					blackboard->write("goal", center);
					return SUCCESS;
				}
				int k = distance - i;
				for (int x = -k; x <= k; ++x) {
					for (int y = std::max(-k, -k - x); y <= std::min(k, k - x); ++y) {
						Hex side = Hex{ x, y } + center;
						if (g->Contains(side) && g->getNode(side).getValue() >= factor * i + 1 && g->atteignable(pos, side) && side != pos) {
							blackboard->write("goal", side);
							return SUCCESS;
						}
					}
				}
			}



			throw std::exception("a marche pas");
			return SUCCESS;
		}

		ClonePtr clone() override {
			return new TaskComputeIntermediateGoal;
		}
	};
};


#endif // !NPC_H



