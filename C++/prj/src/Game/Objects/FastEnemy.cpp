//---------------------------------------------------------------------------
//! @file   FastEnemy.cpp
//! @brief  移動速度が速い敵
//---------------------------------------------------------------------------
#include <Game/Objects/FastEnemy.h>
#include <Game/Objects/Player.h>
#include <Game/Objects/DropItem.h>
#include <System/Scene.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentEffect.h>

// コンストラクタ
FastEnemy::FastEnemy()
{
}

// デストラクタ
FastEnemy::~FastEnemy()
{
}

// 初期化
bool FastEnemy::Init()
{
    __super::Init();

    auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray");

    // モデルコンポーネント(0.07倍)
    AddComponent<ComponentModel>("data/Game/FastEnemy/model.mv1")
        ->SetScaleAxisXYZ({
            0.07f
    })
        ->SetAnimation({
            // アニメーション
            {"run", "data/Game/FastEnemy/Anim/run.mv1", 0, 1.0f},
            {"death", "data/Game/FastEnemy/Anim/Death.mv1", 0, 1.0f},
            {"idle", "data/Game/FastEnemy/Anim/idle.mv1", 0, 1.0f},
            {"kickdamage", "data/Game/FastEnemy/Anim/KickDamage.mv1", 0, 1.0f},
        })
        // モデルのシェーダーを変更
        ->setOverrideShader(nullptr, shader_ps_);

    // コリジョン(カプセル)
    auto col = AddComponent<ComponentCollisionCapsule>();
    col->SetTranslate({0, 0, 0});
    col->SetRadius(3.0f);
    col->SetHeight(13);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ENEMY);
    col->UseGravity();

    AddComponent<ComponentEffect>();

    hp_ = HP_MAX_;

    death_se_ = LoadSoundMem("data/Game/SE/Enemy/death.mp3");

    return true;
}

// 更新処理
void FastEnemy::Update([[maybe_unused]] float delta)
{
    auto mdl = GetComponent<ComponentModel>();

    auto player = Scene::GetObjectPtr<Player>("Player");
    auto target = player->GetTranslate();
    auto efct   = GetComponent<ComponentEffect>();

    // スキル使われたら＆死んだら停止
    if(player->IsUseSkill() == true) {
        // アニメーション停止
        mdl->PlayPause();
        efct->PlayPause();
        return;
    }

    auto   pos  = GetTranslate();
    float3 move = (target - pos);

    // プレイヤーと一定距離近づいたか
    EnemyBase::Engage(move);

    // 死亡時
    if(is_dead_ == true) {
        Dead();
        return;
    }

    // プレイヤーが死んでいなかったら かつ 一定距離以内にいたら
    bool is_player_alive = player->IsDead() == false && is_engage_ == true;
    if(is_player_alive == false) {
        if(mdl->GetPlayAnimationName() != "idle") {
            mdl->PlayAnimation("idle", true);
        }
        return;
    }

    if(player->GetIsClear() == true) {
        return;
    }

    // 止まっていたアニメーションを動かす
    mdl->PlayPause(false);

    // プレイヤーのキックに当たったら
    if(is_hit_kick_ == true) {
        EnemyBase::HitPlayerKick();
        return;
    }

    // モデルを進行方向に向ける
    EnemyBase::RotatoMoveDir(move);
    move_timer_++;

    bool can_move = player->IsUseSkill() == false && is_dead_ == false;
    if(can_move) {
        // 3秒動いた後、1秒止まるとタイマーリセット
        if(240 < move_timer_) {
            move_timer_ = 0;
        }

        // これ以上になると止まる
        constexpr int STOP_TIME = 180;
        if(STOP_TIME < move_timer_) {
            if(mdl->GetPlayAnimationName() != "idle") {
                mdl->PlayAnimation("idle", true);
            }
            return;
        }

        move *= sp_ * (delta * 60.0f);
        // 地面移動スピードを決定する
        AddTranslate(move);
        if(mdl->GetPlayAnimationName() != "run") {
            mdl->PlayAnimation("run", true);
        }
    }
}

// GUI
void FastEnemy::GUI()
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();

        ImGui::DragFloat(u8"速度", &sp_, 0.01f, 0.01f, 10.0f, "%2.2f");
        ImGui::DragFloat(u8"hp", &hp_, 0.01f, 0.01f, 10.0f, "%2.2f");
    }
    ImGui::End();
}

// 接触処理
void FastEnemy::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    auto owner = hitInfo.hit_collision_->GetOwnerPtr();
    if(owner->GetNameDefault() == "Bullet") {
        is_dead_ = true;
    }

    if(owner->GetNameDefault() == "PlayerKick") {
        is_hit_kick_ = true;
        // エフェクト
        auto efct = GetComponent<ComponentEffect>();
        efct->Load("data/Game/Effects/Simple_SpawnMethod1.efkefc");
        efct->Play(false);
        efct->SetScaleAxisXYZ(0.5f);
        efct->SetTranslate({0.0f, 10.0f, 0.0f});
        efct->SetPlaySpeed(0.8f);
    }

    // 当たりで移動させる
    __super::OnHit(hitInfo);
}

// 死亡
void FastEnemy::Dead()
{
    auto mdl = GetComponent<ComponentModel>();
    // 止まっていたアニメーションを動かす
    mdl->PlayPause(false);

    // グループ変更
    auto col = GetComponent<ComponentCollisionCapsule>();
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::GROUND);

    if(mdl->IsPlaying() == false) {
        Scene::CreateObject<DropItem>()->SetTranslate(GetTranslate())->SetName("Item");
        Scene::ReleaseObject(SharedThis());
    }

    if(mdl->GetPlayAnimationName() != "death") {
        mdl->PlayAnimation("death");
    }

    if(se_flag_ == false) {
        EnemyBase::PlaySE(death_se_, 60);

        // エフェクト
        auto efct = GetComponent<ComponentEffect>();
        efct->Load("data/Game/Effects/Simple_SpawnMethod1.efkefc");
        efct->Play(false);
        efct->SetScaleAxisXYZ(0.5f);
        efct->SetTranslate({0.0f, 10.0f, 0.0f});
        efct->SetPlaySpeed(0.8f);

        se_flag_ = true;
    }
}
