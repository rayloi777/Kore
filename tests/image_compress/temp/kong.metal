#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _kong_color_out {
	float4 _0 [[color(0)]];
};

struct _kong_pos_attributes {
	float3 _72_pos [[attribute(0)]];
	float2 _72_uv [[attribute(1)]];
};

struct vertex_out {
	float4 pos [[position]];
	float2 uv [[user(locn0)]];
};

struct _69_type {
	float4x4 mvp;
};

struct mvp {
	constant _69_type *_69 [[id(0)]];
};

struct textures {
	texture2d<float> _70 [[id(0)]];
	sampler _71 [[id(1)]];
};

vertex vertex_out pos(_kong_pos_attributes _kong_stage_in [[stage_in]], constant mvp& argument_buffer0 [[buffer(1)]], constant textures& argument_buffer1 [[buffer(2)]], uint _kong_vertex_id [[vertex_id]], uint _kong_instance_id [[instance_id]]) {
	vertex_out _73;
	float3 _75 = _kong_stage_in._72_pos;
	float _76 = 1.000000;
	float4 _74 = float4(_75, _76);
	float4x4 _77 = argument_buffer0._69->mvp;
	float4 _78 = _77 * _74;
	_73.pos = _78;
	float2 _79 = _kong_stage_in._72_uv;
	_73.uv = _79;
	return _73;
}

fragment _kong_color_out pix(vertex_out _80 [[stage_in]], constant mvp& argument_buffer0 [[buffer(1)]], constant textures& argument_buffer1 [[buffer(2)]]) {
	float2 _83 = _80.uv;
	float4 _82 = argument_buffer1._70.sample(argument_buffer1._71, _83);
	float4 _81;
	_81 = _82;
	{
		_kong_color_out _kong_color;
		_kong_color._0 = _81;
		return _kong_color;
	}
}

