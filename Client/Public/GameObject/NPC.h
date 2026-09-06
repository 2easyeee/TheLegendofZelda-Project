#pragma once
#include "Client_Defines.h"
#include "PawnObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CNPC final : public CPawnObject
{
public:
	typedef struct tagNPCConversation
	{
		vector<_int> vecDialogueIDs;
		vector<_int> vecEventIDs;
	}NPC_CONVERSATION;

private:
	CNPC(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CNPC(const CNPC& _Prototype);
	virtual ~CNPC() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Priority_Update(_float _fTimeDelta) override;
	virtual void Update(_float _fTimeDelta) override;
	virtual void Late_Update(_float _fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	/* Collision*/
	virtual void OnCollisionEnter(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionStay(class CCollider* _pSrc, class CCollider* _Dst) override;
	virtual void OnCollisionExit(class CCollider* _pSrc, class CCollider* _Dst) override;

private:
	/* Collider */
	CCollider* m_pBodyCollider = { nullptr };

	/* NPC */
	_wstring m_NPCName;
	unordered_map<_wstring, NPC_CONVERSATION> m_NPCTable;

	/* Dialogue */
	_int m_iDialogueIndex;

	/* Interaction */
	_bool m_bPlayerInRange = { false };

private:
	HRESULT Ready_Collider();

public:
	static CNPC* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual CGameObject* Clone(void* _pArg) override;
	virtual void Free() override;
};
NS_END