#include "IOGraphe.h"
#include <algorithm>
#include <sstream>

using namespace std;
using namespace rapidjson;


/* Old version
void Graph2JsonFile(Graph& graph, string filename) {
	Document d;
	auto& allocator = d.GetAllocator();
	d.SetObject();

	// Recuperation des donnees a print
	auto nodes = graph.getNodes();
	auto hexes = graph.GetHexes();

	vector<Edge> edges;
	for (auto h : hexes) {
		auto e = graph.getAllConnections(h);
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
	for (Hex h : hexes) {
		nodeList.PushBack(Node2Json(graph, h, allocator), allocator);
	}
	graphJS.AddMember("nodes", nodeList, allocator);

	// Ajout des edges
	Value edgeList(kArrayType);
	for (Edge e : edges) {
		Hex nFrom = e.getFrom();
		Hex nTo = e.getTo();

		Value edgeObj(kObjectType);
		edgeObj.AddMember("from", Node2Json(graph, nFrom, allocator), allocator);
		edgeObj.AddMember("to", Node2Json(graph, nTo, allocator), allocator);

		EObjectType object;
		if (e.getCost(object) == -1) {
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
*/

vector<Edge> getAllEdges(const Graph& graph) {
	const vector<Hex> allHex = graph.GetHexes();
	vector<Edge> allEdges;

	auto getEdgesFromHex = [&allEdges, &graph](const Hex& h) {
		auto edgesFromH = graph.getAllConnections(h);
		allEdges.insert(end(allEdges), begin(edgesFromH), end(edgesFromH));
	};

	for_each(begin(allHex), end(allHex), getEdgesFromHex);

	return allEdges;
}


/* New version
void Graph2JsonFile(Graph& graph, std::string filename)
{
	Document d;
	auto& allocator = d.GetAllocator();
	d.SetObject();

	// Recuperation des donnees a print
	auto nodes = graph.getNodes();
	auto hexes = graph.GetHexes();
	auto edges = getAllEdges(graph);

	// Ajout de metadata
	Value graphJS(kObjectType);
	graphJS.AddMember("nb nodes", nodes.size(), allocator);
	graphJS.AddMember("nb edges", edges.size(), allocator);

	// Ajout des nodes
	Value nodeList(kArrayType);
	for_each(begin(hexes), end(hexes), [&nodeList, &graph, &allocator](Hex h) {
		nodeList.PushBack(Node2Json(graph, h, allocator), allocator);
	});
	graphJS.AddMember("nodes", nodeList, allocator);

	// Ajout des edges
	Value edgeList(kArrayType);
	for_each(begin(edges), end(edges), [&edgeList, &graph, &allocator](Edge e) {
		Value edgeObj = Edge2Json(graph,e, allocator);
		edgeList.PushBack(edgeObj, allocator);
	});
	graphJS.AddMember("edges", edgeList, allocator);
	
	// Ajout du graphe au document
	d.AddMember("graphe", graphJS, allocator);

	// Ecriture du document
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	ofstream os(filename);
	os << buffer.GetString();
}
*/

GraphParser::GraphParser(Graph& graph) : graph(graph), d(rapidjson::Document{})
{
	d.SetObject();
}

void GraphParser::WriteJson(std::string filename)
{
	Value graphJson = Graph2Json();
	d.AddMember("graphe", graphJson, d.GetAllocator());

	// Ecriture du document
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	ofstream os(filename);
	os << buffer.GetString();
}

void GraphParser::WriteJson(int n)
{
	stringstream ss;
	ss << "./OutilDebugLogs/graph2json_" << n << ".json";
	WriteJson(ss.str());
}

rapidjson::Value GraphParser::Graph2Json() {
	auto& allocator = d.GetAllocator();

	// Recuperation des donnees a print
	auto nodes = graph.getNodes();
	auto hexes = graph.GetHexes();
	auto edges = getAllEdges(graph);

	// Ajout de metadata
	Value graphJS(kObjectType);
	graphJS.AddMember("nb nodes", nodes.size(), allocator);
	graphJS.AddMember("nb edges", edges.size(), allocator);

	// Ajout des nodes
	Value nodeList(kArrayType);
	for_each(begin(hexes), end(hexes), [&](Hex h) {
		nodeList.PushBack(Node2Json(h), allocator);
	});
	graphJS.AddMember("nodes", nodeList, allocator);

	// Ajout des edges
	Value edgeList(kArrayType);
	for_each(begin(edges), end(edges), [&](Edge e) {
		Value edgeObj = Edge2Json(e);
		edgeList.PushBack(edgeObj, allocator);
	});
	graphJS.AddMember("edges", edgeList, allocator);

	return graphJS;
}

rapidjson::Value GraphParser::Node2Json(const Hex& h)
{
	auto& allocator = d.GetAllocator();

	Value nodeObj(kObjectType);
	nodeObj.AddMember("q", h.x, allocator);
	nodeObj.AddMember("r", h.y, allocator);
	if (graph.getNode(h).isUnknown()) {
		nodeObj.AddMember("type", -1, allocator);
	}
	else {
		nodeObj.AddMember("type", graph.getNode(h).getTile().type, allocator);
	}

	return nodeObj;
}

rapidjson::Value GraphParser::Edge2Json(const Edge& edge) {
	auto& allocator = d.GetAllocator();

	Hex nFrom = edge.getFrom();
	Hex nTo = edge.getTo();

	Value edgeObj(kObjectType);
	edgeObj.AddMember("from", Node2Json(nFrom), allocator);
	edgeObj.AddMember("to", Node2Json(nTo), allocator);

	EObjectType object;
	if (edge.getCost(object) == -1) {
		edgeObj.AddMember("object", object, allocator);
	}
	else {
		edgeObj.AddMember("object", -1, allocator);
	}

	return edgeObj;
}
