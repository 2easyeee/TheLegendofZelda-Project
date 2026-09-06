#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CObjectManager final : public CBase
{
private:
	CObjectManager();
	virtual ~CObjectManager() = default;

public:
	HRESULT Initialize(_uint _iTotalLevelCnt);
	void Priority_Update(_float _fTimeDelta);
	void Update(_float _fTimeDelta);
	void Late_Update(_float _fTimeDelta);
	void Delete_Update();
	void Clear(_uint _iLevelIndex);

	HRESULT Add_GameObject_ToLayer(_uint _iPrototypeLevelIndex, const _wstring& _strPrototypeTag, _uint _iLayerLevelIndex, const _wstring& _strLayerTag, void* _pArg = nullptr, class CGameObject** ppOut = nullptr);;
	HRESULT Reserve_DeleteObject(class CGameObject* _pGameObject);
	CGameObject* Find_Object(const _wstring& _strObjectID);

	class CComponent* Get_Component(_uint _iLevelIndex, const _wstring& _strLayerTag, const _wstring& _strComponentTag, _uint _iIndex);
	map<const _wstring, class CLayer*>* Get_Layers() const { return m_Layers; }

private:
	class CGameInstance* m_pGameInstance = { nullptr };

	map<const _wstring, class CLayer*>* m_Layers = { nullptr };

	_uint m_iTotalLevelCnt = {};
	vector<class CGameObject*> m_vecDeleteObjects;

	/* Find */
	unordered_map<_wstring, CGameObject*> m_ObjectTable;

private:
	class CLayer* Find_Layer(_uint _iLevelIndex, const _wstring& _strLayerTag);
	HRESULT Delete_GameObject_ToLayer(class CGameObject* _pTarget);

public:
	static CObjectManager* Create(_uint _iTotalLevelCnt);
	virtual void Free() override;
};
NS_END