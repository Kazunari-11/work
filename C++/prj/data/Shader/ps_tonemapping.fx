//----------------------------------------------------------------------------
//!	@file	ps_tonemapping.fx
//!	@brief	HDRトーンマッピング
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

float3 ACESFilm(float3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT	output;

	// テクスチャカラーの読み込み
	float4	color = DiffuseTexture.Sample(DiffuseSampler, input.uv0_);

	output.color0_ = color;

	// 出力パラメータを返す
	return output;
}
