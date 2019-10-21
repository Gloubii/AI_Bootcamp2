#include "Graph.h"
#include <algorithm>

using namespace std;

/*
bool Graph::areConnected(Node n1, Node n2)
{
	auto exist = [n1, n2](Edge e) {return (e.id_n1 == n1.id && e.id_n2 == n2.id) || (e.id_n1 == n2.id && e.id_n2 == n1.id);};
	return any_of(begin(edges), end(edges), exist);
}
*/

Node::Node(STileInfo t) : tile(t) {}

string Node::toString() const
{
	string out;
	out.append(" : Q= ");
	out.append(to_string(getTile().q));
	out.append("	R= ");
	out.append(to_string(getTile().r));
	out.append("\n");
	return out;
}

STileInfo Node::getTile() const
{
	return tile;
}

Edge::Edge(Hex node1, Hex node2, int cost) : hex_from(node1), hex_to(node2), cost(cost) {}

Edge::Edge(Hex node1, Hex node2, int cost, EObjectType object) : hex_from(node1), hex_to(node2), cost(cost), object(object) {}

Hex Edge::getFrom() const 
{
	return hex_from;
}
Hex Edge::getTo() const
{
	return hex_to;
}

int Edge::getCost() const
{
	return cost;
}

int Edge::getCost(EObjectType& object_) const
{
	int cost = getCost();
	if (cost == -1) {
		object_ = object;
	}
	return cost;
}

string Edge::toString() const
{
	string out = "";
	if (cost != -1)
		out.append("lien entre ");
	else
		out.append("obstacle entre ");
	out.append(hex_from.toString());
	out.append(" et ");
	out.append(hex_to.toString());
	out.append("\n");
	return out;
}


Graph::Graph(const SInitData& initData)
{
	// add nodes
	for (int i = 0; i < initData.tileInfoArraySize; ++i) {
		if (initData.tileInfoArray[i].type != EHexCellType::Forbidden)
			nodes.insert_or_assign(Hex{ initData.tileInfoArray[i].q, initData.tileInfoArray[i].r }, initData.tileInfoArray[i]);
		if (initData.tileInfoArray[i].type == EHexCellType::Goal) {
			goals.push_back(Hex{ initData.tileInfoArray[i].q, initData.tileInfoArray[i].r });
		}
	}

	// get obstacles
	std::vector<SObjectInfo> objects;
	for (int i = 0; i < initData.objectInfoArraySize; ++i) {
		objects.push_back(initData.objectInfoArray[i]);
	}
	
	// add edges
	for (auto hex : GetHexes()) {
		for (int dir = 0; dir < 6; ++dir) {
			auto voisin = hex.GetNeighbour((EHexCellDirection)dir);
			if (Contains(voisin)) {
				EObjectType object;
				auto isWall = [hex, voisin, dir, &object](SObjectInfo objet) {
					if (!((objet.q == hex.x && objet.r == hex.y && objet.cellPosition == (EHexCellDirection)dir) ||
						(objet.q == voisin.x && objet.r == voisin.y && objet.cellPosition == (EHexCellDirection)((dir + 3) % 6))))
						return false;
					object = (EObjectType) objet.types[0];
					return true; };
				if (find_if(begin(objects), end(objects), isWall) == end(objects))
					edges.push_back(Edge(hex, voisin, 1));
				else
					edges.push_back(Edge(hex, voisin, -1));
			}
		}
	}
}

bool Graph::Update(const STurnData& turnData)
{
	std::vector<Hex> changed;
	// add nodes
	for (int i = 0; i < turnData.tileInfoArraySize; ++i) {
		if (!Contains(Hex(turnData.tileInfoArray[i].q, turnData.tileInfoArray[i].r))) {
			if (turnData.tileInfoArray[i].type != EHexCellType::Forbidden) {
				nodes.insert_or_assign(Hex{ turnData.tileInfoArray[i].q, turnData.tileInfoArray[i].r }, turnData.tileInfoArray[i]);
				changed.push_back(Hex{ turnData.tileInfoArray[i].q, turnData.tileInfoArray[i].r });
			}
			if (turnData.tileInfoArray[i].type == EHexCellType::Goal) {
				goals.push_back(Hex{ turnData.tileInfoArray[i].q, turnData.tileInfoArray[i].r });
			}
		}
	}


	std::vector<SObjectInfo> objects;
	// get obstacles
	if (changed.size()) {
		for (int i = 0; i < turnData.objectInfoArraySize; ++i) {
			objects.push_back(turnData.objectInfoArray[i]);
		}
	}
	

	// add edges	TODO : optimiser pour ne pas repasser toute la map
	for (auto hex : GetHexes()) {
		for (int dir = 0; dir < 6; ++dir) {
			auto voisin = hex.GetNeighbour((EHexCellDirection)dir);
			if (Contains(voisin)) {
				EObjectType object;
				auto isWall = [hex, voisin, dir, &object](SObjectInfo objet) {
					if (!((objet.q == hex.x && objet.r == hex.y && objet.cellPosition == (EHexCellDirection)dir) ||
						(objet.q == voisin.x && objet.r == voisin.y && objet.cellPosition == (EHexCellDirection)((dir + 3) % 6))))
						return false;
					object = (EObjectType)objet.types[0];
					return true; };
				if (find_if(begin(objects), end(objects), isWall) == end(objects))
					edges.push_back(Edge(hex, voisin, 1));
				else
					edges.push_back(Edge(hex, voisin, -1));
			}
		}
	}
	return changed.size();
}

vector<Edge> Graph::getConnections(const Hex& hex) const
{
	vector<Edge> v;
	auto addEdges = [&v, hex](Edge e) {if (e.hex_from == hex && e.getCost() != -1) v.push_back(e); };
	for_each(begin(edges), end(edges), addEdges);
	return v;
}

vector<Edge> Graph::getAllConnections(const Hex& hex) const
{
	vector<Edge> v;
	auto addEdges = [&v, hex](Edge e) {if (e.hex_from == hex) v.push_back(e); };
	for_each(begin(edges), end(edges), addEdges);
	return v;
}

std::vector<Node> Graph::getNeighbours(const Hex& hex) const
{
	std::vector<Node> neighbours;
	for (auto h : hex.GetNeighbours()) {
		if (Contains(h))
			neighbours.push_back(nodes.at(h));
	}
	return neighbours;
}

bool Graph::Contains(const Hex& hex) const
{
	auto hexes = GetHexes();
	return std::find(hexes.begin(), hexes.end(), hex) != hexes.end();
}

std::vector<Hex> Graph::GetHexes() const
{
	vector<Hex> hexes;
	auto getHex = [&hexes](auto h) {hexes.push_back(h.first); };
	for_each(nodes.cbegin(), nodes.cend(), getHex);
	return hexes;
}

std::vector<Hex> Graph::GetGoals() const
{
	return goals;
}

std::vector<Node> Graph::getNodes() const
{
	vector<Node> v_nodes;
	auto getHex = [&v_nodes](auto h) {v_nodes.push_back(h.second); };
	for_each(nodes.cbegin(), nodes.cend(), getHex);
	return v_nodes;
}


vector<Edge> Graph::aStar(const Hex& start, const Hex& finish) const
{
	// Initialize the record for the start node
	NodeRecord startRecord;
	//startRecord.node = nodes.at(start);
	startRecord.hex_node = start;
	startRecord.costSoFar = 0;
	startRecord.estimatedTotalCost = start.DistanceTo(finish);

	// Initialize the open and closed list
	vector<NodeRecord> open;
	open.push_back(startRecord);
	vector<NodeRecord> closed;

	NodeRecord current;

	while (open.size() > 0) {
		// Find the smallest element in the open list 
		current = open.at(0);
		auto findmin = [&current](NodeRecord n) {if (n.estimatedTotalCost < current.estimatedTotalCost) current = n; };
		for_each(begin(open) + 1, end(open), findmin);

		// If it is the goal node, then terminate
		if (current.hex_node == finish) {
			break;
		}

		// Otherwise get its outgoing connections
		vector<Edge> connections = getConnections(current.hex_node);

		// Loop throught each connection in turn
		for (Edge e : connections) {
			//Node endNode = nodes.at(e.hex_to);
			int endNodeCost = current.costSoFar + e.cost;
			NodeRecord endNodeRecord;
			int endNodeHeuristic;

			// if the node is closed we may have to skip, or remove it from the closed list
			auto isEndNode = [e](NodeRecord nr) {return nr.hex_node == e.getTo(); };
			if (any_of(begin(closed), end(closed), isEndNode)) {
				// here we find the record in the closed list corresponding to the node
				auto ptr_endNodeRecord = find_if(begin(closed), end(closed), isEndNode);
				endNodeRecord = *ptr_endNodeRecord;

				// if we didn't find a shorter route, skip
				if (endNodeRecord.costSoFar <= endNodeCost)
					continue;

				// otherwise remove it from the closed list
				closed.erase(ptr_endNodeRecord);

				// endNodeHeuritic
				endNodeHeuristic = endNodeRecord.estimatedTotalCost - endNodeRecord.costSoFar;
			}
			else if (any_of(begin(open), end(open), isEndNode)) {
				NodeRecord endNodeRecord = *find_if(begin(open), end(open), isEndNode);

				// if out route is no better, the skip
				if (endNodeRecord.costSoFar <= endNodeCost)
					continue;

				// endNodeHeuristic
				endNodeHeuristic = endNodeRecord.estimatedTotalCost - endNodeRecord.costSoFar;
			}
			else {
				//endNodeRecord.node = endNode;
				endNodeRecord.hex_node = e.hex_to;
				endNodeHeuristic = e.hex_to.DistanceTo(finish);
			}

			// we're here if we need to update the node. Update the cost, estimate ans connection
			endNodeRecord.costSoFar = endNodeCost;
			endNodeRecord.edge = e;
			endNodeRecord.estimatedTotalCost = endNodeCost + endNodeHeuristic;

			// and add it to the open list
			if (!any_of(begin(open), end(open), isEndNode))
				open.push_back(endNodeRecord);
		}

		// we've finished looking at the connections for the current node, so add it to the closest list and remove it from the open list
		auto isCurrentNode = [currentNode = current.hex_node](NodeRecord nr) {return nr.hex_node == currentNode; };
		open.erase(find_if(begin(open), end(open), isCurrentNode));
		closed.push_back(current);
	}

	// we're here if we've either found the goal, or we've no more nodes to search, find wich
	if (current.hex_node != finish) {
		return vector<Edge>();
	}
	vector<Edge> path;
	while (current.hex_node != start) {
		path.push_back(current.edge);
		auto isPrec = [node = current.edge.hex_from](NodeRecord nr) {return nr.hex_node == node; };
		current = *find_if(begin(closed), end(closed), isPrec);
	}
	reverse(begin(path), end(path));
	//path.erase(begin(path));
	return path;
}

bool Graph::atteignable(const Hex& start, const Hex& finish) const
{
	return aStar(start,finish).size();
}


Node Graph::getNode(int q, int r) const
{
	return nodes.at(Hex(q, r));
}

Node Graph::getNode(const Hex& hex) const
{
	return nodes.at(hex);
}

EHexCellDirection Graph::getDirection(const Edge& e) const
{
	Node start = nodes.at(e.hex_from);
	Node finish = nodes.at(e.hex_to);
	int q = finish.getTile().q - start.getTile().q;
	int r = finish.getTile().r - start.getTile().r;

	if (q == 0 && r == -1)
		return EHexCellDirection::W;
	if (q == -1 && r == 0)
		return EHexCellDirection::NW;
	if (q == -1 && r == 1)
		return EHexCellDirection::NE;
	if (q == 0 && r == 1)
		return EHexCellDirection::E;
	if (q == 1 && r == 0)
		return EHexCellDirection::SE;
	return EHexCellDirection::SW;
}

string Graph::toString() const
{
	string out = "";
	auto afficheNode = [&out](std::pair<Hex,Node> p)
	{
		out.append(p.second.toString());
	};
	for_each(begin(nodes), end(nodes), afficheNode);
	out.append("nb edges = ");
	out.append(to_string(edges.size()));
	out.append("\n");

	auto afficheEdge = [&out](Edge e)
	{
		out.append(e.toString());
	};
	for_each(begin(edges), end(edges), afficheEdge);
	
	return out;
}