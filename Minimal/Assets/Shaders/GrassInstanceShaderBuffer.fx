///////////////////////////////////////////////////////////////////////////////
// Simple Mesh Instancing Shader
///////////////////////////////////////////////////////////////////////////////

cbuffer PerFrameCB : register(b0)
{
    matrix      matProjection;
    matrix      matView;
    float       time;
    float       padding[3];
};

cbuffer PerDrawCB : register(b1)
{
    matrix matMVP;
};

cbuffer PerWindCB : register(b2)
{
    float3 windDir;
    float windStrength;
    float m_timeStamp;
    float pad[3];
}

struct PerInstanceData
{
    float4x4 matModel;
    float4 colourTint;
};

// resources used to the vertex shader.
StructuredBuffer<PerInstanceData> instanceData : register(t0);

// resources used in the Pixel Shader.
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

struct VertexTemp
{
    float3 vPos;
    float3 vNormal;
};

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453123);
}

VertexOutput VS_Mesh(VertexInput input, uint instanceId : SV_INSTANCEID)
{
    VertexOutput output;
    VertexTemp temp;

    PerInstanceData data = instanceData[instanceId];

    // Build MVP on the fly.
    float4x4 tMVP = mul(data.matModel, mul(matView, matProjection));

    output.vpos = mul(float4(input.pos, 1.0f),tMVP);
    output.normal = input.normal;
    output.color = data.colourTint;
    output.tangent = input.tangent;
    output.uv = input.uv;

    // Move the top part of the grass
    if (input.uv.y <= 0.1f)
    {
        // Create translation vector for the incremental move of the wind
        float3 trans = (windStrength * (windDir * (1 - m_timeStamp)));

        output.vpos.xyz += trans;
        output.normal *= trans;
    }

    return output;
}

float4 PS_Mesh(VertexOutput input) : SV_TARGET
{
    float lightIntensity = dot(normalize(float3(1,1,1)), input.normal);

    float4 materialColor = texture0.Sample(linearMipSampler, input.uv);

    // Alpha test
    clip(materialColor.a < .8f ? -1 : 1);

    return float4(materialColor.xyz * input.color * lightIntensity, 1.0f);

}

///////////////////////////////////////////////////////////////////////////////
