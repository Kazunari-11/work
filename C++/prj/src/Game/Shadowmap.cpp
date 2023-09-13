//---------------------------------------------------------------------------
//! @file   Shadowmap.cpp
//! @brief  シャドウマップ
//---------------------------------------------------------------------------
#include "Shadowmap.h"
#include "System/Graphics/Frustum.h"   // 視錐台
#include "System/SystemMain.h"         // HDRテクスチャ
#include "System/Scene.h"
#include "Game/Objects/Player.h"

namespace
{

struct ShadowInfo
{
    std::shared_ptr<Texture> shadow_color_texture_;   //!< シャドウ用カラーテクスチャ(DxLib用の対策)
    std::shared_ptr<Texture> shadow_depth_texture_;   //!< シャドウ用デプステクスチャ
    matrix                   mat_light_view_ = matrix::identity();   //!< シャドウ用ビュー行列
    matrix                   mat_light_proj_ = matrix::identity();   //!< シャドウ用投影配列
    Frustum                  frustum_;                               //!< 視錐台
    int                      cb_ = -1;                               // [DxLib] 定数バッファ
};

// シャドウ用定数バッファ
struct ShadowCb
{
    matrix mat_light_view_      = matrix::identity();   //!< シャドウ用ビュー行列
    matrix mat_light_proj_      = matrix::identity();   //!< シャドウ用投影配列
    matrix mat_light_view_proj_ = matrix::identity();   //!< シャドウ用ビュー×投影配列
};

ShadowInfo shadow_;
u32        resolution_ = 4096;   // シャドウマップ解像度 (512, 1024, 2048, 4096, 8192)
}   // namespace

// 初期化
bool Shadowmap::Init()
{
    // シャドウバッファを作成
    shadow_.shadow_color_texture_ = std::make_shared<Texture>(resolution_, resolution_, DXGI_FORMAT_R32_FLOAT);
    shadow_.shadow_depth_texture_ = std::make_shared<Texture>(resolution_, resolution_, DXGI_FORMAT_D32_FLOAT);

    //----------------------------------------------------------
    // 定数バッファを作成
    //----------------------------------------------------------
    shadow_.cb_ = CreateShaderConstantBuffer(sizeof(ShadowCb));

    return __super::Init();
}

// 遅い更新 (カメラ位置・プレイヤー位置が確定した後)
void Shadowmap::LateUpdate([[maybe_unused]] float delta)
{
    //---------------------------------------------------------
    // シャドウ用行列を作成
    //---------------------------------------------------------
    float height = 400.0f;   // 光源位置の高さ

    // ビュー行列(視線・方角)
    {
        auto player = Scene::GetObjectPtr<Player>("Player");

        float3 light_dir = normalize(float3(0.5, 1, 0.5));
        float3 look_at   = player->GetTranslate();   // プレイヤーの座標
        float3 position = look_at + light_dir * height;

        shadow_.mat_light_view_ = matrix::lookAtLH(position, look_at);
    }

    // 投影行列(描画範囲・画角)
    {
        float range   = 400.0f;            // 描画範囲(± m)
        float z_range = height + 400.0f;   // 奥行きの描画範囲(m)

        shadow_.mat_light_proj_ = matrix::scale(float3(1.0f / range, 1.0f / range, 1.0f / z_range));
    }

    //---------------------------------------------------------
    // 定数バッファを更新
    //---------------------------------------------------------
    // 更新に必要なメモリの場所を取得
    void* p = GetBufferShaderConstantBuffer(shadow_.cb_);
    {
        auto* info                 = reinterpret_cast<ShadowCb*>(p);
        info->mat_light_view_      = shadow_.mat_light_view_;
        info->mat_light_proj_      = shadow_.mat_light_proj_;
        info->mat_light_view_proj_ = mul(shadow_.mat_light_view_, shadow_.mat_light_proj_);
    }

    // メモリをGPU側へ転送
    UpdateShaderConstantBuffer(shadow_.cb_);
}

// 描画
void Shadowmap::Draw()
{
    auto begin = [&]() {
        // 描画先をシャドウバッファに設定
        SetRenderTarget(shadow_.shadow_color_texture_.get(), shadow_.shadow_depth_texture_.get());
        ClearDepth(1.0f);

        // [DxLib] カメラ行列を光源からの視点に設定
        DxLib::SetCameraViewMatrix(shadow_.mat_light_view_);            // ビュー行列
        DxLib::SetupCamera_ProjectionMatrix(shadow_.mat_light_proj_);   // 投影行列
    };
    auto end = [&]() {
        // 描画先を元に戻す
        SetRenderTarget(GetHdrTexture(), GetDepthStencil());
        ClearColor(GetHdrTexture(), float4(0.5, 0.5f, 0.5f, 0.0f));

        // カメラを元に戻す(ComponentCamera側で設定される)

        // シャドウデプスバッファをテクスチャとして使えるように
        // カラーテクスチャにコピーする (DxLibの仕様上)
        {
            // [DxLib] DirectX11コンテキストを取得
            auto* d3dContext = reinterpret_cast<ID3D11DeviceContext*>(
                const_cast<void*>(GetUseDirect3D11DeviceContext()));

            ID3D11Resource* src = shadow_.shadow_depth_texture_->d3dResource();
            ID3D11Resource* dst = shadow_.shadow_color_texture_->d3dResource();

            // カラーバッファにコピー
            d3dContext->CopySubresourceRegion(dst, 0, 0, 0, 0, src, 0, nullptr);

            //---------------------------------------------------------
            // カメラ情報の定数バッファを設定
            //---------------------------------------------------------
            // b11 = CameraInfo
            SetShaderConstantBuffer(shadow_.cb_, DX_SHADERTYPE_VERTEX, 11);
            SetShaderConstantBuffer(shadow_.cb_, DX_SHADERTYPE_PIXEL, 11);
        }
    };

    SetProc("Shadowmap::begin", begin, ProcTiming::Shadow, Priority::HIGHEST);
    SetProc("Shadowmap::end", end, ProcTiming::Shadow, Priority::LOWEST);
}

// 終了
void Shadowmap::Exit()
{
    // 定数バッファを解放
    DeleteShaderConstantBuffer(shadow_.cb_);

    // シャドウバッファ
    shadow_.shadow_color_texture_.reset();
    shadow_.shadow_depth_texture_.reset();
}

// GUI
void Shadowmap::GUI()
{
}

//! シャドウテクスチャを取得
Texture* Shadowmap::shadowTexture()
{
    return shadow_.shadow_color_texture_.get();
}
