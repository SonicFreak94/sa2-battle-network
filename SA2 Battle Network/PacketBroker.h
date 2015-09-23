#pragma once

#include "typedefs.h"

#include "PlayerObject.h"			// for PlayerObject
#include "MemoryStruct.h"			// for MemStruct

#include <SFML/Network.hpp>			// for sf::Packet
#include "PacketExtensions.h"		// for PacketEx
#include "Networking.h"
#include <unordered_map>

class PacketBroker
{
public:
	explicit PacketBroker(uint timeout);
	void Initialize();

	//
	// Methods
	//

	void ReceiveLoop();

	/// <summary>
	/// Requests the specified message type to be added to the outbound packets.
	/// </summary>
	/// <param name="type">The message type.</param>
	/// <param name="isSafe">If set to <c>true</c>, the request will be added to the safe packet.</param>
	/// <returns>true if added to the outbound packets, false on failure (e.g already in outbound packets).</returns>
	bool Request(uint8 type, bool isSafe)
	{
		return RequestPacket(type, (isSafe) ? safe : fast, (!isSafe) ? safe : fast);
	}

	/// <summary>
	/// Finalizes this instance, sending queued packets.
	/// </summary>
	void Finalize();

	void SendSystem()	{ SendSystem(safe, fast); }
	void SendPlayer()	{ SendPlayer(safe, fast); }
	void SendMenu()		{ SendMenu(safe, fast); }

	bool ConnectionTimedOut() const;
	bool WaitForPlayers(nethax::Message::_Message id);
	void SendReady(nethax::Message::_Message id) const;
	void SetConnectTime();

	const uint ConnectionTimeout;
	ControllerData recvInput, sendInput;

private:
	//
	// Methods
	//

	std::unordered_map<nethax::Message::_Message, bool> things;

	// Requests that the packet type packetType is added to packetAddTo if it is not present in packetIsIn
	bool RequestPacket(const uint8 packetType, PacketEx& packetAddTo, PacketEx& packetIsIn);
	// Requests that the packet type packetType is added to packetAddTo.
	bool RequestPacket(const uint8 packetType, PacketEx& packetAddTo);

	// Called by RequestPacket
	// Adds the packet template for packetType to packet
	bool AddPacket(const uint8 packetType, PacketEx& packet);

	void Receive(sf::Packet& packet, const bool safe);

	// Read and send System variables
	void SendSystem(PacketEx& safe, PacketEx& fast);
	// Read and send Player varaibles
	void SendPlayer(PacketEx& safe, PacketEx& fast);
	// Read and send Menu variables
	void SendMenu(PacketEx& safe, PacketEx& fast);

	// Receive and write Input
	bool ReceiveInput(uint8 type, sf::Packet& packet);
	// Receive game/system variables
	bool ReceiveSystem(uint8 type, sf::Packet& packet);
	// Receive and queue write of Player variables
	bool ReceivePlayer(uint8 type, sf::Packet& packet);
	// Receive and write Menu variables
	bool ReceiveMenu(uint8 type, sf::Packet& packet);

	void PreReceive();
	void PostReceive();

	// Pretty much all of these are just so I can be lazy
	void writeRings();
	void writeSpecials() const;
	void writeTimeStop();

	//
	// Members
	//

	PacketEx safe, fast;
	PlayerObject recvPlayer, sendPlayer;

	// Used for comparison to determine what to send.
	MemStruct local;

	// Toggles and things
	bool firstMenuEntry;
	bool wroteP2Start;

	// Set in ReceivePlayer to true upon receival of a valid player message.
	bool writePlayer;

	// Set in SendSystem on level change to true if playing a relevant character.
	// (Sonic, Shadow, Amy, Metalsonic)
	bool sendSpinTimer;

	bool timedOut;
	uint sentKeepalive, receivedKeepalive;
};
