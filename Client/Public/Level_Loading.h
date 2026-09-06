#pragma once
#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

/* 磊盔_包府_按眉(Loader) 甫 包府(积己/昏力) 开且 荐青 */
NS_BEGIN(Client)
class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVEL _eNextLevel);
	virtual void Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CLoader* m_pLoader = { nullptr };
	LEVEL m_eNextLevel = {};
	
	STATE_TIME m_tMinDisplayTime = { false, 0.f, 1.f };
	_bool m_bLoaderStarted = { false };

private:
	HRESULT Ready_Layer_Background(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Spinner(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Egg(const _wstring& strLayerTag);
	HRESULT Ready_Layer_Spicy(const _wstring& strLayerTag);

public:
	static CLevel_Loading* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, LEVEL _eNextLevel);
	virtual void Free() override;
};
NS_END