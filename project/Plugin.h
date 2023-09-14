#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EBehaviorTree.h"
#include "SteeringStateManagement.h"
#include "InventoryControl.h"
#include "ExploreControl.h"



class IBaseInterface;
class IExamInterface;


class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;
	enum class SteeringBehavior
	{
		wander, seek, flee
	};


private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose
	Elite::BehaviorTree* m_pBehaviorTree{nullptr};

	UINT m_InventorySlot = 0;


	SteeringBehavior m_SteeringBehavior{ SteeringBehavior::flee };

	SteeringPlugin_Output* m_pSteeringOutput{ nullptr };

	SteeringStateManagement* m_pSteeringState{ nullptr };

	std::vector<EntityInfo>* m_pEntityInFov{ nullptr };
	std::vector<HouseInfo>* m_pHouseInFov{ nullptr };

	InventoryControl* m_pInventoryControl;
	ExploreControl* m_pExploreControl;
	

	UINT m_ItemSlotIdx{0};
	
	//save last location
	Elite::Vector2 m_LastLocation;
	bool m_LastLocationConfirmed;

	//searchHouse
	int m_SearchPoint;

	//houses that have been checked
	std::vector<HouseInfo> m_CheckedHouses;

	//original angular Velocity
	float m_OriginalOrientation;

	//bool has spun around
	bool m_IsSpinActivated{false};
	bool m_IsSpinCheckActivated{ false };

	//location outside of purge zone 
	Elite::Vector2 m_LocationOutsidePurgeZone;
	bool m_LocationSet;

	


};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}
