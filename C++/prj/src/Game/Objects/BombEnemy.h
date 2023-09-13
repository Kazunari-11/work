//---------------------------------------------------------------------------
//! @file   BombEnemy.h
//! @brief  爆発する敵
//---------------------------------------------------------------------------
#pragma once
#include <Game/Objects/EnemyBase.h>

//! @class BombEnemy
//! @brief 爆発する敵
class BombEnemy : public EnemyBase
{
public:
    //! @brief コンストラクタ
    BombEnemy();

    //! @brief デストラクタ
    ~BombEnemy();

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
    //! @param mat 各ベクトルを保存する
    //! @param ヒットグループを変更、アニメーションを変更、アイテムを生成、自身を消去
    void Dead(float3& move);
};
