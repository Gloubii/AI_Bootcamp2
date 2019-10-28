#pragma once


#include <set>
#include "NPC.h"
#include "Globals.h"
#include "Graph.h"

class NPC;

class Manager
{
private:
	template <class T>
	struct Connexe {
		std::set<T> composants;
		T representant;
		Connexe(T init) : composants{ { init } }, representant{ init } {}
		bool isInside(T elmt) const noexcept {
			return (std::find(composants.begin(), composants.end(), elmt) != composants.end());
		}
		void fusionner(Connexe<T> autreComposante) {
			composants.merge(autreComposante.composants);
		}
	};

	void updateState();
	void updateConnexite();
	bool allGoalsReachable();

	std::vector<Connexe<Hex>> connexeGoals;
	std::vector<Connexe<NPC*>> connexeNpcs;

public:
	enum ManagerState { INIT, EXPLORATION, GOTO_GOALS };

	Graph* modele;
	std::vector<NPC> npcs;
	std::vector<Hex> goals;
	std::vector<Hex> tempGoals;
	ManagerState state = INIT;

	void initManager(SInitData initData, Graph* modele_);
	void addNpcToConnexite(int id);
	void addGoalToConnexite(const Hex& goal);
	void createBasicbehaviorTree();
	void updateNpc(const STurnData& _turnData);
	bool isOccuped(const Hex& hex);
	NPC* getOccupant(const Hex& hex); //Return nullptr if the hex is empty

	void assignGoals();
	void getNewGoal(NPC* npc);
	std::vector<Edge> getPath(NPC* npc);
	void update();
};

