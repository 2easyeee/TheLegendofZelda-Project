#include "FontManager.h"

#include "CustomFont.h"

CFontManager::CFontManager()
{
}

HRESULT CFontManager::Initialize()
{
    return S_OK;
}

HRESULT CFontManager::Add_Fonts(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pFontTag, const _tchar* _pFontFilePath)
{
    if (nullptr != Find_Fonts(_pFontTag))
        return E_FAIL;

    CCustomFont* pFont = CCustomFont::Create(_pDevice, _pDeviceContext, _pFontFilePath);
    if (nullptr == pFont)
        return E_FAIL;

    m_Fonts.emplace(_pFontTag, pFont);

    return S_OK;
}

HRESULT CFontManager::Add_Message(const _tchar* _pFontTag, const _tchar* _pText, _fvector _vPosition, _float _fScale, _fvector _vColor, _float _fRotation)
{
    CCustomFont* pFont = Find_Fonts(_pFontTag);
    if (nullptr == pFont)
        return E_FAIL;

    FONT_DESC FontDesc = {};
    FontDesc.pFont = pFont;
    FontDesc.wstrText = _pText;
    XMStoreFloat2(&FontDesc.vPosition, _vPosition);
    FontDesc.fScale = _fScale;
    XMStoreFloat4(&FontDesc.vColor, _vColor);
    FontDesc.fRotation = _fRotation;

    m_vecFonts.push_back(FontDesc);

    return S_OK;
}

HRESULT CFontManager::Render_Fonts(ID3D11DeviceContext* _DeviceContext)
{
    if (m_vecFonts.empty())
        return S_OK;

    SpriteBatch SpriteBatch(_DeviceContext);

    SpriteBatch.Begin();
    for (auto& Message : m_vecFonts)
    {
        /* Center Alignment */
        auto lines = Split_Lines(Message.wstrText);

        _float fTotalHeight = 0.f;
        vector<_float> lineHeights;

        /* Compute Line Count */
        for (auto& Line : lines)
        {
            _vector vSize = Message.pFont->Get_Font()->MeasureString(Line.c_str());
            _float2 vTextSize;
            XMStoreFloat2(&vTextSize, vSize);

            _float h = vTextSize.y * Message.fScale;
            lineHeights.push_back(h);
            fTotalHeight += h;
        }

        _float fOffsetY = -fTotalHeight * 0.5f;
        
        /* Render Lines */
        for (size_t i = 0; i < lines.size(); ++i)
        {
            const auto& Line = lines[i];

            _vector vSize = Message.pFont->Get_Font()->MeasureString(Line.c_str());
            _float2 vTextSize;
            XMStoreFloat2(&vTextSize, vSize);

            _float2 vNewCenterPosition = Message.vPosition;
            vNewCenterPosition.x -= (vTextSize.x * Message.fScale * 0.5f);
            vNewCenterPosition.y += fOffsetY;

            Message.pFont->Get_Font()->DrawString(
                &SpriteBatch,
                Line.c_str(),
                vNewCenterPosition,
                XMLoadFloat4(&Message.vColor),
                Message.fRotation,
                XMFLOAT2(0.f, 0.f),
                Message.fScale
            );

            fOffsetY += lineHeights[i];
        }
    }
    SpriteBatch.End();

    /* Reset */
    m_vecFonts.clear();

    return S_OK;
}

CCustomFont* CFontManager::Find_Fonts(const _tchar* _pFontTag)
{
    auto iter = find_if(m_Fonts.begin(), m_Fonts.end(), CTag_Finder(_pFontTag));
    if (iter == m_Fonts.end())
        return nullptr;

    return iter->second;
}

vector<_wstring> CFontManager::Split_Lines(const _wstring& _wstrText)
{
    vector<_wstring> lines;
    size_t start = 0, pos = 0;

    while ((pos = _wstrText.find(L'\n', start)) != _wstring::npos)
    {
        lines.push_back(_wstrText.substr(start, pos - start));
        start = pos + 1;
    }
    lines.push_back(_wstrText.substr(start));
    return lines;
}

CFontManager* CFontManager::Create()
{
    CFontManager* pInstance = new CFontManager();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("FAILED TO CREATED : CFontManager");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CFontManager::Free()
{
    /* map release */
    for (auto& Pair : m_Fonts)
        Safe_Release(Pair.second);
    m_Fonts.clear();
}
