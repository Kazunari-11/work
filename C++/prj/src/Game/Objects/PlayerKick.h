//---------------------------------------------------------------------------
//! @file   PlayerKick.h
//! @brief  プレイヤーキック
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class PlayerKick
//! @brief プレイヤーキック
class PlayerKick : public Object
{
private:
    bool is_kicking_ = false;   //!< キック中かどうかのフラグ

public:
    //! @brief コンストラクタ
    PlayerKick();

    //! @brief デストラクタ
    ~PlayerKick();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief GUI
    void GUI() override;

    //! @brief 接触処理
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

    //! @brief フラグの取得
    //! @detail iskicking_を取得
    //! @return キック中かの判定フラグ
    bool GetIsKicking_() const;
};
