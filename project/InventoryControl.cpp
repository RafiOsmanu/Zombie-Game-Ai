#include "stdafx.h"
#include "InventoryControl.h"

InventoryControl::InventoryControl(IExamInterface* pInterface, UINT& itemSlotIdx)
	:m_pInterface{pInterface}
	,m_ItemSlotIdx{itemSlotIdx}
{
	m_Inventory.resize(pInterface->Inventory_GetCapacity());

	for (size_t i{}; i < m_Inventory.size(); ++i)
	{
		m_Inventory[i].Type = eItemType::RANDOM_DROP;
	}
}

void InventoryControl::GrabAndAdd(EntityInfo& entityInfo)
{
	float inventoryCapacity{ static_cast<float>(m_pInterface->Inventory_GetCapacity()) };
	ItemInfo itemGrabbed{};
	bool didItemGrabbedReplaceItem;
	if (m_pInterface->Item_Grab(entityInfo, itemGrabbed))
	{
		if (itemGrabbed.Type == eItemType::GARBAGE)
		{
			int freeSlot{ GetFreeSlot() };
			//destroy garbage
			if (freeSlot != -1)
			{
				m_pInterface->Inventory_AddItem(freeSlot, itemGrabbed);
				m_pInterface->Inventory_RemoveItem(freeSlot);
				m_Inventory.at(freeSlot).Type = eItemType::RANDOM_DROP;
			}
		}
		else //item grabbed is not garbage
		{
			int freeSlot{ GetFreeSlot() };
			//inventory full?
			if (freeSlot != -1)
			{
				//Inventory Not Full
				//add item to inventory
				m_Inventory[freeSlot] = itemGrabbed;
				m_pInterface->Inventory_AddItem(freeSlot, itemGrabbed);
			}
			else //Inventory Full -> Compare and possible Replace
			{
				didItemGrabbedReplaceItem = false;
				for (size_t i{}; i < m_Inventory.size(); i++)
				{
					if ((m_Inventory[i].Type == itemGrabbed.Type)) //type of item in inventory = item type that u grabbed
					{
						if (!didItemGrabbedReplaceItem)
						{
							switch (itemGrabbed.Type)
							{
							case eItemType::SHOTGUN:
								//if itemGrabbed has more Ammo than item in inventory replace item
								if (m_pInterface->Weapon_GetAmmo(m_Inventory[i]) < m_pInterface->Weapon_GetAmmo(itemGrabbed))
								{
									m_pInterface->Inventory_RemoveItem(i);
									m_pInterface->Inventory_AddItem(i, itemGrabbed);
									m_Inventory[i] = itemGrabbed;
								}
								else
								{
									//m_pInterface->Item_Destroy(entityInfo);
								}
								//ELSE DO NOTHING
								break;

							case eItemType::PISTOL:
								//if itemGrabbed has more Ammo than item in inventory replace item
								if (m_pInterface->Weapon_GetAmmo(m_Inventory[i]) < m_pInterface->Weapon_GetAmmo(itemGrabbed))
								{
									m_pInterface->Inventory_RemoveItem(i);
									m_pInterface->Inventory_AddItem(i, itemGrabbed);
									m_Inventory[i] = itemGrabbed;
								}
								else
								{
									//m_pInterface->Item_Destroy(entityInfo);
								}
								//ELSE DO NOTHING
								break;

							case eItemType::MEDKIT:
								//if itemGrabbed has more Health than item in inventory replace item
								m_pInterface->Inventory_RemoveItem(i);
								m_pInterface->Inventory_AddItem(i, itemGrabbed);
								m_Inventory[i] = itemGrabbed;
								break;

							case eItemType::FOOD:
								//if itemGrabbed has more Energy than item in inventory replace item
								if (m_pInterface->Food_GetEnergy(m_Inventory[i]) < m_pInterface->Weapon_GetAmmo(itemGrabbed))
								{
									m_pInterface->Inventory_RemoveItem(i);
									m_pInterface->Inventory_AddItem(i, itemGrabbed);
									m_Inventory[i] = itemGrabbed;
								}
								//ELSE DO NOTHING
								break;
								
							}
							didItemGrabbedReplaceItem = true;
						}

					}
					else //item grabbed not in inventory yet
					{
						if (!didItemGrabbedReplaceItem)
						{
							int randIdx{ Elite::randomInt(m_Inventory.size()) };

							m_pInterface->Inventory_RemoveItem(randIdx);
							m_pInterface->Inventory_AddItem(randIdx, itemGrabbed);
							m_Inventory[randIdx] = itemGrabbed;
							didItemGrabbedReplaceItem = true;
						}

					}
				}
			}
		}
	}
}

bool InventoryControl::Shoot()
{
	//look for Shotgun
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::SHOTGUN)
		{
			//shotgun found -> check ammo
			if (m_pInterface->Weapon_GetAmmo(m_Inventory[i]) <= 0.f)
			{
				//No ammo -> remove shotgun
				m_pInterface->Inventory_RemoveItem(i);
				m_Inventory.at(i).Type = eItemType::RANDOM_DROP;
				std::cout << "Pistol Removed" << "\n";
				return false;
			}

			//enough ammo -> shoot!
			std::cout << "Booom!" << "\n";
			m_pInterface->Inventory_UseItem(i);
			return true;
		}
		else
		{
			std::cout << "no Shotgun found :(" << "\n";
		}
	}

	//Look for pistol
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::PISTOL)
		{
			//Pistol found -> check ammo
			if (m_pInterface->Weapon_GetAmmo(m_Inventory[i]) <= 0.f)
			{
				//No ammo -> Remove pistol
				m_pInterface->Inventory_RemoveItem(i);
				m_Inventory.at(i).Type = eItemType::RANDOM_DROP;
				std::cout << "Pistol Removed" << "\n";
				return false;
			}

			//Pistol found -> Shoot!
			std::cout << "Phew!" << "\n";
			m_pInterface->Inventory_UseItem(i);
			return true;
			
		}
		else
		{
			std::cout << "no Pistol found :(" << "\n";
		}
	}
	return false;
	
}

void InventoryControl::UseMedkit()
{
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::MEDKIT)
		{
			//check if medkit has health
			if (m_pInterface->Medkit_GetHealth(m_Inventory[i]) <= 0.f)
			{
				m_pInterface->Inventory_RemoveItem(i);
				m_Inventory.at(i).Type = eItemType::RANDOM_DROP;
				break;
			}

			//if health is lower than then
			if (m_pInterface->Agent_GetInfo().Health < 10.f)
			{
				//if it has health
				//use the medKit
				std::cout << "Healed Up!" << "\n";
				m_pInterface->Inventory_UseItem(i);
			}
			break;
		}
	}
}

void InventoryControl::UseFood()
{
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::FOOD)
		{
			//check if medkit has health
			if (m_pInterface->Food_GetEnergy(m_Inventory[i]) <= 0.f)
			{
				m_pInterface->Inventory_RemoveItem(i);
				m_Inventory.at(i).Type = eItemType::RANDOM_DROP;
				break;
			}

			//if energy is lower than then
			if (m_pInterface->Agent_GetInfo().Energy < 10.f)
			{
				//if it has energy
				//Eat The Food
				std::cout << "Energy Up\n";
				m_pInterface->Inventory_UseItem(i);
			}
			break;
		}
	}
}

int InventoryControl::GetFreeSlot()
{
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::RANDOM_DROP)
		{
			return i;
		}
	}
	
	return -1;
}

bool InventoryControl::GunAvailable()
{
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::SHOTGUN || m_Inventory[i].Type == eItemType::PISTOL)
		{
			return true;
		}
	}
	return false;
}

bool InventoryControl::MedKitAvailable()
{
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::MEDKIT)
		{
			
			return true;
		}
	}
	return false;
}

bool InventoryControl::FoodAvailable()
{
	for (size_t i{}; i < m_Inventory.size(); i++)
	{
		if (m_Inventory[i].Type == eItemType::FOOD)
		{
			return true;
		}
	}
	return false;
}
