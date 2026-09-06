#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class IUIManager
{
public:
	enum class UIPREFAB { RUPEE, HP, INVENTORY, EQUIP, END };
	enum class UISTATE { HIDDEN, POPUP, ALWASYSON, END };

public:
	virtual ~IUIManager() = default;

public:
	virtual HRESULT Initialize_HP(_float _fHP) = 0;
	virtual HRESULT Initialize_Rupee(_uint _iRupee) = 0;
	virtual HRESULT Initialize_Inventory() = 0;

	virtual void Update(_float _fTimeDelta) = 0;

	virtual void Set_UIPrefabState(UIPREFAB _ePrefab, UISTATE _eState) = 0;
};
NS_END