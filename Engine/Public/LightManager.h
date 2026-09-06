#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CLightManager final : public CBase
{
private:
	CLightManager();
	virtual ~CLightManager() = default;

public:
	HRESULT Add_Light(const LIGHT_DESC& _tLightDesc, class CTransform* _pTarget = nullptr, _vector _vOffset = XMVectorZero());
	void Update(_float _fTimeDelta);
	void Render(class CShader* _pShader, class CVIBuffer_Rect* _pVIBuffer);
	void Clear_Light();

	const LIGHT_DESC* Get_LightDesc(_uint _iIndex);

	/* Util */
	void Scale_AllPointLightIntensity(_float _fIntensity);
	void Pop_Back_Lights(_uint _iCount);

private:
	list<class CLight*> m_Lights;

public:
	static CLightManager* Create();
	virtual void Free() override;
};
NS_END