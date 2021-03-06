#include "stdafx.h"

#include <vector>
#include <map>

#include "LazyMemory.h"

#include "nop.h"

using namespace std;

map<intptr_t, vector<uint8_t>> nop::backup_data;

bool nop::apply(intptr_t address, size_t length, bool skip_backup)
{
	// If there's already backup data for this address, return false
	// unless the skip_backup override is enabled.
	if (!skip_backup && backup_data.find(address) != backup_data.end())
	{
		return false;
	}

	// Populate an array with "length" NOP instructions
	vector<uint8_t> nop(length, 0x90);

	// If skip_backup isn't enabled, backup the original data.
	if (!skip_backup)
	{
		vector<uint8_t> code(length);
		ReadMemory(address, code.data(), length);
		backup_data[address] = code;
	}

	// Write the NOP instructions to memory.
	WriteMemory(address, nop.data(), length);

	return true;
}

bool nop::restore(intptr_t address, bool skip_erase)
{
	auto it = backup_data.find(address);

	// If no backup data was found for this address, return false.
	if (it == backup_data.end())
	{
		return false;
	}

	// Otherwise, restore the backup data to memory.
	WriteMemory(address, it->second.data(), it->second.size());

	// If skip_erase enabled, do not erase the backup data.
	if (!skip_erase)
	{
		backup_data.erase(address);
	}

	return true;
}