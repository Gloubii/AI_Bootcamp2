#pragma once

struct SNPCInfo;
struct SObjectInfo;
struct STileInfo;

struct STurnData
{
	int turnNb;

	SNPCInfo* npcInfoArray;
	int npcInfoArraySize;

	STileInfo* tileInfoArray;
	int tileInfoArraySize;

	SObjectInfo* objectInfoArray;
	int objectInfoArraySize;

	int turnDelay; //in ms
};
