matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_NormalTexture;

/* Texture */
Texture2D g_MainTexture;
Texture2D g_DistortionTexture;

/* Shader Parameters */
float g_fSize;
float g_Time;
float g_fRandom;

/* Bone */
matrix g_BoneMatrices[512];

sampler DefaultSampler = sampler_state
{
    Filter = Min_Mag_Mip_Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

BlendState AlphaBlend
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState AdditiveBlend
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

RasterizerState NONCULL
{
    CullMode = NONE;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fLife : TEXCOORD1;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fLife : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    float4 worldPos = float4(In.vPosition, 1.f);

    float4 viewPos = mul(worldPos, g_ViewMatrix);
    float4 projPos = mul(viewPos, g_ProjMatrix);

    Out.vPosition = projPos;
    Out.vTexcoord = In.vTexcoord;
    Out.fLife = In.fLife;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fLife : TEXCOORD1;
};

struct PS_OUT
{
    float4 vDiffuse : SV_Target0;
};

PS_OUT PS_MAIN_TRAIL(PS_IN In)
{
    PS_OUT Out;

    // uv.x = Èå¸§ ¹æÇâ (0=Ä®³¡/ÇöÀç, 1=²¿¸®)
    // uv.y = Æø ¹æÇâ   (0=tip¼ÒÄÏ,  1=base¼ÒÄÏ)
    float2 uv = In.vTexcoord;
    float4 tex = g_MainTexture.Sample(DefaultSampler, uv);

    /* tip <-> base */
    float edgeFade = smoothstep(0.0f, 0.18f, uv.y)
                   * smoothstep(1.0f, 0.82f, uv.y);

    /* tail */
    float tailFade = pow(1.0f - uv.x, 1.5f);
    
    /* Center Glow (Áß¾Ó ¹à°Ô) */
    float center = 1.0f - abs(uv.y - 0.5f) * 2.0f;
    center = pow(saturate(center), 0.6f);

    /* alpha */
    float alpha = edgeFade * tailFade;
    alpha = saturate(alpha * 2.5f);

    /* Blend */
    float3 glowColor = float3(0.8f, 0.95f, 1.0f); // »ìÂ¦ ÆÄ¶õ Èò»ö
    float3 col = lerp(tex.rgb, glowColor, center * 0.5f);
    col *= (1.0f + center * 0.8f);

    /* Out */
    Out.vDiffuse = float4(col * alpha, alpha);

    return Out;
}

PS_OUT PS_MAIN_TRAIL_DISTORTION(PS_IN In)
{
    PS_OUT Out;

    float2 uv = In.vTexcoord;

    /* distortion */
    float2 distUV = uv + float2(g_Time * 0.3f, g_Time * 0.15f);
    float2 distortion = g_DistortionTexture.Sample(DefaultSampler, distUV).rg;
    distortion = (distortion - 0.5f) * 2.0f;
    distortion *= 0.04f;

    /* uv */
    float2 warpedUV = uv + distortion;
    warpedUV = saturate(warpedUV);
    float4 tex = g_MainTexture.Sample(DefaultSampler, warpedUV);

    /* edge, tail, center */
    float edgeFade = smoothstep(0.0f, 0.18f, uv.y)
                   * smoothstep(1.0f, 0.82f, uv.y);
    float tailFade = pow(1.0f - uv.x, 1.5f);
    float center = 1.0f - abs(uv.y - 0.5f) * 2.0f;
    center = pow(saturate(center), 0.6f);

    /* distortion */
    float distortionMask = edgeFade * tailFade;

    /* alpha */
    float alpha = edgeFade * tailFade;
    alpha = saturate(alpha * 2.5f);

    /* color */
    float3 glowColor = float3(0.8f, 0.95f, 1.0f);
    float3 col = lerp(tex.rgb, glowColor, center * 0.5f);
    col *= (1.0f + center * 0.8f);

    /* shimmer */
    float shimmer = dot(distortion, distortion) * 30.f * distortionMask;
    col += shimmer;

    Out.vDiffuse = float4(col * alpha, alpha);

    return Out;
}

technique11 DefaultTechnique
{
    pass Trail
    {
        SetBlendState(AdditiveBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_TRAIL();
    }

    pass Trail_Distortion
    {
        SetBlendState(AdditiveBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN_TRAIL_DISTORTION();
    }
}