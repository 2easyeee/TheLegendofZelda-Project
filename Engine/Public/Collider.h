#pragma once
#include "Component.h"
#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"

NS_BEGIN(Engine)
class ENGINE_DLL CCollider final : public CComponent
{
public:
	enum class COLLIDERMODE { PHYSICAL, TRIGGER, END };

private:
	CCollider(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	CCollider(const CCollider& _Prototype);
	virtual ~CCollider() = default;

public:
	virtual HRESULT Initialize_Prototype(COLLIDER _eColliderType);
	virtual HRESULT Initialize(void* _pArg);
	void Update(_fmatrix _WorldMatrix);
	_bool Intersect(CCollider* _pTargetCollider);

	/* Active */
	void Set_Active(_bool _bActive) { m_bActive = _bActive; }
	_bool IsActive() const { return m_bActive; }
	void Set_Active_All(const vector<CCollider*>& _vecColliders, _bool _bActive);

	/* Hit */
	void Clear_HitTargets();
	_bool Hit_Once(class CCollider* _pTarget);

	/* Group */
	void Set_Owner(class CGameObject* _pOwner) { m_pOwner = _pOwner; }
	class CGameObject* Get_Owner() const { return m_pOwner; }
	
	void Set_Group(GROUP _eGroup) { m_eGroup = _eGroup; }
	GROUP Get_Group() const { return m_eGroup; }

	/* Block */
	void Set_Blocked(_bool _bActive) { m_bBlocked = _bActive; }
	_bool IsBlocked() const { return m_bBlocked; }

	/* Trigger */
	void Set_Trigger(_bool _bActive = true);
	_bool IsTrigger() const { return m_eColliderMode == COLLIDERMODE::TRIGGER; }

	/* Scale */
	void Set_Extents(_float3 _vExtents);
	void Set_Center(_float3 _vCenter);

private:
	class CBounding* m_pBounding = { nullptr };
	COLLIDER m_eColliderType = { COLLIDER::END };

	/* Intersect */
	_bool m_isColl = { false };

	/* Active */
	_bool m_bActive = { true };

	/* Hit */
	unordered_set<CCollider*> m_HitTargets;

	/* Block */
	_bool m_bBlocked = { false };

	/* Group */
	GROUP m_eGroup = GROUP::END;
	class CGameObject* m_pOwner = { nullptr };

	/* Collider Type */
	COLLIDERMODE m_eColliderMode = COLLIDERMODE::PHYSICAL;

public:
	static CCollider* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext,
		COLLIDER _eColliderType);
	virtual CComponent* Clone(void* _pArg) override;
	virtual void Free() override;

#ifdef  _DEBUG
private:
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
	BasicEffect* m_pEffect = { nullptr };
	ID3D11InputLayout* m_pInputLayout = { nullptr };

public:
	virtual HRESULT Render() override;
#endif //  _DEBUG

};
NS_END