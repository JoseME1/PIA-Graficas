cbuffer ConstantBuffer
{
	float4x4 WorldViewProjection;
	float4x4 World;
	float3 CameraPos;
	float MaxH;
	float Time;
	float3 pad;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXTURE;
	float3 normal : NORMAL;
	float3 viewDirection : COORDS;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4 vectPos : POSITION;
	float maxHeight : HEIGHT;
	float3 worldPos : WORLDPOS;
	float fogDist : FOGDIST;
};

VS_OUTPUT VS(float4 inPos : POSITION, float4 inTextCoord : TEXTURE, float3 normal : NORMAL, float3 tangent : TANGENT, float3 binormal : BINORMAL)
{
    VS_OUTPUT output;

	float4 worldPos = mul(inPos,World);
	output.worldPos = worldPos.xyz;
	output.fogDist = length(CameraPos.xyz - worldPos.xyz);

    output.Pos = mul(inPos, WorldViewProjection);
	output.normal = mul(normal, World);
    output.TexCoord = inTextCoord;
	output.viewDirection = normalize(CameraPos.xyz - worldPos.xyz);
	output.tangent = mul(tangent, World);
	output.binormal = mul(binormal, World);
	output.vectPos = inPos;
	output.maxHeight = MaxH;

    return output;
}