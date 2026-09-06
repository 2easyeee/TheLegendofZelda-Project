#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "Bone.h"
#include "Animation.h"
#include "Channel.h"
#include "Shader.h"

CModel::CModel(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
    : CComponent{ _pDevice, _pDeviceContext }
{
}

CModel::CModel(const CModel& _Prototype)
    : CComponent(_Prototype)
    , m_iNumMeshes{ _Prototype.m_iNumMeshes }
    , m_iNumMaterials{ _Prototype.m_iNumMaterials }
    , m_PreTransformMatrix { _Prototype.m_PreTransformMatrix }
    , m_iNumBones {_Prototype.m_iNumBones }
    , m_eModelType { _Prototype.m_eModelType }
    , m_AnimNameToIndex { _Prototype.m_AnimNameToIndex }
    , m_BoneNameCache { _Prototype.m_BoneNameCache }
    , m_BoneOverrides{ _Prototype.m_BoneOverrides }
    , m_BoneSnaps { _Prototype.m_BoneSnaps }
{
    /* Bone (Deep_Copy) */
    for (auto& pPrototypeBone : _Prototype.m_vecBones)
        m_vecBones.push_back(pPrototypeBone->Deep_Clone());

    /* Animation (Deep_Copy) */
    for (auto& pPrototypeAnimation : _Prototype.m_vecAnimations)
        m_vecAnimations.push_back(pPrototypeAnimation->Deep_Clone());

    /* Mesh */
    m_vecMeshes.reserve(_Prototype.m_vecMeshes.size());
    for (auto* pMesh : _Prototype.m_vecMeshes)
    {
        Safe_AddRef(pMesh);
        m_vecMeshes.push_back(pMesh);
    }

    /* Material */
    m_vecMaterials.reserve(_Prototype.m_vecMaterials.size());
    for (auto* pMaterial : _Prototype.m_vecMaterials)
    {
        Safe_AddRef(pMaterial);
        m_vecMaterials.push_back(pMaterial);
    }
}

HRESULT CModel::Initialize_Prototype(const _char* _pModelFilePath, _fmatrix _PreTransformMatrix)
{
    {
        ///* 사전 변환 행렬 */
        //XMStoreFloat4x4(&m_PreTransformMatrix, _PreTransformMatrix);

        //m_strMeshPath = _pModelFilePath;
        //// 같은 폴더/이름의 .xml을 material 로 사용
        //filesystem::path path = filesystem::path(_pModelFilePath);
        //path.replace_extension(".xml");
        //m_strMaterialPath = path.string();

        //HANDLE hFile = CreateFileA(m_strMeshPath.c_str(),
        //    GENERIC_READ, FILE_SHARE_READ,
        //    nullptr, OPEN_EXISTING,
        //    FILE_ATTRIBUTE_NORMAL, nullptr);
        //if (hFile == INVALID_HANDLE_VALUE)
        //    return E_FAIL;

        ///* Bone */
        //if (FAILED(Ready_Bones(hFile)))
        //    return E_FAIL;

        ///* Mesh */
        //if (FAILED(Ready_Meshes(hFile)))
        //    return E_FAIL;

        ///* Animation */
        //if (FAILED(Ready_Animations(hFile)))
        //    return E_FAIL;

        //CloseHandle(hFile);

        ///* Material */
        //if (FAILED(Ready_Materials()))
        //    return E_FAIL;
    }

    if (FAILED(Initialize_CPU(_pModelFilePath, _PreTransformMatrix)))
        return E_FAIL;

    if (FAILED(Initialize_GPU()))
        return E_FAIL;

    return S_OK;
}

HRESULT CModel::Initialize(void* _pArg)
{
    return S_OK;
}

HRESULT CModel::Render(_uint _iMeshIndex)
{
    if (_iMeshIndex >= m_iNumMeshes)
        return E_FAIL;

    /* RenderMask */
    {
        CMesh* pMesh = m_vecMeshes[_iMeshIndex];
        if (!pMesh)
            return E_FAIL;

        //_string meshName = pMesh->Get_MeshName();
        _string meshName = pMesh->Get_MaterialName();

        auto iter = m_MeshRenderMask.find(meshName);
        if (iter != m_MeshRenderMask.end())
        {
            if (iter->second == false)
                return S_OK;
        }
    }

    m_vecMeshes[_iMeshIndex]->Bind_Resources();
    m_vecMeshes[_iMeshIndex]->Render();

    return S_OK;
}

CMesh* CModel::Get_Mesh(_uint _iMeshIndex) const
{
    if (_iMeshIndex >= m_vecMeshes.size())
        return nullptr;

    return m_vecMeshes[_iMeshIndex];
}

_int CModel::Get_BoneIndex(const _char* _pBoneName)
{
    _int iBoneIndex = {};
    auto iter = find_if(m_vecBones.begin(), m_vecBones.end(),
        [&](CBone* pBone)->_bool {
            if (true == pBone->Compare_Name(_pBoneName))
                return true;
            ++iBoneIndex;

            return false;
        });
    if (iter == m_vecBones.end())
        return -1;

    return iBoneIndex;
}

_uint CModel::Get_IndexCount(_uint _iMeshIndex) const
{
    if (_iMeshIndex >= m_vecMeshes.size())
        return 0;

    return m_vecMeshes[_iMeshIndex]->Get_IndexCount();
}

void CModel::Set_MeshActive(const _string& _meshName, _bool _bActive)
{   
    m_MeshRenderMask[_meshName] = _bActive;
}

void CModel::Set_AllMeshActive(_bool _bActive)
{
}

HRESULT CModel::Build_StaticBatches()
{
    if (m_vecMeshes.size() <= 1)
        return S_FALSE;

    map<_uint, MESH_DESC> BatcheDescs;

    for (CMesh* pMesh : m_vecMeshes)
    {
        if (nullptr == pMesh)
            continue;

        const _uint iMaterialIndex = pMesh->Get_MaterialIndex();
        MESH_DESC& BatchDesc = BatcheDescs[iMaterialIndex];

        if (BatchDesc.meshName.empty())
        {
            BatchDesc.meshName = "StaticBatch_" + to_string(iMaterialIndex);
            BatchDesc.materialName = pMesh->Get_MaterialName();
            BatchDesc.eModelType = MODEL::NONANIM;
        }

        const vector<_uint>& Indices = pMesh->Get_IndicesRef();

        if (Indices.empty())
            continue;

        const _uint iBaseVertex = static_cast<_uint>(BatchDesc.nonAnimVertices.size());

        if (pMesh->Get_ModelType() == MODEL::NONANIM)
        {
            const vector<VTXMESH>& Vertices = pMesh->Get_NonAnimVerticesRef();
            
            if (Vertices.empty())
                continue;

            if (BatchDesc.nonAnimVertices.size() > static_cast<size_t>(UINT_MAX) - Vertices.size())
                return E_FAIL;

            BatchDesc.nonAnimVertices.insert(BatchDesc.nonAnimVertices.end(), Vertices.begin(), Vertices.end());
        }
        else
        {
            const vector<VTXANIMMESH>& Vertices = pMesh->Get_SkinnedVerticesRef();

            if (Vertices.empty())
                continue;

            if (BatchDesc.nonAnimVertices.size() > static_cast<size_t>(UINT_MAX) - Vertices.size())
                return E_FAIL;

            BatchDesc.nonAnimVertices.reserve(BatchDesc.nonAnimVertices.size() + Vertices.size());

            for (const VTXANIMMESH& SourceVertex : Vertices)
            {
                VTXMESH StaticVertex = {};

                StaticVertex.vPosition = SourceVertex.vPosition;
                StaticVertex.vNormal = SourceVertex.vNormal;
                StaticVertex.vTexcoord = SourceVertex.vTexcoord;
                StaticVertex.vTangent = SourceVertex.vTangent;
                StaticVertex.vBinormal = SourceVertex.vBinormal;

                BatchDesc.nonAnimVertices.push_back(StaticVertex);
            }
        }

        BatchDesc.commonIndices.reserve(BatchDesc.commonIndices.size() + Indices.size());

        for (const _uint iIndex : Indices)
        {
            if (iIndex > UINT_MAX - iBaseVertex)
                return E_FAIL;

            BatchDesc.commonIndices.push_back(
                iBaseVertex + iIndex);
        }
    }

    vector<CMesh*> BatchedMeshes;
    BatchedMeshes.reserve(BatcheDescs.size());

    for (auto& Pair : BatcheDescs)
    {
        const _uint iMaterialIndex = Pair.first;
        MESH_DESC& BatchDesc = Pair.second;

        if (BatchDesc.nonAnimVertices.empty() || BatchDesc.commonIndices.empty())
            continue;

        CMesh* pBatchMesh = CMesh::Create(m_pDevice, m_pDeviceContext, BatchDesc, XMMatrixIdentity());
        if (!pBatchMesh)
        {
            for (CMesh* pCreatedMesh : BatchedMeshes)
                Safe_Release(pCreatedMesh);

            return E_FAIL;
        }

        pBatchMesh->Set_MaterialIndex(iMaterialIndex);
        BatchedMeshes.push_back(pBatchMesh);
    }

    if (BatchedMeshes.empty())
        return S_FALSE;

    for (CMesh* pMesh : m_vecMeshes)
        Safe_Release(pMesh);

    m_vecMeshes = move(BatchedMeshes);
    m_iNumMeshes = static_cast<_uint>(m_vecMeshes.size());

    m_MeshRenderMask.clear();

    for (CMesh* pMesh : m_vecMeshes)
    {
        if (!pMesh)
            continue;

        m_MeshRenderMask[pMesh->Get_MaterialName()] = true;
    }

    return S_OK;
}

const _float4x4* CModel::Get_BoneMatrixPtr(const _char* _pBoneName)
{
    _int iBoneIndex = Get_BoneIndex(_pBoneName);
    if (-1 == iBoneIndex)
    {
        MSG_BOX("Failed to Find : CBone");
        return nullptr;
    }
    return m_vecBones[iBoneIndex]->Get_CombinedTransformMatrixPrt();
}

void CModel::Set_BoneOffset(const _char* _BoneName, _fmatrix _offset)
{
    m_BoneOverrides[_BoneName] = _offset;
}

void CModel::Clear_BoneOffset()
{
    m_BoneOverrides.clear();
}

void CModel::Remove_BoneOffset(const _char* _BoneName)
{
    m_BoneOverrides.erase(_BoneName);
}

void CModel::Set_BoneSnap(const _char* _TargetBoneName, const _char* _SourceBoneName)
{
    auto iterTarget = m_BoneNameCache.find(_TargetBoneName);
    auto iterSource = m_BoneNameCache.find(_SourceBoneName);

    if (iterTarget == m_BoneNameCache.end() || iterSource == m_BoneNameCache.end())
        return;

    _int iTargetIndex = iterTarget->second;
    _int iSourceIndex = iterSource->second;

    m_BoneSnaps[iTargetIndex] = iSourceIndex;
}

void CModel::Remove_BoneSnap(const _char* _TargetBoneName)
{
    auto iterTarget = m_BoneNameCache.find(_TargetBoneName);
    if (iterTarget == m_BoneNameCache.end())
        return;

    m_BoneSnaps.erase(iterTarget->second);
}

void CModel::Clear_BoneSnap()
{
    m_BoneSnaps.clear();
}

HRESULT CModel::Set_Animation_Name(_string _AnimName, _bool _isLoop, _bool _isForce)
{
    if (_AnimName.empty())
    {
        Set_Animation(0, _isLoop);
        return S_OK;
    }

    auto iter = m_AnimNameToIndex.find(_AnimName);
    if (iter == m_AnimNameToIndex.end())
        return E_INVALIDARG;

    Set_Animation(iter->second, _isLoop, _isForce);
    return S_OK;
}

void CModel::Set_Animation(_uint _iAnimIndex, _bool _isLoop, _bool _isForce)
{
    /* 1. 범위 */
    if (_iAnimIndex >= m_vecAnimations.size())
        return;

    /* 2. 같은 Anim 이면, Loop 만 갱신 */
    if (!_isForce && m_iCurrentAnimIndex == _iAnimIndex)
    {
        m_isAnimLoop = _isLoop;
        return;
    }

    ///* 3. Lerp ing... */
    //if (m_tAnimLerp.bActive)
    //    return;

    if (_isForce)
    {
        m_iCurrentAnimIndex = _iAnimIndex;
        m_iNextAnimIndex = _iAnimIndex;
        m_isAnimLoop = _isLoop;

        m_tAnimLerp.bActive = false;

        m_vecAnimations[m_iCurrentAnimIndex]->Reset_TrackPosition(true);
        return;
    }

    m_iNextAnimIndex = _iAnimIndex;
    m_isAnimLoop = _isLoop;

    m_tAnimLerp.bActive = true;
    m_tAnimLerp.fAccTime = 0.f;
}

void CModel::Reset_Animation_TrackPosition()
{
    if (m_iCurrentAnimIndex < 0 || m_iCurrentAnimIndex >= m_vecAnimations.size())
        return;

    CAnimation* pAnimation = m_vecAnimations[m_iCurrentAnimIndex];
    if (!pAnimation)
        return;

    pAnimation->Reset_TrackPosition(true);
}

void CModel::Set_AnimationSpeedMulti(_float _fAnimSpeedMulti)
{
    if (m_iCurrentAnimIndex < m_vecAnimations.size())
        m_vecAnimations[m_iCurrentAnimIndex]->Set_AnimSpeedMulti(_fAnimSpeedMulti);

    if (m_tAnimLerp.bActive && m_iNextAnimIndex < m_vecAnimations.size())
        m_vecAnimations[m_iNextAnimIndex]->Set_AnimSpeedMulti(_fAnimSpeedMulti);;
}

_bool CModel::IsAnimFinished() const
{
    if (m_iCurrentAnimIndex >= m_vecAnimations.size())
        return false;

    return m_vecAnimations[m_iCurrentAnimIndex]->IsAnimFinished();
}

_float CModel::Get_PlayRatio() const
{
    if (m_iCurrentAnimIndex < 0 || m_iCurrentAnimIndex >= m_vecAnimations.size())
        return 0.f;

    CAnimation* pAnim = m_vecAnimations[m_iCurrentAnimIndex];
    if (!pAnim)
        return 0.f;

    return m_vecAnimations[m_iCurrentAnimIndex]->Get_PlayRatio();
}

HRESULT CModel::Bind_BoneMatrices(CShader* _pShader, const _char* _pConstantName, _uint _iMeshIndex)
{
    if (m_eModelType == MODEL::NONANIM)
        return S_OK;

    /* SKINNED */
    CMesh* pMesh = m_vecMeshes[_iMeshIndex];
    if (!pMesh)
        return E_FAIL;
    return m_vecMeshes[_iMeshIndex]->Bind_BoneMatrices(_pShader, _pConstantName, m_vecBones);
}

HRESULT CModel::Bind_Material(CShader* _pShader, const _char* _pConstantName, _uint _iMeshIndex, MATERIAL _eMaterialType,_uint _iTextureIndex)
{
    if (_iMeshIndex >= m_iNumMeshes)
        return E_FAIL;

    _uint iMaterialIndex = m_vecMeshes[_iMeshIndex]->Get_MaterialIndex();
    if (+iMaterialIndex >= m_iNumMaterials)
        return E_FAIL;

    return m_vecMaterials[iMaterialIndex]->Bind_Material(_pShader, _pConstantName, _eMaterialType, _iTextureIndex);
}

HRESULT CModel::Bind_Resources(_uint _iMeshIndex)
{
    if (_iMeshIndex >= m_vecMeshes.size())
        return E_FAIL;

    return m_vecMeshes[_iMeshIndex]->Bind_Resources();
}

_bool CModel::Play_Animation(_float _fTimeDelta)
{
    _bool isFinished = true;
      
    if (m_tAnimLerp.bActive)
    {
        /* 애니메이션 있을 때 */
        Lerp_Animation(_fTimeDelta);
    }
    else
    {
        /* 애니메이션이 없을 때 */
        if (!m_vecAnimations.empty() && m_iCurrentAnimIndex >= 0)
            _bool isFinished = m_vecAnimations[m_iCurrentAnimIndex]->Update_TransformationMatrices(m_isAnimLoop, _fTimeDelta, m_vecBones);
    }

    Update_BonesOverrides();
    Update_BoneSnap();
    Update_Bones();
    return isFinished;
}

/* 모델 파일을 읽고 본, 메시, 애니메이션 데이터를 준비, D3D11 리소스를 생성하지 않는다 */
HRESULT CModel::Initialize_CPU(const _char* _pModelFilePath, _fmatrix _PreTransformMatrix)
{
    if (!_pModelFilePath)
        return E_INVALIDARG;

    /* 사전 변환 행렬 */
    XMStoreFloat4x4(&m_PreTransformMatrix, _PreTransformMatrix);

    m_strMeshPath = _pModelFilePath;
    // 같은 폴더/이름의 .xml을 material 로 사용
    filesystem::path path = filesystem::path(_pModelFilePath);
    path.replace_extension(".xml");
    m_strMaterialPath = path.string();

    HANDLE hFile = CreateFileA(m_strMeshPath.c_str(),
        GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    HRESULT hrResult = S_OK;

    
    if (FAILED(Ready_Bones(hFile)))
    {
        /* Bone */
        hrResult = E_FAIL;
    }
    else if (FAILED(Ready_MeshData(hFile)))
    {
        /* Mesh */
        hrResult = E_FAIL;
    }
    else if (FAILED(Ready_Animations(hFile)))
    {
        /* Animation */
        hrResult = E_FAIL;
    }    

    CloseHandle(hFile);

    return hrResult;
}

/* CPU 준비 결과를 사용하여 메시 버퍼와 텍스쳐 생성 */
HRESULT CModel::Initialize_GPU()
{
    /* Worker 가 준비한 CPU 메시 데이터로 GPU 버퍼를 생성, GPU 리소스 생성 단계는 현재 메인 스레드에서만 호출 */
    if (FAILED(Create_MeshResources()))
        return E_FAIL;

    /* Material */
    if (FAILED(Ready_Materials()))
        return E_FAIL;

    return S_OK;
}

HRESULT CModel::Ready_Bones(HANDLE hFile)
{
    /* 1. Bone */
    _uint iBoneCount = 0;
    ReadRaw(hFile, &iBoneCount, sizeof(_uint));

    m_iNumBones = iBoneCount;
    m_vecBoneData.resize(iBoneCount);

    for (_uint i = 0; i < iBoneCount; ++i)
    {
        _string nodeName;
        _int iNodeIndex = -1;
        _int iParentNodeIndex = -1;
        _float4x4 localTransform;

        nodeName = ReadString(hFile);
        ReadRaw(hFile, &iNodeIndex, sizeof(_int));
        ReadRaw(hFile, &iParentNodeIndex, sizeof(_int));
        ReadRaw(hFile, &localTransform, sizeof(_float4x4));

        BONE_DESC& tBoneDesc = m_vecBoneData[i];
        tBoneDesc.nodeName = nodeName;
        tBoneDesc.nodeIndex = iNodeIndex;
        tBoneDesc.parentNodeIndex = iParentNodeIndex;
        tBoneDesc.localTransform = localTransform;
        tBoneDesc.children.clear();
    }

    /* Children */
    for (_uint i = 0; i < iBoneCount; ++i)
    {
        _int parentNodeIndex = m_vecBoneData[i].parentNodeIndex;
        if (parentNodeIndex >= 0 && parentNodeIndex < (_int)iBoneCount)
            m_vecBoneData[parentNodeIndex].children.push_back((_int)i);
    }

    /* CBone */
    m_vecBones.resize(iBoneCount);
    for (_uint i = 0; i < iBoneCount; ++i)
    {
        CBone* pBone = CBone::Create(m_vecBoneData[i].nodeName, m_vecBoneData[i].parentNodeIndex, m_vecBoneData[i].localTransform);
        if (!pBone)
            return E_FAIL;

        m_vecBones[i] = pBone;
        m_BoneNameCache[m_vecBones[i]->Get_Name()] = i;
    }

    /* Parent <-> Child */
    //for (_uint i = 0; i < m_iNumBones; ++i)
    //{
    //    _int iParentIndex = m_vecBoneData[i].parentNodeIndex;
    //    if (iParentIndex != -1)
    //    {
    //        CBone* pChild = m_vecBones[i];
    //        CBone* pParent = m_vecBones[iParentIndex];

    //        pChild->Set_Parent(pParent);
    //        pParent->Add_Child(pChild);
    //    }
    //}

    return S_OK;
}

HRESULT CModel::Ready_Meshes(HANDLE hFile)
{
    /* 2. Mesh */
    _uint iMeshCount = 0;
    ReadRaw(hFile, &iMeshCount, sizeof(_uint));
    m_iNumMeshes = iMeshCount;
    m_vecMeshes.reserve(iMeshCount);

    vector<MESH_DESC> vecMesheDesc;
    vecMesheDesc.reserve(iMeshCount);

    for (_uint i = 0; i < iMeshCount; ++i)
    {
        MESH_DESC tMeshDesc = {};
        tMeshDesc.meshName = ReadString(hFile);
        tMeshDesc.materialName = ReadString(hFile);
        ReadRaw(hFile, &tMeshDesc.eModelType, sizeof(MODEL));
        /* BindBoneMatrices 용 */
        m_eModelType = tMeshDesc.eModelType;

        /* Vertex */
        _uint iVertexCount = 0;
        ReadRaw(hFile, &iVertexCount, sizeof(_uint));
        if (tMeshDesc.eModelType == MODEL::NONANIM)
        {
            /* NONANIM */
            tMeshDesc.nonAnimVertices.resize(iVertexCount);
            if (iVertexCount > 0)
                ReadRaw(hFile, tMeshDesc.nonAnimVertices.data(), sizeof(VTXMESH) * iVertexCount);
        }
        else
        {
            /* SKINNED */
            tMeshDesc.skinnedVertices.resize(iVertexCount);
            if (iVertexCount > 0)
                ReadRaw(hFile, tMeshDesc.skinnedVertices.data(), sizeof(VTXANIMMESH) * iVertexCount);

            /* Excep. Z-Fighting Fix */
            if (tMeshDesc.materialName == "MI_f_grass_a_03" ||
                tMeshDesc.materialName == "MI_d3_f_floor_h_01")
            {
                for (auto& vtx : tMeshDesc.skinnedVertices)
                    vtx.vPosition.y += 0.001f;
            }

            /* BoneIndex, BoneWeight */
            ReadRaw(hFile, &tMeshDesc.effectTotalBoneCnt, sizeof(_uint));
            
            _uint iEffectBoneIndices = 0;
            ReadRaw(hFile, &iEffectBoneIndices, sizeof(_uint));
            tMeshDesc.effectBoneIndices.resize(iEffectBoneIndices);
            if (iEffectBoneIndices > 0)
                ReadRaw(hFile, tMeshDesc.effectBoneIndices.data(), sizeof(_int) * iEffectBoneIndices);

            _uint iOffsetMatrices = 0;
            ReadRaw(hFile, &iOffsetMatrices, sizeof(_uint));
            tMeshDesc.offsetMatrices.resize(iOffsetMatrices);
            if (iOffsetMatrices > 0)
                ReadRaw(hFile, tMeshDesc.offsetMatrices.data(), sizeof(_float4x4) * iOffsetMatrices);
        }

        /* INDEX */
        _uint iIndexCount = 0;
        ReadRaw(hFile, &iIndexCount, sizeof(_uint));
        tMeshDesc.commonIndices.resize(iIndexCount);
        if (iIndexCount > 0)
            ReadRaw(hFile, tMeshDesc.commonIndices.data(), sizeof(_uint) * iIndexCount);

        vecMesheDesc.push_back(move(tMeshDesc));
    }

    _matrix preTransformMatrix = XMLoadFloat4x4(&m_PreTransformMatrix);

    for (auto& MeshDesc : vecMesheDesc)
    {
        CMesh* pMesh = CMesh::Create(m_pDevice, m_pDeviceContext, MeshDesc, preTransformMatrix);
        if (!pMesh)
            return E_FAIL;

        m_vecMeshes.push_back(pMesh);
    }
    m_iNumMeshes = static_cast<_uint>(m_vecMeshes.size());

    /* Mesh Mask */
    {
        for (auto& pMesh : m_vecMeshes)
        {
            if (!pMesh)
                continue;

            m_MeshRenderMask[pMesh->Get_MaterialName()] = true;
        }
    }

    return S_OK;
}

HRESULT CModel::Ready_MeshData(HANDLE hFile)
{
    /* 2. Mesh */
    _uint iMeshCount = 0;
    ReadRaw(hFile, &iMeshCount, sizeof(_uint));
    m_iNumMeshes = iMeshCount;
    m_vecMeshDesc.clear();
    m_vecMeshDesc.reserve(iMeshCount);

    for (_uint i = 0; i < iMeshCount; ++i)
    {
        MESH_DESC tMeshDesc = {};
        tMeshDesc.meshName = ReadString(hFile);
        tMeshDesc.materialName = ReadString(hFile);
        ReadRaw(hFile, &tMeshDesc.eModelType, sizeof(MODEL));
        /* BindBoneMatrices 용 */
        m_eModelType = tMeshDesc.eModelType;

        /* Vertex */
        _uint iVertexCount = 0;
        ReadRaw(hFile, &iVertexCount, sizeof(_uint));
        if (tMeshDesc.eModelType == MODEL::NONANIM)
        {
            /* NONANIM */
            tMeshDesc.nonAnimVertices.resize(iVertexCount);
            if (iVertexCount > 0)
                ReadRaw(hFile, tMeshDesc.nonAnimVertices.data(), sizeof(VTXMESH) * iVertexCount);
        }
        else
        {
            /* SKINNED */
            tMeshDesc.skinnedVertices.resize(iVertexCount);
            if (iVertexCount > 0)
                ReadRaw(hFile, tMeshDesc.skinnedVertices.data(), sizeof(VTXANIMMESH) * iVertexCount);

            /* Excep. Z-Fighting Fix */
            if (tMeshDesc.materialName == "MI_f_grass_a_03" ||
                tMeshDesc.materialName == "MI_d3_f_floor_h_01")
            {
                for (auto& vtx : tMeshDesc.skinnedVertices)
                    vtx.vPosition.y += 0.001f;
            }

            /* BoneIndex, BoneWeight */
            ReadRaw(hFile, &tMeshDesc.effectTotalBoneCnt, sizeof(_uint));

            _uint iEffectBoneIndices = 0;
            ReadRaw(hFile, &iEffectBoneIndices, sizeof(_uint));
            tMeshDesc.effectBoneIndices.resize(iEffectBoneIndices);
            if (iEffectBoneIndices > 0)
                ReadRaw(hFile, tMeshDesc.effectBoneIndices.data(), sizeof(_int) * iEffectBoneIndices);

            _uint iOffsetMatrices = 0;
            ReadRaw(hFile, &iOffsetMatrices, sizeof(_uint));
            tMeshDesc.offsetMatrices.resize(iOffsetMatrices);
            if (iOffsetMatrices > 0)
                ReadRaw(hFile, tMeshDesc.offsetMatrices.data(), sizeof(_float4x4) * iOffsetMatrices);
        }

        /* INDEX */
        _uint iIndexCount = 0;
        ReadRaw(hFile, &iIndexCount, sizeof(_uint));
        tMeshDesc.commonIndices.resize(iIndexCount);
        if (iIndexCount > 0)
            ReadRaw(hFile, tMeshDesc.commonIndices.data(), sizeof(_uint) * iIndexCount);

        m_vecMeshDesc.push_back(move(tMeshDesc));
    }

    return S_OK;
}

HRESULT CModel::Ready_Materials()
{
    /* 3. Material */
    if (!filesystem::exists(m_strMaterialPath))
        return E_FAIL;

    /* XML */
    tinyxml2::XMLDocument document;
    if (document.LoadFile(m_strMaterialPath.c_str()) != tinyxml2::XML_SUCCESS)
        return E_FAIL;

    auto* pRoot = document.FirstChildElement("Materials");
    if (!pRoot)
        return E_FAIL;

    for (auto& pMaterial : m_vecMaterials)
        Safe_Release(pMaterial);
    m_vecMaterials.clear();

    map<_string, _uint> materialNameToIndex;

    for (auto* pNode = pRoot->FirstChildElement("Material");
        pNode; pNode = pNode->NextSiblingElement("Material"))
    {
        CMaterial* pMaterial = CMaterial::Create(m_pDevice, m_pDeviceContext,
            pNode, m_strMaterialPath.c_str());

        if (!pMaterial)
            continue;

        _uint index = (_uint)m_vecMaterials.size();
        m_vecMaterials.push_back(pMaterial);

        const char* name = pNode->FirstChildElement("Name")->GetText();
        if (name)
            materialNameToIndex[name] = index;
    }

    m_iNumMaterials = (_uint)m_vecMaterials.size();

    for (auto& pMesh : m_vecMeshes)
    {
        const _string& matName = pMesh->Get_MaterialName();
        auto iter = materialNameToIndex.find(matName);
        if (iter != materialNameToIndex.end())
            pMesh->Set_MaterialIndex(iter->second);
        else
            pMesh->Set_MaterialIndex(0);
    }
    return S_OK;
}

HRESULT CModel::Ready_Animations(HANDLE hFile)
{
    /* Animation */
    ReadRaw(hFile, &m_iNumAnimations, sizeof(_uint));
    if (m_iNumAnimations == 0)
        return S_OK;

    m_vecAnimations.reserve(m_iNumAnimations);
    for (size_t i = 0; i < m_iNumAnimations; i++)
    {
        ANIMATION_DESC tAnimDesc = {};
        tAnimDesc.AnimationName = ReadString(hFile);
        ReadRaw(hFile, &tAnimDesc.fDuration, sizeof(_double));
        ReadRaw(hFile, &tAnimDesc.fTicksPerSecond, sizeof(_double));
        ReadRaw(hFile, &tAnimDesc.iNumChannels, sizeof(_uint));

        tAnimDesc.vecChannelDesc.resize(tAnimDesc.iNumChannels);
        for (size_t j = 0; j < tAnimDesc.iNumChannels; j++)
        {
            CHANNEL_DESC& tChannelDesc = tAnimDesc.vecChannelDesc[j];
            tChannelDesc.boneName = ReadString(hFile);

            /* Scaling Keys */
            ReadRaw(hFile, &tChannelDesc.iNumScalingKeys, sizeof(_uint));
            tChannelDesc.vecScalingKeys.resize(tChannelDesc.iNumScalingKeys);
            if (tChannelDesc.iNumScalingKeys > 0)
                ReadRaw(hFile, tChannelDesc.vecScalingKeys.data(), sizeof(Key_F3) * tChannelDesc.iNumScalingKeys);
            /* Raotion Keys */
            ReadRaw(hFile, &tChannelDesc.iNumRoationKeys, sizeof(_uint));
            tChannelDesc.vecRotationKeys.resize(tChannelDesc.iNumRoationKeys);
            if (tChannelDesc.iNumRoationKeys > 0)
                ReadRaw(hFile, tChannelDesc.vecRotationKeys.data(), sizeof(Key_F4) * tChannelDesc.iNumRoationKeys);

            /* Translation Keys */
            ReadRaw(hFile, &tChannelDesc.iNumPositionKeys, sizeof(_uint));
            tChannelDesc.vecPositionKeys.resize(tChannelDesc.iNumPositionKeys);
            if (tChannelDesc.iNumPositionKeys > 0)
                ReadRaw(hFile, tChannelDesc.vecPositionKeys.data(), sizeof(Key_F3) * tChannelDesc.iNumPositionKeys);
        }

        /* 각 애니메이션(동작)마다, 어떤 뼈들을 어떤 상태로 제어해야하는지에 대한 상태들을 저장해주는 작업이 필요하다. */
        CAnimation* pAnimation = CAnimation::Create(this, tAnimDesc);
        if (nullptr == pAnimation)
            return E_FAIL;

        m_vecAnimations.push_back(pAnimation);
        Register_Animation(tAnimDesc.AnimationName, i);
    }
   
    return S_OK;
}

HRESULT CModel::Create_MeshResources()
{
    m_vecMeshes.reserve(m_vecMeshDesc.size());

    _matrix preTransformMatrix = XMLoadFloat4x4(&m_PreTransformMatrix);
    for (auto& MeshDesc : m_vecMeshDesc)
    {
        CMesh* pMesh = CMesh::Create(m_pDevice, m_pDeviceContext, MeshDesc, preTransformMatrix);
        if (!pMesh)
            return E_FAIL;

        m_vecMeshes.push_back(pMesh);
    }
    m_iNumMeshes = static_cast<_uint>(m_vecMeshes.size());
    
    m_MeshRenderMask.clear();
    /* Mesh Mask */
    {
        for (auto& pMesh : m_vecMeshes)
        {
            if (!pMesh)
                continue;

            m_MeshRenderMask[pMesh->Get_MaterialName()] = true;
        }
    }

    /* GPU 버퍼 생성이 끝났으므로 임시 CPU 입력 데이터를 반환, CMesh 가 렌더링과 피킹에 필요한 CPU 데이터는 내부에 별도 보관 */
    m_vecMeshDesc.clear();
    m_vecMeshDesc.shrink_to_fit();  // 로딩 단계에서만 실행

    return S_OK;
}

void CModel::Update_BonesOverrides()
{
    for (auto& Pair : m_BoneOverrides)
    {
       auto iter = m_BoneNameCache.find(Pair.first.c_str());
       if (iter == m_BoneNameCache.end())
            continue;

        _int iIndex = iter->second;
        if (iIndex < 0 || iIndex >= m_vecBones.size())
            continue;

        CBone* pBone = m_vecBones[iIndex];
        if (!pBone)
            continue;

        _matrix LocalMatrix = pBone->Get_TransformationMatrix();
        pBone->Set_TransformationMatrix(Pair.second * LocalMatrix);
    }
}

void CModel::Update_BoneSnap()
{
    for (auto& Pair : m_BoneSnaps)
    {
        _int iTargetIndex = Pair.first;
        _int iSourceIndex = Pair.second;

        CBone* pTarget = m_vecBones[iTargetIndex];
        CBone* pSource = m_vecBones[iSourceIndex];

        if (!pTarget || !pSource)
            return;

        _matrix sourceCombinedMatrix = XMLoadFloat4x4(pSource->Get_CombinedTransformMatrixPrt());
        _int iParentIndex = pTarget->Get_ParentIndex();
        _matrix parentCombinedMatrix = XMMatrixIdentity();

        if (iParentIndex >= 0)
        {
            parentCombinedMatrix = XMLoadFloat4x4(m_vecBones[iParentIndex]->Get_CombinedTransformMatrixPrt());
        }

        /* Just Position */
        _vector vSourcePos = sourceCombinedMatrix.r[3];

        _matrix ParentInverseMatrix = XMMatrixInverse(nullptr, parentCombinedMatrix);
        _vector vLocalPos = XMVector3TransformCoord(vSourcePos, ParentInverseMatrix);

        _matrix TargetLocalMatrix = pTarget->Get_TransformationMatrix();
        TargetLocalMatrix.r[3] = XMVectorSetW(vLocalPos, 1.f);

        /* Set */
        pTarget->Set_TransformationMatrix(TargetLocalMatrix);
    }
}

void CModel::Update_Bones()
{
    for (auto& pBone : m_vecBones)
    {
        pBone->Update_CombinedTransformationMatrix(m_vecBones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

_bool CModel::Lerp_Animation(_float _fTimeDelta)
{
    if (!m_tAnimLerp.bActive)
        return false;

    /* 1. Ratio */
    m_tAnimLerp.fAccTime += _fTimeDelta;
    _float fRatio = min(m_tAnimLerp.fAccTime / m_tAnimLerp.fAccDurationTime, 1.f);

    /* 2. Animation */
    CAnimation* pCurrentAnim = m_vecAnimations[m_iCurrentAnimIndex];
    CAnimation* pNextAnim = m_vecAnimations[m_iNextAnimIndex];
    _uint iNumChannel = pCurrentAnim->Get_NumChannels();

    for (_uint i = 0; i < iNumChannel; ++i)
    {
        CChannel* pCurrentChannel = pCurrentAnim->Get_Channel(i);
        CChannel* pNextChannel = pNextAnim->Get_Channel(i);

        _int iBoneIndex = pCurrentChannel->Get_BoneIndex();
        if (iBoneIndex < 0)
            continue;

        /* 현재 재생중인 애니메이션의 인덱스 <-> 새로 재생할 애니메이션의 첫번째 인덱스 */
        KEYFRAME tSourKeyFrame = pCurrentAnim->Get_CurrentKeyFrame(i);
        KEYFRAME tDestKeyFrame = pNextAnim->Get_FirstKeyFrame(i);

        /* 3. Lerp */
        _vector vScale, vRotation, vTranslation;
        _vector vSourScale = XMLoadFloat3(&tSourKeyFrame.vScale);
        _vector vDestScale = XMLoadFloat3(&tDestKeyFrame.vScale);
        vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);

        _vector vSourRotate = XMLoadFloat4(&tSourKeyFrame.vRotation);
        _vector vDestRotate = XMLoadFloat4(&tDestKeyFrame.vRotation);
        vRotation = XMQuaternionSlerp(vSourRotate, vDestRotate, fRatio);
        vRotation = XMQuaternionNormalize(vRotation);

        _vector vSourTranslation = XMLoadFloat3(&tSourKeyFrame.vTranslation);
        _vector vDestTranslation = XMLoadFloat3(&tDestKeyFrame.vTranslation);
        vTranslation = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);

        _matrix TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation); // w = 1 이니까, 점이 정확히 이동된다. w = 0 으로 설정 시 이동 시 벡터가 변하지 않음.
        m_vecBones[iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
    }

    /* 4. After Lerp */
    if (fRatio >= 1.f)
    {
        m_tAnimLerp.bActive = false;
        m_iCurrentAnimIndex = m_iNextAnimIndex;

        m_vecAnimations[m_iCurrentAnimIndex]->Reset_TrackPosition(true);
    }
    return true;
}

void CModel::Register_Animation(_string _name, _uint _iIndex)
{
    m_AnimNameToIndex[_name] = _iIndex;
}

CModel* CModel::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _char* _pModelFilePath, _fmatrix _PreTransformMatrix)
{
    CModel* pInstance = new CModel(_pDevice, _pDeviceContext);
    if (FAILED(pInstance->Initialize_Prototype(_pModelFilePath, _PreTransformMatrix)))
    {
        MSG_BOX("FAILED TO CREATED : CModel");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CModel::Clone(void* _pArg)
{
    CModel* pInstance = new CModel(*this);
    if (FAILED(pInstance->Initialize(_pArg)))
    {
        MSG_BOX("Failed to Cloned : CModel");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CModel::Free()
{
    __super::Free();

    /* vector release */
    for (auto& pAnimation : m_vecAnimations)
        Safe_Release(pAnimation);
    m_vecAnimations.clear();

    for (auto& pBone : m_vecBones)
        Safe_Release(pBone);
    m_vecBones.clear();

    for (auto& pMaterial : m_vecMaterials)
        Safe_Release(pMaterial);
    m_vecMaterials.clear();

    for (auto& pMesh : m_vecMeshes)
        Safe_Release(pMesh);
    m_vecMeshes.clear();
    m_vecMeshDesc.clear();
}
