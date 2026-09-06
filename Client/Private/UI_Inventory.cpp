#include "UI_Inventory.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "UIObject.h"
#include "UI_Image.h"
#include "UI_Item.h"
#include "UI_Slot.h"

CUI_Inventory::CUI_Inventory()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CUI_Inventory::Initialize_Inventory()
{
    /* Set Init Value */
    m_tInventorySlot.vStartPosition = { g_iWinSizeX * 0.61f, g_iWinSizeY * 0.37f };
    m_tInventorySlot.vSize = { 139.f, 139.f };
    m_tInventorySlot.vGap = { -35.f, -35.f };
    m_tInventorySlot.iMaxRow = 4;
    m_tInventorySlot.iMaxSlot = 12;

    m_tInventoryItem = m_tInventorySlot;
    m_tInventoryItem.vSize = { 144.f, 144.f };

    /* Background */
    Create_Inventory_Background();

    /* Slot */
    for (_uint i = 0; i < m_tInventorySlot.iMaxSlot; ++i)
    {
        Create_Inventory_Slots(Compute_Position(i), m_tInventorySlot.vSize, 0.8f);
    }

    /* Cursor */
    Create_Cursor(0.8f);
    Update_Cursor_Position();

    /* Items */
    Create_Inventory_Item(TEXT("Prototype_Component_Texture_UI_Inventory_ItemBow"), TEXT("Item_Bow"), 0.6f);
    Create_Inventory_Item(TEXT("Prototype_Component_Texture_UI_Inventory_ItemOcarina"), TEXT("Item_Ocarina"), 0.6f);
    Create_Inventory_Item(TEXT("Prototype_Component_Texture_UI_Inventory_ItemRocsFeather"), TEXT("Item_RocsFeather"), 0.6f);

    for (size_t i = 0; i < m_vecItems.size(); ++i)
    {
        auto* pSlot = m_vecSlots[i];
        CTransform* pSelectedSlotTrans = static_cast<CTransform*>(pSlot->Get_Component(TEXT("Com_Transform")));
        _vector vSelectedSlotPos = pSelectedSlotTrans->Get_State(STATE::POSITION);

        auto* pItem = m_vecItems[i];
        CTransform* pItemTrans = static_cast<CTransform*>(pItem->Get_Component(TEXT("Com_Transform")));
        pItemTrans->Set_State(STATE::POSITION, vSelectedSlotPos);
    }

    /* Equip Slot */
    Create_Equip_Slot(0.65f);
}

HRESULT CUI_Inventory::Add_Item(_wstring _iItemID, _uint _iCount)
{
    Create_Inventory_Item(TEXT("Prototype_Component_Texture_UI_Inventory_ItemBow"), TEXT("Item_Bow"), 0.6f);

    auto* pSlot = m_vecSlots.back();
    CTransform* pSelectedSlotTrans = static_cast<CTransform*>(pSlot->Get_Component(TEXT("Com_Transform")));
    _vector vSelectedSlotPos = pSelectedSlotTrans->Get_State(STATE::POSITION);

    auto* pItem = m_vecItems.back();
    CTransform* pItemTrans = static_cast<CTransform*>(pItem->Get_Component(TEXT("Com_Transform")));
    pItemTrans->Set_State(STATE::POSITION, vSelectedSlotPos);

    return S_OK;
}

HRESULT CUI_Inventory::Remove_Item(_wstring _iItemID, _uint _iCount)
{
    return S_OK;
}

HRESULT CUI_Inventory::Use_Item(_uint _iSlotIndex)
{
    return S_OK;
}

void CUI_Inventory::Move_Cursor(DIR _eDir)
{
    _uint iCol = m_iSelectedSlot % m_tInventorySlot.iMaxRow;
    _uint iRow = m_iSelectedSlot / m_tInventorySlot.iMaxRow;

    switch (_eDir)
    {
    case Engine::DIR::LEFT:
        --iCol;
        break;
    case Engine::DIR::RIGHT:
        ++iCol;
        break;
    case Engine::DIR::UP:
        --iRow;
        break;
    case Engine::DIR::DOWN:
        ++iRow;
        break;
    }

    if (iCol < 0 || iCol >= m_tInventorySlot.iMaxRow)
        return;

    /* update new Index */
    _uint iNewIndex = iCol + (iRow * m_tInventorySlot.iMaxRow);
    if (iNewIndex >= m_vecSlots.size())
        return;

    m_iSelectedSlot = iNewIndex;

    Update_Cursor_Position();
}

void CUI_Inventory::Selected_Slot(EQUIPBTN _eEquip)
{
    if (m_iSelectedSlot >= m_vecSlots.size())
        return;

    auto* pSelectedSlot = m_vecSlots[m_iSelectedSlot];
    auto* pSelectedItem = m_vecItems[m_iSelectedSlot];

    if (nullptr == pSelectedSlot)
        return;

    
    _wstring itemID = FindAndRemove_Number(pSelectedItem->Get_ObjectID());

    // 반대 슬롯에 같은 아이템이 있으면 제거
    if (_eEquip == EQUIPBTN::X)
    {
        if (m_pEquipItem_Y && FindAndRemove_Number(m_pEquipItem_Y->Get_ObjectID()) == itemID)
        {
            m_pGameInstance->Reserve_DeleteObject(m_pEquipItem_Y);
            m_pEquipItem_Y = nullptr;

            if (m_pSlot_Y)
            {
                m_pSlot_Y->Equip(ENUM_TO_UINT(EQUIPBTN::NONE));
                m_pSlot_Y = nullptr;
            }
        }
    }
    else if (_eEquip == EQUIPBTN::Y)
    {
        if (m_pEquipItem_X && FindAndRemove_Number(m_pEquipItem_X->Get_ObjectID()) == itemID)
        {
            m_pGameInstance->Reserve_DeleteObject(m_pEquipItem_X);
            m_pEquipItem_X = nullptr;

            if (m_pSlot_X)
            {
                m_pSlot_X->Equip(ENUM_TO_UINT(EQUIPBTN::NONE));
                m_pSlot_X = nullptr;
            }
        }
    }

    /* Change Texture */
    switch (_eEquip)
    {
    case Engine::EQUIPBTN::X:
    {
        if (m_pSlot_X == pSelectedSlot)
            return;

        if (m_pSlot_X)
            m_pSlot_X->Equip(ENUM_TO_UINT(EQUIPBTN::NONE));

        m_pSlot_X = pSelectedSlot;
        m_pSlot_X->Equip(ENUM_TO_UINT(EQUIPBTN::X));

        /* Equip Item */
        Clone_Equip_Item(m_vEquipPositionX, pSelectedItem, &m_pEquipItem_X);
        m_iEquipItemName_X = m_pEquipItem_X->Get_ObjectID();
    }
        break;
    case Engine::EQUIPBTN::Y:
    {
        if (m_pSlot_Y == pSelectedSlot)
            return;

        if (m_pSlot_Y)
            m_pSlot_Y->Equip(ENUM_TO_UINT(EQUIPBTN::NONE));

        m_pSlot_Y = pSelectedSlot;
        m_pSlot_Y->Equip(ENUM_TO_UINT(EQUIPBTN::Y));

        /* Equip Item */
        Clone_Equip_Item(m_vEquipPositionY, pSelectedItem, &m_pEquipItem_Y);
    }
        break;
    }
}

void CUI_Inventory::Hide_Inventory()
{
    m_pBackground->Set_Active(false);
    m_pCursor->Set_Active(false);
    for (auto& SLOT : m_vecSlots)
        SLOT->Set_Active(false);
    for (auto& ITEM : m_vecItems)
        ITEM->Set_Active(false);
}

void CUI_Inventory::PopUp_Inventory()
{
    m_pBackground->Set_Active(true);
    m_pCursor->Set_Active(true);
    for (auto& SLOT : m_vecSlots)
        SLOT->Set_Active(true);
    for (auto& ITEM : m_vecItems)
        ITEM->Set_Active(true);
}

void CUI_Inventory::AlwaysOn_Inventory()
{
}

void CUI_Inventory::Hide_EquipSlot()
{
    m_pEquipSlot_X->Set_Active(false);
    m_pEquipSlot_Y->Set_Active(false);
    if (m_pEquipItem_X)
        m_pEquipItem_X->Set_Active(false);
    if (m_pEquipItem_Y)
        m_pEquipItem_Y->Set_Active(false);
}

void CUI_Inventory::PopUp_EquipSlot()
{
    m_pEquipSlot_X->Set_Active(true);
    m_pEquipSlot_Y->Set_Active(true);
    if (m_pEquipItem_X)
        m_pEquipItem_X->Set_Active(true);
    if (m_pEquipItem_Y)
        m_pEquipItem_Y->Set_Active(true);
}

void CUI_Inventory::AlwaysOn_EquipSlot()
{
}

void CUI_Inventory::Update(_float _fDeltaTime)
{
    Update_Cursor_Scale(_fDeltaTime);
}

_bool CUI_Inventory::IsUpdatable() const
{
    return _bool();
}

HRESULT CUI_Inventory::Create_Inventory_Background()
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Inventory_BG"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Inventory_BG"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Inventory_Background"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.5f;
    tUIDesc.fY = g_iWinSizeY * 0.5f;
    tUIDesc.fSizeX = g_iWinSizeX;
    tUIDesc.fSizeY = g_iWinSizeY;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::BACKGROUND;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &m_pBackground)))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_Inventory::Create_Inventory_Slots(_float2 _fPosition, _float2 _fSize, _float _fScale)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Inventoy_Slot"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Slot"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Inventoy_Slot"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Inventory_ItemSlot"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = _fPosition.x;
    tUIDesc.fY = _fPosition.y;
    tUIDesc.fSizeX = _fSize.x * _fScale;
    tUIDesc.fSizeY = _fSize.y * _fScale;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::POPUP;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject = { nullptr };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject)))
        return E_FAIL;

    if (pGameObject)
    {
        auto pSlot = static_cast<CUI_Slot*>(pGameObject);
        m_vecSlots.push_back(pSlot);
    }
    
    return S_OK;
}

HRESULT CUI_Inventory::Create_Cursor(_float _fScale)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Inventoy_Cursor"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Inventoy_Cursor"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Inventory_ItemSlot_Cursor"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.5f;
    tUIDesc.fY = g_iWinSizeY * 0.5f;
    tUIDesc.fSizeX = 156.f * _fScale;
    tUIDesc.fSizeY = 155.f * _fScale;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::POPUP;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject = { nullptr };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject)))
        return E_FAIL;

    m_pCursor = dynamic_cast<CUI_Image*>(pGameObject);

    return S_OK;
}

HRESULT CUI_Inventory::Create_Equip_Slot(_float _fScale)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Inventoy_Equip_Slot_X"));
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Image"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Inventoy_Equip_Slot"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Inventory_EquipSlot_X"));
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.85f;
    tUIDesc.fY = g_iWinSizeY * 0.072f;
    tUIDesc.fSizeX = 144.f * _fScale;
    tUIDesc.fSizeY = 144.f * _fScale;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::POPUP;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    m_vEquipPositionX = _float2{ tUIDesc.fX, tUIDesc.fY };

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject_X = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject_X)))
        return E_FAIL;

    m_pEquipSlot_X = static_cast<CUI_Image*>(pGameObject_X);

    wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_UI_Inventory_EquipSlot_Y"));
    tUIDesc.fX += 80.f;
    tUIDesc.fY -= 10.f;
    wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Inventoy_Equip_Slot_X"));

    m_vEquipPositionY = _float2{ tUIDesc.fX, tUIDesc.fY };

    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject_Y = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject_Y)))
        return E_FAIL;

    m_pEquipSlot_Y = static_cast<CUI_Image*>(pGameObject_Y);

    return S_OK;
}

HRESULT CUI_Inventory::Clone_Equip_Item(_float2 _vPosition, CUI_Item* _SourceItem, CUI_Item** _ppOut)
{
    if (!_SourceItem || !_ppOut)
        return E_FAIL;

    if (*_ppOut)
    {
        m_pGameInstance->Reserve_DeleteObject(*_ppOut);
        *_ppOut = nullptr;
    }

    if (_SourceItem)
    {
        CGameObject::OBJECT_DESC tObjectDesc = _SourceItem->Get_ObjectDesc();
        _wstring newEquipItemID = _wstring(tObjectDesc.ObjectID) + L"_" + to_wstring(m_iEquipIndex++);
        wcscpy_s(tObjectDesc.ObjectID, newEquipItemID.c_str());
        CUIObject::CUIObject::UI_DESC tUIDesc = {};
        tUIDesc.fX = _vPosition.x;
        tUIDesc.fY = _vPosition.y;
        tUIDesc.fSizeX = 144.f * 0.39f;
        tUIDesc.fSizeY = 144.f * 0.39f;
        tUIDesc.eUILayer = CUIObject::UI_LAYER::POPUP;
        tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
        tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

        CUIObject::UI_INIT_DESC tInitDesc = {};
        tInitDesc.tObjectDesc = tObjectDesc;
        tInitDesc.tuiDesc = tUIDesc;

        CGameObject* pClonedObject = { nullptr };
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
            tObjectDesc.iLevel,
            tObjectDesc.ObjectTag,
            tObjectDesc.iLevel,
            tObjectDesc.LayerTag,
            &tInitDesc, &pClonedObject)))
            return E_FAIL;

        *_ppOut = static_cast<CUI_Item*>(pClonedObject);
    }

    return S_OK;
}

HRESULT CUI_Inventory::Create_Inventory_Item(_wstring _textureTag, _wstring _ObjectID, _float _fScale)
{
    CGameObject::OBJECT_DESC tObjectDesc = {};
    wcscpy_s(tObjectDesc.ObjectID, _ObjectID.c_str());
    tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
    wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Item"));
    wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Inventoy_Item"));
    wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
    wcscpy_s(tObjectDesc.TextureTag, _textureTag.c_str());
    wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
    //wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

    CUIObject::CUIObject::UI_DESC tUIDesc = {};
    tUIDesc.fX = g_iWinSizeX * 0.5f;
    tUIDesc.fY = g_iWinSizeY * 0.5f;
    tUIDesc.fSizeX = 144.f * _fScale;
    tUIDesc.fSizeY = 144.f * _fScale;
    tUIDesc.eUILayer = CUIObject::UI_LAYER::OVERLAY;
    tUIDesc.eUIInteraction = CUIObject::UI_INTERACTION::NON_INTERACTION;
    tUIDesc.eUIBlend = CUIObject::UI_BLEND::ALPHABLEND;

    CUIObject::UI_INIT_DESC tInitDesc = {};
    tInitDesc.tObjectDesc = tObjectDesc;
    tInitDesc.tuiDesc = tUIDesc;

    CGameObject* pGameObject = { nullptr };

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(
        tObjectDesc.iLevel,
        tObjectDesc.ObjectTag,
        tObjectDesc.iLevel,
        tObjectDesc.LayerTag,
        &tInitDesc, &pGameObject)))
        return E_FAIL;

    if (pGameObject)
    {
        auto pItem = static_cast<CUI_Item*>(pGameObject);
        m_vecItems.push_back(pItem);
    }

    return S_OK;
}

void CUI_Inventory::Update_Cursor_Position()
{
    if (nullptr == m_pCursor)
        return;

    if (m_iSelectedSlot >= m_vecSlots.size())
        return;

    auto* pSlot = m_vecSlots[m_iSelectedSlot];
    if (nullptr == pSlot)
        return;

    CTransform* pSelectedSlotTransform = static_cast<CTransform*>(pSlot->Get_Component(TEXT("Com_Transform")));
    _vector vSelectedSlotPos = pSelectedSlotTransform->Get_State(STATE::POSITION);

    CTransform* pCursorTransform = static_cast<CTransform*>(m_pCursor->Get_Component(TEXT("Com_Transform")));
    pCursorTransform->Set_State(STATE::POSITION, vSelectedSlotPos);

    /* Breath Reset */
    m_fCursorIdleTime = 0.f;
    m_bCursorMoveEnbale = true;
}

void CUI_Inventory::Update_Cursor_Scale(_float _fDeltaTime)
{
    if (!m_pCursor)
        return;

    /* Pos */
    auto* pSlot = m_vecSlots[m_iSelectedSlot];
    if (!pSlot)
        return;

    CTransform* pSlotTransform = static_cast<CTransform*>(pSlot->Get_Component(TEXT("Com_Transform")));
    _vector vPos = pSlotTransform->Get_State(STATE::POSITION);

    _float x = XMVectorGetX(vPos) + g_iWinSizeX * 0.5f;
    _float y = -XMVectorGetY(vPos) + g_iWinSizeY * 0.5f;

    m_pCursor->Set_UIPosition(x, y);

    /* Anim */
    m_fCursorIdleTime += _fDeltaTime;

    _float t = m_fCursorIdleTime * m_fCursorBreathSpeed;
    _float scale = 1.f + sinf(t) * 0.1f;

    m_pCursor->Set_UIScale(m_fCursorBaseSizeX * scale, m_fCursorBaseSizeY * scale);

    m_pCursor->Apply_UITransform();
}

void CUI_Inventory::Update_Equip_Item()
{
}

_float2 CUI_Inventory::Compute_Position(_uint _iIndex)
{
    _uint iCol = _iIndex % m_tInventorySlot.iMaxRow;
    _uint iRow = _iIndex / m_tInventorySlot.iMaxRow;

    return _float2{
        m_tInventorySlot.vStartPosition.x + ((m_tInventorySlot.vSize.x + m_tInventorySlot.vGap.x) * iCol),
        m_tInventorySlot.vStartPosition.y + ((m_tInventorySlot.vSize.y + m_tInventorySlot.vGap.y) * iRow)
    };
}

CUI_Inventory* CUI_Inventory::Create()
{
    return new CUI_Inventory();
}

void CUI_Inventory::Free()
{
    __super::Free();

    m_vecSlots.clear();
    m_vecItems.clear();

    Safe_Release(m_pGameInstance);
}
