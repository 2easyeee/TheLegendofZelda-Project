#pragma once
#include "MapTool_Defines.h"
#include "MapObject.h"

NS_BEGIN(Engine)
class CNavigation;
NS_END

/* For. Only Navigation Render */
NS_BEGIN(MapTool)
class CNavHolder final : public CMapObject
{
private:
	CNavHolder(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CNavHolder(const CNavHolder& _Prototype);
	virtual ~CNavHolder() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CNavigation* m_pNavCom = { nullptr };

private:
	HRESULT Ready_Components(_wstring _ComTag);

public:
	static CNavHolder* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END