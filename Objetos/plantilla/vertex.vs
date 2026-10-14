cbuffer ConstantBuffer
{
	float4x4 WorldViewProjection;
	float4x4 World;
	float3 CameraPos;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
};

VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXTCOORD, float3 normal : NORMAL)
{
    VS_OUTPUT output;

	output.Pos = mul(inPos, WorldViewProjection);
    output.TexCoord = inTexCoord;
	output.normal = mul(normal, World);
	output.viewDirection = normalize(CameraPos.xyz - output.Pos.xyz);

    return output;
}