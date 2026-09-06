matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_NormalTexture;
texture2D g_AOTexture;
texture2D g_MTLTexture;
texture2D g_SMTTexture;

sampler DefaultSampler = sampler_state
{
    Filter = Min_Mag_Mip_Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    
    float4 vWorld0 : WORLD0;
    float4 vWorld1 : WORLD1;
    float4 vWorld2 : WORLD2;
    float4 vWorld3 : WORLD3;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    float4x4 matWorld = matrix(In.vWorld0, In.vWorld1, In.vWorld2, In.vWorld3);
    float4x4 matWV, matWVP;
    matWV = mul(matWorld, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), matWorld));
    Out.vTangent = normalize(mul(vector(In.vTangent, 0.f), matWorld));
    Out.vBinormal = normalize(mul(vector(In.vBinormal, 0.f), matWorld));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), matWorld);
    Out.vProjPos = Out.vPosition;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
};

struct PS_OUT
{
    float4 vDiffuse : SV_Target0;
    float4 vNormal : SV_Target1;
    float4 vDepth : SV_TARGET2;
    float4 vMaterial : SV_Target3;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    /* Diffuse */
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vDiffuse.a <= 0.3f)
        discard;
    Out.vDiffuse = vDiffuse;
       
    /* Normal */
    vector vNormalTex = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float3 vNormal = vNormalTex.xyz * 2.f - 1.f;
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz * -1.f, In.vNormal.xyz);
    Out.vNormal = vector(normalize(mul(vNormal, WorldMatrix)) * 0.5f + 0.5f, 0.f);
    
    /* Depth */
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
    
    /* Material */
    float fAO = g_AOTexture.Sample(DefaultSampler, In.vTexcoord).r;
    float AO = pow(fAO, 2.f);
    AO = lerp(0.2f, 1.0f, AO);
    float fSMT = g_SMTTexture.Sample(DefaultSampler, In.vTexcoord).r;
    float fMTL = g_MTLTexture.Sample(DefaultSampler, In.vTexcoord).r;
    float fSpecular = lerp(1.f, 1.f, fMTL);
    
    Out.vMaterial = float4(AO, fSMT, fMTL, fSpecular);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}