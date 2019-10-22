#include "Task.h"
#include "NPC.h"

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
					new NPC::TaskPathEmpty, 
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
}


