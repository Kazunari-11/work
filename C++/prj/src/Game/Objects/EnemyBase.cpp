//---------------------------------------------------------------------------
//! @file   EnemyBase.cpp
//! @brief  敵のベース
//---------------------------------------------------------------------------
#include <Game/Objects/EnemyBase.h>
#include <System/Component/ComponentModel.h>

// コンストラクタ
EnemyBase::EnemyBase()
{
    hp_ = HP_MAX_;
}

// デストラクタ
EnemyBase::~EnemyBase()
{
}

// プレイヤーのキックに当たった時
void EnemyBase::HitPlayerKick(float kick_damage)
{
    auto   objmtx = GetMatrix();
    auto   mdl    = GetComponent<ComponentModel>();
    auto   mtx    = mdl->GetMatrix();
    matrix mat    = mul(objmtx, mtx);
    // 後ろ方向をとるため
    auto vec = mat.axisZ();
    // スケールが小さい影響でかける値が大きくなっている
    vec *= 0.5f;
    SetTranslate(GetTranslate() + vec);

    if(mdl->IsPlaying() == false) {
        is_hit_kick_ = false;
        return;
    }

    if(mdl->GetPlayAnimationName() != "kickdamage") {
        mdl->PlayAnimation("kickdamage");
        hp_ -= kick_damage;
        if(hp_ <= 0.0f) {
            is_dead_ = true;
        }
    }
}

// モデルを進行方向に向ける
void EnemyBase::RotatoMoveDir(float3& move)
{
    if(length(move).x < 0)
        return;

    move        = normalize(move);
    float x     = -move.x;
    float z     = -move.z;
    float theta = atan2(x, z) * RadToDeg - rot_.y;
    auto  mdl   = GetComponent<ComponentModel>();
    mdl->SetRotationAxisXYZ({0, theta, 0});
}

// プレイヤーと一定距離近づいたか
bool EnemyBase::Engage(float3& move, const float leng)
{
    // ある程度近づいたら動き出す
    if(length(move).x < leng) {
        return is_engage_ = true;
    }
    return is_engage_ = false;
}

// ボリュームを設定しSEを再生
void EnemyBase::PlaySE(int se_handle, int volume) const
{
    ChangeVolumeSoundMem(volume, se_handle);
    PlaySoundMem(se_handle, DX_PLAYTYPE_BACK);
}
