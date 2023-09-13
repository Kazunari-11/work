//----------------------------------------------------------------------------
//!	@file	ps_distortion.fx
//!	@brief	歪ませるシェーダー
//----------------------------------------------------------------------------
#include "dxlib_ps.h"

//----------------------------------------------------------------------------
// メイン関数
//----------------------------------------------------------------------------
PS_OUTPUT main(PS_INPUT_3D input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
 
	float	factor = -cos(input.uv0_.x * 2.0 * 3.1415) * 0.5 + 0.5;		// 内側0.0f～外側(1.0f)

	float2	uv = input.position_.xy * float2(1.0 / 1280.0, 1.0 / 720.0);
	float2	distortionOffset = -input.normal_.xy * float2(1.0, -1.0);	// Y方向とV方向が逆
	
	// 歪ませる	
	uv += distortionOffset * 0.025 * factor;

	float4	textureColor = DiffuseTexture.Sample(DiffuseSampler, uv);

	float4	color = textureColor;

	// 色変更　
	color.rgb += float3(0.2, 0.2, 0.5) * factor;
	output.color0_ = color;

	// 出力パラメータを返す
	return output;
}
