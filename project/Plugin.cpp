#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Behavior.h"

using namespace std;
using namespace Elite;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "MinionExam";
	info.Student_FirstName = "Rafi";
	info.Student_LastName = "Osmanu";
	info.Student_Class = "2DAE15";

	

	m_pSteeringOutput = new SteeringPlugin_Output();
	m_pSteeringState = new SteeringStateManagement{ m_pInterface, m_pSteeringOutput };
	m_pEntityInFov = new std::vector<EntityInfo>{};
	m_pInventoryControl = new InventoryControl(m_pInterface, m_ItemSlotIdx);
	m_pExploreControl = new ExploreControl{ m_pInterface, 21 };
	m_pHouseInFov = new std::vector<HouseInfo>{};
	m_LastLocation = {0.f, 0.f};
	m_LastLocationConfirmed = false;
	m_SearchPoint = 0;
	m_CheckedHouses = {};
	m_OriginalOrientation = m_pInterface->Agent_GetInfo().AngularVelocity;
	m_IsSpinActivated = false;
	m_IsSpinCheckActivated = false;
	m_LocationOutsidePurgeZone = {};
	m_LocationSet = false;
	
	
	//Create BlackBoard
	Blackboard* pBlackBoard = new Blackboard();
	pBlackBoard->AddData("Interface", m_pInterface);
	pBlackBoard->AddData("SteeringState", m_pSteeringState);
	pBlackBoard->AddData("EntityInFov", m_pEntityInFov);
	pBlackBoard->AddData("InventoryControl", m_pInventoryControl);
	pBlackBoard->AddData("ExploreControl", m_pExploreControl);
	pBlackBoard->AddData("HouseInFov", m_pHouseInFov);
	pBlackBoard->AddData("LastLocation", m_LastLocation);
	pBlackBoard->AddData("LastLocationConfirmed", m_LastLocationConfirmed);
	pBlackBoard->AddData("SearchPoint", m_SearchPoint);
	pBlackBoard->AddData("CheckedHouses", m_CheckedHouses);

	//Agent getting attacked
	pBlackBoard->AddData("OGOrientation", m_OriginalOrientation);
	pBlackBoard->AddData("IsSpinActivated", m_IsSpinActivated);
	pBlackBoard->AddData("IsSpinCheckActivated", m_IsSpinCheckActivated);

	//purge Zone Escape
	pBlackBoard->AddData("LocationOutsidePurgeZone", m_LocationOutsidePurgeZone);
	pBlackBoard->AddData("LocationSet", m_LocationSet);
	

	//Set up BehaviorTree
	m_pBehaviorTree = new BehaviorTree(pBlackBoard, new BehaviorSelector(
		{
			//Heal Up
		 new BehaviorSequence({
		 
		  //agent low health & u have a medKit
		  new BehaviorConditional(BT_Conditions::IsHealthLowAndHealAvailable),
		  //heal (Use MedKit)
		  new BehaviorAction(BT_Actions::UseMedKit)

		}),
		   //Replenish energy
		 new BehaviorSequence({

			//agent low Energy & u have a food
			new BehaviorConditional(BT_Conditions::IsEnergyLowAndFoodAvailable),
			//heal (Use Food)
			new BehaviorAction(BT_Actions::UseFood)

		  }),
			//Escape Purge Zones
		 new BehaviorSequence({

			//agent In Purge Zone
			new BehaviorConditional(BT_Conditions::IsInPurgeZone),
			//Run from purge Zone
			new BehaviorAction(BT_Actions::RunAwayFromPurgeZone)

		  }),

		//Shoot or run from enemy 
		 new BehaviorSelector({

			new BehaviorSequence({
				 //gun available
				 new BehaviorConditional(BT_Conditions::IsGunAvailable),

				 //enemy nearBy
				 new BehaviorConditional(BT_Conditions::IsEnemyInFov),

				 //flee or run and shoot
				 new BehaviorAction(BT_Actions::RunAndShoot)

			}),

			 new BehaviorSequence({

				 //enemy nearBy
				  new BehaviorConditional(BT_Conditions::IsEnemyInFov),

				  //RunAway
				   new BehaviorAction(BT_Actions::RunAway)
			 })
		 }),

		//if agent attacked turn and shoot
		new BehaviorSequence({

			//gun available
			new BehaviorConditional(BT_Conditions::IsGunAvailable),

			//if agent attacked
			new BehaviorConditional(BT_Conditions::IsAgentAttacked),

			//SpinAround
			new BehaviorAction(BT_Actions::SpinAround)

		  }),

		new BehaviorSequence({

			//if item nearby
			new BehaviorConditional(BT_Conditions::IsItemNearBy),
			//seek
			new BehaviorAction(BT_Actions::changeToSeekEntity),
			//pick Up Item
			new BehaviorAction(BT_Actions::PickUpItem)

		  }),
		 
		 //house Check Logic
		 new BehaviorSelector({

		 new BehaviorSequence({

			//if point is reached
			new BehaviorConditional(BT_Conditions::IsLastSearchPointReached),
			//leave House
			new BehaviorAction(BT_Actions::LeaveHouse)
		  }),

		 new BehaviorSequence({

			 //if true
			 new BehaviorConditional(BT_Conditions::IsHouseInFov),
			 //search House
			 new BehaviorAction(BT_Actions::SearchHouse)


		}),
		}),
		 //fallBack to Explore
		 new BehaviorAction(BT_Actions::Explore)
		}));

}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
	SAFE_DELETE(m_pBehaviorTree);
	SAFE_DELETE(m_pSteeringOutput);
	SAFE_DELETE(m_pSteeringState);
	SAFE_DELETE(m_pEntityInFov);
	SAFE_DELETE(m_pHouseInFov);
	SAFE_DELETE(m_pInventoryControl);
	SAFE_DELETE(m_pExploreControl);
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = true; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = false;
	params.SpawnDebugShotgun = false;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 20;
	params.SpawnZombieOnRightClick = true;
	
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	m_pExploreControl->DrawDebugCells();
	
	//std::cout << "hey brother" << "\n";
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	m_pExploreControl->Update();
	m_pBehaviorTree->Update(dt);

	auto steering = SteeringPlugin_Output();
	
	
	
	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();


	//Use the navmesh to calculate the next navmesh point
	//auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	//OR, Use the mouse target
	auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_Target); //Uncomment this to use mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)

	//get the entitys that are in the Fov
	m_pEntityInFov[0] = GetEntitiesInFOV(); 

	//get the houses in Fov
	m_pHouseInFov[0] = GetHousesInFOV();






	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo;
	//		m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		//std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y << "---Radius: "<< zoneInfo.Radius << std::endl;
	//	}
	//}

	//INVENTORY USAGE DEMO
	//********************

	if (m_GrabItem)
	{
		ItemInfo item;
		//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
		//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
		//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
		//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
		if (m_pInterface->Item_Grab({}, item))
		{
			//Once grabbed, you can add it to a specific inventory slot
			//Slot must be empty
			m_pInterface->Inventory_AddItem(m_InventorySlot, item);
		}
	}

	if (m_UseItem)
	{
		//Use an item (make sure there is an item at the given inventory slot)
		m_pInterface->Inventory_UseItem(m_InventorySlot);
	}

	if (m_RemoveItem)
	{
		//Remove an item from a inventory slot
		m_pInterface->Inventory_RemoveItem(m_InventorySlot);
	}

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	steering.AutoOrient = true; //Setting AutoOrient to TRue overrides the AngularVelocity

	steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

	//SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

//@End (Demo Purposes)
	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	return m_pSteeringOutput[0];
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}


