//---------------------------------------------------------------------------
//! @file   Title.h
//! @brief  タイトル
//---------------------------------------------------------------------------
#pragma once
#include <System/Scene.h>

//! @class Title
//! @brief タイトル
class Title : public Scene::Base
{
public:
    BP_CLASS_TYPE(Title, Scene::Base);

    //! シーン名称
    std::string Name() override { return u8"タイトル"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void Draw() override;                //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示
    void PostDraw() override;            //!< 描画

private:
    int font_handle_ = -1;   //!< フォントハンドル
};
