#include "UI_HP.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "UI_Heart.h"

CUI_HP::CUI_HP()
    : m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CUI_HP::Initialize_HP(_float _fHP)
{
	/* Set Init Value */
	m_fCurrentHP = _fHP;
	m_iOwnedHP = ceil(_fHP);
	m_tHPLayout.vStartPosition = { g_iWinSizeX * 0.05f, g_iWinSizeY * 0.05f };
	m_tHPLayout.vSize = { 40.f * 0.75f, 40.f * 0.75f };
	m_tHPLayout.vGap = { 5.f, 5.f };
	m_tHPLayout.iMaxRow = 10;

	/* Create Init HP */
	for (_uint i = 0; i < m_iOwnedHP; ++i)
	{
		Create_HP(Compute_Position(i), i);
	}
}

void CUI_HP::Damage_HP(_float _fHP)
{
	m_fCurrentHP -= _fHP;
	if (m_fCurrentHP <= 0.f)
		m_fCurrentHP = 0.f;
}

void CUI_HP::Add_HP(_float _fHP)
{
	/* LIMIT MAX HP */
	if (m_fCurrentHP >= m_iOwnedHP)
		return;

	m_fCurrentHP += _fHP;
	if (m_fCurrentHP >= m_iMaxHP)
		m_fCurrentHP = m_iOwnedHP;
}

void CUI_HP::Add_MAXHP(_float _fHP)
{
	/* LIMIT MAX HP */
	if (m_iOwnedHP >= m_iMaxHP)
		return;

	_uint m_iPrevOwnedHP = m_iOwnedHP;

	for (_uint i = 0; i < _fHP; ++i)
	{
		_uint iNewIndex = m_iPrevOwnedHP + i;

		Create_HP(Compute_Position(iNewIndex), iNewIndex);
		++m_iOwnedHP;

		/* HEAL MAX HP */
		m_fCurrentHP = m_iOwnedHP;
	}
}

_float CUI_HP::Get_CurrentHP() const
{
    return m_fCurrentHP;
}

void CUI_HP::Hide_HP()
{
	for (auto& HP : m_vecHP)
		HP->Set_Active(false);
}

void CUI_HP::PopUp_HP()
{
	for (auto& HP : m_vecHP)
		HP->Set_Active(true);
}

void CUI_HP::AlwaysOn_HP()
{
}

void CUI_HP::Update(_float _fDeltaTime)
{
	/* Update HP */
	if (m_vecHP.empty())
		return;

	for (auto& Heart : m_vecHP)
		Heart->Set_HP_State(m_fCurrentHP);
}

_bool CUI_HP::IsUpdatable() const
{
	return _bool();
}

HRESULT CUI_HP::Create_HP(_float2 _fPosition, _uint _iIndex)
{
	CGameObject::OBJECT_DESC tObjectDesc = {};
	wcscpy_s(tObjectDesc.ObjectID, TEXT("UI_Heart"));
	tObjectDesc.iLevel = ENUM_TO_UINT(LEVEL::STATIC);
	wcscpy_s(tObjectDesc.ObjectTag, TEXT("Prototype_GameObject_UI_Heart"));
	wcscpy_s(tObjectDesc.LayerTag, TEXT("UI_Heart"));
	wcscpy_s(tObjectDesc.VIBufferTag, TEXT("Prototype_Component_VIBuffer_Rect"));
	wcscpy_s(tObjectDesc.TextureTag, TEXT("Prototype_Component_Texture_HP"));
	wcscpy_s(tObjectDesc.ShaderTag, TEXT("Prototype_Component_Shader_VtxPosTex"));
	//wcscpy_s(tObjectDesc.ModelTag, m_tComboDesc_Model.wstrSelectedTag.c_str());

	CUIObject::CUIObject::UI_DESC tUIDesc = {};
	tUIDesc.fX = _fPosition.x;
	tUIDesc.fY = _fPosition.y;
	tUIDesc.fSizeX = m_tHPLayout.vSize.x;
	tUIDesc.fSizeY = m_tHPLayout.vSize.y;
	tUIDesc.eUILayer = CUIObject::UI_LAYER::HUD;
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
		auto pHP = static_cast<CUI_Heart*>(pGameObject);
		pHP->Set_Index(_iIndex);

		m_vecHP.push_back(pHP);
	}

    return S_OK;
}

_float2 CUI_HP::Compute_Position(_uint _iIndex)
{
	_uint iCol = _iIndex % m_tHPLayout.iMaxRow;
	_uint iRow = _iIndex / m_tHPLayout.iMaxRow;

	return _float2{
		m_tHPLayout.vStartPosition.x + ((m_tHPLayout.vSize.x + m_tHPLayout.vGap.x) * iCol),
		m_tHPLayout.vStartPosition.y + ((m_tHPLayout.vSize.y + m_tHPLayout.vGap.y) * iRow)
	};
}

CUI_HP* CUI_HP::Create()
{
	return new CUI_HP();
}

void CUI_HP::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
