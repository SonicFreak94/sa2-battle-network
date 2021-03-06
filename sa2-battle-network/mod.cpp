#include "stdafx.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <Windows.h>		// for GetCommandLineW(), GetCurrentProcess()
#include <ShellAPI.h>		// for CommandLineToArgvW
#include <direct.h>			// for _getcwd
#include <SA2ModLoader.h>

#include "typedefs.h"
#include "globals.h"		// PacketHandler, Program, PacketBroker
#include "PacketHandler.h"	// for sws::Address
#include "OnGameState.h"
#include "Hash.h"
#include <locale>
#include <codecvt>

void fake_main(const char* path, int argc, wchar_t** argv);

extern "C"
{
	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };

	__declspec(dllexport) void __cdecl Init(const char* path)
	{
		int argc       = 0;
		wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		fake_main(path, argc, argv);
		LocalFree(argv);
	}
}

std::string build_mod_path(const char* modpath, const char* path)
{
	std::stringstream result;
	char workingdir[FILENAME_MAX];

	result << _getcwd(workingdir, FILENAME_MAX) << "\\" << modpath << "\\" << path;

	return result.str();
}

void parse_config(const std::string& path, Program::Settings& settings, sws::Address& address)
{
	char buffer[255] {};

	GetPrivateProfileStringA("Config", "Nickname", "", buffer, 255, path.c_str());

	if (strlen(buffer))
	{
		settings.nickname = buffer;
	}

	settings.no_specials = GetPrivateProfileIntA("Config", "Specials", 1, path.c_str()) == 0;
	settings.cheats      = GetPrivateProfileIntA("Config", "Cheats", 0, path.c_str()) != 0;

	GetPrivateProfileStringA("Server", "Name", "", buffer, 255, path.c_str());
	if (strlen(buffer))
		settings.server_name = buffer;

	address.port = static_cast<sws::port_t>(GetPrivateProfileIntA("Server", "Port", 21790, path.c_str()));

	GetPrivateProfileStringA("Server", "Password", "", buffer, 255, path.c_str());
	if (strlen(buffer))
	{
		std::string password_a(buffer);

		if (GetPrivateProfileIntA("Server", "PasswordHashed", 0, path.c_str()) != 1)
		{
			Hash hash;
			settings.password = hash.compute_hash(reinterpret_cast<const void*>(password_a.c_str()), password_a.length(), CALG_SHA_256);
			WritePrivateProfileStringA("Server", "Password", Hash::to_string(settings.password).c_str(), path.c_str());
			WritePrivateProfileStringA("Server", "PasswordHashed", "1", path.c_str());
		}
		else
		{
			settings.password = Hash::from_string(password_a);
		}
	}
}

std::string wstring_to_string(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wstr);
}

void fake_main(const char* path, int argc, wchar_t** argv)
{
	bool valid_args = false;
	bool is_server  = false;
	uint timeout    = 15000;

	// This serves multiple purposes.
	// e.g: If connecting, it stores a password for the remote server if applicable.
	Program::Settings settings = {};

	sws::Address address;
	address.address = "localhost";

	parse_config(build_mod_path(path, "config.ini"), settings, address);

	// TODO: fix cases where valid_args would invalidate configuration read from disk.
	for (int i = 1; i < argc; i++)
	{
		if ((!wcscmp(argv[i], L"--host") || !wcscmp(argv[i], L"-h")) && i + 1 < argc)
		{
			address.port = std::stoi(argv[++i]);
			valid_args   = true;
			is_server    = true;
		}
		else if ((!wcscmp(argv[i], L"--connect") || !wcscmp(argv[i], L"-c")) && i + 1 < argc)
		{
			std::wstring ip_w = argv[++i];
			std::string ip = wstring_to_string(ip_w);

			if (ip.empty())
			{
				continue;
			}

			// TODO: parse IPv6
			const auto colon = ip.find_first_of(':');
			const sws::port_t port = colon == std::string::npos ? 21790 : static_cast<sws::port_t>(std::stoi(ip.substr(colon + 1)));

			address.address = ip.substr(0, colon);
			address.port    = port;
			valid_args      = true;
			is_server       = false;
		}
		else if ((!wcscmp(argv[i], L"--timeout") || !wcscmp(argv[i], L"-t")) && i + 1 < argc)
		{
			timeout    = std::max(2500, std::stoi(argv[++i]));
			valid_args = true;
		}
		else if (!wcscmp(argv[i], L"--no-specials"))
		{
			settings.no_specials = true;
		}
		else if (!wcscmp(argv[i], L"--cheats"))
		{
			settings.cheats = true;
			valid_args      = true;
		}
		else if (!wcscmp(argv[i], L"--password") && ++i < argc)
		{
			std::wstring password_w(argv[i]);
			std::string password_a = wstring_to_string(password_w);

			Hash hash;
			settings.password = hash.compute_hash(password_a.c_str(), password_a.length(), CALG_SHA_256);
		}
		else if (!wcscmp(argv[i], L"--local") || !wcscmp(argv[i], L"-l"))
		{
			settings.local = true;
			valid_args     = true;
		}
		else if (!wcscmp(argv[i], L"--netstat"))
		{
			settings.netstat = true;
		}
	}

	Program::apply_settings(settings);

	if (!valid_args)
	{
		if (argc < 2)
		{
			PrintDebug("[SA2:BN] Insufficient parameters.");
		}
		else
		{
			PrintDebug("[SA2:BN] Invalid parameters.");
		}

		return;
	}

	using namespace nethax;

	auto addresses = sws::Address::get_addresses(address.address.c_str(), address.port, sws::AddressFamily::inet);
	address = addresses[0];

	globals::networking = new PacketHandler();
	globals::program    = new Program(settings, is_server, address);
	globals::broker     = new PacketBroker(timeout);

	events::InitOnGameState();
}
