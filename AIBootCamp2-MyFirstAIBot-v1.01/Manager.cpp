#include "Manager.h"
#include <algorithm>
#include <fstream>
#include "Globals.h"
#include <assert.h>

using namespace std;

void Manager::initManager(SInitData initData, Graph* modele_)
{
	// pointeur sur le modele
	modele = modele_;
	goals = modele->GetGoals();

	// recuperation des npcs
	for (int i = 0; i < initData.nbNPCs; ++i) {
		npcs.push_back(NPC{ this,initData.npcInfoArray[i], *modele_ });
	}
	for (int i = 0; i < initData.nbNPCs; i++) {
		addNpcToConnexite(&npcs[i]);
	}
	for (auto& g : goals) {
		addGoalToConnexite(g);
	}
}

void Manager::addNpcToConnexite(NPC* newNPC)
{
	

	// Ajout du npc dans les composantes connexes
	auto itConnexe = find_if(connexeNpcs.begin(), connexeNpcs.end(), [&](const Connexe<NPC*>& c) {
		NPC* representant = c.representant;
		return modele->atteignable(representant->GetPosition(), newNPC->GetPosition());
	});

	if (itConnexe == connexeNpcs.end()) {
		// s'il n'existe pas encore de composante connexe permettant d'introduire le nouveau npc,
		// on en construit une nouvelle
		//Connexe<NPC*> newConnexite;
		//newConnexite.composants.insert(newNpc);
		connexeNpcs.emplace_back(newNPC);
	}
	else {
		// sinon, on ajoute le nouvel npc aux npcs de sa composante connexe
		itConnexe->composants.insert(newNPC);
	}

}

void Manager::addGoalToConnexite(const Hex& goal)
{
	auto itConnex = find_if(begin(connexeGoals), end(connexeGoals), [&](const Connexe<Hex>& c) { return modele->atteignable(c.representant, goal); });
	if (itConnex == connexeGoals.end()) {
		connexeGoals.emplace_back(goal);
	}
	else {
		itConnex->composants.insert(goal);
	}
}

void Manager::createBasicbehaviorTree()
{
	for (auto& npc : npcs) {
		npc.SetBehaviorTree("NpcBehaviorTree");
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
		getNewGoal(&npc);
	}
}

void Manager::getNewGoal(NPC* npc)
{
	fstream file("Manager_getNewGoal.txt", ofstream::app);
	Hex pos = npc->GetPosition();
	//file << "state " << state << " from " << pos.toString() << " to ";

	if (state == EXPLORATION) {
		// on recupere les nodes et on les trie par ordre decroissant de valeur d'exploration
		set<Hex> connexes = modele->getConvexes(npc->GetPosition());
		vector<Node> allNodes;
		auto toNode = [&allNodes, g = modele](Hex h) {
			allNodes.push_back(g->getNode(h));
		};
		for_each(connexes.begin(), connexes.end(), toNode);

		//float delta = sqrt(pow(modele->maxRow, 2) + pow(modele->maxCol, 2)) / 3.14f;
		sort(allNodes.begin(), allNodes.end(), [&](Node a, Node b) {
			Hex npcPosition = npc->GetPosition();
			float d1 = npcPosition.DistanceTo(Hex(a.getTile().q, a.getTile().r));
			float d2 = npcPosition.DistanceTo(Hex(b.getTile().q, b.getTile().r));
			//float m = a.getValue() + b.getValue();
			//auto dm = d1 + d2;
			if (a.getValue() == 1 && b.getValue() == 1)
				return false;
			if (a.getValue() == 1)
				return true;
			if (b.getValue() == 1)
				return false;

			if (d1 > npc->GetVisionRange() && d2 > npc->GetVisionRange())
				return d2 < d1;
			if (d1 > npc->GetVisionRange())
				return true;
			if (d2 > npc->GetVisionRange())
				return false;

			return a.getValue() > b.getValue();
		});
		reverse(allNodes.begin(), allNodes.end());
		file << "ALL NODES :" << endl;
		for (Node n : allNodes) {
			file << n.getTile().q << " ; " << n.getTile().r << "  :  " << n.getValue() << endl;
		}
		file << endl << endl;

		// on conserve les nodes avec la meilleure valeur (max avec cas d'egalite)
		int i = 1;
		vector<Node> bestNodes{ allNodes.at(0) };
		while (i < allNodes.size() && allNodes.at(i).getValue() == allNodes.at(i - 1).getValue()) {
			bestNodes.push_back(allNodes.at(i));
			i++;
		}
		file << "BEST NODES :" << endl;
		for (Node n : bestNodes) {
			file << n.getTile().q << " ; " << n.getTile().r << "  :  " << n.getValue() << endl;
		}
		file << endl << endl;

		Hex npcPosition = npc->GetPosition();
		auto bestNode = min_element(bestNodes.begin(), bestNodes.end(), [&npcPosition](const Node& a, const Node& b) {
			return npcPosition.DistanceTo(Hex{ a.getTile().q, a.getTile().r }) < npcPosition.DistanceTo(Hex{ b.getTile().q, b.getTile().r });
		});

		assert(bestNode != bestNodes.end());

		Hex newGoal(bestNode->getTile().q, bestNode->getTile().r);
		npc->SetGoal(newGoal);
		file << "state " << state << " from " << pos.toString() << " to " << newGoal.toString() << endl << endl << endl;
	}
	else {
		// state == GOTO_GOALS
		vector<Hex> npcTakenGoals;
		for (NPC& otherNpc : npcs) {
			if (otherNpc != *npc)
				npcTakenGoals.push_back(otherNpc.GetGoal());
		}

		// recuperation des goals connexes
		set<Hex> connexeHex = modele->getConvexes(npc->GetPosition());
		vector<Hex> connexeGoals;
		for (Hex h : connexeHex) {
			if (modele->getNode(h).getTile().type == Goal)
				connexeGoals.push_back(h);
		}
		auto it = find_if(connexeGoals.begin(), connexeGoals.end(), [&npcTakenGoals](Hex goal) {
			// retourne le premier true goal qui n'est pas parmi les npc taken goals (pas deja pris)
			return (find(npcTakenGoals.begin(), npcTakenGoals.end(), goal) == npcTakenGoals.end());
		});

		npc->SetGoal(*it);
		//file << it->toString() << endl;
	}
}

vector<Edge> Manager::getPath(NPC* npc)
{
	Hex start = npc->GetPosition();
	Hex finish = npc->GetGoal();
	bool exploration = (state == EXPLORATION);
	vector<Edge> path = modele->aStar(start, finish, exploration);
	return path;
}

void Manager::update()
{
	auto futureGoals = modele->GetGoals();
	if (futureGoals.size() > goals.size()) {
		goals = futureGoals;
		for (auto& g : goals) {
			addGoalToConnexite(g);
		}
	}
	
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
			for (auto& npc : npcs) {
				npc.ClearPath();
				npc.AskNewGoal();
			}
		}
		break;

	case Manager::GOTO_GOALS:
		// etat final
		
		break;

	default:
		break;
	}
}

void Manager::separerConnexite() {
	connexeGoals.clear();
	connexeNpcs.clear();

	for (int i = 0; i < npcs.size(); i++) {
		addNpcToConnexite(&npcs[i]);
	}
	for (auto& g : goals) {
		addGoalToConnexite(g);
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

void Manager::verifyConexite()
{/*
	for (auto &connex : connexeGoals) {
		auto it = remove_if(begin(connex.composants),
			end(connex.composants),
			[&](const Hex &g) {
				return !modele->atteignable(connex.representant, g);
			});
		//for_each(it, end(connex.composants), [&](auto g) {addGoalToConnexite(g); });
		//connex.composants.erase(it, end(connex.composants));
	}
	for (auto& connex : connexeNpcs) {
		auto it = remove_if(begin(connex.composants),
			end(connex.composants),
			[&](NPC* npc) {
				return !modele->atteignable(connex.representant->GetPosition(), npc->GetPosition());
			});
		for_each(it, end(connex.composants), [&](NPC* npc) {addNpcToConnexite(npc); });
		connex.composants.erase(it, end(connex.composants));
	}
	state = ManagerState::EXPLORATION;*/
}

bool Manager::allGoalsReachable()
{
	if (connexeGoals.size() < connexeNpcs.size())
		return false;

	for (Connexe<NPC*> coNPC : connexeNpcs) {
		const NPC* representantNPC = coNPC.representant;
		
		bool foundAssociatedGoals = false;
		for (Connexe<Hex> coGoals : connexeGoals) {
			Hex representantGoal = coGoals.representant;
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

