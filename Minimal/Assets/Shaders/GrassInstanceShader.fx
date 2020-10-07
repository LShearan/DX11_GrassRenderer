///////////////////////////////////////////////////////////////////////////////
// Simple Mesh Instancing Shader
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

static const uint kGridSizeX = 32;
static const uint kGridSizeY = 32;

VertexOutput VS_Mesh(VertexInput input, uint instanceId : SV_INSTANCEID)
{
    VertexOutput output;

    // Generate the model matrix from the instance ID
    // Use integer division and modulus to form a grid.

    uint i = instanceId / kGridSizeX;
    uint j = instanceId % kGridSizeX;

    //// Translation matrix
    //float4x4 tModel = float4x4(1, 0, 0, 0,
    //    0, 1, 0, 0,
    //    0, 0, 1, 0,
    //    j, cos(float(i) * 0.1) * 4, sin(float(i) * 0.2 + time * j) * 4, 1);

    float4x4 tModel = float4x4(1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            j, (float(i) * 0.1) * 4, (float(i) * 0.2 * j) * 4, 1);

    // Build MVP on the fly.
    float4x4 tMVP = mul(tModel, mul(matView, matProjection));

    output.vpos = mul(float4(input.pos, 1.0f), tMVP);
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

///////////////////////////////////////////////////////////////////////////////
