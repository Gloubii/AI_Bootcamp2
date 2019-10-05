#pragma once

#define INTEROP_API_EXPORTS 1

#ifdef INTEROP_API_EXPORTS
#define INTEROP_API __declspec(dllexport)
#else
#define INTEROP_API __declspec(dllimport)
#endif // INTEROP_API_EXPORTS

#include "Globals.h"

class AIBot;

struct SConfigData;
struct SInitData;
struct STurnData;
struct SOrder;

extern "C"
{
	INTEROP_API AIBot* Internal_CreateNativeAIBot();
	INTEROP_API void Internal_ConfigureAIBot(AIBot* _pAIBot, const SConfigData& _configData);
	INTEROP_API void Internal_InitAIBot(AIBot* _pAIBot, const SInitData& _initData);
	INTEROP_API SOrder* Internal_GetAIBotTurnOrders(AIBot* _pAIBot, const STurnData& _turnData, int& nbOrders);
}