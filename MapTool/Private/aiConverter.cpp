#include "aiConverter.h"

CaiConverter::CaiConverter()
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pDeviceContext);
}

HRESULT CaiConverter::Read_AssetFile(_wstring _middlePath)
{    
    m_MiddlePath = _middlePath;

    _string ParameterPath = WSTRTOCHAR(m_MiddlePath).c_str();
    Replace(ParameterPath, "\\", "/");
    _wstring AssetFullPath = m_AssetBasePath + STRTOWSTR(ParameterPath) + TEXT(".fbx");

    auto path = filesystem::path(AssetFullPath);
    assert(filesystem::exists(path));

    /* Import .fbx -> aiScene */
    _uint iAssimpFlag = {
        aiProcess_ConvertToLeftHanded | // DirectX 좌표계
        aiProcess_Triangulate |         // 모든 면을 삼각형으로
        aiProcess_GenUVCoords |         // UV 없으면 생성
        aiProcess_GenNormals |          // 노멀 없으면 생성
        aiProcess_CalcTangentSpace      // 노멀맵용 탄젠트 계산
        // | aiProcess_PreTransformVertices // 애니메이션 없을 때
    };
    m_pAIScene = m_Importer.ReadFile(WSTRTOCHAR(AssetFullPath), iAssimpFlag);
    assert(m_pAIScene != nullptr);

    return S_OK;
}

HRESULT CaiConverter::Export_ModelData()
{
    _wstring MeshFullPath = m_ModelBasePath + m_MiddlePath + TEXT(".mesh");

    filesystem::path path(MeshFullPath);
    filesystem::create_directories(path.parent_path());

    /* 1. aiScene 구조를 순회하며 Bone(Node계층) / Mesh(Vertex, Index) 를 ASBONE 구조체로 변환 */
    Read_ModelData(m_pAIScene->mRootNode, -1, -1);
    /* 2. Animation 를 구조체로 변환 */
    Read_AnimationData();
    /* 3. binary 저장 */
    Write_ModelFile(MeshFullPath);

    return S_OK;
}

HRESULT CaiConverter::Export_MaterialData()
{
    _wstring TextureFullPath = m_ModelBasePath + m_MiddlePath + TEXT(".xml");

    filesystem::path path(TextureFullPath);
    filesystem::create_directories(path.parent_path());

    /* 1. ASMATERAIL 구조체로 변환 */
    Read_MaterialData();
    /* 2. .xml 저장 */
    Write_MaterialFile(TextureFullPath);

    /* Connet XMLPath For Tags */
    m_XMLPath = m_ModelBasePath + m_MiddlePath;

    return S_OK;
}

HRESULT CaiConverter::Read_ModelData(aiNode* _node, _int _iIndex, _int _iParent)
{
    /* BONE */
    ASBONE tBone;
    tBone.nodeName = _node->mName.C_Str();
    _int myBoneIndex = static_cast<_int>(m_vecBones.size());
    tBone.nodeIndex = myBoneIndex; // 파싱한 순서 (-1부터 child로 갈 때마다 증가) // _iIndex
    tBone.parentNodeIndex = _iParent;
       
    /* Assimp 행렬 -> DirextX 행렬 변환 */
    _matrix nodeLocalTransform(_node->mTransformation[0]);
    XMStoreFloat4x4(&tBone.localTransform, XMMatrixTranspose(nodeLocalTransform));
    // (여기서) 부모행렬금지
    m_vecBones.push_back(tBone);

    /* 이 Node 에 붙은 Mesh 읽기 */
    for (_uint i = 0; i < m_pAIScene->mNumMeshes; ++i)
    {
        aiMesh* pAIMesh = m_pAIScene->mMeshes[i];
        if (pAIMesh->HasBones())
            Read_SkinnedMeshData(_node, i, myBoneIndex);
        else
            Read_NonAnimMeshData(_node, i, myBoneIndex);
    }

    /* Child node 재귀 탐색 */
    for (_uint i = 0; i < _node->mNumChildren; i++)
        Read_ModelData(_node->mChildren[i], -1, myBoneIndex);

    return S_OK;
}

HRESULT CaiConverter::Read_NonAnimMeshData(aiNode* _node, _uint _iMeshIndex, _uint _iBone)
{
    /* MESH */
    const aiMesh* pAIMesh = m_pAIScene->mMeshes[_iMeshIndex];
    const aiMaterial* pAIMaterial = m_pAIScene->mMaterials[pAIMesh->mMaterialIndex];
    
    ASMESH tMesh;
    tMesh.meshName = _node->mName.C_Str();
    tMesh.materialName = pAIMaterial->GetName().C_Str();
    tMesh.eModelType = MODEL::NONANIM;

    /* Vertex */
    tMesh.nonAnimVertices.reserve(pAIMesh->mNumVertices);
    for (_uint vertex = 0; vertex < pAIMesh->mNumVertices; vertex++)
    {
        VTXMESH tVertexMesh = {};
        memcpy(&tVertexMesh.vPosition, &pAIMesh->mVertices[vertex], sizeof(_float3));
        if (pAIMesh->HasNormals())
            memcpy(&tVertexMesh.vNormal, &pAIMesh->mNormals[vertex], sizeof(_float3));
        if (pAIMesh->HasTextureCoords(0))
            memcpy(&tVertexMesh.vTexcoord, &pAIMesh->mTextureCoords[0][vertex], sizeof(_float2));
        if (pAIMesh->HasTangentsAndBitangents())
        {                                                      
            memcpy(&tVertexMesh.vTangent, &pAIMesh->mTangents[vertex], sizeof(_float3));
            memcpy(&tVertexMesh.vBinormal, &pAIMesh->mBitangents[vertex], sizeof(_float3));
        }
        tMesh.nonAnimVertices.push_back(tVertexMesh);
    }

    /* Index */
    for (_uint face = 0; face < pAIMesh->mNumFaces; face++)
    {
        aiFace& aiFace = pAIMesh->mFaces[face];
        if (aiFace.mNumIndices != 3)
            continue;

        tMesh.commonIndices.push_back(aiFace.mIndices[0]);
        tMesh.commonIndices.push_back(aiFace.mIndices[1]);
        tMesh.commonIndices.push_back(aiFace.mIndices[2]);
    }

    m_vecMeshes.push_back(move(tMesh));

    return S_OK;
}

HRESULT CaiConverter::Read_SkinnedMeshData(aiNode* _node, _uint _iMeshIndex, _uint _iBone)
{
    /* MESH */
    const aiMesh* pAIMesh = m_pAIScene->mMeshes[_iMeshIndex];
    const aiMaterial* pAIMaterial = m_pAIScene->mMaterials[pAIMesh->mMaterialIndex];

    ASMESH tMesh;
    tMesh.meshName = _node->mName.C_Str();
    tMesh.materialName = pAIMaterial->GetName().C_Str();
    tMesh.eModelType = MODEL::SKINNED;

    /* Vertex */
    tMesh.skinnedVertices.reserve(pAIMesh->mNumVertices);
    for (_uint vertex = 0; vertex < pAIMesh->mNumVertices; vertex++)
    {
        VTXANIMMESH tVertexAnimMesh = {};
        memcpy(&tVertexAnimMesh.vPosition, &pAIMesh->mVertices[vertex], sizeof(_float3));
        if (pAIMesh->HasNormals())
            memcpy(&tVertexAnimMesh.vNormal, &pAIMesh->mNormals[vertex], sizeof(_float3));
        if (pAIMesh->HasTextureCoords(0))
            memcpy(&tVertexAnimMesh.vTexcoord, &pAIMesh->mTextureCoords[0][vertex], sizeof(_float2));
        if (pAIMesh->HasTangentsAndBitangents())
        {
            memcpy(&tVertexAnimMesh.vTangent, &pAIMesh->mTangents[vertex], sizeof(_float3));
            memcpy(&tVertexAnimMesh.vBinormal, &pAIMesh->mBitangents[vertex], sizeof(_float3));
        }
        tVertexAnimMesh.vBlendIndex = XMUINT4(0, 0, 0, 0);
        tVertexAnimMesh.vBlendWeight = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
        tMesh.skinnedVertices.push_back(tVertexAnimMesh);
    }

    tMesh.effectTotalBoneCnt = pAIMesh->mNumBones;
    for (size_t i = 0; i < pAIMesh->mNumBones; i++)
    {
        aiBone* pAIBone = pAIMesh->mBones[i];
        _float4x4 offsetMatrix = {};
        memcpy(&offsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));
        XMStoreFloat4x4(&offsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&offsetMatrix)));
        tMesh.offsetMatrices.push_back(offsetMatrix);

        _int iBoneIndex = {};
        iBoneIndex = Get_BoneIndex_ByNodeName(pAIBone->mName.data);
        strcpy_s(m_szBoneName, pAIMesh->mName.data);
        if (iBoneIndex == -1)
            return E_FAIL;

        tMesh.effectBoneIndices.push_back(iBoneIndex);

        for (size_t j = 0; j < pAIBone->mNumWeights; j++)
        {
            aiVertexWeight AIVertexWeight = pAIBone->mWeights[j];
            auto& vertex = tMesh.skinnedVertices[AIVertexWeight.mVertexId];

            if (0.f == vertex.vBlendWeight.x)
            {
                vertex.vBlendIndex.x = i;
                vertex.vBlendWeight.x = AIVertexWeight.mWeight;
            }

            else if (0.f == vertex.vBlendWeight.y)
            {
                vertex.vBlendIndex.y = i;
                vertex.vBlendWeight.y = AIVertexWeight.mWeight;
            }

            else if (0.f == vertex.vBlendWeight.z)
            {
                vertex.vBlendIndex.z = i;
                vertex.vBlendWeight.z = AIVertexWeight.mWeight;
            }

            else
            {
                vertex.vBlendIndex.w = i;
                vertex.vBlendWeight.w = AIVertexWeight.mWeight;
            }
        }
    }

    if (0 == tMesh.effectTotalBoneCnt)
    {
        tMesh.effectTotalBoneCnt = 1;

        _float4x4 offsetMatrix = {};
        XMStoreFloat4x4(&offsetMatrix, XMMatrixIdentity());
        tMesh.offsetMatrices.push_back(offsetMatrix);

         _int iBoneIndex = Get_BoneIndex_ByNodeName(m_szBoneName); // m_szBoneName _node->mName.C_Str()
        if (iBoneIndex != -1)
            tMesh.effectBoneIndices.push_back(iBoneIndex);

        for (auto& v : tMesh.skinnedVertices)
        {
            v.vBlendIndex = XMUINT4(0, 0, 0, 0);
            v.vBlendWeight = XMFLOAT4(1.f, 0.f, 0.f, 0.f);
        }
    }

    /* Index */
    for (_uint face = 0; face < pAIMesh->mNumFaces; face++)
    {
        aiFace& aiFace = pAIMesh->mFaces[face];
        if (aiFace.mNumIndices != 3)
            continue;

        tMesh.commonIndices.push_back(aiFace.mIndices[0]);
        tMesh.commonIndices.push_back(aiFace.mIndices[1]);
        tMesh.commonIndices.push_back(aiFace.mIndices[2]);
    }

    m_vecMeshes.push_back(move(tMesh));

    return S_OK;
}

HRESULT CaiConverter::Read_AnimationData()
{
    if (!m_pAIScene || m_pAIScene->mNumAnimations == 0)
        return S_OK;

    /* Animation */
    m_vecAnimations.reserve(m_pAIScene->mNumAnimations);
    for (_uint i = 0; i < m_pAIScene->mNumAnimations; ++i)
    {
        aiAnimation* pAIAnim = m_pAIScene->mAnimations[i];
        if (!pAIAnim)
            continue;

        ASANIMATION tAnim = {};
        if (pAIAnim->mName.length == 0)
        {
            tAnim.AnimationName = "ANIM_" + to_string(i);
        }
        else
        {
            _string animName = pAIAnim->mName.C_Str();
            size_t pos = animName.rfind('|');
            if (pos != _string::npos)
                animName = animName.substr(pos + 1);

            tAnim.AnimationName = animName;
        }
        tAnim.fDuration = pAIAnim->mDuration;
        tAnim.fTicksPerSecond = pAIAnim->mTicksPerSecond != 0.0 ? (_float)pAIAnim->mTicksPerSecond : 25.f;
        tAnim.iNumChannels = pAIAnim->mNumChannels;

        /* Channels */
        tAnim.vecChannels.reserve(pAIAnim->mNumChannels);
        for (size_t j = 0; j < pAIAnim->mNumChannels; ++j)
        {
            aiNodeAnim* pAIChannels = pAIAnim->mChannels[j];
            if (!pAIChannels)
                continue;

            ASCHANNEL tChannel = {};
            tChannel.boneName = pAIChannels->mNodeName.C_Str();

            /* ScalingKey */
            tChannel.vecScalingKeys.reserve(pAIChannels->mNumScalingKeys);
            for (_uint sk = 0; sk < pAIChannels->mNumScalingKeys; sk++)
            {
                Key_F3 tKey = {};
                tKey.fTime = pAIChannels->mScalingKeys[sk].mTime;
                memcpy(&tKey.vValue, &pAIChannels->mScalingKeys[sk].mValue, sizeof(_float3));
                tChannel.vecScalingKeys.push_back(tKey);
            }
            /* RotationKey */
            tChannel.vecRotationKeys.reserve(pAIChannels->mNumRotationKeys);
            for (_uint rk = 0; rk < pAIChannels->mNumRotationKeys; rk++)
            {
                Key_F4 tKey = {};
                tKey.fTime = pAIChannels->mRotationKeys[rk].mTime;
                auto QuatRot = pAIChannels->mRotationKeys[rk].mValue;
                tKey.vValue = _float4(QuatRot.x, QuatRot.y, QuatRot.z, QuatRot.w);
                tChannel.vecRotationKeys.push_back(tKey);
            }
            /* PositionKey */
            tChannel.vecPositionKeys.reserve(pAIChannels->mNumPositionKeys);
            for (_uint pk = 0; pk < pAIChannels->mNumPositionKeys; pk++)
            {
                Key_F3 tKey = {};
                tKey.fTime = pAIChannels->mPositionKeys[pk].mTime;
                memcpy(&tKey.vValue, &pAIChannels->mPositionKeys[pk].mValue, sizeof(_float3));
                tChannel.vecPositionKeys.push_back(tKey);
            }
            tAnim.vecChannels.push_back(tChannel);
        }
        m_vecAnimations.push_back(tAnim);
    }

    return S_OK;
}

HRESULT CaiConverter::Write_ModelFile(_wstring _MeshFullPath)
{
    auto MeshFullPath = filesystem::path(_MeshFullPath);
    filesystem::create_directories(MeshFullPath.parent_path());

    /* 파일 생성 */
    HANDLE hFile = CreateFileW(_MeshFullPath.c_str(),
        GENERIC_WRITE, 0,
        nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    assert(hFile != INVALID_HANDLE_VALUE);

    /* Raw 데이터 기록 */
    auto WriteRaw = [&](const void* data, _uint size) {
        DWORD written = 0;
        _bool ok = ::WriteFile(hFile, data, size, &written, nullptr);
        assert(ok && written == size);
        };

    /* 문자열 기록 */
    auto WriteString = [&](const _string& str) {
        _uint size = static_cast<_uint>(str.size());
        WriteRaw(&size, sizeof(_uint));
        if (size > 0)
            WriteRaw(str.data(), size);
        };

    /* Bone Data */
    _uint iBoneCount = static_cast<_uint>(m_vecBones.size());
    WriteRaw(&iBoneCount, sizeof(_uint));
    for (auto& Bone : m_vecBones)
    {
        WriteString(Bone.nodeName);
        WriteRaw(&Bone.nodeIndex, sizeof(_int));
        WriteRaw(&Bone.parentNodeIndex, sizeof(_int));
        WriteRaw(&Bone.localTransform, sizeof(_float4x4));
    }

    /* Mesh Data */
    _uint iMeshCount = static_cast<_uint>(m_vecMeshes.size());
    WriteRaw(&iMeshCount, sizeof(_uint));
    for (auto MeshData : m_vecMeshes)
    {
        WriteString(MeshData.meshName);
        WriteString(MeshData.materialName);

        /* Model Type */
        MODEL eModelType = MeshData.eModelType;
        WriteRaw(&eModelType, sizeof(MODEL));

        /* Vertex Data */
        if (eModelType == MODEL::NONANIM)
        {
            /* NonAnim */
            _uint iVertexCount =
                static_cast<_uint>(MeshData.nonAnimVertices.size());
            WriteRaw(&iVertexCount, sizeof(_uint));

            if (iVertexCount > 0)
                WriteRaw(MeshData.nonAnimVertices.data(), sizeof(VTXMESH) * iVertexCount);
        }
        else
        {
            /* Skinned */
            _uint iVertexCount =
                static_cast<_uint>(MeshData.skinnedVertices.size());
            WriteRaw(&iVertexCount, sizeof(_uint));

            if (iVertexCount > 0)
                WriteRaw(MeshData.skinnedVertices.data(), sizeof(VTXANIMMESH) * iVertexCount);

            WriteRaw(&MeshData.effectTotalBoneCnt, sizeof(_int));

            _uint iEffectBoneIndices = static_cast<_uint>(MeshData.effectBoneIndices.size());
            WriteRaw(&iEffectBoneIndices, sizeof(_uint));
            if (iEffectBoneIndices > 0)
                WriteRaw(MeshData.effectBoneIndices.data(), sizeof(_int) * iEffectBoneIndices);

            _uint iOffsetMatrices = static_cast<_uint>(MeshData.offsetMatrices.size());
            WriteRaw(&iOffsetMatrices, sizeof(_uint));
            if (iOffsetMatrices > 0)
                WriteRaw(MeshData.offsetMatrices.data(), sizeof(_float4x4) * iOffsetMatrices);
        }

        /* Index Data */
        _uint indexCount = static_cast<_uint>(MeshData.commonIndices.size());
        WriteRaw(&indexCount, sizeof(_uint));
        if (indexCount > 0)
            WriteRaw(MeshData.commonIndices.data(), sizeof(_uint) * indexCount);
    }

    /* Animation Data */
    _uint iNumAnimations = static_cast<_uint>(m_vecAnimations.size());
    WriteRaw(&iNumAnimations, sizeof(_uint));
    for (auto& Anim : m_vecAnimations)
    {
        WriteString(Anim.AnimationName);
        WriteRaw(&Anim.fDuration, sizeof(_double));
        WriteRaw(&Anim.fTicksPerSecond, sizeof(_double));
        WriteRaw(&Anim.iNumChannels, sizeof(_uint));

        for (auto& Channel : Anim.vecChannels)
        {
            _uint iBoneNameLength = static_cast<_uint>(Channel.boneName.size());
            WriteRaw(&iBoneNameLength, sizeof(_uint));
            if (iBoneNameLength > 0)
                WriteRaw(Channel.boneName.data(), iBoneNameLength);

            /* Scaling Keys */
            _uint iScalingCount = static_cast<_uint>(Channel.vecScalingKeys.size());
            WriteRaw(&iScalingCount, sizeof(_uint));
            if (iScalingCount > 0)
                WriteRaw(Channel.vecScalingKeys.data(), sizeof(Key_F3) * iScalingCount);

            /* Rotation Keys */
            _uint iRotationCount = static_cast<_uint>(Channel.vecRotationKeys.size());
            WriteRaw(&iRotationCount, sizeof(_uint));
            if (iRotationCount > 0)
                WriteRaw(Channel.vecRotationKeys.data(), sizeof(Key_F4) * iRotationCount);

            /* Position Keys */
            _uint iPositionCount = static_cast<_uint>(Channel.vecPositionKeys.size());
            WriteRaw(&iPositionCount, sizeof(_uint));
            if (iPositionCount > 0)
                WriteRaw(Channel.vecPositionKeys.data(), sizeof(Key_F3) * iPositionCount);
        }
    }

    ::CloseHandle(hFile);

    return S_OK;
}

HRESULT CaiConverter::Read_MaterialData()
{
    for (_int i = 0; i < m_pAIScene->mNumMaterials; i++)
    {
        aiMaterial* srcMaterial = m_pAIScene->mMaterials[i];

        /* Name */
        ASMATERIAL asMaterial = {};
        asMaterial.name = srcMaterial->GetName().C_Str();

        /* Ambient */
        aiColor3D color;
        srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
        asMaterial.ambient = COLOR{ _float4(color.r, color.g, color.b, 1.f) };

        /* Diffuse */
        srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        asMaterial.diffuse = COLOR{ _float4(color.r, color.g, color.b, 1.f) };

        /* Specular */
        srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
        asMaterial.specular = COLOR{ _float4(color.r, color.g, color.b, 1.f) };
        srcMaterial->Get(AI_MATKEY_SHININESS, asMaterial.specular.vColor.w); // 세기 설정

        /* Emissive */
        srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
        asMaterial.emissive = COLOR{ _float4(color.r, color.g, color.b, 1.f) };

        /* Get Texture (Diffuse, Specular, Emissive) */
        asMaterial.diffuseFile.clear();
        asMaterial.normalFile.clear();
        asMaterial.metallicFile.clear();
        asMaterial.roughnessFile.clear();
        aiString file;

        /* Diffuse File */
        if (srcMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
        }

        asMaterial.diffuseFile = asMaterial.name + "_alb.png";

        /* AO File */
        asMaterial.AOFile = asMaterial.name + "_occ.png";

        /* Normal File */
        if (srcMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
            if (Contains(file.C_Str(), "_nml"))
                asMaterial.normalFile = file.C_Str();
        }
        else
        {
            asMaterial.normalFile = asMaterial.name + "_nml.png";
        }

        /* Metallic File */
        if (srcMaterial->GetTextureCount(aiTextureType_METALNESS) > 0)
        {
            srcMaterial->GetTexture(aiTextureType_METALNESS, 0, &file);
            if (Contains(file.C_Str(), "_mtl"))
                asMaterial.metallicFile = file.C_Str();
        }
        else
        {
            asMaterial.metallicFile = asMaterial.name + "_mtl.png";
        }

        /* Roughness File */
        if (srcMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
        {
            srcMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &file);
            if (Contains(file.C_Str(), "_smt"))
                asMaterial.roughnessFile = file.C_Str();
        }
        else
        {
            asMaterial.roughnessFile = asMaterial.name + "_smt.png";
        }

        /* Push_Back */
        m_vecMaterials.push_back(asMaterial);
    }
    return S_OK;
}

HRESULT CaiConverter::Write_MaterialFile(_wstring _TextureFullPath)
{
    /* 저장할 경로 */
    auto TextureFullPath = filesystem::path(_TextureFullPath);
    filesystem::create_directory(TextureFullPath.parent_path());

    /* 저장할 폴더 */
    _string MiddlePath =
        filesystem::relative(
            TextureFullPath.parent_path(),
            m_ModelBasePath
        ).string(); // \NonAnim\Map_MarinTarinHouse

    /* XML */
    tinyxml2::XMLDocument document;

    /* XML 포캣으로 읽을거다. */
    tinyxml2::XMLDeclaration* decl = document.NewDeclaration();
    document.LinkEndChild(decl);

    tinyxml2::XMLElement* root = document.NewElement("Materials");
    document.LinkEndChild(root);

    for (auto& Material : m_vecMaterials)
    {
        /* Root Node */
        tinyxml2::XMLElement* node = document.NewElement("Material");
        root->LinkEndChild(node);

        /* Child Nodes */
        tinyxml2::XMLElement* element = nullptr;

        element = document.NewElement("Name");
        element->SetText(Material.name.c_str());
        node->LinkEndChild(element);

        element = document.NewElement("DiffuseFile");
        element->SetText(Write_TextureFile(MiddlePath, Material.diffuseFile).c_str());
        node->LinkEndChild(element);

        element = document.NewElement("AOFile");
        element->SetText(Write_TextureFile(MiddlePath, Material.AOFile).c_str());
        node->LinkEndChild(element);

        element = document.NewElement("NormalFile");
        element->SetText(Write_TextureFile(MiddlePath, Material.normalFile).c_str());
        node->LinkEndChild(element);

        element = document.NewElement("MetallicFile");
        element->SetText(Write_TextureFile(MiddlePath, Material.metallicFile).c_str());
        node->LinkEndChild(element);

        element = document.NewElement("RoughnessFile");
        element->SetText(Write_TextureFile(MiddlePath, Material.roughnessFile).c_str());
        node->LinkEndChild(element);

        element = document.NewElement("Ambient");
        element->SetAttribute("R", Material.ambient.vColor.x);
        element->SetAttribute("G", Material.ambient.vColor.y);
        element->SetAttribute("B", Material.ambient.vColor.z);
        element->SetAttribute("A", Material.ambient.vColor.w);
        node->LinkEndChild(element);
    
        element = document.NewElement("Diffuse");
        element->SetAttribute("R", Material.diffuse.vColor.x);
        element->SetAttribute("G", Material.diffuse.vColor.y);
        element->SetAttribute("B", Material.diffuse.vColor.z);
        element->SetAttribute("A", Material.diffuse.vColor.w);
        node->LinkEndChild(element);

        element = document.NewElement("Specular");
        element->SetAttribute("R", Material.specular.vColor.x);
        element->SetAttribute("G", Material.specular.vColor.y);
        element->SetAttribute("B", Material.specular.vColor.z);
        element->SetAttribute("A", Material.specular.vColor.w);
        node->LinkEndChild(element);

        element = document.NewElement("Emissive");
        element->SetAttribute("R", Material.emissive.vColor.x);
        element->SetAttribute("G", Material.emissive.vColor.y);
        element->SetAttribute("B", Material.emissive.vColor.z);
        element->SetAttribute("A", Material.emissive.vColor.w);
        node->LinkEndChild(element);
    }
    document.SaveFile(WSTRTOCHAR(TextureFullPath).c_str());

    return S_OK;
}

_string CaiConverter::Write_TextureFile(_string _middlePath, _string _file)
{
    /* ../../Resources/Textures/(여기) 에 모아놓기 */
    _string fileName = filesystem::path(_file).filename().string(); // MI_Chair_01_alb.png
    _string fileNameStem =
        filesystem::path(_file).stem().string(); // MI_Chair_01_alb
    filesystem::path srcPath = filesystem::path(m_AssetBasePath) / _middlePath / fileName;
    filesystem::path dstPath = filesystem::path(m_ModelBasePath) / _middlePath / fileName;

    filesystem::create_directories(dstPath.parent_path());

    const aiTexture* srcTexture = m_pAIScene->GetEmbeddedTexture(fileNameStem.c_str());

    if (srcTexture)
    {
        if (srcTexture->mHeight == 0) // 데이터가 1차원 배열 형태로 저장되어 있다면 바이너리 모드로 만드는 경우.
        {
            ofstream ofs(dstPath, ios::binary);
            ofs.write(
                reinterpret_cast<const char*>(srcTexture->pcData),
                srcTexture->mWidth
            );
            ofs.close();
        }
        else // FBX 에 1차원 배열이 아닌 2차원 텍스쳐가 있다면 내부에 있는 걸 끄집어 내서 별도의 파일로 만드는 경우.
        {
            D3D11_TEXTURE2D_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
            desc.Width = srcTexture->mWidth;
            desc.Height = srcTexture->mHeight;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_IMMUTABLE;

            D3D11_SUBRESOURCE_DATA subResource = { 0 };
            subResource.pSysMem = srcTexture->pcData;

            ID3D11Texture2D* texture = { nullptr };
            HRESULT hr = m_pDevice->CreateTexture2D(&desc, &subResource, &texture);

            ScratchImage img;
            CaptureTexture(m_pDevice, m_pDeviceContext, texture, img);

            /* Save To File*/
            hr = SaveToDDSFile(*img.GetImages(), DDS_FLAGS_NONE, STRTOWSTR(fileName).c_str());
            if (FAILED(hr))
                return "";

            /* release */
            if (texture)
                Safe_Release(texture);
        }
    }
    else
    {
        // TODO : 파일 .xml 과 동일 경로에 복사하는 것. 추후에 하기.
       
        //_string srcStr = srcPath.string();
        //_string dstStr = dstPath.string();

        //Replace(srcStr, "\\", "/");
        //Replace(dstStr, "\\", "/");

        //::CopyFileA(srcStr.c_str(), dstStr.c_str(), FALSE);
    }   

    return fileName;
}

_bool CaiConverter::Contains(const _string& src, const _string& key)
{
    return src.find(key) != _string::npos;
}

void CaiConverter::Replace(OUT _string& str, _string comp, _string rep)
{
    _string temp = str;

    size_t start_pos = 0;
    while ((start_pos = temp.find(comp, start_pos)) != _wstring::npos)
    {
        temp.replace(start_pos, comp.length(), rep); 
        start_pos += rep.length();
    }
    str = temp;
}

void CaiConverter::Replace(OUT _wstring& wstr, _wstring comp, _wstring rep)
{
    _wstring temp = wstr;

    size_t start_pos = 0;
    while ((start_pos = temp.find(comp, start_pos)) != _wstring::npos)
    {
        temp.replace(start_pos, comp.length(), rep);
        start_pos += rep.length();
    }

    wstr = temp;
}

_int CaiConverter::Get_BoneIndex_ByNodeName(_string _nodeName)
{
    for (_uint i = 0; i < m_vecBones.size(); ++i)
    {
        if (m_vecBones[i].nodeName == _nodeName)
            return static_cast<_int>(i);
    }

    return -1;
}

_int CaiConverter::Find_AttachBone_ByNodeName(aiNode* _node)
{
    _int iIndex = Get_BoneIndex_ByNodeName(_node->mName.C_Str());
    if (iIndex != -1)
        return iIndex;

    aiNode* parent = _node->mParent;
    while (parent)
    {
        iIndex = Get_BoneIndex_ByNodeName(parent->mName.C_Str());
        if (iIndex != -1)
            return iIndex;
        parent = parent->mParent;
    }

    return -1;
}

CaiConverter* CaiConverter::Create()
{
    return new CaiConverter();
}

void CaiConverter::Free()
{
    /* release */
    m_Importer.FreeScene();
    m_pAIScene = nullptr;

    Safe_Release(m_pDeviceContext);
    Safe_Release(m_pDevice);
}
