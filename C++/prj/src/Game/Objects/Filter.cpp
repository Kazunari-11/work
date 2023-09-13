//---------------------------------------------------------------------------
//!	@file	Filter.cpp
//!	@brief	エフェクトフィルター
//---------------------------------------------------------------------------
#include "Filter.h"
#include <System/Scene.h>
#include <Game/Objects/Player.h>
#include <Game/Objects/Camera.h>

//  初期化
bool Filter::Init()
{
    // ワークバッファを作成
    work_texture_ = std::make_shared<Texture>(1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM);

    // シェーダーを読込み
    shader_vs_ = std::make_shared<ShaderVs>("data/Shader/vs_3d");
    shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_distortion");

    return Object::Init();
}

//  更新処理
void Filter::Update([[maybe_unused]] float delta)
{
}

// 描画処理
void Filter::Draw()
{
    // 描画
    auto draw = [&]() {
        //  現在のRenderTarget情報を取得
        TargetDesc desc        = GetRenderTarget();
        Texture*   back_buffer = desc.color_targets_[0];

        // (1) ワークバッファにコピー
        {
            Texture* dest_texture = work_texture_.get();
            Texture* src_texture  = back_buffer;   // 現在のRenderTarget

            CopyToRenderTarget(dest_texture,   // 描画先のテクスチャ
                               src_texture);   // コピー元のテクスチャ

            // 今のRenderTarget情報を保存しておく(関数終了時に復元するため)
            auto current_target_desc = GetRenderTarget();
        }

        MATRIX matIdentity = cast(matrix::identity());
        SetTransformToWorld(&matIdentity);

        // 円を描画
        auto player = Scene::GetObjectPtr<Player>("Player");
        if(player->IsUseSkill() == true) {
            DrawCircleS(player->GetTranslate(), player->GetNegaRadius() * 0.2f, 25.0f, 32);
            if(150.0f <= player->GetNegaRadius()) {
                DrawCircleS(player->GetTranslate(), player->GetGrayRadius() * 0.2f, 25.0f, 32);
            }
        }
    };

    // 描画パスに登録
    SetProc(u8"フィルター",       // 名前
            draw,                 // 関数
            ProcTiming::Filter,   // タイミング
            Priority::NORMAL      // 優先度
    );
}

// 終了処理
void Filter::Exit()
{
    work_texture_.reset();   // ワークバッファを解放
    shader_ps_.reset();      // シェーダーを解放
}

// 歪みフィルターをつける円の描画
void Filter::DrawCircleS(const float3& center, float radius, float thickness, u32 divCount)
{
    std::vector<VERTEX3DSHADER> v((divCount + 1) * 2);

    float3 right = float3(1, 0, 0);
    float3 up    = float3(0, 1, 0);

    if(auto camera = Scene::GetObjectPtr<Camera>("PlayerCamera")) {
        auto mat = GetMatrix();
        right    = camera->GetWorldMatrix().axisX();
        up       = camera->GetWorldMatrix().axisY();
    }

    // 頂点データの準備
    for(u32 i = 0; i < divCount + 1; ++i) {
        float theta = 2.0f * PI * (float(i) / divCount);

        float r0 = radius;
        float r1 = radius + thickness;

        float3 uvOffset = float3(cosf(theta), sinf(theta), 0.0f);
        float3 offset   = right * cosf(theta) + up * sinf(theta);
        float3 p0       = center + offset * r0;
        float3 p1       = center + offset * r1;

        // 内側
        v[i * 2 + 0].pos  = cast(p0);
        v[i * 2 + 0].dif  = GetColorU8(255, 255, 255, 255);
        v[i * 2 + 0].norm = cast(uvOffset);   // 2Dの歪み方向
        v[i * 2 + 0].u    = 0.0f;             // 位置情報 (内側=0.0f)
        v[i * 2 + 0].v    = 0.0f;

        // 外側
        v[i * 2 + 1].pos  = cast(p1);
        v[i * 2 + 1].dif  = GetColorU8(255, 255, 255, 255);
        v[i * 2 + 1].norm = cast(uvOffset);   // 2Dの歪み方向
        v[i * 2 + 1].u    = 1.0f;             // 位置情報 (外側=1.0f)
        v[i * 2 + 1].v    = 0.0f;
    }

    // 両面
    SetUseBackCulling(DX_CULLING_NONE);

    // 使用するテクスチャを設定 (slot=0)
    SetUseTextureToShader(0, *work_texture_);

    // 使用する頂点シェーダーを設定
    SetUseVertexShader(*shader_vs_);

    // 使用するピクセルシェーダーを設定
    SetUsePixelShader(*shader_ps_);

    // 描画
    DrawPrimitive3DToShader(v.data(), static_cast<int>(v.size()), DX_PRIMTYPE_TRIANGLESTRIP);
}
