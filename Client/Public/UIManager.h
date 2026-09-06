#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "EventManager.h"

NS_BEGIN(Engine)
class CGameInstance;
class CEventManager;
NS_END

NS_BEGIN(Client)
class CUIManager final : public CBase
{
private:
	CUIManager();
	virtual ~CUIManager() = default;

public:
	HRESULT Initialize();
	void Update(_float _fTimeDelta);
	void Handle_UIEvent(CEventManager::UI_EVENT& _tUIEvent);

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	class CUI_HP* m_pHP = { nullptr };
	class CUI_Rupee* m_pRupee = { nullptr };
	class CUI_Inventory* m_pInventory = { nullptr };
	class CUI_Dialogue* m_pDialogue = { nullptr };

	/* ON/OFF */
	_bool m_bInventoryOpen = { false };

private:
	HRESULT Initialize_HP(_float _fHP);
	HRESULT Initialize_Rupee(_uint _iRupee);
	HRESULT Initialize_Inventory();
	HRESULT Initialize_Dialogue();

	void KeyInput_UI();

public:
	static CUIManager* Create();
	virtual void Free() override;
};
NS_END