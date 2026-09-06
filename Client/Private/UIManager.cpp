#include "UIManager.h"
#include "GameInstance.h"
#include "UI_HP.h"
#include "UI_Rupee.h"
#include "UI_Inventory.h"
#include "UI_Dialogue.h"

CUIManager::CUIManager()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CUIManager::Initialize()
{
	Initialize_HP(3);
	Initialize_Rupee(0);
	Initialize_Inventory();
	Initialize_Dialogue();

	return S_OK;
}

HRESULT CUIManager::Initialize_HP(_float _fHP)
{
	/* Create HP */
	m_pHP = CUI_HP::Create();
	if (nullptr == m_pHP)
		return E_FAIL;
	m_pHP->Initialize_HP(_fHP);
	m_pHP->Hide_HP();

	return S_OK;
}

HRESULT CUIManager::Initialize_Rupee(_uint _iRupee)
{
	/* Create Rupee */
	m_pRupee = CUI_Rupee::Create();
	if (nullptr == m_pRupee)
		return E_FAIL;
	m_pRupee->Initialize_Rupee(_iRupee);
	m_pRupee->Hide_Rupee();

	return S_OK;
}

HRESULT CUIManager::Initialize_Inventory()
{
	/* Create Inventory */
	m_pInventory = CUI_Inventory::Create();
	if (nullptr == m_pInventory)
		return E_FAIL;
	m_pInventory->Initialize_Inventory();
	m_pInventory->Hide_Inventory();
	m_pInventory->Hide_EquipSlot();

	return S_OK;
}

HRESULT CUIManager::Initialize_Dialogue()
{
	/* Create Dialogue */
	m_pDialogue = CUI_Dialogue::Create();
	if (nullptr == m_pDialogue)
		return E_FAIL;
	m_pDialogue->Initialize_Dialogue();
	m_pDialogue->Hide_AllMessageBox();
}

void CUIManager::KeyInput_UI()
{
#ifdef DEBUG
	/* TEST : UI_HP */
	if (m_pGameInstance->Get_DIKeyDown(DIK_5)) // DIK_NUMPAD1
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_DAMAGE;
		tUIEvent.fValue = 0.5f;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_6)) // DIK_NUMPAD2
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_ADD;
		tUIEvent.fValue = 1.f;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_7)) // DIK_NUMPAD3
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_ADD_MAXHP;
		tUIEvent.fValue = 1.f;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}
	/* TEST : UI_Rupee */
	if (m_pGameInstance->Get_DIKeyDown(DIK_9)) // DIK_NUMPAD5
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::RUPEE_ADD;
		tUIEvent.iValue = 123;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_0)) // DIK_NUMPAD6
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::RUPEE_SPEND;
		tUIEvent.iValue = 250;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}
#endif // DEBUG
	if (m_pGameInstance->Get_DIKeyDown(DIK_5)) // DIK_NUMPAD1
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_DAMAGE;
		tUIEvent.fValue = 0.5f;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_6)) // DIK_NUMPAD2
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::HP_ADD;
		tUIEvent.fValue = 1.f;

		m_pGameInstance->Push_UIEVENT(tUIEvent);
	}

	/* Test : UI_Inventory_Cursor */
	if (m_pGameInstance->Get_DIKeyDown(DIK_TAB))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		if (m_bInventoryOpen)
		{
			tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_OFF;

			m_bInventoryOpen = false;
		}
		else
		{
			tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_ON;
			
			m_bInventoryOpen = true;
		}
		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_XY.wav", SOUND::EFFECT);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_LEFT))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIDir = DIR::LEFT;
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_MOVE_CURSOR;

		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_Move.wav", SOUND::EFFECT);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_RIGHT))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIDir = DIR::RIGHT;
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_MOVE_CURSOR;

		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_Move.wav", SOUND::EFFECT);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_UP))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIDir = DIR::UP;
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_MOVE_CURSOR;

		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_Move.wav", SOUND::EFFECT);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_DOWN))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eUIDir = DIR::DOWN;
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_MOVE_CURSOR;

		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_Move.wav", SOUND::EFFECT);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_X))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eEquipBtn = EQUIPBTN::X;
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_SELECTED_SLOT;

		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_XY.wav", SOUND::EFFECT);
	}
	if (m_pGameInstance->Get_DIKeyDown(DIK_Y))
	{
		CEventManager::UI_EVENT tUIEvent = {};
		tUIEvent.eEquipBtn = EQUIPBTN::Y;
		tUIEvent.eUIEvent = CEventManager::UIEVENTTYPE::INVENTORY_SELECTED_SLOT;

		m_pGameInstance->Push_UIEVENT(tUIEvent);

		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Inventory_XY.wav", SOUND::EFFECT);
	}

	/* Dialogue */
	if (m_pGameInstance->Get_DIKeyDown(DIK_J))
	{
		if (m_pGameInstance->IsNodeFinished())
		{
			/* Get Node */
			const CDialogueManager::DIALOGUE_NODE* currentNode = m_pGameInstance->GetCurrentNode();
			if (currentNode && currentNode->vecChoices.empty())
				m_pGameInstance->NextNode();
		}
		else
		{
			/* Typing FX Btn */
			m_pGameInstance->NextNode();
		}
	}

	/* Select Btn */
	if (m_pGameInstance->Get_DIKeyDown(DIK_1))
	{
		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Chat_Next.wav", SOUND::EFFECT);

		/* Choice */
		m_pGameInstance->SelectChoice(0);
	}

	if (m_pGameInstance->Get_DIKeyDown(DIK_2))
	{
		/* SFX */
		m_pGameInstance->Play_Sound(L"UI_Chat_Next.wav", SOUND::EFFECT);

		/* Choice */
		m_pGameInstance->SelectChoice(1);
	}
}

void CUIManager::Update(_float _fTimeDelta)
{
	/* KeyInput */
	KeyInput_UI();

	/* EventManager */
	CEventManager::UI_EVENT tUIEvent = {};
	while (m_pGameInstance->Pop_UIEvent(tUIEvent))
	{
		Handle_UIEvent(tUIEvent);
	}

	/* HP */
	if (m_pHP)
		m_pHP->Update(_fTimeDelta);
	
	/* Rupee */
	if (m_pRupee)
		m_pRupee->Update(_fTimeDelta);

	/* Inventory */
	if (m_pInventory)
		m_pInventory->Update(_fTimeDelta);

	/* Dialogue */
	if (m_pDialogue)
		m_pDialogue->Update(_fTimeDelta);
}

void CUIManager::Handle_UIEvent(CEventManager::UI_EVENT& _tUIEvent)
{
	switch (_tUIEvent.eUIEvent)
	{
	case CEventManager::UIEVENTTYPE::RUPEE_ADD:
	{
		m_pRupee->Add_Rupee(_tUIEvent.iValue);
		m_pRupee->PopUp_Rupee();
	}
		break;
	case CEventManager::UIEVENTTYPE::RUPEE_SPEND:
	{
		m_pRupee->Spend_Rupee(_tUIEvent.iValue);
		m_pRupee->PopUp_Rupee();
	}
		break;
	case CEventManager::UIEVENTTYPE::HP_DAMAGE:
	{
		m_pHP->Damage_HP(_tUIEvent.fValue);
		m_pHP->PopUp_HP();
	}
		break;
	case CEventManager::UIEVENTTYPE::HP_ADD:
	{
		m_pHP->Add_HP(_tUIEvent.fValue);
		m_pHP->PopUp_HP();
	}
	break;
	case CEventManager::UIEVENTTYPE::HP_ADD_MAXHP:
	{
		m_pHP->Add_MAXHP(_tUIEvent.fValue);
		m_pHP->PopUp_HP();
	}
	break;
	case CEventManager::UIEVENTTYPE::INVENTORY_ON:
	{
		m_pInventory->PopUp_Inventory();
	}
	break;
	case CEventManager::UIEVENTTYPE::INVENTORY_OFF:
	{
		m_pInventory->Hide_Inventory();
	}
	break;
	case CEventManager::UIEVENTTYPE::INVENTORY_MOVE_CURSOR:
	{
		m_pInventory->Move_Cursor(_tUIEvent.eUIDir);
	}
	break;
	case CEventManager::UIEVENTTYPE::INVENTORY_SELECTED_SLOT:
	{
		m_pInventory->Selected_Slot(_tUIEvent.eEquipBtn);
	}
	break;
	case CEventManager::UIEVENTTYPE::DIALOGUE_ALL_OFF:
	{
		m_pDialogue->Hide_AllMessageBox();
	}
	break;
	case CEventManager::UIEVENTTYPE::DIALOGUE_CHOICE_ON:
	{
		m_pDialogue->PopUp_ChoiceMessageBox();
	}
	break;
	case CEventManager::UIEVENTTYPE::DIALOGUE_CHOICE_OFF:
	{
		m_pDialogue->Hide_ChoiceMessageBox();
	}
	break;
	case CEventManager::UIEVENTTYPE::DIALOGUE_MESSAGEBOX_ON:
	{
		m_pDialogue->PopUp_MessageBox();
	}
	break;
	case CEventManager::UIEVENTTYPE::DIALOGUE_GETMESSAGEBOX_ON:
	{
		m_pDialogue->PopUp_GetMessageBox();
	}
	break;
	case CEventManager::UIEVENTTYPE::DIALOGUE_NEXT:
	{
		m_pDialogue->PopUp_MessageNext();
	}
	break;
	case CEventManager::UIEVENTTYPE::ALL_OFF:
	{
		if (m_pHP)
			m_pHP->Hide_HP();

		if (m_pRupee)
			m_pRupee->Hide_Rupee();

		if (m_pInventory)
		{
			m_pInventory->Hide_Inventory();
			m_pInventory->Hide_EquipSlot();
		}

		m_bInventoryOpen = false;
	}
	break;
	case CEventManager::UIEVENTTYPE::INIT_ON:
	{
		if (m_pHP)
			m_pHP->PopUp_HP();

		if (m_pInventory)
			m_pInventory->PopUp_EquipSlot();
	}
	break;
	}
}

CUIManager* CUIManager::Create()
{
	CUIManager* pInstance = new CUIManager();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("FAILED TO CREATED : CUIManager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUIManager::Free()
{
	__super::Free();

	Safe_Release(m_pDialogue);
	Safe_Release(m_pInventory);
	Safe_Release(m_pRupee);
	Safe_Release(m_pHP);

	Safe_Release(m_pGameInstance);
}
