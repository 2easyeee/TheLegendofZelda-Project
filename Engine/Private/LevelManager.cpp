#include "LevelManager.h"

#include "GameInstance.h"
#include "Level.h"

CLevelManager::CLevelManager()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CLevelManager::Update(_float _fTimeDelta)
{
	m_pCurrentLevel->Update(_fTimeDelta);
}

HRESULT CLevelManager::Render()
{
	if (!m_pCurrentLevel)
		return S_OK;

	return m_pCurrentLevel->Render();
}

HRESULT CLevelManager::Reserve_NextLevel(_uint _iLevelIndex, CLevel* _pNewLevel)
{
	if (!_pNewLevel)
		return E_FAIL;

	/* 0. Turn on _bool */
	m_bIsNextLevel = true;

	/* 1. Reserve New Level */
	m_pReservedLevel = _pNewLevel;
	m_iReservedLevelIndex = _iLevelIndex;

	return S_OK;
}

void CLevelManager::Switch_NextLevel(_uint _iLevelIndex, CLevel* _pNewLevel)
{
	/* Excep. Set New Level "Directly" */
	if ((nullptr != _pNewLevel) && (_iLevelIndex > 0))
		m_bIsNextLevel = true;

	if (!m_bIsNextLevel)
		return;

	/* 0. Clear Level */
	if (nullptr != m_pCurrentLevel)
		m_pGameInstance->Clear(m_iCurrentLevelIndex);

	if (0 != Safe_Release(m_pCurrentLevel))
		return;

	if ((nullptr != _pNewLevel) && (_iLevelIndex > 0))
	{
		/* Excep. Set New Level "Directly" */
		m_pCurrentLevel = _pNewLevel;
		m_iCurrentLevelIndex = _iLevelIndex;
	}
	else
	{
		/* 1. Set New Level */
		m_pCurrentLevel = m_pReservedLevel;
		m_iCurrentLevelIndex = m_iReservedLevelIndex;

		/* 2. Reset */
		m_pReservedLevel = nullptr;
		m_iReservedLevelIndex = 0;
	}

	m_bIsNextLevel = false;
}

CLevelManager* CLevelManager::Create()
{
	return new CLevelManager();
}

void CLevelManager::Free()
{
	__super::Free();

	Safe_Release(m_pReservedLevel);
	Safe_Release(m_pCurrentLevel);
	Safe_Release(m_pGameInstance);
}
