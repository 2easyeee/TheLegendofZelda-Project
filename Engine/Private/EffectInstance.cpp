#include "EffectInstance.h"

CEffectInstance::CEffectInstance()
	: CBase {}
{
}

void CEffectInstance::Initialize(_vector _vPosition, _vector _vScale, _vector _vRotation, _float _fDuration)
{
	XMStoreFloat3(&m_vPosition, _vPosition);
	XMStoreFloat3(&m_vScale, _vScale);
	XMStoreFloat3(&m_vRotation, _vRotation);

	m_tLifeTime.fAccTime = 0.f;
	m_tLifeTime.fAccDurationTime = _fDuration;
	m_bAlive = true;
}

void CEffectInstance::Update(_float _fTimeDelta)
{
	if (!m_bAlive)
		return;

	m_tLifeTime.fAccTime += _fTimeDelta;
	if (m_tLifeTime.fAccTime >= m_tLifeTime.fAccDurationTime)
	{
		m_bAlive = false;
		return;
	}
}

_bool CEffectInstance::Is_Alive() const
{
	return m_bAlive;
}

void CEffectInstance::Get_WorldMatrix(_float4x4& _Out)
{
	_matrix matScale = XMMatrixScaling(
			m_vScale.x,
			m_vScale.y,
			m_vScale.z);

	_matrix matRot =
			XMMatrixRotationX(m_vRotation.x) *
			XMMatrixRotationY(m_vRotation.y) *
			XMMatrixRotationZ(m_vRotation.z);

	_matrix matPos = XMMatrixTranslation(
			m_vPosition.x,
			m_vPosition.y,
			m_vPosition.z);

	_matrix matWorld = matScale * matRot * matPos;

	XMStoreFloat4x4(&_Out, matWorld);
}

void CEffectInstance::Free()
{
}
