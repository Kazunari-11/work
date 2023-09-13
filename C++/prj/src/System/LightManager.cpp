//---------------------------------------------------------------------------
//! @file   LightManager.cpp
//! @brief  光源管理
//---------------------------------------------------------------------------
#include "LightManager.h"

//---------------------------------------------------------------------------
//! 初期化
//---------------------------------------------------------------------------
bool LightManager::initialize()
{
    //----------------------------------------------------------
    // 定数バッファを作成
    //----------------------------------------------------------
    cb_handle_ = CreateShaderConstantBuffer(sizeof(info_));

    // [TEST] ライトパラメーター設定
    info_.light_count_directional_ = 0;

    info_.light_directional_[0].light_dir_ = float3(0.0f, 1.0f, 0.0f);
    info_.light_directional_[0].color_     = float3(0.5f, 0.8f, 1.0f);

    info_.light_directional_[1].light_dir_ = float3(0.0f, -1.0f, 0.0f);
    info_.light_directional_[1].color_     = float3(0.5f, 0.3f, 0.0f);

    // [TEST] 点光源
    info_.light_count_point_        = 1;
    info_.light_point_[0].color_    = float3(1.0f, 1.0f, 1.0f) * 500;
    info_.light_point_[0].position_ = float3(10.0f, 10.0f, 0.0f);
    info_.light_point_[0].radius_   = 5.0f;

    return true;
}

//---------------------------------------------------------------------------
//! 更新
//---------------------------------------------------------------------------
void LightManager::update()
{
    //----------------------------------------------------------
    // 定数バッファを更新
    //----------------------------------------------------------
    void* p = GetBufferShaderConstantBuffer(cb_handle_);
    memcpy(p, &info_, sizeof(LightInfo));

    // メモリをGPU側へ転送
    UpdateShaderConstantBuffer(cb_handle_);

    //----------------------------------------------------------
    // 定数バッファを設定
    //----------------------------------------------------------
    // b11 = LightInfo
    SetShaderConstantBuffer(cb_handle_, DX_SHADERTYPE_PIXEL, 11);
}

//---------------------------------------------------------------------------
//! 解放
//---------------------------------------------------------------------------
void LightManager::finalize()
{
    //----------------------------------------------------------
    // 定数バッファを解放
    //----------------------------------------------------------
    DeleteShaderConstantBuffer(cb_handle_);
    cb_handle_ = -1;
}
