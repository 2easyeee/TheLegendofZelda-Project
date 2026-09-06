#pragma once
#include "PrototypeManager.h"
#include "EventManager.h"
#include "DialogueManager.h"
#include "ContainerObject.h"
#include "EffectManager.h"

NS_BEGIN(Engine)
class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance);
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	HRESULT Initialize_Engine(const ENGINE_DESC& _tEngineDesc, ID3D11Device** _ppDevice, ID3D11DeviceContext** _ppContext);
	void Update_Engine(_float _fTimeDelta);
	HRESULT Begin_Draw(const _float4& _vClearColor);
	HRESULT Draw();
	HRESULT End_Draw();
	HRESULT Clear(_uint _iClearLevelIndex);

	_float Random(_float _fMin, _float _fMax);

#pragma region TIMER_MANAGER
public:
	_float Get_TimeDelta(const _wstring& strTimerTag);
	_float Get_GlobalDelta(const _wstring& strTimerTag);
	HRESULT Add_Timer(const _wstring& strTimerTag);
	void Compute_TimeDelta(const _wstring& strTimerTag);
	void Start_HitStop(_float _fDuration, _float _fTimeScale = 0.f);
	void Set_TimeScale(_float _fTimeScale);
#pragma endregion

#pragma region LEVEL_MANAGER
public:
	void PrepareStartLevel(_uint _iLevelIndex, class CLevel* _pNewLevel);
	HRESULT RequestToChangeLevel(_uint _iLevelIndex, class CLevel* _pNewLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Prototype(_uint _iLevelIndex, const _wstring& _strPrototypeTag, class CBase* _pPrototype);
	CBase* Clone_Prototype(PROTOTYPE _ePrototype, _uint _iLevelIndex, const _wstring& _strPrototypeTag, void* _pArg);
	void Get_PrototypeTag(_uint _iLevelIndex, vector<_wstring>& _outTags) const;
#pragma endregion

#pragma region OBJECT_MANAGER
	HRESULT Add_GameObject_ToLayer(_uint _iPrototypeLevelIndex, const _wstring& _strPrototypeTag, _uint _iLayerLevelIndex, const _wstring& _strLayerTag, void* _pArg = nullptr, class CGameObject** _ppOut = nullptr);
	class CComponent* Get_Component(_uint _iLevelIndex, const _wstring& _strLayerTag, const _wstring& _strComponentTag, _uint _iIndex);
	HRESULT Reserve_DeleteObject(class CGameObject* _pGameObject);
	map<const _wstring, class CLayer*>* Get_Layers() const;
	HRESULT Clear_Objects(_uint _iClearLevelIndex);
	CGameObject* Find_Object(const _wstring& _strObjectID);
#pragma endregion

#pragma region RENDERER
	HRESULT Add_RenderGroup(RENDERGROUP _eGroupID, class CGameObject* _pRenderObject);
	void Add_Instance(class CModel* _Model, const _float4x4& _World);
	void Add_Instance(class CModel* _Model, class CShader* _pShader, const _float4x4& _World);
	void Set_InstancingEnabled(_bool _bEnabled);
	_bool Is_InstancingEnabled() const;
#ifdef _DEBUG
	HRESULT Add_Renderer_DebugComponent(class CComponent* _pDebugComponent);
#endif
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Fonts(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pFontTag, const _tchar* _pFontFilePath);
	HRESULT Add_Message(const _tchar* _pFontTag, const _tchar* _pText, _fvector _vPosition, _float _fScale = 1.f, 
		_fvector _vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float _fRotation = 0.f);
	HRESULT Render_Fonts(ID3D11DeviceContext* _pDeviceContext);
#pragma endregion
	
#pragma region EVENT_MANAGER
	void Push_UIEVENT(CEventManager::UI_EVENT& _tUIEvent);
	_bool Pop_UIEvent(CEventManager::UI_EVENT& _tUIEvent);
	void Push_CameraEVENT(CEventManager::CAMERA_EVENT& _tCameraEvent);
	_bool Pop_CameraEVENT(CEventManager::CAMERA_EVENT& _tCameraEvent);
	void Push_GameEVENT(CEventManager::GAME_EVENT& _tGameEvent);
	_bool Pop_GameEVENT(CEventManager::GAME_EVENT& _tGameEvent);
#pragma endregion

#pragma region PIPELINE
	HRESULT Bind_CameraPosition(class CShader* _pShader, const _char* _pConstantName);
	HRESULT Bind_TransformState(class CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState);
	HRESULT Bind_InvTransformState(class CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState);
	void Set_Transform(D3DTS _eTransformState, _fmatrix _TransformMatrix);
	const _float4x4* Get_Transform(D3DTS _eTransformState);
	const _float4x4* Get_InvTransform(D3DTS _eTransformState);
	const _float4* Get_CameraPosition();
#pragma endregion

#pragma region INPUT_DEVICE
	_byte Get_DIKeyState(_ubyte _byKeyID);
	_byte Get_DIKeyDown(_ubyte _byKeyID);
	_byte Get_DIKeyUp(_ubyte _byKeyID);
	_byte Get_DIKeyPressing(_ubyte _byKeyID);
	_byte Get_DIMouseState(DIMB _eMouse);
	_long Get_DIMouseMove(DIMM _eMouseState);
#pragma endregion

#pragma region FILE_IO
	HRESULT Save_MapObjects(_uint iLevelTotalNum, _uint iFileIndex);
	HRESULT Load_ResourceOnly(_uint _iInstanceLevel);
	HRESULT Load_InstancesOnly(_uint _iInstanceLevel, _uint _iFileIndex = 0);
	HRESULT Load_ResourceAndInstances(_uint _iInstanceLevel, _uint _iFileIndex = 0);
	HRESULT Load_ContainerXML(_uint _iInstanceLevel, const _wstring& _ContainerID, CContainerObject::CONTAINERR_INIT_DESC& _OutDesc, _wstring _OutMeshPath);
	HRESULT Clone_Instance_Container(_uint _iInstanceLevel, CContainerObject::CONTAINERR_INIT_DESC _ContainerTag, class CGameObject** _ppOut = nullptr);
	HRESULT Parse_XML_TAGS(_wstring _XMLPath, EXPORT_TAGS& _outTags);
	HRESULT Register_EffectMeshes(_uint _iInstanceLevel);
	HRESULT Save_EffectXML(SHADER_PARAM_DESC _Param, SHADER_TIMELINE _Timeline, SHADER_TEXTURE_DESC _Textures);
	HRESULT Load_EffectXML(_wstring _EffectName, SHADER_PARAM_DESC& _OutParam, SHADER_TIMELINE& _OutTimeline, SHADER_TEXTURE_DESC& _OutTextures);
	HRESULT Load_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex = 0);
	HRESULT Clone_LightBinary(_uint _iInstanceLevel, _uint _iFileIndex = 0);
#pragma endregion

#pragma region PICKING
	void Transform_Picking_ToLocalSpace(const _float4x4* _pWorldMatrix);
	_bool isPicked_InLocalSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut);
	_bool isPicked_InWorldSpace(const _float3* _pPointA, const _float3* _pPointB, const _float3* _pPointC, _float3* _pOut, _float* _pOutDist);
	_bool Picking_PlaneY(_float _fY, _float3& _outPos);
	_bool isPicked_AABB(const _float3& vMin, const _float3& vMax, _float3* _pOutHitPos);
#pragma endregion

#pragma region DIALOGUE_MANAGER
	HRESULT Load_XMLFile(const _wstring& _wstrFileName);
	HRESULT Start_Dialogue(const _wstring& _wstrDialogueID);
	void EndDialogue();
	void NextNode();
	void SelectChoice(_uint _iIndex);
	_bool IsDialogueRunning() const;
	_bool IsNodeFinished() const;
	const CDialogueManager::DIALOGUE_NODE* GetCurrentNode() const;
#pragma endregion

#pragma region COLLISION_MANAGER
	_bool Raycast_GroundPlane(_fvector _vStart, _fvector _vEnd, _float _fGroundY = 0.f);
	void Register_Collider(CCollider* _pCollider);
	void Remove_Collider(class CCollider* _pCollider);
	void Reset_Collider(_bool _bResetConfig = false);
	_bool Check_Block(CCollider* _pCollider, _vector* _outNormalXZ = nullptr, _float* _outDepth = nullptr);
#pragma endregion

#pragma region ACTOR_MANAGER
	void Set_Player(class CGameObject* _pPlayer);
	class CGameObject* Get_Player();
	void Reset_Player_Position(_float _fX = 0.f, _float _fY = 0.f, _float _fZ = 0.f);
#pragma endregion

#pragma region FRUSTUM
	_bool IsInFrustum(const _float3& _vWorldPos, _float _fRange);
	_bool IsInFrustum(const _float3& _vCenter, const _float3& _vExtent);
#pragma endregion

#pragma region RENDERTARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& _TargetTag, _uint _iWidth, _uint _iHeight, DXGI_FORMAT _eFormat, const _float4& _vClearColor);
	HRESULT Add_MRT(const _wstring& _MRTTag, const _wstring& _TargetTag);
	HRESULT Begin_MRT(const _wstring& _MRTTag);
	HRESULT Begin_MRT(const _wstring& _MRTTag, ID3D11DepthStencilView* _pDSV);
	HRESULT End_MRT();
	HRESULT Bind_RT_SRV(const _wstring& _TargetTag, class CShader* _pShader, const _char* _pConstantName);
#ifdef  _DEBUG
	HRESULT Ready_RT_Debug(const _wstring& _TargetTag, _float _fX, _float _fY, _float _fSizeX, _float _fSizeY);
	HRESULT Render_MRT(const _wstring& _MRTTag, class CShader* _pShader, class CVIBuffer_Rect* _pVIBuffer);
#endif //  _DEBUG
#pragma endregion

#pragma region LIGHT_MANAGER
	HRESULT Add_Light(const LIGHT_DESC& _tLightDesc, class CTransform* _pTarget = nullptr, _vector _vOffset = XMVectorZero());
	void Render_Light(class CShader* _pShader, class CVIBuffer_Rect* _pVIBuffer);
	void Clear_Light();
	const LIGHT_DESC* Get_LightDesc(_uint _iIndex);
	void Scale_AllPointLightIntensity(_float _fIntensity);
	void Pop_Back_Lights(_uint _iCount);
#pragma endregion

#pragma region EFFECT_MANAGER
	void Push_EffectEVENT(CEffectManager::EFFECT_EVENT& _tEffectEvent);
	HRESULT Register_EffectEvent(_wstring _EventName, CEffect::EFFECT_INIT_DESC _tDesc);
	HRESULT Register_EffectEvent(_wstring _EventName, CTrailEffect::TRAIL_INIT_DESC _tDesc);
#pragma endregion

#pragma region SOUND_MANAGER
	void Play_Sound(const TCHAR* _pSoundKey, SOUND _eID, float _fVolume = 1.f);
	void Play_BGM(const TCHAR* _pSoundKey, float _fVolume = 1.f, bool _bUseCrossFade = false);
	void Play_RandomSound(const TCHAR* _pBaseName, int _iCount, float _fVolume = 1.f);
	void Play_Sound_Range(const TCHAR* _pSoundKey, float _fStartRatio, float _fEndRatio, float _fVolume = 1.f);
	void Stop_Sound(SOUND _eID);
	void Stop_AllSounds();
	void Set_ChannelVolume(SOUND _eID, float _fVolume);
#pragma endregion

#pragma region SHADOW
	HRESULT Add_ShadowLight(const SHADOW_LIGHT_DESC& _tShadowLightDesc);
	HRESULT Bind_ShadowTransformState(class CShader* _pShader, const _char* _pConstantName, D3DTS _eTransformState);
#pragma endregion

private:
	class CGraphicDevice*		m_pGraphicDevice = { nullptr };
	class CTimerManager*		m_pTimerManager = { nullptr };
	class CLevelManager*		m_pLevelManager = { nullptr };
	class CPrototypeManager*	m_pPrototypeManager = { nullptr };
	class CObjectManager*		m_pObjectManager = { nullptr };
	class CRenderer*			m_pRenderer = { nullptr };
	class CFontManager*			m_pFontManager = { nullptr };
	class CEventManager*		m_pEventManager = { nullptr };
	class CPipeLine*			m_pPipeLine = { nullptr };
	class CInputDevice*			m_pInputDevice = { nullptr };
	class CFileIO*				m_pFileIO = { nullptr };
	class CPicking*				m_pPicking = { nullptr };
	class CDialogueManager*		m_pDialogueManager = { nullptr };
	class CCollisionManager*	m_pCollisionManager = { nullptr };
	class CActorManager*		m_pActorManager = { nullptr };
	class CFrustum*				m_pFrustum = { nullptr };
	class CRenderTargetManager* m_pRenderTargetManager = { nullptr };
	class CLightManager*		m_pLightManager = { nullptr };
	class CEffectManager*		m_pEffectManager = { nullptr };
	class CSoundManager*		m_pSoundManager = { nullptr };
	class CShadow*				m_pShadow = { nullptr };

public:
	void Release_Engine();
	virtual void Free() override;
};
NS_END