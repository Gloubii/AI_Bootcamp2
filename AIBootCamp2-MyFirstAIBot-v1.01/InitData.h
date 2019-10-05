#pragma once

struct SNPCInfo;
struct SObjectInfo;
struct STileInfo;

struct SInitData
{
	int initDelay; //in ms

	int rowCount;
	int colCount;

	int turnDelay; //in ms
	int maxTurnNb;

	bool omniscient;

	SNPCInfo* npcInfoArray;
	int nbNPCs;

	STileInfo* tileInfoArray;
	int tileInfoArraySize;

	SObjectInfo* objectInfoArray;
	int objectInfoArraySize;
};
