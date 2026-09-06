#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CEffectInstance : public CBase
{
public:
	CEffectInstance();
	virtual ~CEffectInstance() = default;

public:
	void Initialize(_vector _vPosition, _vector _vScale, _vector _vRotation, _float _fDuration);
	void Update(_float _fTimeDelta);

public:
	_bool Is_Alive() const;
	void Get_WorldMatrix(_float4x4& _Out);

private:
	/* RunTime */
	_float3 m_vPosition = { 0.f, 0.f, 0.f };
	_float3 m_vScale = { 1.f, 1.f, 1.f };
	_float3 m_vRotation = { 0.f, 0.f, 0.f };
	STATE_TIME m_tLifeTime = { false, 0.f, 0.f };
	_bool m_bAlive = { true };

public:
	virtual void Free() override;
};
NS_END