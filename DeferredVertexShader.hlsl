struct DVS_IN
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
};

struct DVS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
DVS_OUT DVS_main(DVS_IN input)
{
	DVS_OUT output;

	output.Pos = float4(input.Pos, 1.0f);
	output.Tex = input.Tex;

	return output;
}