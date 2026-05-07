//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer cbNeverChanges : register(b0)
{
    matrix View;
};

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
};

struct VS_INPUT
{
    float4 pos : POSITION;
    float4 nor : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 nor : NORMAL;
    float2 uv : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT vsMain(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.pos = mul(input.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.nor = float4(normalize(mul(input.nor.xyz, (float3x3) World)), 0.0);
    output.uv = input.uv;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 psMain(PS_INPUT input) : SV_Target
{
    return tex.Sample(samLinear, input.uv);
}
