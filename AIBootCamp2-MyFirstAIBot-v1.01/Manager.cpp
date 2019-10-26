#include "Manager.h"
#include <algorithm>

using namespace std;

void Manager::initManager(SInitData initData, Graph* modele_)
{
	// pointeur sur le modele
	modele = modele_;

	// recuperation des npcs
	for (int i = 0; i < initData.nbNPCs; ++i) {
		npcs.push_back(NPC{ this,initData.npcInfoArray[i], *modele_ });
		addNpcToConnexite(&npcs.at(i));
	}
}

void Manager::addNpcToConnexite(NPC* newNpc)
{
	// Ajout du npc dans les composantes connexes
	auto itConnexe = find_if(connexeNpcs.begin(), connexeNpcs.end(), [&](Connexe<NPC*> c) {
		NPC* representant = *c.composants.begin();
		return modele->atteignable(representant->GetPosition(), newNpc->GetPosition());
	});
	
	if (itConnexe == connexeNpcs.end()) {
		// s'il n'existe pas encore de composante connexe permettant d'introduire le nouveau npc,
		// on en construit une nouvelle
		Connexe<NPC*> newConnexite;
		newConnexite.composants.emplace(newNpc);
	}
	else {
		// sinon, on ajoute le nouvel npc aux npcs de sa composante connexe
		itConnexe->composants.emplace(newNpc);
	}
}

void Manager::createBasicbehaviorTree()
{
	for (auto& npc : npcs) {
		npc.SetBehaviorTree("NpcABitLessBasicTree");
	}
}

void Manager::updateNpc(const STurnData& _turnData)
{
	for (int i = 0; i < _turnData.npcInfoArraySize; ++i) {
		npcs[i].Update(_turnData.npcInfoArray[i]);
	}
}

bool Manager::isOccuped(const Hex& hex){
	return std::find_if(begin(npcs), end(npcs), [hex](auto &npc) {return npc.GetPosition() == hex; }) != end(npcs);
}

NPC* Manager::getOccupant(const Hex& hex)
{
	auto p = std::find_if(begin(npcs), end(npcs), [hex](auto& npc) {return npc.GetPosition() == hex; });
	return p != end(npcs) ? &*p : nullptr;
}

void Manager::assignGoals()
{
	for (NPC& npc : npcs) {
		
		auto nearest = std::min_element(goals.cbegin(), goals.cend(), 
			[start = npc.GetPosition()](const Hex& g1, const Hex& g2) 
			{
				return g1.DistanceTo(start) < g2.DistanceTo(start); 
			}
		);
		npc.SetGoal(*nearest);
		goals.erase(nearest);
	}
}

void Manager::getNewGoal(NPC* npc)
{
	if (state == EXPLORATION) {
		// on recupere les nodes et on les trie par ordre decroissant de valeur d'exploration
		vector<Node> allNodes = modele->getNodes();
		sort(allNodes.begin(), allNodes.end(), [](Node a, Node b) {
			return a.getValue() < b.getValue();
		});
		reverse(allNodes.begin(), allNodes.end());

		// on conserve les nodes avec la meilleure valeur (max avec cas d'egalite)
		int i = 1;
		vector<Node> bestNodes;
		while (i < allNodes.size() && allNodes.at(i).getValue() == allNodes.at(i - 1).getValue()) {
			bestNodes.push_back(allNodes.at(i));
			i++;
		}

		Hex npcPosition = npc->GetPosition();
		auto bestNode = min_element(bestNodes.begin(), bestNodes.end(), [&npcPosition](const Node& a, const Node& b) {
			return npcPosition.DistanceTo(Hex{ a.getTile().q, a.getTile().r }) < npcPosition.DistanceTo(Hex{ b.getTile().q, b.getTile().r });
		});

		Hex newGoal(bestNode->getTile().q, bestNode->getTile().r);
		npc->SetGoal(newGoal);
	}
	else {
		// state == GOTO_GOALS
		vector<Hex> npcTakenGoals;
		for (NPC& npc : npcs) {
			npcTakenGoals.push_back(npc.GetGoal());
		}

		auto it = find_if(goals.begin(), goals.end(), [&npcTakenGoals](Hex goal) {
			// retourne le premier true goal qui n'est pas parmi les npc taken goals (pas deja pris)
			return (find(npcTakenGoals.begin(), npcTakenGoals.end(), goal) == npcTakenGoals.end());
		});

		npc->SetGoal(*it);
	}
}

void Manager::getPath(NPC* npc)
{
	
}

void Manager::update()
{
	goals = modele->GetGoals();
	updateConnexite();
	updateState();
}

void Manager::updateState()
{
	switch (state)
	{
	case Manager::INIT:
		if (npcs.size() > 0) {
			state = EXPLORATION;
		}
		break;

	case Manager::EXPLORATION:
		if (allGoalsReachable()) {
			state = GOTO_GOALS;
		}
		break;

	case Manager::GOTO_GOALS:
		// etat final
		break;

	default:
		break;
	}
}


void Manager::updateConnexite()
{
	int i = 1;
	while (i < connexeGoals.size()) {
		Hex nodeI = *(connexeGoals.at(i).composants.begin());
		Hex nodeH = *(connexeGoals.at(static_cast<int>(i - 1)).composants.begin());

		if (modele->atteignable(nodeI, nodeH)) {
			// on fusionne les deux composantes connexes
			// et on en retire une de la liste des composantes (pas de doublon)
			connexeGoals.at(static_cast<int>(i - 1)).fusionner(connexeGoals.at(static_cast<int>(i)));
			connexeGoals.erase(connexeGoals.begin() + i);
		}
		else {
			i++;
		}
	}

	i = 1;
	while (i < connexeNpcs.size()) {
		NPC* npcI = *connexeNpcs.at(i).composants.begin();
		NPC* npcH = *connexeNpcs.at(static_cast<int>(i - 1)).composants.begin();

		Hex nodeI = npcI->GetPosition();
		Hex nodeH = npcH->GetPosition();

		if (modele->atteignable(nodeI, nodeH)) {
			// on fusionne les deux composantes connexes
			// et on en retire une de la liste des composantes (pas de doublon)
			connexeNpcs.at(static_cast<int>(i - 1)).fusionner(connexeNpcs.at(static_cast<int>(i)));
			connexeNpcs.erase(connexeNpcs.begin() + i);
		}
		else {
			i++;
		}
	}
}

bool Manager::allGoalsReachable()
{
	if (connexeGoals.size() < connexeNpcs.size())
		return false;

	for (Connexe<NPC*> coNPC : connexeNpcs) {
		const NPC* representantNPC = *coNPC.composants.begin();
		
		bool foundAssociatedGoals = false;
		for (Connexe<Hex> coGoals : connexeGoals) {
			Hex representantGoal = *coGoals.composants.begin();
			if (modele->atteignable(representantGoal, representantNPC->GetPosition())) {
				foundAssociatedGoals = true;

				if (coNPC.composants.size() > coGoals.composants.size())
					return false; // il n'y a pas assez de goals atteignables par les NPCs de ce cluster

				break;
			}
		}

		if (!foundAssociatedGoals) {
			return false; // les NPC du cluster de son representant ne peuvent atteindre aucun goal connu
		}
	}

	return true; // pour tous les clusters de NPC, on a trouve leur cluster de Goal associe et il contient assez de goals
}

