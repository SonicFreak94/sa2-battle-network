#include "stdafx.h"
#include "AdventurePacketOverloads.h"
#include "AddressList.h"

sf::Packet& operator <<(sf::Packet& packet, const Rotation& data)
{
	return packet << data.x << data.y << data.z;
}
sf::Packet& operator >>(sf::Packet& packet, Rotation& data)
{
	return packet >> data.x >> data.y >> data.z;
}

sf::Packet& operator <<(sf::Packet& packet, const NJS_VECTOR& data)
{
	return packet << data.x << data.y << data.z;
}
sf::Packet& operator >>(sf::Packet& packet, NJS_VECTOR& data)
{
	return packet >> data.x >> data.y >> data.z;
}

sf::Packet& operator <<(sf::Packet& packet, const AnalogThing& data)
{
	return packet << (int)data.direction << data.magnitude;
}
sf::Packet& operator >>(sf::Packet& packet, AnalogThing& data)
{
	return packet >> *(int*)&data.direction >> data.magnitude;
}
