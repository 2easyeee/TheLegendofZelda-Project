#pragma once
#include "Base.h"
#include "Collider.h"

class CCollider;

NS_BEGIN(Engine)
class ENGINE_DLL CCollisionManager final : public CBase
{
private:
	struct PairKey
	{
		CCollider* pDst;
		CCollider* pSrc;

		bool operator==(const PairKey& o) const noexcept
		{
			return (pDst == o.pDst) && (pSrc == o.pSrc);
		}
	};

	struct PairHash
	{
		size_t operator()(const PairKey& k) const noexcept
		{
			auto a = reinterpret_cast<uintptr_t>(k.pDst);
			auto b = reinterpret_cast<uintptr_t>(k.pSrc);
			return (a >> 4) ^ (b << 7);
		}
	};

private:
	CCollisionManager();
	virtual ~CCollisionManager() = default;

public:
	HRESULT Initialize();
	
	/* Collision (Loop) */
	void Begin();
	void Tick();
	void Flush();

	void Set_CollisionGroup(GROUP _eA, GROUP _eB, _bool _bEnable);
	void Set_BlockGroup(GROUP _eA, GROUP _eB, _bool _bEnable);

	void Remove_Collider(class CCollider* _pCollider);

	void Register(CCollider* _pCollider);
	void Unregister(CCollider* _pCollider);
	void Reset_All(_bool _bResetConfig = false);

	void Set_ExitrHysteresis(_int _frames);

	/* Raycast */
	_bool Raycast_GroundPlane(_fvector _vStart, _fvector _vEnd, _float _fGroundY = 0.f);
	_bool Raycast_GroundPlane(_fvector _vStart, _fvector _vEnd, _float3& _outHitPos, _float _fGroundY = 0.f);

	/* Block */
	_bool Check_Block(CCollider* _pCollider, _vector* _outNormalXZ = nullptr, _float* _outDepth = nullptr);

private:
	/* Collision */
	vector<class CCollider*> m_Groups[ENUM_TO_UINT(GROUP::END)];
	_bool m_Collisions[ENUM_TO_UINT(GROUP::END)][ENUM_TO_UINT(GROUP::END)] = {};
	_bool m_Blocks[ENUM_TO_UINT(GROUP::END)][ENUM_TO_UINT(GROUP::END)] = {};

	unordered_set<PairKey, PairHash>		m_CurrCollision;
	unordered_set<PairKey, PairHash>		m_PrevCollision;
	unordered_map<PairKey, _int, PairHash>	m_MissCount;

	_int m_iExitHysteresis = { 1 };

private:
	/* Collision */
	void Check_Group(vector<CCollider*>& _vecA, vector<CCollider*>& _vecB);
	_vector ComputeNormal_Axis(_vector _MyPos, _vector _OtherPos);
	_vector ComputeNormal_Center(_vector _MyPos, _vector _OtherPos);

public:
	static CCollisionManager* Create();
	virtual void Free() override;
};
NS_END