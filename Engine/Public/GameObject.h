#pragma once
#include "Base.h"
#include "Transform.h"

NS_BEGIN(Engine)
class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct tagObjectDesc
	{
		_tchar ObjectID[MAX_PATH] = {TEXT("Unknown")};
		_uint iLevel;
		_tchar LayerTag[MAX_PATH];
		_tchar ObjectTag[MAX_PATH];
		_tchar VIBufferTag[MAX_PATH];
		_tchar ShaderTag[MAX_PATH];
		_tchar TextureTag[MAX_PATH];
		_tchar ModelTag[MAX_PATH];
	}OBJECT_DESC;

protected:
	CGameObject(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CGameObject(const CGameObject& _Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* _pArg);
	virtual void Priority_Update(_float _fTimeDelta); 
	virtual void Update(_float _fTimeDelta);
	virtual void Late_Update(_float _fTimeDelta);
	virtual HRESULT Render();

	/* Shadow */
	virtual HRESULT Render_Shadow();

public:
	class CComponent* Get_Component(const _wstring& _strComponentTag);
	_wstring Get_ObjectID() const { return m_wstrObjectID; }
	_bool Compare_ObjectID(const _wstring& _strObjectID);
	OBJECT_DESC Get_ObjectDesc() const { return m_ObjectDesc; }
	void Set_Active(_bool _bEnable) { m_bActive = _bEnable; }
	_bool IsActive() { return m_bActive; }
	void Set_RenderActive(_bool _bEnable) { m_OnlyRenderActive = _bEnable; }

	/* Collision */
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) {}
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) {}
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) {}

	/* Collision (Trigger) */
	virtual void OnTriggerEnter(class CCollider* _pSrc, class CCollider* _Dst) {}
	virtual void OnTriggerStay(class CCollider* _pSrc, class CCollider* _Dst) {}
	virtual void OnTriggerExit(class CCollider* _pSrc, class CCollider* _Dst) {}

protected:
	class CGameInstance* m_pGameInstance = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pDeviceContext = { nullptr };

	map<const _wstring, class CComponent*> m_Components;
	class CTransform* m_pTransformCom = { nullptr };
	_bool m_bActive = { true };
	_bool m_OnlyRenderActive = { true };

	OBJECT_DESC m_ObjectDesc = {};

protected:
	HRESULT Add_Component(_uint _iPrototypeLevelID, const _wstring& _strPrototypeTag, const _wstring& _strComponentTag, CComponent** _ppOut, void* _pArg = nullptr);
	HRESULT Remove_Component(const _wstring& _strComponentTag);

private:
	_wstring m_wstrObjectID = { TEXT("Unknown") };

public:
	virtual CGameObject* Clone(void* _pArg) = 0;
	virtual void Free() override;
};
NS_END