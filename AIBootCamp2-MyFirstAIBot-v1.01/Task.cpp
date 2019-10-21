#include "Task.h"

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
