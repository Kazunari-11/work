//---------------------------------------------------------------------------
//! @file   NormalEnemy.cpp
//! @brief  通常の敵
//---------------------------------------------------------------------------
#include <Game/Objects/NormalEnemy.h>
#include "Game/Scenes/Game.h"
#include <Game/Objects/Player.h>
#include <Game/Objects/DropItem.h>
#include <System/Scene.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentEffect.h>

// コンストラクタ
NormalEnemy::NormalEnemy()
{
}

// デストラクタ
NormalEnemy::~NormalEnemy()
{
}

// 初期化
bool NormalEnemy::Init()
{
    __super::Init();

    auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray");

    // モデルコンポーネント(0.085倍)
    AddComponent<ComponentModel>("data/Game/NormalEnemy/model.mv1")
        ->SetScaleAxisXYZ({
            0.085f
    })
        ->SetAnimation({
            // アニメーション
            {"run", "data/Game/NormalEnemy/Anim/Run.mv1", 0, 1.0f},
            {"death", "data/Game/NormalEnemy/Anim/Death.mv1", 0, 1.0f},
            {"idle", "data/Game/NormalEnemy/Anim/Idle.mv1", 0, 1.0f},
            {"kickdamage", "data/Game/NormalEnemy/Anim/KickDamage.mv1", 0, 1.0f},
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

    death_se_ = LoadSoundMem("data/Game/SE/Enemy/death.mp3");

    return true;
};

// 更新処理
void NormalEnemy::Update([[maybe_unused]] float delta)
{
    auto mdl = GetComponent<ComponentModel>();

    auto player = Scene::GetObjectPtr<Player>("Player");
    auto target = player->GetTranslate();
    auto efct   = GetComponent<ComponentEffect>();

    // スキル使われたら
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

    // チュートリアルじゃなければ動く
    if(is_tutorial_ == true) {
        if(mdl->GetPlayAnimationName() != "idle") {
            mdl->PlayAnimation("idle", true);
        }
        se_flag_ = false;
        return;
    }

    bool can_move = player->IsUseSkill() == false && is_dead_ == false;
    if(can_move) {
        move *= sp_ * (delta * 60.0f);
        // 地面移動スピードを決定する
        AddTranslate(move);
        if(mdl->GetPlayAnimationName() != "run") {
            mdl->PlayAnimation("run", true);
        }
    }
}

// GUI
void NormalEnemy::GUI()
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
void NormalEnemy::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
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
void NormalEnemy::Dead()
{
    auto mdl = GetComponent<ComponentModel>();
    // 止まっていたアニメーションを動かす
    mdl->PlayPause(false);

    if(se_flag_ == false) {
        EnemyBase::PlaySE(death_se_, 60);

        if(mdl->GetPlayAnimationName() != "death") {
            mdl->PlayAnimation("death");
        }

        // エフェクト
        auto efct = GetComponent<ComponentEffect>();
        efct->Load("data/Game/Effects/Simple_SpawnMethod1.efkefc");
        efct->Play(false);
        efct->SetScaleAxisXYZ(0.5f);
        efct->SetTranslate({0.0f, 10.0f, 0.0f});
        efct->SetPlaySpeed(0.8f);

        se_flag_ = true;
    }

    // チュートリアル中なら自身を消さずに無限に立たせる
    if(is_tutorial_ == true) {
        // チュートリアル時の死亡処理
        TutorialDead();
        return;
    }
    // ゲーム時の死亡処理
    GameDead();
}

// チュートリアル時の死亡処理
void NormalEnemy::TutorialDead()
{
    // グループ変更
    auto col = GetComponent<ComponentCollisionCapsule>();
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::GROUND);

    auto mdl = GetComponent<ComponentModel>();
    if(mdl->IsPlaying() == false) {
        is_dead_ = false;
        hp_      = 4.0f;

        // グループ変更
        col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::GROUND |
                                  (u32)ComponentCollision::CollisionGroup::WEAPON |
                                  (u32)ComponentCollision::CollisionGroup::PLAYER);
    }
}

// ゲーム時の死亡処理
void NormalEnemy::GameDead()
{
    // グループ変更
    auto col = GetComponent<ComponentCollisionCapsule>();
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::GROUND);

    auto mdl = GetComponent<ComponentModel>();
    if(mdl->IsPlaying() == false) {
        // 弾数を増やすアイテム生成
        Scene::CreateObject<DropItem>()->SetTranslate(GetTranslate())->SetName("Item");
        Scene::ReleaseObject(SharedThis());
    }
}