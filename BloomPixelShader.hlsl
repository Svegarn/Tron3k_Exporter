Texture2D BloomMap : register(t0);

SamplerState samplerClamp : register(s0);

struct VS_OUT{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

// Main
float4 PS_main(VS_OUT input) : SV_Target
{
	matrix <float, 3, 3> blurMatrix = {
		0.077847, 0.123317, 0.077847,
		0.123317, 0.195346, 0.123317,
		0.077847, 0.123317, 0.077847
	};

	float3 outputColor = {0.0f, 0.0f, 0.0f};
	float2 texSize = { 1.0f / 1920.0f, 1.0f / 1080.0f };

	[unroll]
	for (int j = 0; j < 3; j++){
		[unroll]
		for (int i = 0; i < 3; i++){
			outputColor += BloomMap.Sample(samplerClamp, input.Tex + ((j * i) * texSize)).rgb * blurMatrix[j][i];
			outputColor += BloomMap.Sample(samplerClamp, input.Tex - ((j * i) * texSize)).rgb * blurMatrix[j][i];
		}
	}
	
	outputColor = saturate(BloomMap.Sample(samplerClamp, input.Tex).rgb * outputColor);

	return float4(outputColor, 0.0f);
}