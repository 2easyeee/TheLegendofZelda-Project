#ifndef Engine_Struct_h__
#define Engine_Struct_h__

#include "Engine_Typedef.h"

namespace Engine
{
	typedef struct tagEngineDesc
	{
		HINSTANCE	 hInstance;
		HWND		 hWnd;
		WINMODE		 eMode;
		unsigned int iWinSizeX;
		unsigned int iWinSizeY;
		unsigned int iTotalLevelCnt;
	}ENGINE_DESC;

	typedef struct tagStateTime
	{
		_bool		 bActive = { false };
		_float		 fAccTime;				// 누적 시간
		_float		 fAccDurationTime;		// 목표 시간

		void Start(_float _fDuration)
		{
			bActive = true;
			fAccTime = 0.f;
			fAccDurationTime = _fDuration;
		}

		_float Get_Ratio() const
		{
			return fAccDurationTime > 0.f ? min(fAccTime / fAccDurationTime, 1.f) : 1.f;
		}

		_bool IsFinished() const
		{
			return fAccTime >= fAccDurationTime;
		}

		_bool Tick(_float _fTimeDelta)
		{
			if (!bActive)
				return false;

			fAccTime += _fTimeDelta;

			if (fAccTime >= fAccDurationTime)
			{
				bActive = false;
				return true;
			}

			return false;
		}

		_bool Tick_Loop(_float _fTimeDelta)
		{
			if (!bActive)
				return false;

			fAccTime += _fTimeDelta;

			if (fAccTime >= fAccDurationTime)
			{
				fAccTime -= fAccDurationTime;
				return true;
			}

			return false;
		}

		_bool Tick_Ratio(_float _fTimeDelta, _float _fTriggerRatio)
		{
			if (!bActive)
				return false;

			_float fPrevRatio = Get_Ratio();

			fAccTime += _fTimeDelta;

			_float fCurRatio = Get_Ratio();

			if (fPrevRatio < _fTriggerRatio && fCurRatio >= _fTriggerRatio)
				return true;

			return false;
		}

	}STATE_TIME;

	/* NavMesh Triangle */
	typedef struct tagVertexPosition
	{
		XMFLOAT3		vPosition;

		static const unsigned int			iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXPOS;

	/* Color, Texture */
	typedef struct tagVertexPositionTexcoord
	{
		XMFLOAT3 vPosition;
		XMFLOAT2 vTexcoord;

		static const unsigned int iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXPOSTEX;

	/* Texture(Normal) */
	typedef struct tagVertexPositionNormalTexcoord
	{
		XMFLOAT3 vPosition;
		XMFLOAT3 vNormal;
		XMFLOAT2 vTexcoord;

		static const unsigned int iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXNORTEX;

	/* Line */
	typedef struct tagVertexLine
	{
		XMFLOAT3 vPosition;
		XMFLOAT4 vColor;

		static const unsigned int iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXLINE;

	/* Mesh (NONANIM) */
	typedef struct tagVertexMesh
	{
		XMFLOAT3 vPosition;
		XMFLOAT3 vNormal;
		XMFLOAT2 vTexcoord;
		XMFLOAT3 vTangent;
		XMFLOAT3 vBinormal;

		static const unsigned int iNumElements = { 5 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,	0},
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXMESH;

	/* Model (SKINNED) */
	typedef struct tagVertexAnimMesh
	{
		XMFLOAT3 vPosition;
		XMFLOAT3 vNormal;
		XMFLOAT2 vTexcoord;
		XMFLOAT3 vTangent;
		XMFLOAT3 vBinormal;

		XMUINT4 vBlendIndex;
		XMFLOAT4 vBlendWeight;

		static const unsigned int iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	   0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,	   0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BLENDINDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BLENDWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXANIMMESH;
	
	/* Mesh Instancing (NONANIM) */
	typedef struct tagVertexMeshInstance
	{
		static const unsigned int iNumElements = { 9 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
		{
			/* Slot 0: Mesh Vertex */
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			/* Slot 1: Instance World Matrix */
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
	} VTXMESH_INSTANCE;

	using VERTEXTYPE = VTXMESH;

	/* Particle */
	typedef struct tagVertexParticleRect
	{
		static const unsigned int			iNumElements = { 8 };

		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT,			1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 72, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTX_PARTICLE_RECT;

	typedef struct tagVertexParticlePoint
	{
		static const unsigned int			iNumElements = { 6 };

		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	  1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			//{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 72, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTX_PARTICLE_POINT;

	/* Trail */
	typedef struct tagVertexTrail
	{
		XMFLOAT3 vPosition;
		XMFLOAT2 vTexcoord;
		float     fLife;

		static const unsigned int iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXTRAIL;

	/* Bone */
	typedef struct tagBone
	{
		_string				nodeName;
		_int				nodeIndex = { -1 };
		_int				parentNodeIndex = { -1 };
		_float4x4			localTransform;
		vector<_int>		children;
	}BONE_DESC;

	/* Mesh */
	typedef struct tagSkinBoneDesc
	{
		_int		nodeIndex;
		_float4x4	offsetMatrix;
	}SKINBONE_DESC;
	typedef struct tagMeshDesc
	{
		_string				meshName;
		_string				materialName;

		MODEL				eModelType;
		vector<VTXMESH>		nonAnimVertices;
		vector<VTXANIMMESH> skinnedVertices;
		vector<_uint>		commonIndices;

		_int				effectTotalBoneCnt; // iNumBones 이 메시에게 영향을 주는 뼈의 개수
		vector<_int>		effectBoneIndices; // m_Bones 이 몸통 메시에 영향을 주는 뼈들 중 blendIndex (== 이 몸통 MEsh 의 몇번쨋것) 을 저장
		vector<_float4x4>	offsetMatrices;
	}MESH_DESC;

	/* Animation (Channel) */
	typedef struct tagKeyF3
	{
		_double fTime;
		_float3 vValue;
	}Key_F3;

	typedef struct tagKeyF4
	{
		_double fTime;
		_float4 vValue;
	}Key_F4;

	typedef struct tagChannelDesc
	{
		_string boneName;
		_uint iNumScalingKeys;
		_uint iNumRoationKeys;
		_uint iNumPositionKeys;
		vector<Key_F3> vecScalingKeys;
		vector<Key_F4> vecRotationKeys;
		vector<Key_F3> vecPositionKeys;
	}CHANNEL_DESC;

	/* Animation */
	typedef struct tagAnimationDesc
	{
		_string AnimationName;
		_double fDuration;
		_double fTicksPerSecond;
		_uint iNumChannels;
		vector<CHANNEL_DESC> vecChannelDesc;
	}ANIMATION_DESC;

	/* KeyFrame */
	typedef struct tagKeyFrame
	{
		XMFLOAT3			vScale;
		XMFLOAT4			vRotation;
		XMFLOAT3			vTranslation;
		_float				fTrackPosition;
	}KEYFRAME;

	/* TAGS */
	typedef struct tagExportTags
	{
		_wstring	ObjectID;
		_wstring	LayerTag;
		_wstring	GameObjectTag;
		_wstring	ShaderTag;
		_wstring	ModelTag;
		_wstring	MeshPath;
	}EXPORT_TAGS;
	typedef struct tagExportPartTags
	{
		PART		PartType;
		_wstring	ObjectID;
		_wstring	LayerTag;
		_wstring	GameObjectTag;
		_wstring	ShaderTag;
		_wstring	ModelTag;
		_wstring	MeshPath;
		_wstring	SocketName;
	}EXPORT_PART_TAGS;
	
	/* Particle */
	typedef struct tagVertexParticle
	{
		XMFLOAT4		vRight, vUp, vLook, vTranslation;
		XMFLOAT2		vLifeTime;
		XMFLOAT4		vColor;
	}VTXPARTICLE;

	/* Shader */
	typedef struct tagShaderParamDesc
	{
		/* Name  */
		_wstring EffectName = { TEXT("Unknown") };

		/* LifeTime */
		_float4 vScale = { 1.f, 1.f, 1.f, 1.f};
		_float4 vRotation = { 0.f, 0.f, 0.f, 0.f };
		_float4 vDirection = { 0.f, 1.f, 0.f, 0.f};
		_float fDuration = { 0.1f };

		/* Color */
		_float4 vColor = { 1.f, 1.f, 1.f, 1.f };
		_float fAlpha = { 1.f };

		/* Time */
		_float fTime = { 0.f };

		/* UV */
		_float2 vUVScale = { 1.f,1.f };
		_float2 vUVSpeed = { 0.f,0.f };

		/* Noise 0 */
		_float2 vNoiseSpeed_0 = { 0.f, 0.f };
		_float2 vNoiseScale_0 = { 1.f, 1.f };

		/* Noise 1 */
		_float2 vNoiseSpeed_1 = { 0.f, 0.f };
		_float2 vNoiseScale_1 = { 1.f, 1.f };

		/* Distortion */
		_float fDistortion = { 0.f };
		_float2 vDistortionDir = { 1.f,0.f };
		_float fDistortionSpeed = { 1.f };

		/* Dissolve */
		_float fDissolve = { 0.f };
		_float4 vDissolveColor = { 1.f, 0.5f, 0.f, 1.f };
		_float fDissolveEdge = { 0.05f };

		/* Emissive */
		_float fEmissive = { 1.f };

		/* Flipbook */
		_int iFrameX = { 1 };
		_int iFrameY = { 1 };
		_float fFrameSpeed = { 1.f };
	}SHADER_PARAM_DESC;

	typedef struct tagShaderTextureDesc
	{
		_wstring MainTexture;
		_wstring Noise_0;
		_wstring Noise_1;
		_wstring Mask;
		_wstring Distortion;
		_wstring Dissolve;
	}SHADER_TEXTURE_DESC;

	typedef struct tagShaderTimeline
	{
		_float3 vScaleStart = { 1.f, 1.f, 1.f };
		_float3 vScaleEnd = { 1.f, 1.f, 1.f };

		_float3 vRotationAxis = { 0.f, 1.f, 0.f};
		_float  fRotationSpeed = { 1.f };

		_float fAlphaStart = { 1.f };
		_float fAlphaEnd = { 1.f };

		_float fDissolveStart;
		_float fDissolveEnd;
	}SHADER_TIMELINE;

	/* Light */
	typedef struct tagLightDesc
	{
		LIGHT eType;
		XMFLOAT4 vDirection;
		XMFLOAT4 vPosition;

		float fRange;

		XMFLOAT4 vDiffuse;
		XMFLOAT4 vAmbient;
		XMFLOAT4 vSpecular;

		/* Player */
		LIGHT_USAGE eUsage = LIGHT_USAGE::STATIC;
		float fIntensity;
	}LIGHT_DESC;

	/* Shadow */
	typedef struct tagShadowLightDesc
	{
		_float4 vEye;
		_float4 vAt;

		_float fFovy;
		_float fNear;
		_float fFar;
		_float fAspect;
	}SHADOW_LIGHT_DESC;
}

#endif // Engine_Struct_h__