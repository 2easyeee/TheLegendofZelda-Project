matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;
texture2D g_Texture;

/* GBuffer Data */
texture2D g_DiffuseTexture;
texture2D g_NormalTexture;
texture2D g_ShadeTexture;
texture2D g_MaterialTexture;
texture2D g_SpecularTexture;
texture2D g_DepthTexture;

/* Light Info */
vector g_vLightDirectional;
vector g_vLightPos;
float g_fLightRange;
float g_fLightIntensity;

/* Light Color */
vector g_vLightDiffuse;
vector g_vLightAmbient;
vector g_vLightSpecular;

/* Material */
vector g_vMtrlAmbient = 1.f;
vector g_vMtrlSpecular = 1.f;

/* Camera */
vector g_vCameraPosition;

/* Shadow */
matrix g_LightViewMatrix;
matrix g_LightProjMatrix;
texture2D g_ShadowTexture;
float2 g_vShadowTexelSize;

/* FXAA */
float2 g_vInverseScreenSize;
uint g_iUseFXAA;

sampler DefaultSampler = sampler_state
{
    Filter = Min_Mag_Mip_Linear;
};

sampler ShadowSampler = sampler_state
{
    Filter = Min_Mag_Mip_Point;
    AddressU = Clamp;
    AddressV = Clamp;
};

sampler FXAASampler = sampler_state
{
    Filter = Min_Mag_Mip_Linear;
    AddressU = Clamp;
    AddressV = Clamp;
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
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION; 
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;  
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

struct PS_OUT_LIGHT
{
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_Target1;
};

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{    
    PS_OUT_LIGHT Out;
    
    /* Normal */
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vNormal = normalize(vector(vNormalDesc.xyz * 2.f - 1.f, 0.f));
    
    /* Depth */ 
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    /* ViewZ */ 
    float fViewZ = vDepthDesc.y * 1000.f;
    
    /* Material */
    float4 material = g_MaterialTexture.Sample(DefaultSampler, In.vTexcoord);
    float AOValue = material.r;
    float RoughnessValue = material.g;
    float MetallicValue = material.b;
    float SpecularValue = material.a;
    
    /* Half-Lamber */
    float3 lightDir = normalize(g_vLightDirectional) * -1.f;
    
    /* Half Lambert */
    float NdotL = dot(vNormal, lightDir);
    NdotL = saturate(NdotL);
    NdotL = pow(NdotL, 0.8f);
    
    /* Diffuse */
    float3 diffuse = g_vLightDiffuse.rgb * NdotL * 1.2f;
    
    /* Ambient */
    float3 ambient = g_vLightAmbient.rgb * 0.6f;
    ambient *= lerp(0.6f, 1.0f, AOValue);
    
    /* G-Buffer의 깊이로 월드 위치 복원 */
    float4 vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    vWorldPos *= fViewZ;
    
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    /* ViewDir */
    float3 viewDir = normalize(g_vCameraPosition.xyz - vWorldPos.xyz);
    
    /* Blinn-Phong Specular */
    float3 halfDir = normalize(lightDir + viewDir);
    
    float specPower = lerp(64.f, 8.f, RoughnessValue);
    float specIntensity = lerp(0.4f, 0.1f, RoughnessValue);
    
    float spec = pow(saturate(dot(vNormal, halfDir)), specPower);
    
    float3 specular = g_vLightSpecular.rgb 
                    * spec
                    * specIntensity
                    * SpecularValue;
    
    /* 확산광과 반사광 결과 분리 */
    Out.vShade = float4(diffuse + ambient, 1.f);
    Out.vSpecular = float4(specular, 1.f);
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    /* Normal */
    vector vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    
    /* Depth */
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    /* Blinn-Phong */
    float4 material = g_MaterialTexture.Sample(DefaultSampler, In.vTexcoord);
    float AOValue = material.r;
    float RoughnessValue = material.g;
    float MetallicValue = material.b;
    float SpecularValue = material.a;
    
    /* WorldPos */
    vector vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* ViewSpace */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos /= vWorldPos.w;
    
    /* WorldSpace */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    /* Normal */
    float4 vNormal = normalize(vector(vNormalDesc.xyz * 2.f - 1.f, 0.f));
    
    /* Light */
    vector vLightDir = g_vLightPos - vWorldPos;
    float fDistance = length(vLightDir);
    vector vLightDirN = normalize(vLightDir);
    
    /* Attenuation */
    float fAtt = saturate(1.0 - (fDistance / g_fLightRange));
    
    /* Lambert */
    float NdotL = saturate(dot(vLightDirN, vNormal));

    /* Diffuse */
    float3 baseColor = g_vLightDiffuse.rgb * NdotL * fAtt * g_fLightIntensity;
    Out.vShade = float4(baseColor, 1.f);
    
    /* Blinn-Phong Specular */
    /* 광원과 시선 방향의 Half Vector로 반사 벡터 계산을 생략 */
    /* H = normalize(L + V) */
    float3 viewDir = normalize(g_vCameraPosition.xyz - vWorldPos.xyz);
    float3 halfDir = normalize(vLightDirN.xyz + viewDir);
    
    /* Roughness에 따라 반사광의 크기와 강도를 조절 */
    float specPower = lerp(64.f, 8.f, RoughnessValue);
    float specIntensity = lerp(0.4f, 0.1f, RoughnessValue);
    
    /* 재질의 Specular 값을 반사광 강도에 반영 */
    float spec = pow(saturate(dot(vNormal.xyz, halfDir)), specPower);
    
    float3 specular = g_vLightSpecular.rgb
                        * spec
                        * specIntensity
                        * SpecularValue
                        * fAtt
                        * g_fLightIntensity;
    
    Out.vSpecular = float4(specular, 1.f);
    
    return Out;
}

PS_OUT PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(DefaultSampler, In.vTexcoord);
    vector vSpecular = g_SpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    
    /* Final */
    float3 color = vDiffuse * vShade + vSpecular;
    
    /* Shadow */
    float4 vDepthDesc = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float fViewZ = vDepthDesc.y * 1000.f;
    
    float4 vWorldPos;
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= fViewZ;
    
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    float4 vLightClipPos = mul(vWorldPos, g_LightViewMatrix);
    vLightClipPos = mul(vLightClipPos, g_LightProjMatrix);
    
    float2 vShadowUV;
    vShadowUV.x = (vLightClipPos.x / vLightClipPos.w) * 0.5f + 0.5f;
    vShadowUV.y = (vLightClipPos.y / vLightClipPos.w) * -0.5f + 0.5f;
 
    /* Shadow Map 범위 안의 픽셀만 계산하여 반복 샘플링을 방지 */
    float fShadowRatio = 0.f;
    bool bInsideShadowMap = vShadowUV.x >= 0.f && vShadowUV.x <= 1.f && vShadowUV.y >= 0.f && vShadowUV.y <= 1.f && vLightClipPos.w > 0.f;
    if (bInsideShadowMap)
    {
        const float fBias = 0.1f;
        const float fPCFRadius = 1.f;
        
        float fCurrentDepth = vLightClipPos.w;
        
        /* 중심 Shadow UV 주변 3x3 Texel의 깊이를 비교해 그림자 비율 계산 */
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            [unroll]
            for (int x = -1; x <= 1; ++x)
            {
                float2 vOffset = float2(x, y) * g_vShadowTexelSize * fPCFRadius;
                
                float2 vSampleUV = clamp(vShadowUV + vOffset, g_vShadowTexelSize * 0.5f, 1.f - g_vShadowTexelSize * 0.5f);
                float fStoredDepth = g_ShadowTexture.Sample(ShadowSampler, vSampleUV).y * 1000.f;
                
                if (fStoredDepth <= fCurrentDepth - fBias)
                    fShadowRatio += 1.f;
            }
        }
        
        /* 9개 비교 결과의 평균을 0~1 그림자 강도로 변환 */
        fShadowRatio /= 9.f;
    }
    
    const float fShadowBrightness = 0.5f;
    color *= lerp(1.f, fShadowBrightness, fShadowRatio);
    
    /* Gamma */
    color = pow(saturate(color), 1.f / 1.5f); // 감마 보정
    
    Out.vColor = float4(color, 1.f);
    
    return Out;
}

float Get_Luminance(float3 vColor)
{
    return dot(vColor, float3(0.299f, 0.587f, 0.114f));
}

/* FXAA : Fast Approximate Anti-Aliasing */
/* 주변 픽셀의 명암 차이로 Edge 방향을 계산하고 경계의 계단 현상을 완화 */

PS_OUT PS_MAIN_FXAA (PS_IN In)
{
    PS_OUT Out;

    const float2 vTexel = g_vInverseScreenSize;
    const float2 vUV = In.vTexcoord;
    
    float4 vColorM = g_Texture.Sample(FXAASampler, vUV);

    /* FXAA가 꺼진 경우에도 SceneTarget은 Back Buffer로 복사한다. */
    if (0 == g_iUseFXAA)
    {
        Out.vColor = float4(vColorM.rgb, 1.f);
        return Out;
    }
    
    float4 vColorNW = g_Texture.Sample(FXAASampler, vUV + float2(-1.f, -1.f) * vTexel);
    float4 vColorNE = g_Texture.Sample(FXAASampler, vUV + float2(1.f, -1.f) * vTexel);
    float4 vColorSW = g_Texture.Sample(FXAASampler, vUV + float2(-1.f, 1.f) * vTexel);
    float4 vColorSE = g_Texture.Sample(FXAASampler, vUV + float2(1.f, 1.f) * vTexel);
    
    float fLumaM = Get_Luminance(vColorM.rgb);
    float fLumaNW = Get_Luminance(vColorNW.rgb);
    float fLumaNE = Get_Luminance(vColorNE.rgb);
    float fLumaSW = Get_Luminance(vColorSW.rgb);
    float fLumaSE = Get_Luminance(vColorSE.rgb);
    
    float fLumaMin = min(fLumaM, min(min(fLumaNW, fLumaNE), min(fLumaSW, fLumaSE)));
    
    float fLumaMax = max(fLumaM, max(max(fLumaNW, fLumaNE), max(fLumaSW, fLumaSE)));
    
    /* 대각선 픽셀의 명암 차이로 Edge 방향 계산 */
    float2 vDirection;
    vDirection.x = -((fLumaNW + fLumaNE) - (fLumaSW + fLumaSE));
    vDirection.y = ((fLumaNW + fLumaSW) - (fLumaNE + fLumaSE));
    
    /* 어두운 영역의 작은 노이즈가 과도하게 보정되는 현상 방지 */
    const float fReduceMultiplier = 1.f / 8.f;
    const float fReduceMinimum = 1.f / 128.f;
    const float fMaximumSpan = 8.f;
    
    float fDirectionReduce = max((fLumaNW + fLumaNE + fLumaSW + fLumaSE) * (0.25f * fReduceMultiplier), fReduceMinimum);
    float fInverseDirectionMin = 1.f / (min(abs(vDirection.x), abs(vDirection.y)) + fDirectionReduce);
    vDirection = clamp(vDirection * fInverseDirectionMin, -fMaximumSpan, fMaximumSpan) * vTexel;
    
    /* Edge 방향 안쪽의 두 지점을 샘플링 */
    float4 vColorA = 0.5f * (g_Texture.Sample(FXAASampler, vUV + vDirection * (1.f / 3.f - 0.5f)) 
                    + g_Texture.Sample(FXAASampler, vUV + vDirection * (2.f / 3.f - 0.5f)));
    
    /* Edge 양 끝을 추가 샘플링해 부드러운 후보 생성 */
    float4 vColorB = vColorA * 0.5f 
        + 0.25f * (g_Texture.Sample(FXAASampler, vUV + vDirection * -0.5f)
            + g_Texture.Sample(FXAASampler, vUV + vDirection * 0.5f));
    
    float fLumaB = Get_Luminance(vColorB.rgb);
    
    /* 주변 명암 범위를 벗어나면 과도한 Blur로 판단 */
    float3 vFinalColor = (fLumaB < fLumaMin || fLumaB > fLumaMax) ? vColorA.rgb : vColorB.rgb;
    Out.vColor = float4(vFinalColor, 1.f);

    return Out;
}

technique11 DefaultTechnique
{
    pass Debug
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Light_Directional
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }

    pass Light_Point
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }

    pass Combined
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }

    pass FXAA
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FXAA();
    }
}
