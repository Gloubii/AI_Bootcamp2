#include "Task.h"
#include "NPC.h"
#include "Manager.h"

std::unordered_map<BehaviorTree::BehaviorTreeKey, BehaviorTree::ClonePtr> BehaviorTree::behaviorTreeLibrary;

Task::ReturnValue operator! (Task::ReturnValue res) {
	switch (res)
	{
	case Task::FAILLURE:
		return Task::SUCCESS;
		break;
	case Task::SUCCESS:
		return Task::FAILLURE;
		break;
	case Task::RUNNING:
		return Task::RUNNING;
		break;
	default:
		break;
	}
	return res;
}

Task::ClonePtr TaskInterrupter::clone() {
	auto clone = new TaskInterrupter(child->clone());
	std::for_each(begin(clonedPerformers), end(clonedPerformers), [&clone](InteruptPerformer p) {
		p->setInterupter(clone);
	});
	lastCloned = clone;
	clonedPerformers.clear();
	return clone;
}

BehaviorTree BehaviorTree::createBehaviorTree(BehaviorTreeKey key)
{
	return BehaviorTree{ behaviorTreeLibrary[key]->clone() };
}

void BehaviorTree::initBehaviorTree()
{
	behaviorTreeLibrary["NpcBasicTree"] = new TaskSequence({
		new TaskInverter{ new TaskSelector{{
				new TaskSequence {{
					new TaskInverter{ new NPC::TaskPathNonEmpty },
					new TaskPredicate{[](Task::BlackboardPtr b) {return b->getValue<Hex>("goal") == b->getValue<Hex>("currentPos"); }},
					new NPC::TaskWait
				}},
				new TaskFaillure{ new NPC::TaskGetPath}
		}}},
		new TaskSequence{{
				new TaskInverter{ new TaskSequence {{
						new NPC::TaskNextBlocked, new NPC::TaskWait
				}}},
				new NPC::TaskMove
		}}
		});

	behaviorTreeLibrary["NpcGetPathOrWait"] = new TaskSequence{{
			new TaskInverter{ new TaskSequence {{
				new TaskInverter{ new NPC::TaskPathNonEmpty },
				new TaskPredicate{[](Task::BlackboardPtr b) {return b->getValue<NPC*>("npc")->IsOnGoal(); }},
				new NPC::TaskWait
			}} },
			new TaskFaillure{ new TaskUntilFail{ new TaskSequence{{
					new NPC::TaskGetPath,
					new TaskPredicate{[](Task::BlackboardPtr b) {return !b->getValue<NPC::Path_t*>("path")->size(); }},
					new TaskBasicAction{[](Task::BlackboardPtr b) { // Ask a new goal
							auto npc = b->getValue<NPC*>("npc");
							npc->AskNewGoal();
							b->overwrite("goal", npc->GetGoal());
					} }
			}} } }
	}} ;

	behaviorTreeLibrary["NpcPathBlockedReaction"] = new TaskSelector{ {
			new TaskSequence {{
				new TaskPredicate { [](Task::BlackboardPtr b) {return b->getValue<NPC*>("other")->IsOnGoal(); } },
				new TaskBasicAction{[](Task::BlackboardPtr b) { // Swap goal with the blocking NPC
						auto npc = b->getValue<NPC*>("npc");
						auto other = b->getValue<NPC*>("other");
						npc->SetGoal(other->GetGoal());
						b->getValue<NPC::Path_t*>("path")->clear();
						other->SetGoal(b->getValue<Hex>("goal"));
						b->overwrite("goal", npc->GetGoal());
				} },
				new NPC::TaskWait
			}},
			new TaskSequence {{
				new TaskPredicate { [](Task::BlackboardPtr b) {return b->getValue<NPC*>("other")->GetIntention() == b->getValue<Hex>("currentPos"); }},
				new NPC::TaskMove
			}},
			new NPC::TaskWait
	} };

	behaviorTreeLibrary["NpcBasicTree"] = new TaskSequence({
			new SubTreeReference("NpcGetPathOrWait"),
			new TaskSequence{{
				new TaskInverter{ new TaskSequence {{
						new NPC::TaskNextBlocked,
						new SubTreeReference("NpcPathBlockedReaction")
				}} },
				new NPC::TaskMove
			}}
	});

	behaviorTreeLibrary["NpcGetPath"] = new TaskSelector{{
			new NPC::TaskPathNonEmpty,
			new TaskFaillure{ new TaskSequence {{
				new TaskPredicate{[](Task::BlackboardPtr b) {return b->getValue<NPC*>("npc")->IsOnGoal(); }},
				new TaskBasicAction{[](Task::BlackboardPtr b) {
						auto npc = b->getValue<NPC*>("npc");
						npc->AskNewGoal();
						b->overwrite("goal", npc->GetGoal());
				}}
			}} },
			new TaskInverter{ new TaskSequence{{
					new NPC::TaskGetPath,
					new TaskInverter{ new NPC::TaskPathNonEmpty },
					new NPC::TaskWait
			}} }
	}};

	behaviorTreeLibrary["NpcBehaviorTree"] = new TaskSequence({
			new SubTreeReference("NpcGetPath"),
			new TaskSequence{{
				new TaskSelector {{
						new TaskPredicate {[](Task::BlackboardPtr b) { //Check if the next hex is atteignable
							auto npc = b->getValue<NPC*>("npc");
							auto next = b->getValue<NPC::Path_t*>("path")->front().getTo();
							auto connexions = b->getValue<Graph*>("graph")->getConnections(npc->GetPosition());
							return std::find_if(begin(connexions), end(connexions), [next](Edge& e) {return e.getTo() == next; }) != end(connexions);
						}},
						new TaskSequence {{new TaskBasicAction{[](Task::BlackboardPtr b) {
								auto npc = b->getValue<NPC*>("npc");
								b->getValue<NPC::Manager_t*>("manager")->separerConnexite();
								b->getValue<NPC::Manager_t*>("manager")->state = Manager::EXPLORATION;
								npc->AskNewGoal();
								b->overwrite("goal", npc->GetGoal());
							}},
							new NPC::TaskGetPath,
							new TaskInverter {new TaskSequence {{
								new TaskInverter{ new NPC::TaskPathNonEmpty },
								new NPC::TaskWait
							}} }
						}}
				}},
				new TaskInverter{ new TaskSequence {{
						new NPC::TaskNextBlocked,
						new SubTreeReference("NpcPathBlockedReaction")
				}} },
				new NPC::TaskMove
			}}
		});
}

void BehaviorTree::clearBehaviorTree()
{
	for (auto it = begin(behaviorTreeLibrary); it != end(behaviorTreeLibrary); ++it) {
		delete it->second;
	}
}


