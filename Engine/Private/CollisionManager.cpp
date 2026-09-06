#include "CollisionManager.h"
#include "GameObject.h"

CCollisionManager::CCollisionManager()
{
}

HRESULT CCollisionManager::Initialize()
{
    Set_CollisionGroup(GROUP::PLAYER, GROUP::MONSTER, true);
    Set_CollisionGroup(GROUP::MONSTER, GROUP::MONSTER, true);
    Set_CollisionGroup(GROUP::WEAPON_PLAYER, GROUP::MONSTER, true);
    Set_CollisionGroup(GROUP::SHIELD_MONSTER, GROUP::WEAPON_PLAYER, true);
    Set_CollisionGroup(GROUP::WEAPON_MONSTER, GROUP::PLAYER, true);
    Set_CollisionGroup(GROUP::SENSOR_MONSTER, GROUP::PLAYER, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::SHIELD_MONSTER, true);
    Set_CollisionGroup(GROUP::WEAPON_MONSTER, GROUP::SHIELD_PLAYER, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::MAP, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::NPC, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::TRIGGER, true);
    Set_CollisionGroup(GROUP::TRIGGER, GROUP::PLAYER, true);
    Set_CollisionGroup(GROUP::WEAPON_MONSTER, GROUP::MAP, true);
    Set_CollisionGroup(GROUP::SHIELD_PLAYER, GROUP::MONSTER, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::WIND, true);
    Set_CollisionGroup(GROUP::MONSTER, GROUP::MAP, true);
    Set_CollisionGroup(GROUP::MAP, GROUP::MONSTER, true);
    Set_CollisionGroup(GROUP::MAP_OBJECT, GROUP::WEAPON_PLAYER, true);
    Set_CollisionGroup(GROUP::ITEM, GROUP::PLAYER, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::ITEM, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::ATTACK_WINDOW_MONSTER, true);
    Set_CollisionGroup(GROUP::ATTACK_WINDOW_MONSTER, GROUP::PLAYER, true);
    Set_CollisionGroup(GROUP::PLAYER, GROUP::BOUNDARY, true);
    Set_CollisionGroup(GROUP::BOUNDARY, GROUP::PLAYER, true);

    Set_BlockGroup(GROUP::PLAYER, GROUP::MONSTER, true);
    Set_BlockGroup(GROUP::MONSTER, GROUP::PLAYER, true);
    Set_BlockGroup(GROUP::MONSTER, GROUP::MONSTER, true);
    Set_BlockGroup(GROUP::PLAYER, GROUP::MAP, false);
    Set_BlockGroup(GROUP::PLAYER, GROUP::NPC, true);
    Set_BlockGroup(GROUP::WEAPON_PLAYER, GROUP::MONSTER, false);
    Set_BlockGroup(GROUP::WEAPON_MONSTER, GROUP::PLAYER, false);
    Set_BlockGroup(GROUP::MONSTER, GROUP::MAP, true);
    Set_BlockGroup(GROUP::MAP, GROUP::MONSTER, true);
    Set_BlockGroup(GROUP::PLAYER, GROUP::MAP_OBJECT, true);
    Set_BlockGroup(GROUP::PLAYER, GROUP::BOUNDARY, true);
    Set_BlockGroup(GROUP::BOUNDARY, GROUP::PLAYER, true);

    m_iExitHysteresis = 1;

    return S_OK;
}

void CCollisionManager::Begin()
{
    m_CurrCollision.clear();
    
    /* Block */
    for (_int i = 0; i < ENUM_TO_UINT(GROUP::END); ++i)
    {
        for (auto Collider : m_Groups[i]) 
        {
            if (Collider)
                Collider->Set_Blocked(false);
        }
    }
}

void CCollisionManager::Tick()
{
    for (_int i = 0; i < ENUM_TO_UINT(GROUP::END); ++i)
    {
        for (_int j = i; j < ENUM_TO_UINT(GROUP::END); ++j) 
        {
            if (m_Groups[i].empty() || m_Groups[j].empty())
                continue;

            if (!m_Collisions[i][j])
                continue;

            Check_Group(m_Groups[i], m_Groups[j]);
        }
    }
}

void CCollisionManager::Flush()
{
    /* Enter / Stay */
    for (auto& Pair : m_CurrCollision)
    {
        _bool was = (m_PrevCollision.find(Pair) != m_PrevCollision.end());

        m_MissCount[Pair] = 0;

        CCollider* pA = Pair.pSrc;
        CCollider* pB = Pair.pDst;

        _bool isTrigger = pA->IsTrigger() || pB->IsTrigger();

        if (!was)
        {
            if (isTrigger)
            {
                pA->Get_Owner()->OnTriggerEnter(pA, pB);
                pB->Get_Owner()->OnTriggerEnter(pB, pA);
            }
            else
            {
                pA->Get_Owner()->OnCollisionEnter(pA, pB);
                pB->Get_Owner()->OnCollisionEnter(pB, pA);
            }
        }
        else
        {
            if (isTrigger)
            {
                pA->Get_Owner()->OnTriggerStay(pA, pB);
                pB->Get_Owner()->OnTriggerStay(pB, pA);
            }
            else
            {
                pA->Get_Owner()->OnCollisionStay(pA, pB);
                pB->Get_Owner()->OnCollisionStay(pB, pA);
            }
        }

    }

    /* Exit */
    for (auto& Pair : m_PrevCollision)
    {
        if (m_CurrCollision.find(Pair) != m_CurrCollision.end())
            continue;

        _int& miss = m_MissCount[Pair];
        miss++;

        if (miss >= m_iExitHysteresis)
        {
            CCollider* pA = Pair.pSrc;
            CCollider* pB = Pair.pDst;

            _bool isTrigger = pA->IsTrigger() || pB->IsTrigger();

            if (isTrigger)
            {
                pA->Get_Owner()->OnTriggerExit(pA, pB);
                pB->Get_Owner()->OnTriggerExit(pB, pA);
            }
            else
            {
                pA->Get_Owner()->OnCollisionExit(pA, pB);
                pB->Get_Owner()->OnCollisionExit(pB, pA);
            }

            m_MissCount.erase(Pair);
        }
    }

    m_PrevCollision = m_CurrCollision;
}

void CCollisionManager::Set_CollisionGroup(GROUP _eA, GROUP _eB, _bool _bEnable)
{
    m_Collisions[ENUM_TO_UINT(_eA)][ENUM_TO_UINT(_eB)] = _bEnable;
    m_Collisions[ENUM_TO_UINT(_eB)][ENUM_TO_UINT(_eA)] = _bEnable;
}

void CCollisionManager::Set_BlockGroup(GROUP _eA, GROUP _eB, _bool _bEnable)
{
    m_Blocks[ENUM_TO_UINT(_eA)][ENUM_TO_UINT(_eB)] = _bEnable;
    m_Blocks[ENUM_TO_UINT(_eB)][ENUM_TO_UINT(_eA)] = _bEnable;
}

void CCollisionManager::Remove_Collider(CCollider* _pCollider)
{
    if (!_pCollider)
        return;

    auto& vecGroups = m_Groups[ENUM_TO_UINT(_pCollider->Get_Group())];
    vecGroups.erase(remove(vecGroups.begin(), vecGroups.end(), _pCollider), vecGroups.end());
    
    Unregister(_pCollider);
}

void CCollisionManager::Register(CCollider* _pCollider)
{
    if (!_pCollider)
        return;

    m_Groups[ENUM_TO_UINT(_pCollider->Get_Group())].push_back(_pCollider);

    wchar_t buf[128];
    swprintf_s(buf, L"[Register] group=%d owner=%p collider=%p\n",
        (int)_pCollider->Get_Group(), _pCollider->Get_Owner(), _pCollider);
    OutputDebugString(buf);
}

void CCollisionManager::Unregister(CCollider* _pCollider)
{
    if (!_pCollider)
        return;

    /* Current */
    for (auto it = m_CurrCollision.begin(); it != m_CurrCollision.end(); )
    {
        if (it->pSrc == _pCollider || it->pDst == _pCollider)
            it = m_CurrCollision.erase(it);
        else
            ++it;
    }

    /* Prev */
    for (auto it = m_PrevCollision.begin(); it != m_PrevCollision.end(); )
    {
        if (it->pSrc == _pCollider || it->pDst == _pCollider)
            it = m_PrevCollision.erase(it);
        else
            ++it;
    }

    /* Miss */
    for (auto it = m_MissCount.begin(); it != m_MissCount.end(); )
    {
        if (it->first.pSrc == _pCollider || it->first.pDst == _pCollider)
            it = m_MissCount.erase(it);
        else
            ++it;
    }
}

void CCollisionManager::Reset_All(_bool _bResetConfig)
{
    OutputDebugString(L"[Collision] Reset_All called\n");

    for (int i = 0; i < ENUM_TO_UINT(GROUP::END); ++i)
    {
        /* Excep. Player */
        if (i >= ENUM_TO_UINT(GROUP::PLAYER) &&
            i <= ENUM_TO_UINT(GROUP::SHIELD_PLAYER))
        {
            continue;
        }

        m_Groups[i].clear();
    }

    m_CurrCollision.clear();
    m_PrevCollision.clear();
    m_MissCount.clear();

    if (_bResetConfig)
        m_iExitHysteresis = 1;
}

void CCollisionManager::Set_ExitrHysteresis(_int _frames)
{
    m_iExitHysteresis = (_frames < 0) ? 0 : _frames;
}

_bool CCollisionManager::Raycast_GroundPlane(_fvector _vStart, _fvector _vEnd, _float _fGroundY)
{
    _float fY0 = XMVectorGetY(_vStart);
    _float fY1 = XMVectorGetY(_vEnd);

    if (fabsf(fY0 - fY1) < 1e-6f)
        return false;

    _bool bCrosses = (fY0 >= _fGroundY && fY1 <= _fGroundY) || (fY0 <= _fGroundY && fY1 >= _fGroundY);
    if (!bCrosses)
        return false;

    _float t = (_fGroundY - fY0) / (fY1 - fY0);

    if (t < 0.f || t > 1.f)
        return false;

    return true;
}

_bool CCollisionManager::Raycast_GroundPlane(_fvector _vStart, _fvector _vEnd, _float3& _outHitPos, _float _fGroundY)
{
    _float fY0 = XMVectorGetY(_vStart);
    _float fY1 = XMVectorGetY(_vEnd);

    if (fabsf(fY0 - fY1) < 1e-6f)
        return false;

    _bool bCrosses = (fY0 >= _fGroundY && fY1 <= _fGroundY) || (fY0 <= _fGroundY && fY1 >= _fGroundY);
    if (!bCrosses)
        return false;

    _float t = (_fGroundY - fY0) / (fY1 - fY0);

    if (t < 0.f || t > 1.f)
        return false;

    _vector vHit = _vStart + (_vEnd - _vStart) * t;
    XMStoreFloat3(&_outHitPos, vHit);
    _outHitPos.y = _fGroundY;

    return true;
}

_bool CCollisionManager::Check_Block(CCollider* _pCollider, _vector* _outNormalXZ, _float* _outDepth)
{
    if (!_pCollider)
        return false;

    _int myGroup = ENUM_TO_UINT(_pCollider->Get_Group());
    CTransform* pMyTransform = static_cast<CTransform*>(_pCollider->Get_Owner()->Get_Component(TEXT("Com_Transform")));
    _vector myPos = pMyTransform->Get_State(STATE::POSITION);

    for (_int i = 0; i < ENUM_TO_UINT(GROUP::END); ++i)
    {
        if (!m_Collisions[myGroup][i])
            continue;
        if (!m_Blocks[myGroup][i])
            continue;

        auto& vecGroup = m_Groups[i];
        if (vecGroup.empty())
            continue;

        for (auto Other : vecGroup)
        {
            if (!Other || Other == _pCollider)
                continue;
            if (!Other->IsActive())
                continue;

            // 같은 소유자
            if (_pCollider->Get_Owner() == Other->Get_Owner())
                continue;

            if (_pCollider->Intersect(Other))
            {
                CTransform* pOtherTransform = static_cast<CTransform*>(Other->Get_Owner()->Get_Component(TEXT("Com_Transform")));
                _vector otherPos = pOtherTransform->Get_State(STATE::POSITION);
                _vector myPos = pMyTransform->Get_State(STATE::POSITION);

                _vector vNormal = XMVectorZero();
                if (Other->Get_Group() == GROUP::MAP || Other->Get_Group() == GROUP::BOUNDARY)
                    vNormal = ComputeNormal_Axis(myPos, otherPos);
                else
                    vNormal = ComputeNormal_Center(myPos, otherPos);

                if (_outNormalXZ)
                    *_outNormalXZ = vNormal;

                if (_outDepth)
                {
                    _float fDist = XMVectorGetX(XMVector3Length(myPos - otherPos));
                    _float fMinDist = 0.6f;
                    *_outDepth = max(0.f, fMinDist - fDist );
                }
                return true;
            }
        }
    }

    return false;
}

void CCollisionManager::Check_Group(vector<CCollider*>& _vecA, vector<CCollider*>& _vecB)
{
    for (size_t i = 0; i < _vecA.size(); ++i)
    {
        _int GroupBStart = 0;
        if (&_vecA == &_vecB)
            GroupBStart = i + 1;

        CCollider* A = _vecA[i];
        if (!A) continue;

        for (size_t j = GroupBStart; j < _vecB.size(); ++j)
        {
            CCollider* B = _vecB[j];
            if (!B || A == B)
                continue;
            
            /* 비활성화 오브젝트 건뛰 */
            if (!A->IsActive() || !B->IsActive())
                continue;

            /* 본인 소유 Parts 건뛰 */
            if (A->Get_Owner() == B->Get_Owner())
                continue;

            if (A->Intersect(B))
            {
                PairKey key = (A < B) ?
                    PairKey{ A, B } : PairKey{ B, A };

                m_CurrCollision.insert(key);

                //OutputDebugString(L"[COLLISION DETECTED]\n");
            }
        }
    }
}

_vector CCollisionManager::ComputeNormal_Axis(_vector _MyPos, _vector _OtherPos)
{
    _float dx = XMVectorGetX(_MyPos) - XMVectorGetX(_OtherPos);
    _float dz = XMVectorGetZ(_MyPos) - XMVectorGetZ(_OtherPos);

    if (fabs(dx) > fabs(dz))
        return XMVectorSet((dx > 0.f) ? 1.f : -1.f, 0.f, 0.f, 0.f);
    else
        return XMVectorSet(0.f, 0.f, (dz > 0.f) ? 1.f : -1.f, 0.f);
}

_vector CCollisionManager::ComputeNormal_Center(_vector _MyPos, _vector _OtherPos)
{
    _vector vNormal = _MyPos - _OtherPos;
    vNormal = XMVectorSetY(vNormal, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vNormal)) < 0.00001f)
        vNormal = XMVectorSet(1.f, 0.f, 0.f, 0.f);

    return XMVector3Normalize(vNormal);
}

CCollisionManager* CCollisionManager::Create()
{
    CCollisionManager* pInstance = new CCollisionManager();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CollisionManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCollisionManager::Free()
{
    __super::Free();
}
