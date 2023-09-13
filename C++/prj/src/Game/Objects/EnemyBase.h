//---------------------------------------------------------------------------
//! @file   EnemyBase.h
//! @brief  敵のベース
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class EnemyBase
//! @brief 敵のベース
class EnemyBase : public Object
{
protected:
    float3      rot_         = {0.0f, 0.0f, 0.0f};   //!< 回転
    float       sp_          = 0.0f;                 //!< 移動速度
    float       hp_          = 0.0f;                 //!< 体力
    const float HP_MAX_      = 4.0f;                 //!< 体力最大値
    bool        is_dead_     = false;                //!< 死んでいるかどうか
    int         death_se_    = -1;                   //!< 死亡SE
    bool        se_flag_     = false;                //!< SEを鳴らすフラグ
    bool        is_engage_   = false;                //!< プレイヤーと接敵したか
    bool        is_hit_kick_ = false;                //!< プレイヤーのキックに当たったか

public:
    //! @brief コンストラクタ
    EnemyBase();

    //! @brief デストラクタ
    virtual ~EnemyBase();

public:
    //! @brief 移動速度をセット
    //! @param s 速度
    //! @detail sp_にsをセット
    void SetSpeed(float s) { sp_ = s; }

    //! @brief 生死の取得
    //! @detail is_dead_を取得
    //! @return 生死フラグ
    bool IsDead() const { return is_dead_; }

protected:
    //! @brief プレイヤーのキックに当たった時
    //! @detail ノックバックし、kickdamageアニメーションを再生
    void HitPlayerKick(float kick_damage = 0.5f);

    //! @brief モデルを進行方向に向ける
    //! @param move ベクトル保存変数
    void RotatoMoveDir(float3& move);

    //! @brief プレイヤーと一定距離近づいたか
    //! @param move ターゲットへのベクトル
    //! @param length 検知距離
    //! @detail 一定距離近内ならis_engage_をtrueに以外ならfalseに
    bool Engage(float3& move, const float leng = 300.0f);

    // SE再生
    //! @brief ボリュームを設定しSEを再生
    //! @param se_handle seのハンドル
    //! @param volume ボリューム
    void PlaySE(int se_handle, int volume = 255) const;
};
