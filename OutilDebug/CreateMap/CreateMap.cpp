// CreateMap.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <vector>
#include "InitData.h"
#include <fstream>
#include "Globals.h"
#include "MyBotLogic.h"
#include "TurnData.h"


using namespace rapidjson;
using namespace std;

SInitData jsonToInitData(string fileName) {
	ifstream is(fileName);
	string json((std::istreambuf_iterator<char>(is)),
		(std::istreambuf_iterator<char>()));
	
	Document d;
	 
	d.Parse(json.c_str());
	
	SInitData initData;
	initData.initDelay = 1000;
	initData.omniscient = true;
	initData.objectInfoArraySize = 0;
	initData.maxTurnNb = 40;
	initData.turnDelay = 1000;
	
	initData.rowCount = d["w"].GetInt();
	initData.colCount = d["h"].GetInt();

	auto npcArray = d["npcs"].GetArray();

	SNPCInfo *npc;
	npc = static_cast<SNPCInfo*>(malloc(npcArray.Size() * sizeof(SNPCInfo)));
	for (auto i = npcArray.Begin(); i != npcArray.End(); ++i) {
		Value n = i->GetObject();
		npc[i - npcArray.Begin()] = SNPCInfo();
		npc[i - npcArray.Begin()].q = n["q"].GetInt();
		npc[i - npcArray.Begin()].r = n["r"].GetInt();
		npc[i - npcArray.Begin()].uid = n["uid"].GetInt();
		npc[i - npcArray.Begin()].visionRange = n["visionRange"].GetInt();

	}

	initData.nbNPCs = npcArray.Size();
	initData.npcInfoArray = npc;

	

	auto tileArray = d["hexes"].GetArray();

	STileInfo* tiles;
	tiles = static_cast<STileInfo*>(malloc(tileArray.Size() * sizeof(STileInfo)));
	for (auto i = tileArray.Begin(); i != tileArray.End(); ++i) {
		Value t = i->GetObject();
		tiles[i - tileArray.Begin()] = STileInfo();
		tiles[i - tileArray.Begin()].q = t["q"].GetInt();
		tiles[i - tileArray.Begin()].r = t["r"].GetInt();
		tiles[i - tileArray.Begin()].type = EHexCellType(t["type"].GetInt());

	}

	initData.tileInfoArraySize = tileArray.Size();
	initData.tileInfoArray = tiles;

	return initData;
}

Document OrdersToJson(std::vector<std::list<SOrder>> orders)
{
	Document d;
	auto& allocator = d.GetAllocator();
	d.SetObject();

	Value v(kArrayType);
	for (auto const& os : orders)
	{
		Value turnOrders(kArrayType);
		for (auto const& o : os)
		{
			Value order(kObjectType);
			order.AddMember("orderType", o.orderType, allocator);
			order.AddMember("npcUID", o.npcUID, allocator);
			order.AddMember("direction", o.direction, allocator);
			order.AddMember("objectUID", o.objectUID, allocator);
			order.AddMember("interactionType", o.interactionType, allocator);


			turnOrders.PushBack(order, allocator);
		}

		v.PushBack(turnOrders, allocator);
	}

	d.AddMember("allOrders", v, allocator);
	return d;
}

int main(int argc, char** argv) {
	
	MyBotLogic myBot;
	
	SInitData initData = jsonToInitData("map5.json");
	

	
	myBot.Init(initData);

	vector<list<SOrder>> allOrders;
	for (int i = 0; i < initData.maxTurnNb; ++i) {
		list<SOrder> orders;
		STurnData placebo;
		myBot.GetTurnOrders(placebo, orders);
		allOrders.push_back(orders);
	}
	
	Document doc = OrdersToJson(allOrders);

	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);

	ofstream os("orders.json");
	os << buffer.GetString();

	return 0;
}