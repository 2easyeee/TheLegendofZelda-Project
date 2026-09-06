#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class IInventory
{
public:
	virtual ~IInventory() = default;

public:
	virtual void Initialize_Inventory() = 0;
	virtual HRESULT Add_Item(_wstring _wstrItemID, _uint _iCount = 1) = 0;
	virtual HRESULT Remove_Item(_wstring _wstrItemID, _uint _iCount = 1) = 0;
	virtual HRESULT Use_Item(_uint _iSlotIndex) = 0;
	virtual void Move_Cursor(DIR _eDir) = 0;
	virtual void Selected_Slot(EQUIPBTN _eEquip) = 0;
};
NS_END