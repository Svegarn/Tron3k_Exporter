Texture2D g_Color : register(t0);
Texture2D g_Normal : register(t1);
Texture2D g_lightDepth : register(t2);
Texture2D g_Wpos : register(t3);
Texture2D g_Specular : register(t4);

SamplerState samplerWrap : register(s0);
SamplerState samplerClamp : register(s1);

cbuffer cBuffer1 : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 viewMatrixInverse;
	float4x4 lightViewMatrix;
	float4x4 lightProjMatrix;
	float4 camPos;
}

cbuffer cBuffer2 : register(b1)
{
	float lightIntensity;
	float3 lightDirection;
	float3 lightColor;
	float padding;
}

struct PointLight{
	float3 pLightPosition;
	float pLightIntensity;
	float3 pLightColor;
	float pLightRange;
	float3 pLightAtt;
	float padding;
};

cbuffer cBuffer3 : register(b2)
{
	PointLight pLightStruct[20];
	float4 pNumberOfLights;
}

struct DVS_OUT{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float3 dirLight(float3 color, float3 normal, float3 wpos, float3 lightpos, float intensity, float4 specular){
	float3 n = normalize(normal);
	float3 diffuseLight = color * max(dot(n, normalize(lightpos)), 0.0f);

	return intensity * diffuseLight;
}

float3 pointLightPhong(float3 color, float3 normal, float3 wpos, float3 f_lightPosition, float3 f_lightColor, float3 f_lightAtt, float f_lightRange, float f_intensity, float4 specular){
	float3 finalColor = { 0.0f, 0.0f, 0.0f };
	float3 specularLight = { 0.0f, 0.0f, 0.0f };
	float3 lightToPixel = f_lightPosition - wpos;
	float d = length(lightToPixel);

	[flatten]
	if (d > f_lightRange){
		return float3(0.0f, 0.0f, 0.0f); // Pixel too far away from light
	}

	float3 n = normalize(normal);

	float howMuchLight = saturate(dot(lightToPixel, normalize(normal))); // Cull

	[flatten]
	if (howMuchLight > 0.0f){
		finalColor += howMuchLight * color * f_lightColor;
		finalColor /= f_lightAtt.x + (f_lightAtt.y * d) + (f_lightAtt.z * (d*d));

		//Specular
		lightToPixel = normalize(lightToPixel);
		float3 v = normalize(camPos.xyz - wpos);
		float3 r = reflect(-lightToPixel, n);
		specularLight = specular.rgb * pow(max(dot(r, v), 0.1f), specular.a);
	}

	return saturate(finalColor) + (f_intensity * specularLight);
}

float3 pointLightLambert(float3 color, float3 normal, float3 wpos, float3 f_lightPosition, float3 f_lightColor, float3 f_lightAtt, float f_lightRange, float4 specular){
	float3 finalColor = { 0.0f, 0.0f, 0.0f };

	float3 lightToPixel = f_lightPosition - wpos;
	float d = length(lightToPixel); // Light distance to pixel

	[flatten]
	if (d > f_lightRange){
		return finalColor; // Pixel too far away from light
	}

	float howMuchLight = saturate(dot(lightToPixel, normalize(normal))); // Cull

	[flatten]
	if (howMuchLight > 0.0f){
		finalColor += howMuchLight * color * f_lightColor;
		finalColor /= f_lightAtt.x + (f_lightAtt.y * d) + (f_lightAtt.z * (d*d));
	}

	return saturate(finalColor);
}

float4 DPS_main(DVS_OUT input) : SV_TARGET
{
	float3 outputColor = { 0.0f, 0.0f, 0.0f };

	float3 colorSamp = g_Color.Sample(samplerWrap, input.Tex).rgb;
	float3 normalSamp = g_Normal.Sample(samplerWrap, input.Tex).rgb;
	float3 wposSamp = g_Wpos.Sample(samplerWrap, input.Tex).rgb;
	float4 specularSamp = g_Specular.Sample(samplerWrap, input.Tex);

	for (int i = 1; i < 6; i++){
		outputColor += pointLightLambert(colorSamp, normalSamp, wposSamp, pLightStruct[i].pLightPosition, pLightStruct[i].pLightColor, pLightStruct[i].pLightAtt, pLightStruct[i].pLightRange, specularSamp);
		//outputColor += pointLightPhong(colorSamp, normalSamp, wposSamp, pLightStruct[i].pLightPosition, pLightStruct[i].pLightColor, pLightStruct[i].pLightAtt, pLightStruct[i].pLightRange, pLightStruct[i].pLightIntensity, specularSamp);
	}

	outputColor += pointLightPhong(colorSamp, normalSamp, wposSamp, pLightStruct[0].pLightPosition, pLightStruct[0].pLightColor, pLightStruct[0].pLightAtt, pLightStruct[0].pLightRange, pLightStruct[0].pLightIntensity, specularSamp);
	outputColor += dirLight(colorSamp, normalSamp, float3(0.0f, 0.0f, 0.0f), lightDirection, lightIntensity, specularSamp);

	return float4(outputColor, 1.0f);
}