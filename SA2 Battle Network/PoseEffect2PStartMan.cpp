#include "stdafx.h"
#include "Globals.h"

void __cdecl PoseEffect2PStartMan_Delete(ObjectMaster* obj)
{
	using namespace nethax;
	using namespace Globals;

	if (isConnected())
	{
		Broker->SendReady(MessageID::S_RoundStart);
		Broker->WaitForPlayers(MessageID::S_RoundStart);
	}
}

ObjectMaster* __stdcall LoadObjectBypass(ObjectFuncPtr mainSub, int list, char* name)
{
	ObjectMaster* obj = LoadObject(mainSub, list, name);

	if (obj != nullptr)
		obj->DeleteSub = PoseEffect2PStartMan_Delete;

	return obj;
}

void __declspec(naked) LoadObjectBypass_asm()
{
	__asm
	{
		push [esp + 4]
		push esi
		push edi
		call LoadObjectBypass
		retn
	}
}

void InitPoseEffect2PStartMan()
{
	WriteCall((void*)0x00477AAA, LoadObjectBypass_asm);
}
