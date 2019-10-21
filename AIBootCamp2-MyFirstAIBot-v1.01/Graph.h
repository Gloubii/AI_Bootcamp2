#pragma once
#include <vector>
#include "Globals.h"
#include "InitData.h"
#include "TurnData.h"
#include <string>
#include <unordered_map>
#include "Hex.h"

class NodeData
{
	STileInfo tile;
};


class Node
{
	friend class Graph;
	//Hex hex;
	STileInfo tile;
	float value;
	bool unknown;

public:
	Node(STileInfo t);
	Node();				// Create an unknown node
	bool isTile(const STileInfo* _tile) {
		return _tile->q == tile.q && _tile->r == tile.r;
	}
	STileInfo getTile() const;
	bool operator==(const Node& node) { return tile.q == node.tile.q && tile.r == node.tile.r; }
	std::string toString() const;
	float getValue() const;
	bool isUnknown() const;
};

class Edge
{
	friend class Graph;
	Hex hex_from;
	Hex hex_to;
	float cost;			// -1 -> object		-2 -> red		-3 -> unknown
	EObjectType object;
public:
	Edge(Hex node1, Hex node2, float cost);
	Edge(Hex node1, Hex node2, float cost, EObjectType object);
	Edge() = default;

	Hex getFrom() const;
	Hex getTo() const;
	int getCost() const;
	std::string toString() const;
};


class Graph
{
	struct NodeRecord
	{
		//Node node;
		Hex hex_node;
		Edge edge;
		int costSoFar;
		int estimatedTotalCost;
		//NodeRecord(Node node_, Edge edge_, int costSoFar_, int estimatedTotalCost_) : node(node_), edge(edge_), costSoFar(costSoFar_), estimatedTotalCost(estimatedTotalCost_) {}
		NodeRecord() = default;
	};

public:
	Graph() = default;
	Graph(const SInitData& initData);

	//Return True if the grid changed
	bool Update(const STurnData& turnData);

	bool Contains(const Hex& hex) const;
	bool atteignable(const Hex& start, const Hex& finish) const;
	Node getNode(int q, int r) const;
	Node getNode(const Hex& hex) const;
	std::vector<Hex> GetHexes() const;
	std::vector<Hex> GetGoals() const;
	// Get connections used by the pathfinding
	std::vector<Edge> getConnections(const Hex& n) const;
	// Get all connections including walls ans connections to red 
	std::vector<Edge> getAllConnections(const Hex& n) const;
	EHexCellDirection getDirection(const Edge& e) const;
	std::vector<Node> getNeighbours(const Hex& hex) const;
	std::vector<Node> getNodes() const;
	bool connected(Hex n1, Hex n2, bool allConnection) const;
	float updateValue(const Hex& n) const;

	std::vector<Edge> aStar(const Hex& start, const Hex& finish) const;

	std::string toString() const;

private:
	//std::vector<Node> nodes;
	std::unordered_map<Hex, Node> nodes;
	std::vector<Edge> edges;
	std::vector<Hex> goals;
	int maxRow;
	int maxCol;
};

