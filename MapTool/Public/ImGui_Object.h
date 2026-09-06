#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(MapTool)
class CImGui_Object abstract : public CBase
{
protected:
	CImGui_Object(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CImGui_Object() = default;

public:
	virtual HRESULT Initialize() { return S_OK; };
	virtual void Update(_float fTimeDelta) = 0;
	virtual void LateUpdate(_float fTimeDelta) = 0;
	virtual HRESULT Render() = 0;

protected:
	class CGameInstance* m_pGameInstance = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	_wstring m_strTitle = { TEXT("Empty Title") };

public:
	void Free() override;
};
NS_END