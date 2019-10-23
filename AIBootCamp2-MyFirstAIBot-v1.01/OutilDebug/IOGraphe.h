#pragma once
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <fstream>

#include "./../Graph.h"


std::string const QtCreatorPath = "E:/AI_Bootcamp/Iteration2/IAMapCreator/Maps/";


class GraphParser {
public:
	GraphParser(Graph& graph);
	void WriteJson(std::string filename = "./OutilDebugLogs/graph2json.json");

private:
	rapidjson::Value Graph2Json();
	rapidjson::Value Node2Json(const Hex& h);
	rapidjson::Value Edge2Json(const Edge& e);

	Graph& graph;
	rapidjson::Document d;
};