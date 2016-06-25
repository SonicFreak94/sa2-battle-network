#include "stdafx.h"
#include <SA2ModLoader.h>
#include <Trampoline.h>
#include "Networking.h"
#include "Globals.h"
#include "ItemBoxItems.h"
#include "FunctionPointers.h"

using namespace nethax;

static Trampoline* Speedup_Trampoline;
static Trampoline* NBarrier_Trampoline;
static Trampoline* TBarrier_Trampoline;
static Trampoline* Invincibility_Trampoline;
static Trampoline* DisplayItemBoxItem_Trampoline;

inline void __cdecl ItemBox_Original(Trampoline* trampoline, ObjectMaster* object, int pnum)
{
	((decltype(ItemBoxItem::Code))trampoline->Target())(object, pnum);
}

inline void __cdecl UserCallOriginal(void* original, int pnum)
{
	__asm
	{
		mov eax, [pnum]
		call [original]
	}
}

void __stdcall events::NBarrier_original(int pnum)
{
	UserCallOriginal(NBarrier_Trampoline->Target(), pnum);
}
void __stdcall events::Speedup_original(int pnum)
{
	UserCallOriginal(Speedup_Trampoline->Target(), pnum);
}
void __stdcall events::TBarrier_original(int pnum)
{
	UserCallOriginal(TBarrier_Trampoline->Target(), pnum);
}
void __stdcall events::Invincibility_original(ObjectMaster* object, int pnum)
{
	ItemBox_Original(Invincibility_Trampoline, object, pnum);
}

void events::DisplayItemBoxItem_original(int pnum, int tnum)
{
	_FunctionPointer(void, original, (int, int), DisplayItemBoxItem_Trampoline->Target());
	original(pnum, tnum);
}

static void __cdecl NBarrier_hax(int n)
{
	if (Globals::isConnected())
	{
		if (n != Globals::Broker->GetPlayerNumber())
			return;

		Globals::Broker->Append(MessageID::S_NBarrier, Protocol::TCP, nullptr);
	}

	events::NBarrier_original(n);
}
static void __declspec(naked) NBarrier_asm()
{
	__asm
	{
		push eax
		call NBarrier_hax
		pop eax
		retn
	}
}

static void __cdecl Speedup_hax(int n)
{
	if (Globals::isConnected())
	{
		if (n != Globals::Broker->GetPlayerNumber())
			return;

		Globals::Broker->Append(MessageID::S_Speedup, Protocol::TCP, nullptr);
	}

	events::Speedup_original(n);
}
static void __declspec(naked) Speedup_asm()
{
	__asm
	{
		push eax
		call Speedup_hax
		pop eax
		retn
	}
}

static void __cdecl TBarrier_hax(int n)
{
	if (Globals::isConnected())
	{
		if (n != Globals::Broker->GetPlayerNumber())
			return;

		Globals::Broker->Append(MessageID::S_TBarrier, Protocol::TCP, nullptr);
	}

	events::TBarrier_original(n);
}
static void __declspec(naked) TBarrier_asm()
{
	__asm
	{
		push eax
		call TBarrier_hax
		pop eax
		retn
	}
}

static void __cdecl Invincibility_hax(ObjectMaster* obj, int n)
{
	if (Globals::isConnected())
	{
		if (n != Globals::Broker->GetPlayerNumber())
			return;

		Globals::Broker->Append(MessageID::S_Invincibility, Protocol::TCP, nullptr);
	}

	events::Invincibility_original(obj, n);
}

static void __cdecl DisplayItemBoxItem_hax(int pnum, int tnum)
{
	if (Globals::isConnected())
	{
		if (pnum != Globals::Broker->GetPlayerNumber())
			return;

		sf::Packet packet;
		packet << tnum;
		Globals::Broker->Append(MessageID::S_ItemBoxItem, Protocol::TCP, &packet, true);
	}

	events::DisplayItemBoxItem_original(pnum, tnum);
}

static bool MessageHandler(MessageID type, PlayerNumber pnum, sf::Packet& packet)
{
	if (!roundStarted())
		return false;

	switch (type)
	{
		case MessageID::S_NBarrier:
			events::NBarrier_original(pnum);
			break;

		case MessageID::S_TBarrier:
			events::TBarrier_original(pnum);
			break;

		case MessageID::S_Speedup:
			events::Speedup_original(pnum);
			break;

		case MessageID::S_Invincibility:
			events::Invincibility_original(nullptr, pnum);
			break;

		case MessageID::S_ItemBoxItem:
		{
			int tnum;
			packet >> tnum;
			events::DisplayItemBoxItem_original(pnum, tnum);
			break;
		}

		default:
			return false;
	}

	return true;
}

void events::InitItemBoxItems()
{
	NBarrier_Trampoline           = new Trampoline(0x0046E2E0, 0x0046E2FE, NBarrier_asm);
	Speedup_Trampoline            = new Trampoline(0x0046E120, 0x0046E126, Speedup_asm);
	TBarrier_Trampoline           = new Trampoline(0x0046E180, 0x0046E19E, TBarrier_asm);
	Invincibility_Trampoline      = new Trampoline(0x006C98F0, 0x006C98F5, Invincibility_hax);
	DisplayItemBoxItem_Trampoline = new Trampoline(0x006DF440, 0x006DF445, DisplayItemBoxItem_hax);

	Globals::Broker->RegisterMessageHandler(MessageID::S_NBarrier, MessageHandler);
	Globals::Broker->RegisterMessageHandler(MessageID::S_TBarrier, MessageHandler);
	Globals::Broker->RegisterMessageHandler(MessageID::S_Speedup, MessageHandler);
	Globals::Broker->RegisterMessageHandler(MessageID::S_Invincibility, MessageHandler);
	Globals::Broker->RegisterMessageHandler(MessageID::S_ItemBoxItem, MessageHandler);
}

void events::DeinitItemBoxItems()
{
	delete NBarrier_Trampoline;
	delete Speedup_Trampoline;
	delete TBarrier_Trampoline;
	delete Invincibility_Trampoline;
	delete DisplayItemBoxItem_Trampoline;
}
