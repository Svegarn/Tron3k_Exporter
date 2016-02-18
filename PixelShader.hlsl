Texture2D ShadowMap : register(t0);
Texture2D TextureMap : register(t1);
Texture2D NormalMap : register(t2);

SamplerState samplerWrap : register(s0);
SamplerState samplerClamp : register(s1);

cbuffer cbLight : register(b0)
{
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
	float4 lightDirection;
	float4 mainCamPos;
}

cbuffer cbMaterial : register(b1){
	int mat_hasTexture;
	int mat_hasNormalMap;
	float mat_specularPower;
	float mat_bumpValue;
	float3 mat_color;
	float3 mat_specularColor;
	float2 mat_padding;
}

cbuffer cbEntity : register(b2)
{
	float stats_hitPoints;
	float3 padding;
}

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 BiNormal : BINORMAL;
	float4 LightViewPos : POSITION0;
	float4 Wpos : POSITION1;
};

struct PS_OUT
{
	float4 Diffuse;
	float4 Normal;
	float4 WposMap;
};

float3 CalcBumpedNormal(VS_OUT input);

PS_OUT PS_main(VS_OUT input) : SV_TARGET
{
	PS_OUT output = (PS_OUT)0;

	float3 color = mat_color;
	float3 normal = input.Normal.xyz;
	
	[flatten]
	if (mat_hasTexture == 1){
		color = TextureMap.Sample(samplerClamp, input.Tex).xyz;
	}

	[flatten]
	if (mat_hasNormalMap == 1){
		normal = normalize(CalcBumpedNormal(input));
	}

	color.y *= stats_hitPoints;
	color.z *= stats_hitPoints;

	float bias = 0.002f;
	float2 projectTexCoord;
	float depthValue, depthValueSelfShadow;
	float lightDepthValue, lightDepthValueSelfShadow;

	//From WVP to NDC(-1 - 1) to uv(0 - 1)
	projectTexCoord.x = (input.LightViewPos.x / input.LightViewPos.w) / 2.0f + 0.5f;
	projectTexCoord.y = (-input.LightViewPos.y / input.LightViewPos.w) / 2.0f + 0.5f;

	[flatten]
	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		depthValue = ShadowMap.Sample(samplerWrap, projectTexCoord).x;

		lightDepthValueSelfShadow = input.LightViewPos.z / input.LightViewPos.w;
		lightDepthValue = lightDepthValueSelfShadow - bias;

		[flatten]
		if (lightDepthValue > depthValue)
		{
			color /= 2.0f;
		}
	}

	output.Diffuse = float4(color, 1.0f);
	output.Normal = float4(normal, 0.0f);
	output.WposMap = input.Wpos;

	return output;
}

float3 CalcBumpedNormal(VS_OUT input)
{
	//float3 NewTangent = normalize(input.Tangent - dot(input.Tangent, input.Normal) * input.Normal);
	//float3 NewBinormal = cross(NewTangent, input.Normal);

	float3 BumpMapNormal = NormalMap.Sample(samplerClamp, input.Tex).xyz;
	BumpMapNormal = (2.0f * BumpMapNormal) - 1.0f;
	float3 NewNormal;
	float3x3 TBN = float3x3(input.Tangent, input.BiNormal, input.Normal);
	NewNormal = normalize(mul(BumpMapNormal, TBN));
	return NewNormal;
}