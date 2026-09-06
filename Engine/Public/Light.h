#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CLight final : public CBase
{
private:
	CLight();
	virtual ~CLight() = default;

public:
	HRESULT Initialize(const LIGHT_DESC& _tLightDesc);
	void Update(_float _fTimeDelta);
	void Render(class CShader* _pShader, class CVIBuffer_Rect* _pVIBuffer);

	const LIGHT_DESC* Get_Light();
	void Set_Target(class CTransform* _pTarget);
	void Set_Offset(_vector _vOffset);
	void Set_Position(_vector _vPos);

private:
	LIGHT_DESC m_tLightDesc = {};

	/* USAGE */
	class CTransform* m_pTarget = { nullptr };
	_vector m_vOffset = XMVectorZero();

public:
	static CLight* Create(const LIGHT_DESC& _tLightDesc);
	virtual void Free() override;
};
NS_END