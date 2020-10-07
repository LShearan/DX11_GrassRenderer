///////////////////////////////////////////////////////////////////////////////
// Simple Mesh Shader
///////////////////////////////////////////////////////////////////////////////


cbuffer PerFrameCB : register(b0)
{
    matrix matProjection;
    matrix matView;
    float  time;
    float  padding[3];
};

cbuffer PerDrawCB : register(b1)
{
    matrix matMVP;
};

Texture2D texture0 : register(t0);

SamplerState linearMipSampler : register(s0);


struct VertexInput
{
    float3 pos   : POSITION;
    float4 color : COLOUR;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct VertexOutput
{
    float4 vpos  : SV_POSITION;
    float4 color : COLOUR;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

VertexOutput VS_Mesh(VertexInput input)
{
    VertexOutput output;
    output.vpos = mul(float4(input.pos, 1.0f), matMVP);
    output.color = input.color;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.uv = input.uv;

    return output;
}

float4 PS_Mesh(VertexOutput input) : SV_TARGET
{
    float lightIntensity = dot(normalize(float3(1,1,1)), input.normal);

    float4 materialColor = texture0.Sample(linearMipSampler, input.uv);

    return float4(materialColor.xyz * lightIntensity, 1.0f);
}