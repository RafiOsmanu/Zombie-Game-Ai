#pragma once
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"


class InventoryControl
{
public:
	InventoryControl(IExamInterface* pInterface, UINT& itemSlotIdx);
	~InventoryControl() = default;

	void GrabAndAdd(EntityInfo& entityInfo);
	bool Shoot();
	void UseMedkit();
	bool MedKitAvailable();
	bool FoodAvailable();
	void UseFood();
	int GetFreeSlot();
	bool GunAvailable();


private:
	IExamInterface* m_pInterface{nullptr};
	UINT& m_ItemSlotIdx;
	std::vector<ItemInfo> m_Inventory;
	bool m_DidSchoot{ false };
	bool m_UsedMedKit{ false };
};

