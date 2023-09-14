#include "stdafx.h"
#include "SteeringStateManagement.h"
using namespace Elite;

SteeringStateManagement::SteeringStateManagement(IExamInterface* pExamInterFace, SteeringPlugin_Output* pSteering)
	:m_pInterface{pExamInterFace}
	, m_pSteering{pSteering}
{
}

void SteeringStateManagement::Seek(const Elite::Vector2 target)
{
	m_pSteering->AutoOrient = true;
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();
	auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(target);

	m_pSteering->LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	m_pSteering->LinearVelocity.Normalize(); //Normalize Desired Velocity
	m_pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
}

void SteeringStateManagement::FaceAndSeek(const Elite::Vector2 target)
{
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();
	auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(target);

	m_pSteering->AutoOrient = false;
	AgentInfo agent{ m_pInterface->Agent_GetInfo() };

	const Elite::Vector2 vecToTarget = target - agent.Position;
	float angleToTarget = atan2(vecToTarget.y, vecToTarget.x);


	//check if angle to target is greater than 180 degrees
	if (abs(angleToTarget - agent.Orientation) > static_cast<float>(M_PI))
	{

		if (angleToTarget > agent.Orientation)
		{
			angleToTarget -= 2.f * static_cast<float>(M_PI);
		}
		else
		{
			angleToTarget += 2.f * static_cast<float>(M_PI);
		}
	}

	//if angular diffrence between orientation and target is > 0 turn clockwise in
	if (angleToTarget - agent.Orientation > 0)
	{
		m_pSteering->AngularVelocity = agent.MaxAngularSpeed * 4.f;
	}
	else //if angular diffrence between orientation and target is < 0 turn clockwise 
	{
		m_pSteering->AngularVelocity = -agent.MaxAngularSpeed * 4.f;
	}

	if (abs(angleToTarget - agent.Orientation) <= 0.2f)
	{
		m_pSteering->AngularVelocity = 0.f;
	}

	m_pSteering->LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	m_pSteering->LinearVelocity.Normalize(); //Normalize Desired Velocity
	m_pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
}


void SteeringStateManagement::Flee(const Elite::Vector2 target)
{
	AgentInfo& agentInfo{ m_pInterface->Agent_GetInfo() };
	Elite::Vector2 fromTarget = agentInfo.Position - target;
	float distance = fromTarget.Magnitude();

	m_pSteering->LinearVelocity = agentInfo.Position - target;
	m_pSteering->LinearVelocity.Normalize();
	m_pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed;
}


void SteeringStateManagement::Wander()
{
	m_pSteering->AutoOrient = true;

	AgentInfo agent{ m_pInterface->Agent_GetInfo() };
	//wander
	Vector2 cirleCenter{ agent.Position };
	const float circleOffset{ 4.f };
	const float angleChange{ 45.f };
	//apply offset
	cirleCenter += agent.LinearVelocity.GetNormalized() * 6.f;
	float wanderAngle{};
	wanderAngle += randomFloat(-Elite::ToRadians(angleChange), Elite::ToRadians(angleChange));
	//get random pos on circle
	cirleCenter.x += circleOffset * cosf(wanderAngle);
	cirleCenter.y += circleOffset * sinf(wanderAngle);
	Vector2 target{};
	target = cirleCenter;

	//use seek to go to random points on circle = wander
	Seek(target);

}

void SteeringStateManagement::Sprint(bool sprintActive)
{
	m_pSteering->RunMode = sprintActive;
}

bool SteeringStateManagement::Face(const Elite::Vector2 target)
{
	m_pSteering->AutoOrient = false; 
	AgentInfo agent{ m_pInterface->Agent_GetInfo() };

	const Elite::Vector2 vecToTarget = target - agent.Position;
	float angleToTarget = atan2(vecToTarget.y, vecToTarget.x);

	//check if angle to target is greater than 180 degrees
	if (abs(angleToTarget - agent.Orientation) > static_cast<float>(M_PI))
	{

		if (angleToTarget > agent.Orientation)
		{
			angleToTarget -= 2.f * static_cast<float>(M_PI);
		}
		else
		{
			angleToTarget += 2.f * static_cast<float>(M_PI);
		}
	}

	//if angular diffrence between orientation and target is > 0 turn clockwise in
	if (angleToTarget - agent.Orientation > 0)
	{
		m_pSteering->AngularVelocity = agent.MaxAngularSpeed * 4.f;
	}
	else //if angular diffrence between orientation and target is < 0 turn clockwise 
	{
		m_pSteering->AngularVelocity = -agent.MaxAngularSpeed * 4.f;
	}

	//if angular difrence is < 0.2f stop rotation 
	if (abs(angleToTarget - agent.Orientation) <= 0.08f)
	{
		//m_pSteering->AngularVelocity = 0.f;
		return true;
	}

	return false;

}

void SteeringStateManagement::SpinAround()
{

	m_pSteering->AutoOrient = false;
	AgentInfo agent{ m_pInterface->Agent_GetInfo() };

	m_pSteering->AngularVelocity = -agent.MaxAngularSpeed;
	//m_pSteering->LinearVelocity = { 0,0 };

}
