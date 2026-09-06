#include "Collider.h"
#include "GameInstance.h"
#include "Debug/DebugDraw.h"

CCollider::CCollider(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CComponent{ _pDevice, _pDeviceContext }
{
}

CCollider::CCollider(const CCollider& _Prototype)
	: CComponent (_Prototype)
	, m_eColliderType { _Prototype.m_eColliderType }
#ifdef  _DEBUG
	, m_pBatch { _Prototype.m_pBatch }
	, m_pEffect { _Prototype.m_pEffect }
	, m_pInputLayout { _Prototype.m_pInputLayout }
#endif //  _DEBUG
{
#ifdef _DEBUG
	Safe_AddRef(m_pInputLayout);
#endif // _DEBUG

}

HRESULT CCollider::Initialize_Prototype(COLLIDER _eColliderType)
{
	m_eColliderType = _eColliderType;

#ifdef _DEBUG
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pDeviceContext);
	m_pEffect = new BasicEffect(m_pDevice);
	
	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode = { nullptr };
	size_t iShaderByteCodeLength = {};

	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

	if (FAILED(m_pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		pShaderByteCode,
		iShaderByteCodeLength,
		&m_pInputLayout)))
		return E_FAIL;
#endif // _DEBUG

	return S_OK;
}

HRESULT CCollider::Initialize(void* _pArg)
{
	switch (m_eColliderType)
	{
	case Engine::COLLIDER::AABB:
		m_pBounding = CBounding_AABB::Create(static_cast<const CBounding::BOUNDING_DESC*>(_pArg));
		break;
	case Engine::COLLIDER::OBB:
		m_pBounding = CBounding_OBB::Create(static_cast<const CBounding::BOUNDING_DESC*>(_pArg));
		break;
	case Engine::COLLIDER::SPHERE:
		m_pBounding = CBounding_Sphere::Create(static_cast<const CBounding::BOUNDING_DESC*>(_pArg));
		break;
	}
	return S_OK;
}

void CCollider::Update(_fmatrix _WorldMatrix)
{
	if (m_pBounding)
		m_pBounding->Update(_WorldMatrix);
}

_bool CCollider::Intersect(CCollider* _pTargetCollider)
{
	if (!m_bActive || !_pTargetCollider->m_bActive)
		return false;

	if (!m_pBounding)
		return false;

	m_isColl = m_pBounding->Intersect(
		_pTargetCollider->m_eColliderType,
		_pTargetCollider->m_pBounding);

	return m_isColl;
}

void CCollider::Set_Active_All(const vector<CCollider*>& _vecColliders, _bool _bActive)
{
	for (auto& pCollider : _vecColliders)
	{
		if (pCollider)
			pCollider->Set_Active(_bActive);
	}
}

void CCollider::Clear_HitTargets()
{
	m_HitTargets.clear();
}

_bool CCollider::Hit_Once(CCollider* _pTarget)
{
	if (!m_bActive)
		return false;

	/* Dup Hit Check */
	if (m_HitTargets.find(_pTarget) != m_HitTargets.end())
		return false;

	if (!Intersect(_pTarget))
		return false;

	m_HitTargets.insert(_pTarget);

	return true;
}

void CCollider::Set_Trigger(_bool _bActive)
{
	if (_bActive)
		m_eColliderMode = COLLIDERMODE::TRIGGER;
	else
		m_eColliderMode = COLLIDERMODE::PHYSICAL;
}

void CCollider::Set_Extents(_float3 _vExtents)
{
	if (m_pBounding)
		m_pBounding->Set_Extents(_vExtents);
}

void CCollider::Set_Center(_float3 _vCenter)
{
	if (m_pBounding)
		m_pBounding->Set_Center(_vCenter);
}

CCollider* CCollider::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, COLLIDER _eColliderType)
{
	CCollider* pInstance = new CCollider(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype(_eColliderType)))
	{
		MSG_BOX("Failed to Created : CCollider");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CCollider::Clone(void* _pArg)
{
	CCollider* pInstance = new CCollider(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("Failed to Cloned : CCollider");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCollider::Free()
{
	/* For. MapTool */
	// if (m_pGameInstance)
	// 	m_pGameInstance->Remove_Collider(this);

	__super::Free();

	Safe_Release(m_pBounding);

#ifdef _DEBUG
	if (false == m_isCloned)
	{
		Safe_Delete(m_pBatch);
		Safe_Delete(m_pEffect);
	}
	Safe_Release(m_pInputLayout);
#endif // _DEBUG

}

#ifdef _DEBUG
HRESULT CCollider::Render()
{
	if (!m_bActive)
		return S_OK;

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(m_pGameInstance->Get_Transform(D3DTS::VIEW)));
	m_pEffect->SetProjection(XMLoadFloat4x4(m_pGameInstance->Get_Transform(D3DTS::PROJ)));

	m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	m_pEffect->Apply(m_pDeviceContext);

	m_pBatch->Begin();

	_vector	vColor = m_isColl == true ?
		XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f);
	m_pBounding->Render(m_pBatch, vColor);

	m_pBatch->End();

	return S_OK;
}
#endif // _DEBUG

