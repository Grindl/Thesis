#include "User.hpp"
#include "Connection.hpp"
#include "Primitives/Color3b.hpp"

User::User()
{
	m_isInGame = false;
	m_mostRecentOrderNum = 0;
}

CS6Packet User::sendInput()
{
	if (m_userType == USER_REMOTE || !m_isInGame)
	{
		return CS6Packet();
	}
	else
	{
		Color3b tempColor = Color3b(m_unit.m_color);
		CS6Packet outPacket;
		outPacket.packetType = TYPE_Update;
		memcpy(outPacket.playerColorAndID, (char*)&tempColor, sizeof(tempColor));
		outPacket.data.updated.xPosition = m_unit.m_position.x;
		outPacket.data.updated.yPosition = m_unit.m_position.y;
		outPacket.data.updated.xVelocity = m_unit.m_velocity.x;
		outPacket.data.updated.yVelocity = m_unit.m_velocity.y;
		outPacket.data.updated.yawDegrees = m_unit.m_orientationDegrees;
		g_serverConnection->sendPacket(outPacket);
		return outPacket;
	}
}

void User::processUpdatePacket(CS6Packet newData)
{
	//TODO TODO this
	switch(newData.packetType)
	{
	case 0:
		{
			//BAD
			break;
		}
	case TYPE_Acknowledge:
		{
			//TODO remove the pending packet waiting for ack
			break;
		}
	case TYPE_Update:
		{
			if (m_mostRecentOrderNum <= newData.packetNumber)
			{
				m_unit.m_target.x = newData.data.updated.xPosition;
				m_unit.m_target.y = newData.data.updated.yPosition;
				m_unit.m_velocity.x = newData.data.updated.xVelocity;
				m_unit.m_velocity.y = newData.data.updated.yVelocity;
				m_unit.m_orientationDegrees = newData.data.updated.yawDegrees;
				m_mostRecentOrderNum = newData.packetNumber;
				Color3b tempColor;
				memcpy(&tempColor, newData.playerColorAndID, sizeof(tempColor));
				m_unit.m_color = Color4f(tempColor);
				if (m_unit.m_position == Vector2f(0,0))
				{
					m_unit.m_position.x = newData.data.updated.xPosition;
					m_unit.m_position.y = newData.data.updated.yPosition;
				}
			}
			break;
		}
	case TYPE_Victory:
		{
			CS6Packet ackForVictoryPacket;
			ackForVictoryPacket.packetType = TYPE_Acknowledge;
			Color3b tempColor;
			memcpy(&tempColor, newData.playerColorAndID, sizeof(tempColor));
			memcpy(ackForVictoryPacket.playerColorAndID, &tempColor, sizeof(tempColor));
			ackForVictoryPacket.data.acknowledged.packetType = TYPE_Victory;
			ackForVictoryPacket.data.acknowledged.packetNumber = newData.packetNumber;
			g_serverConnection->sendPacket(ackForVictoryPacket);
		}
	}
}

void User::update(float deltaSeconds)
{
	m_unit.update(deltaSeconds);
	sendInput();
}

void User::render()
{
	m_unit.render();
}