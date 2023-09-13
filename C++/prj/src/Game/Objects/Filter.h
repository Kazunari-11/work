//---------------------------------------------------------------------------
//!	@file	Filter.h
//!	@brief	エフェクトフィルター
//---------------------------------------------------------------------------
#pragma once

#include <System/Object.h>

//! @class Filter
//! @brief エフェクトフィルター
class Filter final : public Object
{
private:
    std::shared_ptr<Texture>  work_texture_;   //!< ワークバッファ
    std::shared_ptr<ShaderPs> shader_ps_;      //!< シェーダー
    std::shared_ptr<ShaderVs> shader_vs_;      //!< 頂点シェーダー

public:
    //! @brief コンストラクタ
    Filter() = default;

    //! @brief 初期化
    //! @return 初期化が終わったか
    virtual bool Init() override;

    //! @brief 更新処理
    virtual void Update(float delta) override;

    //! @brief 描画処理
    virtual void Draw() override;

    //! @brief 終了処理
    virtual void Exit() override;

    //! @brief 歪みフィルターをつける円の描画
    //! @param center 中心
    //! @param radius 半径
    //! @param thickness 厚さ
    //! @param divCount 分割数
    void DrawCircleS(const float3& center, float radius, float thickness, u32 divCount);
};