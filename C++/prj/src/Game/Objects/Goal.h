//---------------------------------------------------------------------------
//! @file   Goal.h
//! @brief  ゴール地点
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class Goal
//! @brief ゴール地点
class Goal : public Object
{
private:
    bool is_hit_ = false;   //!< プレイヤーが接触したかのフラグ

public:
    //! @brief 当たっているかのフラグの取得
    //! @detail is_hit_を取得
    //! @return 当たっているフラグ
    bool GetIsHit() const;

    //! @brief コンストラクタ
    Goal();

    //! @brief デストラクタ
    ~Goal();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief GUI
    void GUI() override;

    //! @brief 接触処理
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

    //! @brief 描画処理
    void PostDraw() override;
};
