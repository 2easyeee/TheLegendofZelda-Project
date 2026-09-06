#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CLevelManager final : public CBase
{
private:
	CLevelManager();
	virtual ~CLevelManager() = default;

public:
	void Update(_float _fTimeDelta);
	HRESULT Render();

	HRESULT Reserve_NextLevel(_uint _iLevelIndex, class CLevel* _pNewLevel);
	void Switch_NextLevel(_uint _iLevelIndex, class CLevel* _pNewLevel);

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	class CLevel* m_pCurrentLevel = { nullptr };
	_uint m_iCurrentLevelIndex = {};

	class CLevel* m_pReservedLevel = { nullptr };
	_uint m_iReservedLevelIndex = {};

	_bool m_bIsNextLevel = { false };

public:
	static CLevelManager* Create();
	virtual void Free() override;
};
NS_END