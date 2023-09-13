//----------------------------------------------------------------------------
//!	@file	ps_model.fx
//!	@brief	MV1モデルピクセルシェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"
#include "shadow.h" 

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT_3D input)
{
	PS_OUTPUT	output;

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

	float	shadow = calcShadow(input.worldPosition_, input.position_.w);
	shadow = min(shadow, saturate(dot(N, L)));	// Lampertと暗いほうを選択

	//------------------------------------------------------------
	// テクスチャカラーを読み込み
	//------------------------------------------------------------
	float4	textureColor = DiffuseTexture.Sample(DiffuseSampler, uv);

	// アルファテスト
	if (textureColor.a < 0.5) discard;

	// 平行光源
	// DIFFUSE
	float	diffuse = saturate(dot(N, L));	// Lambert

	// 環境光
	float3	ambient = float3(0.5, 0.5, 0.5);

	float4	color = textureColor;
	color.rgb = color.rgb * (saturate(diffuse) + ambient + shadow);

	output.color0_ = color;
	// 出力パラメータを返す
	return output;
}
