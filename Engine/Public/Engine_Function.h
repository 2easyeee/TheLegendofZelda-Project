#ifndef Engine_Function_h__
#define Engine_Function_h__

#include "Engine_Typedef.h"

namespace Engine
{
	/* 00. Delete */
	template<typename T>
	void Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}
	
	template<typename T>
	void Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete[] Pointer;
			Pointer = nullptr;
		}
	}

	/* 01. RefCnt */
	template<typename T>
	unsigned int Safe_AddRef(T& pInstance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != pInstance)
			iRefCnt = pInstance->AddRef();

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_Release(T& pInstance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != pInstance)
		{
			iRefCnt = pInstance->Release();

			if (0 == iRefCnt)
				pInstance = nullptr;
		}
		return iRefCnt;
	}

	class CTag_Finder
	{
	private:
		const TCHAR* m_pStr;
	public:
		CTag_Finder(const TCHAR* pStr) : m_pStr(pStr) {}

	public:
		template<typename T>
		bool operator()(T& Pair)
		{
			if (!lstrcmp(m_pStr, Pair.first))
				return true;
			return false;
		}
	};

	/* CHARACTER */
	inline _string WSTRTOCHAR(const _wstring& _wstr)
	{
		if (_wstr.empty())
			return _string();

		int size = WideCharToMultiByte(
			CP_UTF8, 0,
			_wstr.c_str(), -1, 
			nullptr, 0, nullptr, nullptr);

		if (size <= 0)
			return _string();

		_string result;
		result.resize(size - 1);

		WideCharToMultiByte(
			CP_UTF8, 0,
			_wstr.c_str(), -1,
			&result[0], size,
			nullptr, nullptr);

		return result;
	};

	inline _wstring STRTOWSTR(const _string& str)
	{
		if (str.empty())
			return std::wstring();

		int size = MultiByteToWideChar(
			CP_UTF8, 0,
			str.c_str(), -1,
			nullptr, 0);

		if (size <= 0)
			return std::wstring();

		std::wstring result;
		result.resize(size - 1);

		MultiByteToWideChar(
			CP_UTF8, 0,
			str.c_str(), -1,
			&result[0], size);

		return result;
	}

	inline _wstring CHARTOWSTR(const char* str)
	{
		if (!str || str[0] == '\0')
			return _wstring();

		int size = MultiByteToWideChar(
			CP_UTF8, 0,
			str, -1,
			nullptr, 0);

		if (size <= 0)
			return _wstring();

		_wstring result;
		result.resize(size - 1);

		MultiByteToWideChar(
			CP_UTF8, 0,
			str, -1,
			&result[0], size);

		return result;
	}

	inline void Replace(OUT _string& str, _string comp, _string rep)
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

	inline _wstring FindAndRemove_Number(const _wstring& objectID)
	{
		if (objectID.empty())
			return objectID;

		const size_t pos = objectID.rfind(L'_');
		if (pos == _wstring::npos)
			return objectID;

		for (size_t i = pos + 1; i < objectID.size(); ++i)
		{
			if (!iswdigit(objectID[i]))
				return objectID;
		}

		return objectID.substr(0, pos);
	}

	/* XML */
	inline void ReadRaw(HANDLE hFile, void* data, DWORD size)
	{
		DWORD read = 0;
		ReadFile(hFile, data, size, &read, nullptr);
		assert(read == size);
	}

	inline string ReadString(HANDLE hFile)
	{
		_uint len = 0;
		ReadRaw(hFile, &len, sizeof(_uint));

		_string str;
		if (len > 0)
		{
			str.resize(len);
			ReadRaw(hFile, str.data(), len);
		}
		return str;
	}

	/* EFFECT */
	inline _float EaseInCubic(_float _t)
	{
		_t = max(0.f, min(1.f, _t));
		return _t * _t * _t;
	}

	inline _float EaseOutCubic(_float _t)
	{
		_t = max(0.f, min(1.f, _t));
		_float inv = 1.f - _t;
		return 1.f - inv * inv * inv;
	}

	inline _float EaseInOutCubic(_float _t)
	{
		_t = max(0.f, min(1.f, _t));
		if (_t < 0.5f)
			return 4.f * _t * _t * _t;

		_float u = -2.f * _t + 2.f;
		return 1.f - (u * u * u) * 0.5f;
	}

	/* Lerp */
	inline _float Lerp(_float a, _float b, _float t)
	{
		return a + (b - a) * t;
	}

	inline _float3 Lerp3(const _float3& a, const _float3& b, float t)
	{
		return _float3(
			Lerp(a.x, b.x, t),
			Lerp(a.y, b.y, t),
			Lerp(a.z, b.z, t));
	}
}

#endif // Engine_Function_h__