#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CLevel abstract : public CBase
{
protected:
	CLevel(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CLevel() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	class CGameInstance* m_pGameInstance = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

protected:
	_bool m_bIsNextLevel = { false };

protected:
	virtual HRESULT Ready_Layer_Shadow();

public:
	virtual void Free() override;
};
NS_END