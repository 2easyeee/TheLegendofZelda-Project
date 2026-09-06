#pragma once
#ifndef MapTool_Struct_h__
#define MapTool_Struct_h__

namespace MapTool
{
	/* COLOR */
	typedef struct Color
	{
		_float4 vColor;
	}COLOR;

	/* BONE */
	typedef struct asBone
	{
		_string		nodeName;
		_int		nodeIndex = { -1 };
		_int		parentNodeIndex = { -1 };
		_float4x4	localTransform;
	}ASBONE;

	/* MESH */
	typedef struct asSkinBone
	{
		_int		nodeIndex;
		_float4x4	offsetMatrix;
	}ASSKINBONE;
	typedef struct asMesh
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
	}ASMESH;

	/* MATERIAL */
	typedef struct asMaterial
	{
		_string		name;
		COLOR		ambient;
		COLOR		diffuse;
		COLOR		specular;
		COLOR		emissive;
		_string		diffuseFile;
		_string		AOFile;
		_string		normalFile;
		_string		metallicFile;
		_string		roughnessFile;
	}ASMATERIAL;

	/* ANIMATION (CHANNEL) */
	typedef struct asChannel
	{
		_string boneName;
		vector<Key_F3> vecScalingKeys;
		vector<Key_F4> vecRotationKeys;
		vector<Key_F3> vecPositionKeys;
	}ASCHANNEL;

	/* ANIMATION */
	typedef struct asAnimation
	{
		_string AnimationName;
		_double fDuration;
		_double fTicksPerSecond;
		_uint iNumChannels;
		vector<ASCHANNEL> vecChannels;
	}ASANIMATION;
}

#endif // MapTool_Struct_h__