#pragma once

#include <Windows.h>

struct SConfigData
{
	const char* const* cmdlineStrArray;
	int cmdlineStrArraySize;
	int configDelay; //in ms
	LPSTR logpath;
};
