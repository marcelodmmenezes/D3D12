cbuffer CBPerObject : register(b0)
{
    float4x4 world_view_proj;
};

struct VertexIN
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VertexOUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VertexOUT VS(VertexIN v_in)
{
    VertexOUT v_out;
    
    v_out.pos = mul(float4(v_in.pos, 1.0f), world_view_proj);
    v_out.color = v_in.color;

    return v_out;
}

float4 PS(VertexOUT p_in) : SV_TARGET
{
    return p_in.color;
}
