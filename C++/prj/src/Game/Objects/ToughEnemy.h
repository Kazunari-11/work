//---------------------------------------------------------------------------
//! @file   ToughEnemy.h
//! @brief  体力の多い敵
//---------------------------------------------------------------------------
#pragma once
#include <Game/Objects/EnemyBase.h>

//! @class ToughEnemy
//! @brief 体力の多い敵
class ToughEnemy : public EnemyBase
{
private:
    bool damage_flag_ = false;   // ダメージを受けたかのフラグ

public:
    //! @brief コンストラクタ
    ToughEnemy();

    //! @brief デストラクタ
    ~ToughEnemy();

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