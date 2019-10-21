#include "IOGraphe.h"

using namespace std;
using namespace rapidjson;

void Graph2JsonFile(Graph& graph, std::string filename)
{
	Document d;
	auto& allocator = d.GetAllocator();
	d.SetObject();

	// Recuperation des donnees a print
	auto nodes = graph.getNodes();
	auto hexes = graph.GetHexes();
	
	std::vector<Edge> edges;
	for (auto h : hexes) {
		auto e = graph.getConnections(h);
		for (Edge ed : e) {
			edges.push_back(ed);
		}
	}

	// Ajout de metadata
	Value graphJS(kObjectType);
	graphJS.AddMember("nb nodes", nodes.size(), allocator);
	graphJS.AddMember("nb edges", edges.size(), allocator);

	// Ajout des nodes
	Value nodeList(kArrayType);
	for (Node n : nodes) {
		nodeList.PushBack(Node2Json(n, allocator), allocator);
	}
	graphJS.AddMember("nodes", nodeList, allocator);

	// Ajout des edges
	Value edgeList(kArrayType);
	for (Edge e : edges) {
		Node nFrom = graph.getNode(e.getFrom());
		Node nTo = graph.getNode(e.getTo());

		Value edgeObj(kObjectType);
		edgeObj.AddMember("from", Node2Json(nFrom, allocator), allocator);
		edgeObj.AddMember("to", Node2Json(nTo, allocator), allocator);
		
		EObjectType object;
		if (e.getCost(object)) {
			edgeObj.AddMember("object", object, allocator);
		}
		else {
			edgeObj.AddMember("object", -1, allocator);
		}

		edgeList.PushBack(edgeObj, allocator);
	}
	graphJS.AddMember("edges", edgeList, allocator);
	
	// Ajout du graphe au document
	d.AddMember("graphe", graphJS, allocator);

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	ofstream os(filename);
	os << buffer.GetString();
}

rapidjson::Value Node2Json(Node& node, MemoryPoolAllocator<>& allocator)
{
	Value nodeObj(kObjectType);
	nodeObj.AddMember("q", node.getTile().q, allocator);
	nodeObj.AddMember("r", node.getTile().r, allocator);
	nodeObj.AddMember("type", node.getTile().type, allocator);

	return nodeObj;
}
