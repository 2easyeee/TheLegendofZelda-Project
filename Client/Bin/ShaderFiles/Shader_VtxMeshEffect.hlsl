matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_NormalTexture;
texture2D g_DepthTexture;

/* Texture */
Texture2D g_MainTexture;
Texture2D g_NoiseTexture_0;
Texture2D g_NoiseTexture_1;
Texture2D g_MaskTexture;
Texture2D g_DistortionTexture;
Texture2D g_DissolveTexture;

/* Shader Parameters */
float4 g_vColor;
float g_fAlpha;
float g_fSize;

float2 g_vUVScale;
float2 g_vUVSpeed;

float2 g_vNoiseScale_0;
float2 g_vNoiseSpeed_0;

float2 g_vNoiseScale_1;
float2 g_vNoiseSpeed_1;

float g_fDistortion;
float2 g_vDistortionDir;
float g_fDistortionSpeed;

float g_fDissolve;
float g_fDissolveEdge;
float4 g_vDissolveColor;

float g_fEmissive;

float g_Time;

float g_fRandom;

/* Billboard */
vector g_vCamPosition;

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
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    float4x4 BoneMatrix =
        g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
        g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
        g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
        g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    float size = g_fSize;
    vector vPosition = mul(float4(In.vPosition * size, 1.f), BoneMatrix);
    vector vNormal = mul(float4(In.vNormal, 0.f), BoneMatrix);
    
    float4x4 matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
   
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    /* UV */
    float2 uv = In.vTexcoord;
    
    uv *= g_vUVScale;
    uv += g_vUVSpeed * g_Time;
    
    Out.vTexcoord = uv;
    
    return Out;
}

VS_OUT VS_BILLBOARD(VS_IN In)
{
    VS_OUT Out;

    /* Bone Position만 사용 */
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);

    float4x4 BoneMatrix =
        g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
        g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
        g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
        g_BoneMatrices[In.vBlendIndex.w] * fWeightW;

    float4 bonePos = mul(float4(0, 0, 0, 1), BoneMatrix);
    float4 worldCenter = mul(bonePos, g_WorldMatrix);
    
    float3 toCam = g_vCamPosition.xyz - worldCenter.xyz;
    float len = length(toCam);
    float3 camForward = (len > 0.0001f) ? toCam / len : float3(0, 0, 1);

    float3 worldUp = abs(camForward.y) > 0.99f ? float3(0, 0, 1) : float3(0, 1, 0);

    float3 camRight = normalize(cross(worldUp, camForward));
    float3 camUp = normalize(cross(camForward, camRight));

    float3 worldPos = worldCenter.xyz
    + camRight * In.vPosition.x * g_fSize
    + camUp * In.vPosition.y * g_fSize;

    float4 finalPos = float4(worldPos, 1.f);

    /* WVP */
    float4 viewPos = mul(finalPos, g_ViewMatrix);
    float4 projPos = mul(viewPos, g_ProjMatrix);

    Out.vPosition = projPos;
    Out.vProjPos = projPos;

    /* Normal은 의미 없음 → 그냥 up */
    Out.vNormal = float4(camUp, 0.f);

    /* UV */
    float2 uv = In.vTexcoord;
    uv *= g_vUVScale;
    uv += g_vUVSpeed * g_Time;

    Out.vTexcoord = uv;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse : SV_Target0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;
    
    /* Noise */
    float2 noiseUV0 = uv * g_vNoiseScale_0 + g_vNoiseSpeed_0 * g_Time;
    float2 noiseUV1 = uv * g_vNoiseScale_1 + g_vNoiseSpeed_1 * g_Time;
    
    float noise0 = g_NoiseTexture_0.Sample(DefaultSampler, noiseUV0).r;
    float noise1 = g_NoiseTexture_1.Sample(DefaultSampler, noiseUV1).r;
    
    float noise = lerp(noise0, noise1, 0.5f);
    
    /* Distortion */
    float2 distortionUV = uv + noise * g_vDistortionDir * g_fDistortion;
    float distortion = g_DistortionTexture.Sample(DefaultSampler, distortionUV).r;
    float2 finalUV = uv
               + noise * g_vDistortionDir * g_fDistortion
               + distortion * g_vDistortionDir * g_fDistortion;
    
    /* Main Texture */
    float4 baseColor = g_MainTexture.Sample(DefaultSampler, finalUV);
    
    /* Combine */
    float4 color = baseColor * g_vColor;
    
    /* Alpha */
    color.a *= g_fAlpha;
    
    /* Mask */
#ifdef USE_MASK
    float mask = g_MaskTexture.Sample(DefaultSampler, finalUV).r;
    color.a *= mask;
#endif
    
    /* Dissolve */
    float dissovle = g_DissolveTexture.Sample(DefaultSampler, finalUV).r;
    // clip(g_fDissolve - dissovle);
    clip(dissovle - g_fDissolve);
    // float edge = smoothstep(g_fDissolve - g_fDissolveEdge, g_fDissolve, dissovle);
    float edge = smoothstep(g_fDissolve, g_fDissolve - g_fDissolveEdge, dissovle);
    color.rgb = lerp(g_vDissolveColor.rgb, color.rgb, edge);
    
    /* Emissive */
    color.rgb *= (1 + g_fEmissive);
    
    /* Diffuse Alpha discar */
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    if (vDiffuse.a <= 0.3f)
        discard;
   
    /* Export */
    Out.vDiffuse = color;
    
    return Out;
}

PS_OUT PS_MAIN_FIRE_01(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;
    
    /* Noise */
    float2 noiseUV0 = uv * g_vNoiseScale_0 + g_vNoiseSpeed_0 * g_Time;
    float2 noiseUV1 = uv * g_vNoiseScale_1 + g_vNoiseSpeed_1 * g_Time;
    
    float noise0 = g_NoiseTexture_0.Sample(DefaultSampler, noiseUV0).r;
    float noise1 = g_NoiseTexture_1.Sample(DefaultSampler, noiseUV1).r;
    
    float noise = lerp(noise0, noise1, 0.5f);
    
    /* Distortion */
    float2 distortionUV = uv + noise * g_vDistortionDir * g_fDistortion;
    float distortion = g_DistortionTexture.Sample(DefaultSampler, distortionUV).r;
    
    float2 finalUV = uv
        + noise * g_vDistortionDir * g_fDistortion
        + distortion * g_vDistortionDir * g_fDistortion;
    
    /* flipBook */
    float2 atlasUV = finalUV;
    
    float2 atlasCount = float2(8, 8);
    float frame = floor(g_Time * 8) % 64;
    float2 frameOffset;
    frameOffset.x = fmod(frame, atlasCount.x);
    frameOffset.y = floor(frame / atlasCount.x);
    
    atlasUV /= atlasCount;
    atlasUV += frameOffset / atlasCount;
   
    /* Main Texture */
    float4 baseColor = g_MainTexture.Sample(DefaultSampler, atlasUV);
    
    /* Alpha */
    float alpha = baseColor.r;
    alpha = pow(alpha, 1.2f);
    alpha *= g_fAlpha * 0.7f;
    alpha *= (1.f - uv.y * 0.8f);
    alpha *= (0.9f + noise * 0.2f);
    if (alpha < 0.05f)
        discard;
    
    /* Fire Grad */
    float height = saturate(uv.y);
    float center = 1.0 - abs(uv.x - 0.5) * 2.0;
    center = saturate(center);
    center = pow(center, 2.0);
    float3 colRed = float3(1.0, 0.1, 0.0);
    float3 colOrange = float3(1.0, 0.5, 0.0);
    float3 colYellow = float3(1.0, 1.0, 0.0);
    
    float3 fireColor = lerp(colRed, colOrange, height);
    
    fireColor = lerp(fireColor, colYellow, center);
    
    float4 color = baseColor;
    color.rgb *= fireColor;
    color.a *= g_fAlpha;
    color.a *= (1.0 - uv.y * 0.7); // 위쪽 더 투명
    
    /* Emissive */
    // color.rgb *= (1.f + g_fEmissive * 2.f);
    
    /* 살짝 흔들림 */
    color.rgb *= (0.8f + noise * 0.4f);
    
    /* Export */
    Out.vDiffuse = color;
    
    return Out;
}

PS_OUT PS_MAIN_FIRE_02(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;
    uv.y = 1.f - uv.y; // y축 반전
    
    /* Noise */
    float2 noiseUV0 = uv * g_vNoiseScale_0 + g_vNoiseSpeed_0 * g_Time;
    float2 noiseUV1 = uv * g_vNoiseScale_1 + g_vNoiseSpeed_1 * g_Time;
    
    float n0 = g_NoiseTexture_0.Sample(DefaultSampler, noiseUV0).r;
    float n1 = g_NoiseTexture_1.Sample(DefaultSampler, noiseUV1).r;
    float noise = lerp(n0, n1, 0.5f);
    
    float strength = pow(uv.y, 2.5f); // 위쪽으로 갈수록 강해짐
    
    /* 좌우 + 위쪽 흔들림 */
    float offsetX = (noise - 0.5f) * g_fDistortion * strength * 2.5f;
    float offsetY = (noise - 0.5f) * g_fDistortion * strength * 1.5f;
    
    float2 finalUV = uv;
    float wave = sin(g_Time * 10.0f + uv.y * 20.0f) * 0.05f;
    finalUV.x += wave * strength;
    finalUV.y += cos(g_Time * 8.0f + uv.x * 15.0f) * 0.03f * strength;

    /* Main Texture */
    float base = g_MainTexture.Sample(DefaultSampler, finalUV).r;
    
    float alpha = base;
    if (alpha < 0.02f)
        discard;
    
    /* 색 변화: 아래쪽 붉게, 위쪽 노랗게 */
    float3 fireColor = lerp(float3(1.0, 0.3, 0.0), float3(1.0, 1.0, 0.8), uv.y);
    float3 color = fireColor * alpha;
    
    /* Emissive */
    color *= (1.f + g_fEmissive + uv.y);
    
    /* Export */
    Out.vDiffuse = float4(color, alpha);
   
    return Out;
}

PS_OUT PS_MAIN_TRAIL(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;
    
    float4 baseColor = g_MainTexture.Sample(DefaultSampler, uv);

    // 위로 갈수록 사라짐
    float fadeY = pow(1.0f - uv.y, 1.5f);

    // 가운데 밝고 양옆 약하게
    float center = 1.0f - abs(uv.x - 0.5f) * 2.0f;
    center = saturate(center);
    center = pow(center, 2.0f);

    float shape = fadeY * center;
    
    float alpha = baseColor.r;
    alpha *= shape;
    alpha *= g_fAlpha;

    if (alpha < 0.02f)
        discard;

    float dissolve = g_DissolveTexture.Sample(DefaultSampler, uv).r;

    // Trail은 "뒤에서 앞으로 사라져야 자연스러움"
    float dissolveMask = smoothstep(g_fDissolve - g_fDissolveEdge, g_fDissolve, dissolve);

    alpha *= dissolveMask;

    /* Edge Glow */
    float edge = smoothstep(g_fDissolve - g_fDissolveEdge, g_fDissolve, dissolve);
    float3 edgeColor = g_vDissolveColor.rgb * (1.0f - edge);

    
    float3 color = baseColor.rgb;
    
    float3 colA = float3(1.0, 0.3, 0.0);
    float3 colB = float3(1.0, 0.8, 0.3);

    float3 fireColor = lerp(colA, colB, uv.y);

    color *= fireColor;

    color += edgeColor;

    float2 noiseUV = uv * g_vNoiseScale_0 + g_vNoiseSpeed_0 * g_Time;
    float noise = g_NoiseTexture_0.Sample(DefaultSampler, noiseUV).r;

    color *= (0.85f + noise * 0.3f);

    color *= (1.0f + g_fEmissive);

    Out.vDiffuse = float4(color, alpha);

    return Out;
}


PS_OUT PS_MAIN_GRASS_CUT(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;
    
    /* sway */
    float sway = sin(g_Time * 10 + uv.y * 5) * 0.03;
    float2 finalUV = uv;
    finalUV.x += sway;
    
    float4 baseColor = g_MainTexture.Sample(DefaultSampler, finalUV);
    
    if (baseColor.a < 0.3f)
        discard;
    
    /* Fake Normal */
    float3 N = float3(0.0f, 1.0f, 0.0f);
    
    float bend = (finalUV.x - 0.5f) * 2.0f;
    N.x += bend * 0.5f;
    N.y += finalUV.y * 0.3f;
    
    float wind = sin(g_Time * 5.0f + finalUV.y * 10.0f) * 0.2f;
    N.x += wind;
    
    N = normalize(N);
    
    /* Fake Specular */
    float3 L = normalize(float3(0.3f, 1.0f, 0.2f));
    float3 V = float3(0.0f, 0.0f, 1.0f);
    float3 H = normalize(L + V);
    float spec = pow(saturate(dot(N, H)), 8.0f);
    spec *= 0.8f;
    
    float3 specColor = float3(0.8f, 1.0f, 0.6f);
    
    /* Hightlight */
    float flow = frac(g_Time * 1.5f + finalUV.y);
    
    float highlight = smoothstep(0.45f, 0.5f, flow)
                    - smoothstep(0.5f, 0.55f, flow);
    
    float center = 1.0f - abs(finalUV.x - 0.5f) * 2.0f;
    center = saturate(center);
    center = pow(center, 2.0f);
    
    highlight *= center;
    
    /* 강도 */
    float3 flowColor = float3(0.6f, 1.0f, 0.6f);
    
    /* Rim */
    float rim = 1.0f - saturate(dot(N, V));
    rim = pow(rim, 2.0f);
    rim *= 0.5f;
    
    float3 rimColor = float3(0.3f, 0.8f, 0.3f);
    
    /* Final */
    float3 finalColor = baseColor.rgb;
    
    /* 기본 spec */
    finalColor += spec * specColor;
    
    /* main */
    finalColor += highlight * flowColor * 1.5f;
    
    /* rim */
    finalColor += rim * rimColor;
    
    finalColor = saturate(finalColor);

    float fade = 1.0 - uv.y;
    float life = saturate(1.0f - g_Time * 1.2f);
    
    float alpha = baseColor.a * fade * life;
    
    /* Soft Effect */
    float2 vTexcoord;
    vTexcoord.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
    vTexcoord.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    float fOldViewZ = vDepthDesc.y * 1000.f;
    alpha *= saturate(fOldViewZ - In.vProjPos.w);
    
    Out.vDiffuse = float4(finalColor, alpha);
    
    return Out;
}

PS_OUT PS_MAIN_STAR(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;

    /* life (0~1로 사용) */
    float life = saturate(g_Time);

    /* scale (초반 빠르게 커짐) */
    float scale = lerp(0.3f, 1.2f, smoothstep(0.0f, 0.25f, life));
    float2 centerUV = (uv - 0.5f) / scale;

    float dist = length(centerUV);

    /* core */
    float core = smoothstep(0.15f, 0.0f, dist);

    /* glow */
    float glow = smoothstep(0.5f, 0.0f, dist);

    /* cross sparkle */
    float crossX = smoothstep(0.02f, 0.0f, abs(centerUV.x)) * smoothstep(0.3f, 0.0f, abs(centerUV.y));
    float crossY = smoothstep(0.02f, 0.0f, abs(centerUV.y)) * smoothstep(0.3f, 0.0f, abs(centerUV.x));
    float cross = max(crossX, crossY);

    /* flicker (짧고 빠르게) */
    float flicker = sin(g_Time * 20.0f);
    flicker = saturate(flicker * 0.5f + 0.5f);

    core *= flicker;
    glow *= flicker;
    cross *= flicker;

    /* fade (후반 급격히 사라짐) */
    float fade = 1.0f - smoothstep(0.5f, 1.0f, life);

    /* 색 (보라 + 핑크 + 흰색) */
    float3 purple = float3(0.5f, 0.0f, 1.0f);
    float3 pink = float3(1.0f, 0.4f, 0.8f);
    float3 white = float3(1.0f, 1.0f, 1.0f);

    float3 baseColor = (g_fRandom < 0.5f) ? purple : pink;
    float3 color = lerp(baseColor, white, core);

    /* intensity */
    float intensity = core + glow * 0.6f + cross * 1.5f;

    intensity *= fade;

    float3 final = color * intensity;

    float alpha = intensity;

    if (alpha < 0.01f)
        discard;

    Out.vDiffuse = float4(final, alpha);
    
    return Out;
}

PS_OUT PS_MAIN_PURPLE_SMOKE(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;
    
    /* Flipbook */
    float2 atlasCount = float2(4, 4);
    float totalFrame = atlasCount.x * atlasCount.y;

    float frame = floor(g_Time * 12.0f);
    frame = min(frame, totalFrame - 1);

    float2 frameOffset;
    frameOffset.x = fmod(frame, atlasCount.x);
    frameOffset.y = floor(frame / atlasCount.x);

    /* 정확한 UV */
    float2 cellSize = 1.0f / atlasCount;
    float2 minUV = frameOffset * cellSize;
    float2 maxUV = minUV + cellSize;

    float2 atlasUV = lerp(minUV, maxUV, uv);

    /* bleed 방지 */
    atlasUV = clamp(atlasUV, minUV + 0.001, maxUV - 0.001);

    /* sample */
    float4 tex = g_MainTexture.Sample(DefaultSampler, atlasUV);

    /* 핵심 수정 */
    float alpha = tex.a;
    float3 color = tex.rgb;
    color *= 1.3f;
    
    /* alpha 강화 */
    alpha = pow(alpha, 0.8f);
    
    float3 purpleBoost = float3(1.5f, 0.3f, 3.0f);
    color *= purpleBoost;
    
    color.b = max(color.b, color.r * 1.5f);
    
    /* fade */
    float fade = 1.0f - uv.y;
    alpha *= fade;

    /* emissive */
    color *= (1.0f + g_fEmissive);
    
    /* Soft Effect */
    float2 vTexcoord;
    vTexcoord.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
    vTexcoord.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;
    
    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    float fOldViewZ = vDepthDesc.y * 1000.f;
    alpha *= saturate(fOldViewZ - In.vProjPos.w);

    /* 최종 */
    Out.vDiffuse = float4(color, alpha * g_fAlpha);

    return Out;
}

PS_OUT PS_MAIN_DUST_SMOKE(PS_IN In)
{
    PS_OUT Out;
    
    float2 uv = In.vTexcoord;

    /* 1. Noise - UV 살짝 흔들기 */
    float2 noiseUV = uv * g_vNoiseScale_0 + g_vNoiseSpeed_0 * g_Time;
    float noise = g_NoiseTexture_0.Sample(DefaultSampler, noiseUV).r;

    /* 2. UV 왜곡 최소화 */
    float2 finalUV = uv;
    finalUV += (noise - 0.5f) * 0.02f;

    /* 3. Main Texture - 이미지 한 장 */
    float4 texColor = g_MainTexture.Sample(DefaultSampler, finalUV);
    
    // 알파 or 밝기 기반으로 알파 추출
    float base = texColor.a > 0.01f ? texColor.a : texColor.r;
    base = pow(base, 0.5f); // 감마 보정으로 밝게

    /* 4. 원형 소프트 엣지 */
    float2 centerUV = uv - 0.5f;
    float dist = length(centerUV);
    float soft = smoothstep(0.5f, 0.05f, dist);

    float alpha = base * soft;

    if (alpha < 0.01f)
        discard;

    /* 5. 색 - 회색 먼지 */
    float3 colInner = float3(0.85f, 0.85f, 0.85f); // 중심 밝은 회색
    float3 colOuter = float3(0.45f, 0.45f, 0.45f); // 바깥 어두운 회색

    float3 color = lerp(colInner, colOuter, saturate(dist * 2.0f));

    // 텍스처 RGB도 살짝 반영 (연기 디테일 살리기)
    color *= (texColor.rgb * 0.4f + 0.6f);

    // g_vColor 틴팅
    color = lerp(color, g_vColor.rgb, 0.15f);

    // 노이즈 명암
    color *= (0.8f + noise * 0.4f);

    /* 6. Emissive */
    color *= (1.0f + g_fEmissive * 0.3f);

    /* 7. Depth Fade */
    float2 vTexcoord;
    vTexcoord.x = In.vProjPos.x / In.vProjPos.w * 0.5f + 0.5f;
    vTexcoord.y = In.vProjPos.y / In.vProjPos.w * -0.5f + 0.5f;

    vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * 1000.f;
    float depthFade = saturate(fOldViewZ - In.vProjPos.w);
    alpha *= depthFade;

    /* 8. 알파 증폭 */
    alpha *= g_fAlpha;
    alpha = pow(alpha, 0.55f); // 감마 보정
    alpha = saturate(alpha * 2.0f); // 증폭

    Out.vDiffuse = float4(color, alpha);

    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetBlendState(AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Fire_01
    {
        SetBlendState(AdditiveBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_BILLBOARD();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FIRE_01();
    }

    pass Fire_02
    {
        SetBlendState(AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_BILLBOARD();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FIRE_02();
    }

    pass Grass_01
    {
        SetBlendState(AlphaBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_GRASS_CUT();
    }

    pass Star
    {
        SetRasterizerState(NONCULL);
        SetBlendState(AdditiveBlend, float4(0, 0, 0, 0), 0xffffffff);
        SetDepthStencilState(DepthNoWrite, 0);

        VertexShader = compile vs_5_0 VS_BILLBOARD();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_STAR();
    }

    pass Purple_Smoke
    {
        SetRasterizerState(NONCULL);
        SetBlendState(AdditiveBlend, float4(0, 0, 0, 0), 0xffffffff);
        SetDepthStencilState(DepthNoWrite, 0);

        VertexShader = compile vs_5_0 VS_BILLBOARD();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_PURPLE_SMOKE();
    }

    pass Dust_Smoke
    {
        SetRasterizerState(NONCULL);
        SetBlendState(AdditiveBlend, float4(0, 0, 0, 0), 0xffffffff);

        VertexShader = compile vs_5_0 VS_BILLBOARD();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DUST_SMOKE();
    }
}