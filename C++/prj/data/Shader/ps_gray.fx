//----------------------------------------------------------------------------
//!	@file	ps_gray.fx
//!	@brief	白黒シェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"
#include "shadow.h" 

cbuffer FadeInfo : register(b5)
{
	float4	center_position_;	// [xyz_] オブジェクトの中心位置
	float	nega_radius;		// 半径
	float	gray_radius;		// 半径
};

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT_3D input)
{
	PS_OUTPUT	output;

	float3	center_position = center_position_.xyz;

	float3	world_position = input.worldPosition_;	// ピクセルのワールド座標

	float2	uv = input.uv0_;
	float3	N = normalize(input.normal_);	// 法線

	//------------------------------------------------------------
	// 法線マップ
	//------------------------------------------------------------
	N = Normalmap(N, input.worldPosition_, uv);

	//------------------------------------------------------------
	// シャドウ生成
	//------------------------------------------------------------
	float3	L = normalize(float3(1, 1, -1));

	float	shadow = calcShadow(world_position, input.position_.w);
	shadow = min(shadow, saturate(dot(N, L)));	// Lampertと暗いほうを選択

	// 元の色
	float4	textureColor = DiffuseTexture.Sample(DiffuseSampler, input.uv0_);

	// アルファテスト
	if (textureColor.a < 0.5) discard;

	// 平行光源
	// DIFFUSE
	float	diffuse = saturate(dot(N, L));	// Lambert

	// 環境光
	float3	ambient = float3(0.5, 0.5, 0.5);

	float4	color = textureColor;
	color.rgb = color.rgb * (saturate(diffuse) + ambient + shadow);
#ifdef USE_DIFFUSE
	color *= input.diffuse_;
#endif

	color.rgb = saturate(color.rgb);

	// 違う色にしたもの(色を反転させる)
	if (length(center_position - world_position) < nega_radius) {
		float4 change_color = color;
		color.rgb = 1 - change_color.rgb;
	}
	// 違う色にしたもの(色を白黒に)
	if (length(center_position - world_position) < gray_radius) {
		float gray = color.r * 0.3f + color.g * 0.6f + color.b * 0.1f;
		color.rgb = gray;
	}

	output.color0_ = color;

// 出力パラメータを返す
	return output;
}
