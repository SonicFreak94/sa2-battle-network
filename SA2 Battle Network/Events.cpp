#include "stdafx.h"

#include "Events.h"

#include "PoseEffect2PStartMan.h"
#include "ItemBoxItems.h"
#include "EmeraldSync.h"
#include "CharacterSync.h"
#include "AddHP.h"
#include "AddRings.h"
#include "Damage.h"
#include "OnStageChange.h"
#include "OnResult.h"
#include "Random.h"

void nethax::events::Initialize()
{
	InitCharacterSync();
	InitEmeraldSync();
	InitItemBoxItems();
	InitPoseEffect2PStartMan();
	InitAddHP();
	InitAddRings();
	InitDamage();
	InitOnStageChange();
	InitOnResult();
	random::InitRandom();
}

void nethax::events::Deinitialize()
{
	DeinitCharacterSync();
	DeinitEmeraldSync();
	DeinitItemBoxItems();
	DeinitPoseEffect2PStartMan();
	DeinitAddHP();
	DeinitAddRings();
	DeinitDamage();
	DeinitOnStageChange();
	DeinitOnResult();
	random::DeinitRandom();
}