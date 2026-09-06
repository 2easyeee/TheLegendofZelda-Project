#pragma once

#include "Transform.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "VIBuffer_Rect.h"
#include "VIBuffer_Terrain.h"
#include "Mesh.h"
#include "Material.h"
#include "VIBuffer_Line.h"
#include "VIBuffer_Plane.h"
#include "Collider.h"
#include "Navigation.h"
#include "VIBuffer_Particle_Rect.h"
#include "VIBuffer_Particle_Point.h"
#include "VIBuffer_Sphere.h"
#include "VIBuffer_Trail.h"

NS_BEGIN(Engine)
class CPrototypeManager final : public CBase
{
private:
	CPrototypeManager();
	virtual ~CPrototypeManager() = default;

public:
	HRESULT Initailize(_uint _iTotalLevelCnt);
	void Clear(_uint _iLevelIndex);

	HRESULT Add_Prototype(_uint _iLevelIndex, const _wstring& _strPrototypeTag, class CBase* _pPrototype);
	CBase* Clone_Prototype(PROTOTYPE _ePrototype, _uint _iLevelIndex, const _wstring& _strPrototypeTag, void* _pArg);

	void Get_PrototypeTag(_uint _iLevelIndex, vector<_wstring>& _outTags) const;

private:
	map<const _wstring, class CBase*>* m_Prototypes = { nullptr };

	_uint m_iTotalLevelCnt = {};

private:
	CBase* Find_Prototype(_uint _iLevelIndex, const _wstring& _strPrototypeTag);

public:
	static CPrototypeManager* Create(_uint _iTotalLevelCnt);
	virtual void Free() override;

};
NS_END