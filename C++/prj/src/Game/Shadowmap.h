//---------------------------------------------------------------------------
//! @file   Shadowmap.h
//! @brief  シャドウマップ
//---------------------------------------------------------------------------
#pragma once
#include "System/Object.h"

//! @class Shadowmap
//! @brief シャドウマップ
class Shadowmap : public Object
{
public:
    virtual bool Init() override;   //!< 初期化
    virtual void
                 LateUpdate([[maybe_unused]] float delta) override;   //!< 遅い更新 (カメラ位置・プレイヤー位置が確定した後)
    virtual void Draw() override;                                     //!< 描画
    virtual void Exit() override;                                     //!< 終了
    virtual void GUI() override;                                      //!< GUI

    //! シャドウテクスチャを取得
    static Texture* shadowTexture();
};