//---------------------------------------------------------------------------
//! @file   Tutorial.h
//! @brief  チュートリアル
//---------------------------------------------------------------------------
#pragma once
#include <System/Scene.h>

//! @class Tutorial
//! @brief チュートリアル
class Tutorial : public Scene::Base
{
public:
    BP_CLASS_TYPE(Tutorial, Scene::Base);

    //! シーン名称
    std::string Name() override { return u8"サンプル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void PostDraw() override;            //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示

private:
    std::shared_ptr<Model> model_;
    float3                 position_    = float3(0.0f, 0.0f, 0.0f);   //!<　位置
    int                    sky_model_   = -1;                         // !< スカイドーム
    int                    font_handle_ = -1;                         //!< フォントハンドル
};
