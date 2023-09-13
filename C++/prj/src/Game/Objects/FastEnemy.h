//---------------------------------------------------------------------------
//! @file   FastEnemy.h
//! @brief  移動速度が速い敵
//---------------------------------------------------------------------------
#pragma once
#include <Game/Objects/EnemyBase.h>

//! @class FastEnemy
//! @brief 移動速度が速い敵
class FastEnemy : public EnemyBase
{
private:
    int move_timer_ = 0;   //!< 動ける時間

public:
    //! @brief コンストラクタ
    FastEnemy();

    //! @brief デストラクタ
    ~FastEnemy();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief GUI
    void GUI() override;

    //! @brief 接触処理
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

private:
    //! @brief 死亡
    //! @detail ヒットグループを変更、アニメーションを変更、アイテムを生成、自身を消去
    void Dead();
};