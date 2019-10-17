#pragma once
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <fstream>

#include "./../Graph.h"


void Graph2JsonFile(Graph& graph, std::string filename = "./OutilDebugLogs/graph2json.json");
rapidjson::Value Node2Json(Node& node, rapidjson::MemoryPoolAllocator<>& allocator);

//Graph Json2Graph(std::string filename);