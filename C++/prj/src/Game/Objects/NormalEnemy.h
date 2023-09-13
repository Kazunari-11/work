//---------------------------------------------------------------------------
//! @file   NormalEnemy.h
//! @brief  通常の敵
//---------------------------------------------------------------------------
#pragma once
#include <Game/Objects/EnemyBase.h>

//! @class NormalEnemy
//! @brief 通常の敵
class NormalEnemy : public EnemyBase
{
private:
    bool is_tutorial_ = false;   //!< チュートリアル中かどうかのフラグ

public:
    // チュートリアル状態にするか
    bool SetTutorial(bool flag = false) { return is_tutorial_ = flag; }

    //! @brief コンストラクタ
    NormalEnemy();

    //! @brief デストラクタ
    ~NormalEnemy();

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
    void Dead();

    //! @brief チュートリアル時の死亡処理
    //! @detail ヒットグループを変更、アニメーションを変更、HPを最大値に
    void TutorialDead();

    //! @brief ゲーム時の死亡処理
    //! @detail ヒットグループを変更、アニメーションを変更、アイテムを生成、自身を消去
    void GameDead();
};
