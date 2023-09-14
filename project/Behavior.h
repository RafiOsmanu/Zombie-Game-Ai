#pragma once
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include <iostream>
#include "EBehaviorTree.h"
#include "SteeringStateManagement.h"
#include "Plugin.h"
#include "InventoryControl.h"


//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// ACTIONS
// --------------------------------------------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState changeToWander(Elite::Blackboard* pBlackboard)
	{
		SteeringStateManagement* pSteeringState{};

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pSteeringState->Wander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState RunAndShoot(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntityInFov{nullptr};
		SteeringStateManagement* pSteeringState{nullptr};
		IExamInterface* pInterface{nullptr};
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<int> enemyIdx{};
		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::ENEMY)
			{
				enemyIdx.push_back(i);
			}
		}

		AgentInfo agentInfo = pInterface->Agent_GetInfo();
		EntityInfo closestEnemy{};
		closestEnemy.Location.x = 999999.f;

		//get closest Enemy
		for (auto index : enemyIdx)
		{
			if (agentInfo.Position.Distance(pEntityInFov->at(index).Location) < agentInfo.Position.Distance(closestEnemy.Location))
			{
				closestEnemy = (pEntityInFov->at(index));
			}
		}
		
		float agentRange{ agentInfo.FOV_Range / 2.f };
		float faceRange{ agentInfo.FOV_Range};
		Elite::Vector2 agentPos{ agentInfo.Position };

		//if enemy is too close sprint 
		if ((closestEnemy.Location - agentPos).MagnitudeSquared() < agentRange * agentRange)
		{
			pSteeringState->Sprint(true);
		}
		else
		{
			pSteeringState->Sprint(false);
		}

		if ((closestEnemy.Location - agentPos).MagnitudeSquared() < faceRange * faceRange)
		{
			//face the enemy
			if (pSteeringState->Face(closestEnemy.Location))
			{
				//shoot the enemy 
				pInventory->Shoot();
			}
		}

		pSteeringState->Flee(closestEnemy.Location);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState changeToSeekEntity(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntityInFov{nullptr};
		SteeringStateManagement* pSteeringState{nullptr};
		IExamInterface* pInterface{nullptr};

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<int> enemyIdx{};
		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::ITEM)
			{
				enemyIdx.push_back(i);
			}
		}

		AgentInfo agentInfo = pInterface->Agent_GetInfo();
		EntityInfo closestItem{};
		closestItem.Location.x = 999999.f;

		//get closest enemy
		for (auto index : enemyIdx)
		{
			if (agentInfo.Position.Distance(pEntityInFov->at(index).Location) < agentInfo.Position.Distance(closestItem.Location))
			{
				closestItem = (pEntityInFov->at(index));
			}
		}

		std::cout << "seeking" << "\n";
		pSteeringState->Seek(closestItem.Location);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState PickUpItem(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntitysInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("EntityInFov", pEntitysInFov) || pEntitysInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (pEntitysInFov->empty())
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pInterface->Agent_GetInfo();
		EntityInfo closestItem{};
		closestItem.Location.x = 999999.f;

		for (const auto& entity : *pEntitysInFov)
		{
			//check if entity is item
			if (entity.Type == eEntityType::ITEM)
			{
				if (agentInfo.Position.Distance(entity.Location) < agentInfo.Position.Distance(closestItem.Location))
				{
					//closest item in fov
					closestItem = entity;
				}
			}
		}

		Elite::Vector2 target{ closestItem.Location };
		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position};
		float grabRange{ pInterface->Agent_GetInfo().GrabRange};
		
		//check if item is in grab Range
		if ((target - agentPos).MagnitudeSquared() < grabRange * grabRange)
		{
			pInventory->GrabAndAdd(closestItem);
			std::cout << "Item Picked Up" << " Capacity: " << pInterface->Inventory_GetCapacity() << "\n";
			return Elite::BehaviorState::Success;
		}
		
		return Elite::BehaviorState::Success;
	}

	//unused
	Elite::BehaviorState GoIntoPrimaryHouse(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo>* pHousesInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		SteeringStateManagement* pSteeringState{ nullptr };
		Elite::Vector2 lastLocation{ 0.f, 0.f };
		bool lastLocationConfirmed{};
		std::vector<HouseInfo> checkedHouses;
		
		if (!pBlackboard->GetData("HouseInFov", pHousesInFov) || pHousesInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("LastLocation", lastLocation))
		{
			return Elite::BehaviorState::Failure;
		}

		if(!pBlackboard->GetData("LastLocationConfirmed", lastLocationConfirmed))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("CheckedHouses", checkedHouses))
		{
			return Elite::BehaviorState::Failure;
		}

		if (pHousesInFov->empty())
		{
			return Elite::BehaviorState::Failure;
		}
		pSteeringState->Sprint(false);


		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		//saves last location and makes sure it only happens once
		if (!lastLocationConfirmed)
		{
			pBlackboard->ChangeData("LastLocation", agentInfo.Position);
			pBlackboard->ChangeData("LastLocationConfirmed", true);
		}

		//std::cout << "center: " << pHousesInFov->begin()->Center << " Size: " << pHousesInFov->begin()->Size << "\n";
		std::cout << "Going into house" << "\n";


		//targets the first house you see
		Elite::Vector2 target{ pHousesInFov->begin()->Center };
		pSteeringState->Face(target);
		pSteeringState->FaceAndSeek(target);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SearchHouse(Elite::Blackboard* pBlackboard)
	{
		//make sure purgezone location is reset
		pBlackboard->ChangeData("LocationSet", false);

		std::vector<HouseInfo>* pHousesInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		SteeringStateManagement* pSteeringState{ nullptr };
		int searchPointReached{};
		Elite::Vector2 lastLocation{};
		std::vector<HouseInfo> checkedHouses;
		bool lastLocationConfirmed{};


		if (!pBlackboard->GetData("HouseInFov", pHousesInFov) || pHousesInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SearchPoint", searchPointReached))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("LastLocation", lastLocation))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("CheckedHouses", checkedHouses))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("LastLocationConfirmed", lastLocationConfirmed))
		{
			return Elite::BehaviorState::Failure;
		}


		if (pHousesInFov->empty())
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };
		float width{ pHousesInFov->begin()->Size.x };
		float height{ pHousesInFov->begin()->Size.y - 6.f };
		Elite::Vector2 centerBottom{ pHousesInFov->begin()->Center.x , pHousesInFov->begin()->Center.y - height/ 2.f  };
		Elite::Vector2 searchPoint{ centerBottom };

		if (searchPointReached == 0)
		{
			searchPoint = pHousesInFov->begin()->Center;
		}
		
		//Change search location
		if (searchPointReached == 1)
		{
			searchPoint = centerBottom;
		}
		else if (searchPointReached == 2)
		{
			searchPoint.y += height;
		}
		else if (searchPointReached == 3)
		{
			searchPoint.y += height / 2.f;
			searchPoint.x += (width / 2.f) - 3.f;
		}
		else if(searchPointReached == 4)
		{
			searchPoint.y += height / 2.f;
			searchPoint.x -= (width / 2.f) - 3.f;
		}

		
		float agentRange{ pInterface->Agent_GetInfo().GrabRange };

		//check if searchpoint is in Range
		if (((searchPoint - agentPos).MagnitudeSquared() < agentRange * agentRange))
		{
			//POINT 1
			//point 0 has been reached change point to point 1st search point
			if (searchPointReached == 0)
			{
				std::cout << "point 1 reached!!!" << "\n";
				std::cout << "point 1 reached!!!" << "\n";
				std::cout << "point 1 reached!!!" << "\n";
				pBlackboard->ChangeData("SearchPoint", 1);
			}

			//POINT 2
			//first point has been reached change to point 2
			else if (searchPointReached == 1)
			{
				std::cout << "point 2 reached" << "\n";
				pBlackboard->ChangeData("SearchPoint", 2);
			}

			//POINT 3
			//second point has been reached change to point 3
			else if (searchPointReached == 2)
			{
				std::cout << "point 3 reached" << "\n";
				pBlackboard->ChangeData("SearchPoint", 3);
			}

			//POINT 4
			//3th point has been reached change to point 4th
			else if (searchPointReached == 3)
			{
				std::cout << "point 4 reached" << "\n";
				pBlackboard->ChangeData("SearchPoint", 4);
			}

			//POINT 4
			//4th point has been reached change to point 5th
			else if (searchPointReached == 4)
			{
				std::cout << "point 5 reached" << "\n";
				pBlackboard->ChangeData("SearchPoint", 5);
				//add the house as checked
				checkedHouses.push_back(*pHousesInFov->begin());
				pBlackboard->ChangeData("CheckedHouses", checkedHouses);
			}

		}

		//if agent is low on health or food search house faster
		if (agentInfo.Health <= 1.5f || agentInfo.Energy <= 1.5f)
		{
			pSteeringState->Sprint(true);
		}
		else
		{
			pSteeringState->Sprint(false);
		}

		//if 5th point has been reached go direction outside house
		if(searchPointReached == 5)
		{
			pSteeringState->Seek(lastLocation);
			pInterface->Draw_Point(lastLocation, 10.f, { 1.0f, .0f, .0f });
			return Elite::BehaviorState::Success;
		}

		//debug 
		std::cout << "Searching House" << "\n";
		pInterface->Draw_Point(searchPoint, 10.f, { 1.0f, .0f, .0f });

		//targets Search point
		if (searchPointReached == 0)
		{
			//saves last location and makes sure it only happens once
			if (!lastLocationConfirmed)
			{
				pBlackboard->ChangeData("LastLocation", agentInfo.Position);
				pBlackboard->ChangeData("LastLocationConfirmed", true);
			}
			pSteeringState->Face(searchPoint);
			pSteeringState->FaceAndSeek(searchPoint);
		}
		else
		{
			pSteeringState->Seek(searchPoint);
		}
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState LeaveHouse(Elite::Blackboard* pBlackboard)
	{
		//make sure purgezone location is reset
		pBlackboard->ChangeData("LocationSet", false);

		std::vector<HouseInfo>* pHousesInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		SteeringStateManagement* pSteeringState{ nullptr };
		int searchPointReached{};
		Elite::Vector2 lastLocation{};
		std::vector<HouseInfo> checkedHouses;


		if (!pBlackboard->GetData("HouseInFov", pHousesInFov) || pHousesInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SearchPoint", searchPointReached))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("LastLocation", lastLocation))
		{
			return Elite::BehaviorState::Failure;
		}

		float agentRange{ pInterface->Agent_GetInfo().GrabRange };
		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };

		float grabRange{ pInterface->Agent_GetInfo().GrabRange };

		//Reset last location
		pBlackboard->ChangeData("LastLocationConfirmed", false);

		//check if item is in grab Range
		if ((lastLocation - agentPos).MagnitudeSquared() < grabRange * grabRange)
		{
			// reset house Search Point
			pBlackboard->ChangeData("SearchPoint", 0);
		}


		//if agent is low on health or food search house faster
		if (pInterface->Agent_GetInfo().Health <= 1.5f || pInterface->Agent_GetInfo().Energy <= 1.5f)
		{
			pSteeringState->Sprint(true);
		}
		else
		{
			pSteeringState->Sprint(false);
		}
		
		pSteeringState->Seek(lastLocation);
		pInterface->Draw_Point(lastLocation, 10.f, { 0.0f, 1.0f, .0f });
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState UseMedKit(Elite::Blackboard* pBlackboard)
	{
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pInventory->UseMedkit();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState UseFood(Elite::Blackboard* pBlackboard)
	{
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pInventory->UseFood();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState Explore(Elite::Blackboard* pBlackboard)
	{
		//make sure purgezone location is reset
		pBlackboard->ChangeData("LocationSet", false);

		ExploreControl* pExplore{ nullptr };
		SteeringStateManagement* pSteeringState{ nullptr };
		IExamInterface* pInterface{ nullptr };

		if (!pBlackboard->GetData("ExploreControl", pExplore) || pExplore == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pSteeringState->Sprint(false);

		Elite::Vector2 target{ pExplore->GetNextExplorationPoint(pBlackboard) };
		std::cout << target << "\n";
		pSteeringState->Seek(target);
		std::cout << "Going To Next Point\n";
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState SpinAround(Elite::Blackboard* pBlackboard)
	{
		SteeringStateManagement* pSteeringState{ nullptr };

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pSteeringState->SpinAround();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState RunAway(Elite::Blackboard* pBlackboard)
	{
		SteeringStateManagement* pSteeringState{ nullptr };
		std::vector<EntityInfo>* pEntityInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<int> enemyIdx{};
		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::ENEMY)
			{
				enemyIdx.push_back(i);
			}
		}
		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		EntityInfo closestEnemy{};
		closestEnemy.Location.x = 999999.f;

		//Get closest Enemy
		for (auto index : enemyIdx)
		{
			if (agentInfo.Position.Distance(pEntityInFov->at(index).Location) < agentInfo.Position.Distance(closestEnemy.Location))
			{
				closestEnemy = (pEntityInFov->at(index));
			}
		}

		float agentRange{ agentInfo.FOV_Range};
		Elite::Vector2 agentPos{ agentInfo.Position };

		//if enemy is too close sprint 
		if ((closestEnemy.Location - agentPos).MagnitudeSquared() < agentRange * agentRange)
		{
			pSteeringState->Sprint(true);
		}
		else
		{
			pSteeringState->Sprint(false);
		}

		
		pSteeringState->Flee(closestEnemy.Location);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState RunAwayFromPurgeZone(Elite::Blackboard* pBlackboard)
	{
		SteeringStateManagement* pSteeringState{ nullptr };
		std::vector<EntityInfo>* pEntityInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		Elite::Vector2 locationOutsidePurgeZone{};
		bool locationSet{};

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("LocationOutsidePurgeZone", locationOutsidePurgeZone))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("LocationSet", locationSet))
		{
			return Elite::BehaviorState::Failure;
		}

		std::vector<int> purgeZoneIdx{};
		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::PURGEZONE)
			{
				purgeZoneIdx.push_back(i);
			}
		}
		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		EntityInfo closestPurgeZone{};
		closestPurgeZone.Location.x = 999999.f;

		//Get closest Enemy
		for (auto index : purgeZoneIdx)
		{
			if (agentInfo.Position.Distance(pEntityInFov->at(index).Location) < agentInfo.Position.Distance(closestPurgeZone.Location))
			{
				closestPurgeZone = (pEntityInFov->at(index));
			}
		}
		float zoneRadius{ 35.f };

		//save purgezone leave location
		Elite::Vector2 purgeZoneEscapePos{ closestPurgeZone.Location + Elite::Vector2{-cosf(agentInfo.Orientation), -sinf(agentInfo.Orientation)} * zoneRadius };
		pInterface->Draw_Point( locationOutsidePurgeZone, 10.f ,{1.0f, 0.f, 1.0f} );
		if (!locationSet)
		{
			pBlackboard->ChangeData("LocationSet", true);
			pBlackboard->ChangeData("LocationOutsidePurgeZone", purgeZoneEscapePos);
		}

		//is pos 
		Elite::Vector2 center{ closestPurgeZone.Location };
		if ((agentInfo.Position.x - center.x) * (agentInfo.Position.x - center.x) + (agentInfo.Position.y - center.y) * (agentInfo.Position.y - center.y) < zoneRadius * zoneRadius)
		{
			pSteeringState->Sprint(true);
			pSteeringState->Seek(locationOutsidePurgeZone);
			return Elite::BehaviorState::Success;
		}
		else
		{
			pSteeringState->Sprint(true);
			pSteeringState->Flee(closestPurgeZone.Location);
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Success;
		
	}
}

//--------------------------------------------------------------------------------------------------
// CONDITIONS
// --------------------------------------------------------------------------------------------------

namespace BT_Conditions
{
	bool IsEnemyInFov(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntityInFov{nullptr};

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return false;
		}

		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::ENEMY)
			{
				return true;
			}
		}
		return false;
	}

	bool IsItemNearBy(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntityInFov{ nullptr };

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return false;
		}

		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::ITEM)
			{
				return true;
			}
		}
		return false;
	}

	bool IsHouseInFov(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		std::vector<HouseInfo>* pHouseInFov{ nullptr };
		std::vector<HouseInfo> checkedHouses;

		if (!pBlackboard->GetData("HouseInFov", pHouseInFov) || pHouseInFov == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("CheckedHouses", checkedHouses))
		{
			return false;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}


		for (const auto& checkedHouse : checkedHouses)
		{
			for (size_t i{}; i < pHouseInFov->size(); ++i)
			{
				if (pHouseInFov->at(i).Center == checkedHouse.Center)
				{
					pHouseInFov->erase(pHouseInFov->begin() + i);
				}

			}
		}
		pBlackboard->ChangeData("HouseInFov", pHouseInFov);

		//check if there are houses in the FOV
		if (pHouseInFov->size() <= 0)
			return false;

		float width{ pHouseInFov->begin()->Size.x };
		float height{ pHouseInFov->begin()->Size.y };
		Elite::Vector2 leftBottom{ pHouseInFov->begin()->Center.x - width / 2.f,  pHouseInFov->begin()->Center.y - height / 2.f };

		//Draw House circumference
		Elite::Vector2 rectangle[4];
		rectangle[0] = (leftBottom);
		rectangle[1] = { leftBottom.x, leftBottom.y + height };
		rectangle[2] = { leftBottom.x + width, leftBottom.y + height };
		rectangle[3] = { leftBottom.x + width, leftBottom.y };
		pInterface->Draw_Polygon(rectangle, 4, Elite::Vector3{ 0.f ,0.f, 1.0f }, -1.0f);



		return (pHouseInFov->size() > 0);
		 
	}

	//unused
	bool IsAgentInHouse(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo>* pHouseInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		std::vector<HouseInfo> checkedHouses;


		if (!pBlackboard->GetData("HouseInFov", pHouseInFov) || pHouseInFov == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("CheckedHouses", checkedHouses))
		{
			return false;
		}

		if (pHouseInFov->empty())
		{
			return false;
		}

		//check if agent is in house and if house is not already checked
		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };
		

		for (const auto& checkedHouse : checkedHouses)
		{
			for (size_t i{}; i < pHouseInFov->size(); ++i)
			{
				if (pHouseInFov->at(i).Center == checkedHouse.Center)
				{
					pHouseInFov->erase(pHouseInFov->begin() + i);
				}

			}
		}

		//change houses in Fov
		pBlackboard->ChangeData("HouseInFov", pHouseInFov);

		if (pHouseInFov->size() <= 0)
			return false;

		float width{ pHouseInFov->begin()->Size.x};
		float height{ pHouseInFov->begin()->Size.y };
		Elite::Vector2 leftBottom{ pHouseInFov->begin()->Center.x - width / 2.f,  pHouseInFov->begin()->Center.y - height / 2.f };

		

		if (agentPos.x > leftBottom.x && agentPos.x < agentPos.x + width)
		{
			if (agentPos.y > leftBottom.y && agentPos.y < agentPos.y + height)
			{
				return true;
			}
		}
		return false;

	}


	bool IsHouseInFovChecked(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo>* pHouseInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		std::vector<HouseInfo> checkedHouses;


		if (!pBlackboard->GetData("HouseInFov", pHouseInFov) || pHouseInFov == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("CheckedHouses", checkedHouses))
		{
			return false;
		}

		if (pHouseInFov->empty())
		{
			return false;
		}

		
		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };
		float width{ pHouseInFov->begin()->Size.x };
		float height{ pHouseInFov->begin()->Size.y };
		Elite::Vector2 leftBottom{ pHouseInFov->begin()->Center.x - width / 2.f, pHouseInFov->begin()->Center.y - height / 2.f };

		//check if house is checked and agent is in house
		for (const auto& checkedHouse : checkedHouses)
		{
			for (const auto& houseInFov : *pHouseInFov)
			{
				if (houseInFov.Center == checkedHouse.Center)
				{
					if (agentPos.x >= leftBottom.x && agentPos.x <= agentPos.x + width)
					{
						if (agentPos.y >= leftBottom.y && agentPos.y <= agentPos.y + height)
						{
							return true;
						}
					}
				}
			}
		}
		return false;

	}
	//unused
	bool LeftTheHouse(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo>* pHouseInFov{ nullptr };
		IExamInterface* pInterface{ nullptr };
		std::vector<HouseInfo> checkedHouses;


		if (!pBlackboard->GetData("HouseInFov", pHouseInFov) || pHouseInFov == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("CheckedHouses", checkedHouses))
		{
			return false;
		}

		if (pHouseInFov->empty())
		{
			return false;
		}

		Elite::Vector2 agentPos{ pInterface->Agent_GetInfo().Position };
		for (const auto& checkedHouse : checkedHouses)
		{
			float width{ checkedHouse.Size.x };
			float height{ checkedHouse.Size.y };
			Elite::Vector2 leftBottom{ checkedHouse.Center.x - width / 2.f, checkedHouse.Center.y - height / 2.f };

			if (agentPos.x > leftBottom.x && agentPos.x < agentPos.x + width)
			{
				if (agentPos.y > leftBottom.y && agentPos.y < agentPos.y + height)
				{
					return false;
				}
			}
		}
		return true;
	}

	bool IsLastSearchPointReached(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo>* pHouseInFov{ nullptr };
		int searchPointReached{};
		
		if (!pBlackboard->GetData("HouseInFov", pHouseInFov) || pHouseInFov == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("SearchPoint", searchPointReached))
		{
			return false;
		}

		if (searchPointReached == 5)
		{
			return true;
		}

		return false;
	}

	bool IsHealthLowAndHealAvailable(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return false;
		}

		AgentInfo agent{pInterface->Agent_GetInfo()};
		if (agent.Health < 10.f && pInventory->MedKitAvailable())
		{
			return true;
		}

		return false;
	}

	bool IsEnergyLowAndFoodAvailable(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return false;
		}

		AgentInfo agent{ pInterface->Agent_GetInfo() };

		if (agent.Energy < 6.f && pInventory->FoodAvailable())
		{
			return true;
		}

		return false;
	}

	bool IsAgentAttacked(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		SteeringStateManagement* pSteeringState{ nullptr };
		float originalAngularVelocity{};
		bool isSpinActivated{ false };
		bool isCheckActivated{false};

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("SteeringState", pSteeringState) || pSteeringState == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("OGOrientation", originalAngularVelocity))
		{
			return false;
		}

		if (!pBlackboard->GetData("IsSpinActivated", isSpinActivated))
		{
			return false;
		}

		if (!pBlackboard->GetData("IsSpinCheckActivated", isCheckActivated))
		{
			return false;
		}

		AgentInfo agent{pInterface->Agent_GetInfo()};

		if (agent.WasBitten)
		{
			//save current orientation
			pBlackboard->ChangeData("OGOrientation", agent.Orientation + 0.3f);

			//set spin active & check active
			pBlackboard->ChangeData("IsSpinActivated", true);
			pBlackboard->ChangeData("IsSpinCheckActivated", true);

			return true;
		}
	
		//if agent orientation passes 180 degrees make sure it doenst go negative
		if (abs(originalAngularVelocity - agent.Orientation) > static_cast<float>(M_PI))
		{

			if (originalAngularVelocity > agent.Orientation)
			{
				originalAngularVelocity -= 2.f * static_cast<float>(M_PI);
			}
			else
			{
				originalAngularVelocity += 2.f * static_cast<float>(M_PI);
			}
		}

		//if check activated is set back to false
		if (!isCheckActivated)
		{
			//orientation +- lines up with saved Orientaion
			if (originalAngularVelocity - agent.Orientation <= 0.08f)
			{
				pBlackboard->ChangeData("IsSpinActivated", false);
			}
		}

		//if spin is not false keep on spinning
		if (isSpinActivated)
		{
			pBlackboard->ChangeData("IsSpinCheckActivated", false);
			return true;
		}

		return false;
	}

	bool IsGunAvailable(Elite::Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{ nullptr };
		InventoryControl* pInventory{ nullptr };

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("InventoryControl", pInventory) || pInventory == nullptr)
		{
			return false;
		}

		AgentInfo agent{ pInterface->Agent_GetInfo() };
		if (pInventory->GunAvailable())
		{
			return true;
		}

		return false;
	}

	bool IsInPurgeZone(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntityInFov{ nullptr };

		if (!pBlackboard->GetData("EntityInFov", pEntityInFov) || pEntityInFov == nullptr)
		{
			return false;
		}

		for (size_t i{ 0 }; i < pEntityInFov->size(); ++i)
		{
			if (pEntityInFov->at(i).Type == eEntityType::PURGEZONE)
			{
				return true;
			}
		}
		return false;
	}
}

