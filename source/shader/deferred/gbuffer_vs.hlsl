#include "../common/octahedral.hlsl"
#include "../common/gbuffer.hlsl"

cbuffer pass_constants : register(b0)
{
    float4x4 view_proj;

    float4x4 view_matrix;
    float4x4 prev_view_matrix;

    uint geometry_constant_index;
    uint3 pad;
};

StructuredBuffer<GeometryConstant> geometry_constant_buffer : register(t6);

struct VertexInput
{
    float3 local_space_position : POSITION;
    float3 local_space_normal : NORMAL;
    float3 local_space_tangent  : TANGENT;
    float2 uv : TEXCOORD;
};

struct VertexOutput
{
    float4 sv_position : SV_Position;

    float3 world_space_position : WORLD_POSITION;

    float3 view_space_position : VIEW_POSITION;
    float3 prev_view_space_position : PREV_VIEW_POSITION;
    
    float3 world_space_normal : NORMAL;
    float3 world_space_tangent  : TANGENT;
    float2 uv        : TEXCOORD;
};


VertexOutput main(VertexInput input)
{
    GeometryConstant constant = geometry_constant_buffer[geometry_constant_index];

    VertexOutput output;
    float4 world_pos = mul(float4(input.local_space_position, 1.0f), constant.world_matrix);

    output.sv_position = mul(world_pos, view_proj);

    output.world_space_position = world_pos.xyz;

    output.view_space_position = mul(world_pos, view_matrix).xyz;
    output.prev_view_space_position = mul(world_pos, prev_view_matrix).xyz;

    output.world_space_normal = normalize(mul(float4(input.local_space_normal, 1.0f), constant.inv_trans_world)).xyz;
    output.world_space_tangent = normalize(mul(float4(input.local_space_tangent, 1.0f), constant.inv_trans_world)).xyz;
    output.uv = input.uv;

    return output;
}

