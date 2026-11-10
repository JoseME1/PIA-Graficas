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
};

VS_OUTPUT VS(float4 inPos : POSITION, float4 inTextCoord : TEXTURE, float3 normal : NORMAL, float3 tangent : TANGENT, float3 binormal : BINORMAL)
{
    VS_OUTPUT output;

	float4 displacedPos = inPos;
	displacedPos.y += sin(inPos.x * 0.5f + Time * 2.0f) * 0.3f;
	displacedPos.y += cos(inPos.z * 0.5f + Time * 1.6f) * 0.3f;

	float4 worldPos = mul(displacedPos, World);
	output.Pos = mul(displacedPos, WorldViewProjection);

	output.normal = mul(normal, World);
    output.TexCoord = inTextCoord;
	output.viewDirection = normalize(CameraPos.xyz - worldPos.xyz);
	output.tangent = mul(tangent, World);
	output.binormal = mul(binormal, World);
	output.vectPos = inPos;
	output.maxHeight = MaxH;

    return output;
}