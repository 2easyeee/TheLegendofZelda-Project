#include "GameObject/Weapon_Monster.h"
#include "GameInstance.h"

CWeapon_Monster::CWeapon_Monster(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CPartObject { _pDevice, _pDeviceContext }
{
}

CWeapon_Monster::CWeapon_Monster(const CWeapon_Monster& _Prototype)
	: CPartObject (_Prototype)
{
}

HRESULT CWeapon_Monster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon_Monster::Initialize(void* _pArg)
{
	/* 1. PartOBject */
	PART_CREATE_DESC* tInitDesc = static_cast<PART_CREATE_DESC*>(_pArg);
	if (FAILED(CPartObject::Initialize(tInitDesc)))
		return E_FAIL;

	if (&tInitDesc->tPartDesc)
	{
		m_pParentState = tInitDesc->tPartDesc.pPartentSTATE;
		m_pSocketMatrix = tInitDesc->tPartDesc.pSocketMatrix;
	}

	if (&tInitDesc->tObjectDesc)
	{
		/* 2. Component */
		if (FAILED(Ready_Components(&tInitDesc->tObjectDesc)))
			return E_FAIL;
	}

	/* Here Transform */
	//m_pTransformCom->Set_Scale(0.1f, 0.1f, 0.1f);
	//m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 90.f);
	//m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSet(0.8f, 0.f, 0.f, 1.f));

	return S_OK;
}

void CWeapon_Monster::Priority_Update(_float _fTimeDelta)
{
}

void CWeapon_Monster::Update(_float _fTimeDelta)
{
	/* 1. SocketMatrix */
	_matrix WorldMatrix = XMMatrixIdentity();
	_matrix SocketMatrix = XMLoadFloat4x4(m_pSocketMatrix);
	for (size_t i = 0; i < 3; i++)
	{
		SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);
	}
	WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * SocketMatrix;

	/* 2. Update WorldMatrix */
	CPartObject::Update_WorldMatrix(WorldMatrix);
}

void CWeapon_Monster::Late_Update(_float _fTimeDelta)
{
	CPartObject::Late_Update(_fTimeDelta);

	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::NONBLEND, this)))
		return;
}

HRESULT CWeapon_Monster::Render()
{
	/* Transform */
	if (FAILED(CPartObject::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	/* Perspective */
	if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_TransformState(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	/* Model */
	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		/* Material */
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MATERIAL::DIFFUSE)))
			return E_FAIL;
		/* Bone */
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		/* Shader */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
		/* Mesh */
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CWeapon_Monster::Ready_Components(OBJECT_DESC* _Desc)
{
	if (FAILED(Add_Component(
		RESOURCE_LEVEL_STATIC,
		_Desc->ShaderTag,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (FAILED(Add_Component(
		_Desc->iLevel,
		_Desc->ModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

CWeapon_Monster* CWeapon_Monster::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CWeapon_Monster* pInstance = new CWeapon_Monster(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("FAILED TO CREATED : CWeapon_Monster");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWeapon_Monster::Clone(void* _pArg)
{
	CWeapon_Monster* pInstance = new CWeapon_Monster(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("FAILED TO CLONED : CWeapon_Monster");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWeapon_Monster::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
