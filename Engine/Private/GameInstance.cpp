#include "GameInstance.h"

#include "GraphicDevice.h"
#include "TimerManager.h"
#include "LevelManager.h"
#include "PrototypeManager.h"
#include "ObjectManager.h"
#include "Renderer.h"
#include "FontManager.h"
#include "PipeLine.h"
#include "InputDevice.h"
#include "FileIO.h"
#include "Picking.h"
#include "DialogueManager.h"
#include "CollisionManager.h"
#include "ActorManager.h"
#include "Frustum.h"
#include "RenderTargetManager.h"
#include "LightManager.h"
#include "SoundManager.h"
#include "Shadow.h"

IMPLEMENT_SINGLETON(CGameInstance);

CGameInstance::CGameInstance()
{
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& _tEngineDesc, ID3D11Device** _ppDevice, ID3D11DeviceContext** _ppContext)
{
    /* Graphic_Device */
    m_pGraphicDevice = CGraphicDevice::Create(
        _tEngineDesc.hWnd, _tEngineDesc.eMode,
        _tEngineDesc.iWinSizeX, _tEngineDesc.iWinSizeY,
        _ppDevice, _ppContext);
    if (nullptr == m_pGraphicDevice)
        return E_FAIL;

    /* Time_Manager*/
    m_pTimerManager = CTimerManager::Create();
    if (nullptr == m_pTimerManager)
        return E_FAIL;

    /* Level_Manager */
    m_pLevelManager = CLevelManager::Create();
    if (nullptr == m_pLevelManager)
        return E_FAIL;

    /* Prototype_Manager */
    m_pPrototypeManager = CPrototypeManager::Create(_tEngineDesc.iTotalLevelCnt);
    if (nullptr == m_pPrototypeManager)
        return E_FAIL;

    /* Object_Manager */
    m_pObjectManager = CObjectManager::Create(_tEngineDesc.iTotalLevelCnt);
    if (nullptr == m_pObjectManager)
        return E_FAIL;

    /* RenderTarget_Manager */
    m_pRenderTargetManager = CRenderTargetManager::Create(*_ppDevice, *_ppContext);
    if (nullptr == m_pRenderTargetManager)
        return E_FAIL;

    /* Renderer */
    m_pRenderer = CRenderer::Create(*_ppDevice, *_ppContext);
    if (nullptr == m_pRenderer)
        return E_FAIL;

    /* Font_Manager */
    m_pFontManager = CFontManager::Create();
    if (nullptr == m_pFontManager)
        return E_FAIL;

    /* Event_Manager */
    m_pEventManager = CEventManager::Create();
    if (nullptr == m_pEventManager)
        return E_FAIL;

    /* PipeLine */
    m_pPipeLine = CPipeLine::Create();
    if (nullptr == m_pPipeLine)
        return E_FAIL;

    /* Input_Device */
    m_pInputDevice = CInputDevice::Create(_tEngineDesc.hInstance, _tEngineDesc.hWnd);
    if (nullptr == m_pInputDevice)
        return E_FAIL;

    /* FileIO */
    m_pFileIO = CFileIO::Create(*_ppDevice, *_ppContext);
    if (nullptr == m_pFileIO)
        return E_FAIL;

    /* Picking */
    m_pPicking = CPicking::Create(_tEngineDesc.hWnd, *_ppDevice, *_ppContext, _tEngineDesc.iWinSizeX, _tEngineDesc.iWinSizeY);
    if (nullptr == m_pPicking)
        return E_FAIL;

    /* Dialogue_Manager */
    m_pDialogueManager = CDialogueManager::Create();
    if (nullptr == m_pDialogueManager)
        return E_FAIL;
    
    /* Collision_Manager */
    m_pCollisionManager = CCollisionManager::Create();
    if (nullptr == m_pCollisionManager)
        return E_FAIL;

    /* Actor_Manager */
    m_pActorManager = CActorManager::Create();
    if (nullptr == m_pActorManager)
        return E_FAIL;

    /* Frustum */
    m_pFrustum = CFrustum::Create(*_ppDevice, *_ppContext);
    if (nullptr == m_pFrustum)
        return E_FAIL;

    /* Light_Manager */
    m_pLightManager = CLightManager::Create();
    if (nullptr == m_pLightManager)
        return E_FAIL;

    /* Effect_Manager */
    m_pEffectManager = CEffectManager::Create();
    if (nullptr == m_pEffectManager)
        return E_FAIL;

    /* Sound_Manager */
    m_pSoundManager = CSoundManager::Create();
    if (nullptr == m_pSoundManager)
        return E_FAIL;

    /* Shadow */
    m_pShadow = CShadow::Create();
    if (nullptr == m_pShadow)
        return E_FAIL;

    return S_OK;
}

void CGameInstance::Update_Engine(_float _fTimeDelta)
{
    /* 무조건 마지막에 */
    m_pLevelManager->Switch_NextLevel(-1, nullptr);

    m_pInputDevice->Update();

    m_pObjectManager->Priority_Update(_fTimeDelta);

    m_pObjectManager->Update(_fTimeDelta);

    m_pObjectManager->Late_Update(_fTimeDelta);

    m_pLightManager->Update(_fTimeDelta);

    m_pEffectManager->Update(_fTimeDelta);

    m_pPipeLine->Update();

    m_pFrustum->Update();

    m_pPicking->Update();

    m_pLevelManager->Update(_fTimeDelta);

    m_pDialogueManager->Update(_fTimeDelta);

    m_pCollisionManager->Begin();

    m_pCollisionManager->Tick();

    m_pCollisionManager->Flush();

    //m_pObjectManager->Delete_Update();
}

HRESULT CGameInstance::Begin_Draw(const _float4& _vClearColor)
{
    m_pGraphicDevice->Clear_BackBuffer_View(&_vClearColor);
    m_pGraphicDevice->Clear_DepthStencil_View();

    return S_OK;
}

HRESULT CGameInstance::Draw()
{
    m_pRenderer->Draw();

    m_pDialogueManager->Render();

    return m_pLevelManager->Render();
}

HRESULT CGameInstance::End_Draw()
{
    m_pGraphicDevice->Present();

    m_pObjectManager->Delete_Update();

    return S_OK;
}

HRESULT CGameInstance::Clear(_uint _iClearLevelIndex)
{
    m_pObjectManager->Clear(_iClearLevelIndex);

    m_pPrototypeManager->Clear(_iClearLevelIndex);

    return S_OK;
}

_float CGameInstance::Random(_float _fMin, _float _fMax)
{
    _float fRandNormal = static_cast<_float>(rand()) / RAND_MAX;
    return ((_fMax - _fMin) * fRandNormal) + _fMin;
}

_float CGameInstance::Get_TimeDelta(const _wstring& strTimerTag)
{
    return m_pTimerManager->Get_TimeDelta(strTimerTag);
}

_float CGameInstance::Get_GlobalDelta(const _wstring& strTimerTag)
{
    return m_pTimerManager->Get_GlobalDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const _wstring& strTimerTag)
{
    return m_pTimerManager->Add_Timer(strTimerTag);
}

void CGameInstance::Compute_TimeDelta(const _wstring& strTimerTag)
{
    m_pTimerManager->Compute_TimeDelta(strTimerTag);
}

void CGameInstance::Start_HitStop(_float _fDuration, _float _fTimeScale)
{
    m_pTimerManager->Start_HitStop(_fDuration, _fTimeScale);
}

void CGameInstance::Set_TimeScale(_float _fTimeScale)
{
    m_pTimerManager->Set_TimeScale(_fTimeScale);
}

void CGameInstance::PrepareStartLevel(_uint _iLevelIndex, CLevel* _pNewLevel)
{
    m_pLevelManager->Switch_NextLevel(_iLevelIndex, _pNewLevel);
}

HRESULT CGameInstance::RequestToChangeLevel(_uint _iLevelIndex, CLevel* _pNewLevel)
{
    return m_pLevelManager->Reserve_NextLevel(_iLevelIndex, _pNewLevel);
}

HRESULT CGameInstance::Add_Prototype(_uint _iLevelIndex, const _wstring& _strPrototypeTag, CBase* _pPrototype)
{
    return m_pPrototypeManager->Add_Prototype(_iLevelIndex, _strPrototypeTag, _pPrototype);
}

CBase* CGameInstance::Clone_Prototype(PROTOTYPE _ePrototype, _uint _iLevelIndex, const _wstring& _strPrototypeTag, void* _pArg)
{
    return m_pPrototypeManager->Clone_Prototype(_ePrototype, _iLevelIndex, _strPrototypeTag, _pArg);
}

void CGameInstance::Get_PrototypeTag(_uint _iLevelIndex, vector<_wstring>& _outTags) const
{
    m_pPrototypeManager->Get_PrototypeTag(_iLevelIndex, _outTags);
}

HRESULT CGameInstance::Add_GameObject_ToLayer(_uint _iPrototypeLevelIndex, const _wstring& _strPrototypeTag, _uint _iLayerLevelIndex, const _wstring& _strLayerTag, void* _pArg, CGameObject** _ppOut)
{
    return m_pObjectManager->Add_GameObject_ToLayer(_iPrototypeLevelIndex, _strPrototypeTag, _iLayerLevelIndex, _strLayerTag, _pArg, _ppOut);
}

CComponent* CGameInstance::Get_Component(_uint _iLevelIndex, const _wstring& _strLayerTag, const _wstring& _strComponentTag, _uint _iIndex)
{
    return m_pObjectManager->Get_Component(_iLevelIndex, _strLayerTag, _strComponentTag, _iIndex);
}

HRESULT CGameInstance::Reserve_DeleteObject(CGameObject* _pGameObject)
{
    return m_pObjectManager->Reserve_DeleteObject(_pGameObject);
}

map<const _wstring, class CLayer*>* CGameInstance::Get_Layers() const
{
    return m_pObjectManager->Get_Layers();
}

HRESULT CGameInstance::Clear_Objects(_uint _iClearLevelIndex)
{
    m_pObjectManager->Clear(_iClearLevelIndex);
    
    return S_OK;
}

CGameObject* CGameInstance::Find_Object(const _wstring& _strObjectID)
{
    return m_pObjectManager->Find_Object(_strObjectID);
}

HRESULT CGameInstance::Add_RenderGroup(RENDERGROUP _eGroupID, CGameObject* _pRenderObject)
{
    return m_pRenderer->Add_RenderGroup(_eGroupID, _pRenderObject);
}

void CGameInstance::Add_Instance(CModel* _pModel, const _float4x4& _matWorld)
{
    m_pRenderer->Add_Instance(_pModel, _matWorld);
}

void CGameInstance::Add_Instance(CModel* _Model, CShader* _pShader, const _float4x4& _World)
{
    m_pRenderer->Add_Instance(_Model, _pShader, _World);
}

void CGameInstance::Set_InstancingEnabled(_bool _bEnabled)
{
    m_pRenderer->Set_InstnacingEnabled(_bEnabled);
}

_bool CGameInstance::Is_InstancingEnabled() const
{
    return m_pRenderer->Is_InstancingEnabled();
}

#ifdef _DEBUG
HRESULT CGameInstance::Add_Renderer_DebugComponent(CComponent* _pDebugComponent)
{
    return m_pRenderer->Add_DebugComponent(_pDebugComponent);
}
#endif

HRESULT CGameInstance::Add_Fonts(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pFontTag, const _tchar* _pFontFilePath)
{
    return m_pFontManager->Add_Fonts(_pDevice, _pDeviceContext, _pFontTag, _pFontFilePath);
}

HRESULT CGameInstance::Add_Message(const _tchar* _pFontTag, const _tchar* _pText, _fvector _vPosition, _float _fScale, _fvector _vColor, _float _fRotation)
{
    return m_pFontManager->Add_Message(_pFontTag, _pText, _vPosition, _fScale, _vColor, _fRotation);
}

HRESULT CGameInstance::Render_Fonts(ID3D11DeviceContext* _pDeviceContext)
{
    return m_pFontManager->Render_Fonts(_pDeviceContext);
}

void CGameInstance::Push_UIEVENT(CEventManager::UI_EVENT& _tUIEvent)
{
    m_pEventManager->Push_UIEVENT(_tUIEvent);
}

_bool CGameInstance::Pop_UIEvent(CEventManager::UI_EVENT& _tUIEvent)
{
    return m_pEventManager->Pop_UIEvent(_tUIEvent);
}

void CGameInstance::Push_CameraEVENT(CEventManager::CAMERA_EVENT& _tCameraEvent)
{
    m_pEventManager->Push_CameraEVENT(_tCameraEvent);
}

_bool CGameInstance::Pop_CameraEVENT(CEventManager::CAMERA_EVENT& _tCameraEvent)
{
    return m_pEventManager->Pop_CameraEVENT(_tCameraEvent);
}

void CGameInstance::Push_GameEVENT(CEventManager::GAME_EVENT& _tGameEvent)
{
    m_pEventManager->Push_GameEVENT(_tGameEvent);
}

_bool CGameInstance::Pop_GameEVENT(CEventManager::GAME_EVENT& _tGameEvent)
{
    return m_pEventManager->Pop_GameEVENT(_tGameEvent);
}

HRESULT CGameInstance::Bind_CameraPosition(CShader* _pShader, const _char* _pConstantName)
{
    return m_pPipeLine->Bind_CameraPosition(_pShader, _pConstantName);
}

HRESULT CGameInstance::Add_ShadowLight(const SHADOW_LIGHT_DESC& _tShadowLightDesc)
{
    return m_pShadow->Add_ShadowLight(_tShadowLightDesc);
}

HRESULT CGameInstance::Bind_ShadowTransformState(CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState)
{
    return m_pShadow->Bind_ShadowTransformState(_pShader, _pConstantName, _eTransformState);
}

HRESULT CGameInstance::Bind_TransformState(CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState)
{
    return m_pPipeLine->Bind_TransformState(_pShader, _pConstantName, _eTransformState);
}

HRESULT CGameInstance::Bind_InvTransformState(CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState)
{
    return m_pPipeLine->Bind_InvTransformState(_pShader, _pConstantName, _eTransformState);
}

void CGameInstance::Set_Transform(D3DTS _eTransformState, _fmatrix _TransformMatrix)
{
    m_pPipeLine->Set_Transform(_eTransformState, _TransformMatrix);
}

const _float4x4* CGameInstance::Get_Transform(D3DTS _eTransformState)
{
    return m_pPipeLine->Get_Transform(_eTransformState);
}

const _float4x4* CGameInstance::Get_InvTransform(D3DTS _eTransformState)
{
    return m_pPipeLine->Get_InvTransform(_eTransformState);
}

const _float4* CGameInstance::Get_CameraPosition()
{
    return m_pPipeLine->Get_CameraPosition();
}

_byte CGameInstance::Get_DIKeyState(_ubyte _byKeyID)
{
    return m_pInputDevice->Get_DIKeyState(_byKeyID);
}

_byte CGameInstance::Get_DIKeyDown(_ubyte _byKeyID)
{
    return m_pInputDevice->Get_DIKeyDown(_byKeyID);
}

_byte CGameInstance::Get_DIKeyUp(_ubyte _byKeyID)
{
    return m_pInputDevice->Get_DIKeyUp(_byKeyID);
}

_byte CGameInstance::Get_DIKeyPressing(_ubyte _byKeyID)
{
    return m_pInputDevice->Get_DIKeyPressing(_byKeyID);
}

_byte CGameInstance::Get_DIMouseState(DIMB _eMouse)
{
    return m_pInputDevice->Get_DIMouseState(_eMouse);
}

_long CGameInstance::Get_DIMouseMove(DIMM _eMouseState)
{
    return m_pInputDevice->Get_DIMouseMove(_eMouseState);
}

HRESULT CGameInstance::Save_MapObjects(_uint iLevelTotalNum, _uint iFileIndex)
{
    return m_pFileIO->Save_MapBinary(iLevelTotalNum, iFileIndex);
}

HRESULT CGameInstance::Load_ResourceOnly(_uint _iInstanceLevel)
{
    return m_pFileIO->Load_ResourceOnly(_iInstanceLevel);
}

HRESULT CGameInstance::Load_InstancesOnly(_uint _iInstanceLevel, _uint _iFileIndex)
{
    return m_pFileIO->Load_InstancesOnly(_iInstanceLevel, _iFileIndex);
}

HRESULT CGameInstance::Load_ResourceAndInstances(_uint _iInstanceLevel, _uint _iFileIndex)
{
    return m_pFileIO->Load_ResourceAndInstances(_iInstanceLevel, _iFileIndex);
}

HRESULT CGameInstance::Load_ContainerXML(_uint _iInstanceLevel, const _wstring& _ContainerID, CContainerObject::CONTAINERR_INIT_DESC& _OutDesc, _wstring _OutMeshPath)
{
    return m_pFileIO->Load_ContainerXML(_iInstanceLevel, _ContainerID, _OutDesc, _OutMeshPath);
}

HRESULT CGameInstance::Clone_Instance_Container(_uint _iInstanceLevel, CContainerObject::CONTAINERR_INIT_DESC _ContainerTag, CGameObject** _ppOut)
{
    return m_pFileIO->Clone_Instance_Container(_iInstanceLevel, _ContainerTag, _ppOut);
}

HRESULT CGameInstance::Parse_XML_TAGS(_wstring _XMLPath, EXPORT_TAGS& _outTags)
{
    return m_pFileIO->Parse_XML_TAGS(_XMLPath, _outTags);
}

HRESULT CGameInstance::Register_EffectMeshes(_uint _iInstanceLevel)
{
    return m_pFileIO->Register_EffectMeshes(_iInstanceLevel);
}

HRESULT CGameInstance::Save_EffectXML(SHADER_PARAM_DESC _Param, SHADER_TIMELINE _Timeline, SHADER_TEXTURE_DESC _Textures)
{
    return m_pFileIO->Save_EffectXML(_Param, _Timeline, _Textures);
}

HRESULT CGameInstance::Load_EffectXML(_wstring _EffectName, SHADER_PARAM_DESC& _OutParam, SHADER_TIMELINE& _OutTimeline, SHADER_TEXTURE_DESC& _OutTextures)
{
    return m_pFileIO->Load_EffectXML(_EffectName, _OutParam, _OutTimeline, _OutTextures);
}

HRESULT CGameInstance::Load_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex)
{
    return m_pFileIO->Load_LightBinary(_iInstanceLevel, _iFileIndex);
}

HRESULT CGameInstance::Clone_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex)
{
    return m_pFileIO->Clone_LightBinary(_iInstanceLevel, _iFileIndex);
}

void CGameInstance::Transform_Picking_ToLocalSpace(const _float4x4* _pWorldMatrix)
{
    m_pPicking->Transform_Picking_ToLocalSpace(_pWorldMatrix);
}

_bool CGameInstance::isPicked_InLocalSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut)
{
    return m_pPicking->isPicked_InLocalSpace(_pPointA, _pPointB, _pPointC, _pOut);
}

_bool CGameInstance::isPicked_InWorldSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut, _float* _pOutDist)
{
    return m_pPicking->isPicked_InWorldSpace(_pPointA, _pPointB, _pPointC, _pOut, _pOutDist);
}

_bool CGameInstance::Picking_PlaneY(_float _fY, _float3& _outPos)
{
    return m_pPicking->Picking_PlaneY(_fY, _outPos);
}

_bool CGameInstance::isPicked_AABB(const _float3& vMin, const _float3& vMax, _float3* _pOutHitPos)
{
    return m_pPicking->isPicked_AABB(vMin, vMax, _pOutHitPos);
}

HRESULT CGameInstance::Load_XMLFile(const _wstring& _wstrFileName)
{
    return m_pDialogueManager->Load_XMLFile(_wstrFileName);
}

HRESULT CGameInstance::Start_Dialogue(const _wstring& _wstrDialogueID)
{
    return m_pDialogueManager->Start_Dialogue(_wstrDialogueID);
}

void CGameInstance::EndDialogue()
{
    m_pDialogueManager->EndDialogue();
}

void CGameInstance::NextNode()
{
    m_pDialogueManager->NextNode();
}

void CGameInstance::SelectChoice(_uint _iIndex)
{
    m_pDialogueManager->SelectChoice(_iIndex);
}

_bool CGameInstance::IsDialogueRunning() const
{
    return m_pDialogueManager->IsDialogueRunning();
}

_bool CGameInstance::IsNodeFinished() const
{
    return m_pDialogueManager->IsNodeFinished();
}

const CDialogueManager::DIALOGUE_NODE* CGameInstance::GetCurrentNode() const
{
    return m_pDialogueManager->GetCurrentNode();
}

_bool CGameInstance::Raycast_GroundPlane(_fvector _vStart, _fvector _vEnd, _float _fGroundY)
{
    return m_pCollisionManager->Raycast_GroundPlane(_vStart, _vEnd, _fGroundY);
}

void CGameInstance::Register_Collider(CCollider* _pCollider)
{
    m_pCollisionManager->Register(_pCollider);
}

void CGameInstance::Remove_Collider(CCollider* _pCollider)
{
    m_pCollisionManager->Remove_Collider(_pCollider);
}

void CGameInstance::Reset_Collider(_bool _bResetConfig)
{
    m_pCollisionManager->Reset_All(_bResetConfig);
}

_bool CGameInstance::Check_Block(CCollider* _pCollider, _vector* _outNormalXZ, _float* _outDepth)
{
    return m_pCollisionManager->Check_Block(_pCollider, _outNormalXZ, _outDepth);
}

void CGameInstance::Set_Player(CGameObject* _pPlayer)
{
    m_pActorManager->Set_Player(_pPlayer);
}

CGameObject* CGameInstance::Get_Player()
{
    return m_pActorManager->Get_Player();
}

void CGameInstance::Reset_Player_Position(_float _fX, _float _fY, _float _fZ)
{
    m_pActorManager->Reset_Player_Position(_fX, _fY, _fZ);
}

_bool CGameInstance::IsInFrustum(const _float3& _vWorldPos, _float _fRange)
{
    return m_pFrustum->IsIn(_vWorldPos, _fRange);
}

_bool CGameInstance::IsInFrustum(const _float3& _vCenter, const _float3& _vExtent)
{
    return m_pFrustum->IsIn(_vCenter, _vExtent);
}

HRESULT CGameInstance::Add_RenderTarget(const _wstring& _TargetTag, _uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor)
{
    return m_pRenderTargetManager->Add_RenderTarget(_TargetTag, _iWidth, _iHeight, _eFormat, _vClearColor);
}

HRESULT CGameInstance::Add_MRT(const _wstring& _MRTTag, const _wstring& _TargetTag)
{
    return m_pRenderTargetManager->Add_MRT(_MRTTag, _TargetTag);
}

HRESULT CGameInstance::Begin_MRT(const _wstring& _MRTTag)
{
    return m_pRenderTargetManager->Begin_MRT(_MRTTag);
}

HRESULT CGameInstance::Begin_MRT(const _wstring& _MRTTag, ID3D11DepthStencilView* _pDSV)
{
    return m_pRenderTargetManager->Begin_MRT(_MRTTag, _pDSV);
}

HRESULT CGameInstance::End_MRT()
{
    return m_pRenderTargetManager->End_MRT();
}

HRESULT CGameInstance::Bind_RT_SRV(const _wstring& _TargetTag, CShader* _pShader, const _char* _pConstantName)
{
    return m_pRenderTargetManager->Bind_RT_SRV(_TargetTag, _pShader, _pConstantName);
}

#ifdef  _DEBUG
HRESULT CGameInstance::Ready_RT_Debug(const _wstring& _TargetTag, _float _fX, _float _fY, _float _fSizeX, _float _fSizeY)
{
    return m_pRenderTargetManager->Ready_RT_Debug(_TargetTag, _fX, _fY, _fSizeX, _fSizeY);
}

HRESULT CGameInstance::Render_MRT(const _wstring& _MRTTag, CShader* _pShader, CVIBuffer_Rect* _pVIBuffer)
{
    return m_pRenderTargetManager->Render_MRT(_MRTTag, _pShader, _pVIBuffer);
}
#endif //  _DEBUG

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& _tLightDesc, CTransform* _pTarget, _vector _vOffset)
{
    return m_pLightManager->Add_Light(_tLightDesc, _pTarget, _vOffset);
}
void CGameInstance::Render_Light(CShader* _pShader, CVIBuffer_Rect* _pVIBuffer)
{
    m_pLightManager->Render(_pShader, _pVIBuffer);
}
void CGameInstance::Clear_Light()
{
    m_pLightManager->Clear_Light();
}
const LIGHT_DESC* CGameInstance::Get_LightDesc(_uint _iIndex)
{
    return m_pLightManager->Get_LightDesc(_iIndex);
}

void CGameInstance::Scale_AllPointLightIntensity(_float _fIntensity)
{
    m_pLightManager->Scale_AllPointLightIntensity(_fIntensity);
}

void CGameInstance::Pop_Back_Lights(_uint _iCount)
{
    m_pLightManager->Pop_Back_Lights(_iCount);
}

void CGameInstance::Push_EffectEVENT(CEffectManager::EFFECT_EVENT& _tEffectEvent)
{
    m_pEffectManager->Push_EffectEVENT(_tEffectEvent);
}

HRESULT CGameInstance::Register_EffectEvent(_wstring _EventName, CEffect::EFFECT_INIT_DESC _tDesc)
{
    return m_pEffectManager->Register_EffectEvent(_EventName, _tDesc);
}

HRESULT CGameInstance::Register_EffectEvent(_wstring _EventName, CTrailEffect::TRAIL_INIT_DESC _tDesc)
{
    return m_pEffectManager->Register_EffectEvent(_EventName, _tDesc);
}

void CGameInstance::Play_Sound(const TCHAR* _pSoundKey, SOUND _eID, float _fVolume)
{
    m_pSoundManager->Play_Sound(_pSoundKey, _eID, _fVolume);
}

void CGameInstance::Play_BGM(const TCHAR* _pSoundKey, float _fVolume, bool _bUseCrossFade)
{
    m_pSoundManager->Play_BGM(_pSoundKey, _fVolume, _bUseCrossFade);
}

void CGameInstance::Play_RandomSound(const TCHAR* _pBaseName, int _iCount, float _fVolume)
{
    m_pSoundManager->Play_RandomSound(_pBaseName, _iCount, _fVolume);
}

void CGameInstance::Play_Sound_Range(const TCHAR* _pSoundKey, float _fStartRatio, float _fEndRatio, float _fVolume)
{
    m_pSoundManager->Play_Sound_Range(_pSoundKey, _fStartRatio, _fEndRatio, _fVolume);
}

void CGameInstance::Stop_Sound(SOUND _eID)
{
    m_pSoundManager->Stop_Sound(_eID);
}

void CGameInstance::Stop_AllSounds()
{
    m_pSoundManager->Stop_All();
}

void CGameInstance::Set_ChannelVolume(SOUND _eID, float _fVolume)
{
    m_pSoundManager->Set_ChannelVolume(_eID, _fVolume);
}

void CGameInstance::Release_Engine()
{   
    Safe_Release(m_pShadow);
    Safe_Release(m_pSoundManager);
    Safe_Release(m_pEffectManager);
    Safe_Release(m_pLightManager);
    Safe_Release(m_pRenderTargetManager);
    Safe_Release(m_pFrustum);
    Safe_Release(m_pActorManager);
    Safe_Release(m_pCollisionManager);
    Safe_Release(m_pDialogueManager);
    Safe_Release(m_pPicking);
    Safe_Release(m_pFileIO);
    Safe_Release(m_pInputDevice);
    Safe_Release(m_pPipeLine);
    Safe_Release(m_pEventManager);
    Safe_Release(m_pFontManager);
    Safe_Release(m_pRenderer);
    Safe_Release(m_pObjectManager);
    Safe_Release(m_pPrototypeManager);
    Safe_Release(m_pLevelManager);
    Safe_Release(m_pTimerManager);
    Safe_Release(m_pGraphicDevice);

    DestroyInstance();
}

void CGameInstance::Free()
{
    __super::Free();
}
