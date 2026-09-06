/* Transform */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_NormalTexture;
texture2D g_DepthTexture;

/* Texture */
Texture2D g_MainTexture;

/* Shader Parameters */
float g_fSize;

int g_iFrameX;
int g_iFrameY;
float g_fFrameSpeed;

float g_Time;

/* Billboard */
vector g_vCamPosition;

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

DepthStencilState DepthNoWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
    DepthFunc = LESS_EQUAL;
};

RasterizerState NONCULL
{
    CullMode = NONE;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    /* Transform */
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vProjPos = Out.vPosition;
    
    /* UV */
    float2 uv = In.vTexcoord;
    Out.vTexcoord = uv;
    
    return Out;
}

VS_OUT VS_MAIN_BILLBOARD(VS_IN In)
{
    VS_OUT Out;

    /* Billboard (Y축 고정) */
    float3 vWorldPos = g_WorldMatrix[3].xyz;

    float3 vLook = normalize(g_vCamPosition.xyz - vWorldPos);
    vLook.y = 0.f; // Y 고정
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook));
    float3 vUp = float3(0.f, 1.f, 0.f);
    
    float3 vLocalPos = In.vPosition * g_fSize;

    float3 vBillboardPos =
        vWorldPos +
        vRight * vLocalPos.x +
        vUp * vLocalPos.y +
        vLook * vLocalPos.z;

    /* Transform */
    float4 vViewPos = mul(float4(vBillboardPos, 1.f), g_ViewMatrix);
    float4 vProjPos = mul(vViewPos, g_ProjMatrix);

    Out.vPosition = vProjPos;
    Out.vProjPos = vProjPos;
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

float4 PS_MAIN_FLIPBOOK(VS_OUT In) : SV_Target
{
    float2 uv = In.vTexcoord;
    
    /* flipbook */
    int totalFrame = g_iFrameX * g_iFrameY;

    float frame = g_Time * g_fFrameSpeed;
    int frameIndex = (int) frame % totalFrame;

    float2 frameUV;
    frameUV.x = (frameIndex % g_iFrameX) / (float) g_iFrameX;
    frameUV.y = (frameIndex / g_iFrameX) / (float) g_iFrameY;

    float2 finalUV;
    finalUV = uv / float2(g_iFrameX, g_iFrameY);
    finalUV += frameUV;

    float4 baseColor = g_MainTexture.Sample(DefaultSampler, finalUV);
    
    /* Soft Effect */
    float2 vTexcoord;
    vTexcoord.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
    vTexcoord.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * 1000.f;
    
    float diff = fOldViewZ - In.vProjPos.w;
    float depthFade = saturate(diff * 0.05f);
    
    float alpha = baseColor.a;
    alpha *= lerp(1.0f, depthFade, 0.5f);
    
    /* 최종 */
    return float4(baseColor.rgb, alpha);
}

float4 PS_MAIN_FLIPBOOK_BACKBLACK(VS_OUT In) : SV_Target
{
    float2 uv = In.vTexcoord;
    
    /* flipbook */
    int totalFrame = g_iFrameX * g_iFrameY;

    float frame = g_Time * g_fFrameSpeed;
    int frameIndex = (int) frame % totalFrame;

    float2 frameUV;
    frameUV.x = (frameIndex % g_iFrameX) / (float) g_iFrameX;
    frameUV.y = (frameIndex / g_iFrameX) / (float) g_iFrameY;

    float2 finalUV;
    finalUV = uv / float2(g_iFrameX, g_iFrameY);
    finalUV += frameUV;

    float4 baseColor = g_MainTexture.Sample(DefaultSampler, finalUV);
    
    float brightness = dot(baseColor.rgb, float3(0.299, 0.587, 0.114));
    clip(brightness - 0.1f);
    
    /* Soft Effect */
    float2 vTexcoord;
    vTexcoord.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
    vTexcoord.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * 1000.f;
    
    float diff = fOldViewZ - In.vProjPos.w;
    float depthFade = saturate(diff * 0.05f);
    
    float alpha = brightness;
    alpha *= lerp(1.0f, depthFade, 0.5f);
    
    /* 최종 */
    return float4(baseColor.rgb, alpha);
}


technique11 DefaultTechnique
{
    pass AlphaBlendPass
    {
        SetDepthStencilState(DepthNoWrite, 0);
        SetBlendState(AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);
        SetRasterizerState(NONCULL);

        VertexShader = compile vs_5_0 VS_MAIN_BILLBOARD();
        PixelShader = compile ps_5_0 PS_MAIN_FLIPBOOK_BACKBLACK();
    }
}