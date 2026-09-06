#pragma once
#include "Client_Defines.h"
#include "Effect.h"

NS_BEGIN(Engine)
class CEffect;
class CVIBuffer_Trail;
NS_END

NS_BEGIN(Client)
class CSwordSlash final : public CEffect
{
private:
	CSwordSlash(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CSwordSlash(const CSwordSlash& _Prototype);
	virtual ~CSwordSlash() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	static CSwordSlash* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END