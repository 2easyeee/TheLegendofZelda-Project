#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "IInventory.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)
class CUI_Inventory final : public CBase, public IInventory
{
public:
	typedef struct tagInventoryLayout
	{
		_float2 vStartPosition;
		_float2 vSize;
		_float2 vGap;
		_uint iMaxRow;
		_uint iMaxSlot;
	}Inventory_Layout;

private:
	CUI_Inventory();
	virtual ~CUI_Inventory() = default;

public:
	virtual void Initialize_Inventory() override;
	virtual HRESULT Add_Item(_wstring _wstrItemID, _uint _iCount = 1) override;
	virtual HRESULT Remove_Item(_wstring _wstrItemID, _uint _iCount = 1) override;
	virtual HRESULT Use_Item(_uint _iSlotIndex) override;
	virtual void Move_Cursor(DIR _eDir) override;
	virtual void Selected_Slot(EQUIPBTN _eEquip) override;
	void Update_Cursor_Scale(_float _fDeltaTime);

public:
	void Hide_Inventory();
	void PopUp_Inventory();
	void AlwaysOn_Inventory();

	void Hide_EquipSlot();
	void PopUp_EquipSlot();
	void AlwaysOn_EquipSlot();

	void Update(_float _fDeltaTime);
	_bool IsUpdatable() const;

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	class CGameObject* m_pBackground = { nullptr };
	class CUI_Image* m_pCursor = { nullptr };
	vector<class CUI_Slot*> m_vecSlots;
	vector<class CUI_Item*> m_vecItems;

	Inventory_Layout m_tInventorySlot = {};
	Inventory_Layout m_tInventoryItem = {};
	_uint m_iSelectedSlot = { 0 };

	class CUI_Slot* m_pSlot_X = { nullptr };
	class CUI_Slot* m_pSlot_Y = { nullptr };

	class CUI_Item* m_pEquipItem_X = { nullptr };
	class CUI_Item* m_pEquipItem_Y = { nullptr };
	_wstring m_iEquipItemName_X = {};
	_wstring m_iEquipItemName_Y = {};

	class CUI_Image* m_pEquipSlot_X = { nullptr };
	class CUI_Image* m_pEquipSlot_Y = { nullptr };
	_uint m_iEquipIndex = { 0 };

	_float2 m_vEquipPositionX = {};
	_float2 m_vEquipPositionY = {};

	_float m_fCursorIdleTime = 0.f;
	_float m_fCursorBreathSpeed = 2.5f;
	_float m_fCursorBreathScale = 0.08f;
	_float m_fCursorBaseSclae = 1.f;
	_bool m_bCursorMoveEnbale = { false };

	_float m_fCursorBaseSizeX = 128.f;
	_float m_fCursorBaseSizeY = 128.f;

private:
	HRESULT Create_Inventory_Background();
	HRESULT Create_Inventory_Slots(_float2 _fPosition, _float2 _fSize,_float _fScale);
	HRESULT Create_Inventory_Item(_wstring _textureTag, _wstring _ObjectID, _float _fScale);
	HRESULT Create_Cursor(_float _fScale);
	HRESULT Create_Equip_Slot(_float _fScale);
	HRESULT Clone_Equip_Item(_float2 _vPosition, class CUI_Item* _SourceItem, class CUI_Item** _ppOut);

	void Update_Cursor_Position();
	void Update_Equip_Item();

	_float2 Compute_Position(_uint _iIndex);
	
public:
	static CUI_Inventory* Create();
	virtual void Free() override;
};
NS_END