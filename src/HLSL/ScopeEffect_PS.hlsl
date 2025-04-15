#include "Triangle.hlsli"

sampler BackBuffer;



float4 main(float4 vpos : SV_Position, float2 texcoord : TEXCOORD0) : SV_Target 
{
	float4 abseyeDirectionLerp = mul(float4((eyeDirectionLerp),1),CameraRotation);
    const float epsilon = 0.001;
    abseyeDirectionLerp.y = sign(abseyeDirectionLerp.y) * clamp(abs(abseyeDirectionLerp.y), epsilon, 1.0);

    float2 eye_velocity = clampMagnitude(abseyeDirectionLerp.xy , 2);

    float2 FTS_ScreenPosInPixel = FTS_ScreenPos;

	//float2 adjTex = aspect_ratio_correction(texcoord);
    float2 adjTex = texcoord;
    static const float Xoffset = (0.5 - BUFFER_HEIGHT * rcp(BUFFER_WIDTH) * 0.5);
    adjTex.x += Xoffset;
	
    float2 mulTex = (adjTex - float2(0.5, 0.5)) * rcp((ScopeEffect_Zoom)) + float2(0.5, 0.5);
	float4 color = tBACKBUFFER.Sample(gSamLinear, mulTex);
	
	float2 ReticleCoord = aspect_ratio_correction(texcoord - Reticle_Offset);
	ReticleCoord = ((ReticleCoord - float2(0.5,0.5)) * 16 *rcp(ReticleSize)) + float2(0.5,0.5);

	float4 ReticleColor = ReticleTex.Sample(gSamLinear,ReticleCoord);
	color = ReticleColor * ReticleColor.a + color * (1-ReticleColor.a);

	float2 parallax_offset = float2(0.5 + eye_velocity.x  , 0.5 - eye_velocity.y);
	float distToParallax = distance(adjTex, parallax_offset);
	float2 scope_center = float2(0.5,0.5) + ScopeEffect_Offset;
	float distToCenter = distance(adjTex, scope_center);

	float4 nColor = EnableNV * NVGEffect(color, texcoord);
    color = lerp(color, nColor, nColor.a);
	color.rgb *=   (step(distToCenter, 2) * getparallax(distToParallax,float2(1,1),1));
	return color;
}

