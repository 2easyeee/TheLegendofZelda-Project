#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	void Priority_Update(_float _fTimeDelta);
	void Update(_float _fTimeDelta);
	void Late_Update(_float _fTimeDelta);

	HRESULT Add_GameObject(class CGameObject* _pGameObject);
	HRESULT Find_GameObject(const _wstring& _strObjectID, CGameObject** _ppOut);
	HRESULT Remove_GameObject(const _wstring& _wstrObjectID);

	class CComponent* Get_Component(const _wstring& _strComponentTag, _uint _iIndex);

	list<CGameObject*> Get_GameObjects() const { return m_GameObjectList; }
	
private:
	list<class CGameObject*> m_GameObjectList;

public:
	static CLayer* Create();
	virtual void Free() override;
};
NS_END