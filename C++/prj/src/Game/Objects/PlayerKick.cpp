//---------------------------------------------------------------------------
//! @file   PlayerKick.cpp
//! @brief  プレイヤーキック
//---------------------------------------------------------------------------
#include <Game/Objects/PlayerKick.h>
#include <Game/Objects/Player.h>
#include <System/Scene.h>
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentAttachModel.h>

// コンストラクタ
PlayerKick::PlayerKick()
{
}

// デストラクタ
PlayerKick::~PlayerKick()
{
}

// 初期化
bool PlayerKick::Init()
{
    is_kicking_ = false;
    __super::Init();

    // コリジョン(カプセル)
    auto col = AddComponent<ComponentCollisionCapsule>();
    col->SetRadius(6.0f);
    col->SetHeight(2.5f);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::PLAYER);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY);
    col->Overlap();

    // プレイヤーの足に付ける
    auto player = Scene::GetObjectPtr<Player>("Player");
    auto attach = AddComponent<ComponentAttachModel>();
    attach->SetAttachObject(player, "mixamorig:RightFoot");

    SetName("PlayerKick");

    return true;
};

// 更新処理
void PlayerKick::Update([[maybe_unused]] float delta)
{
    auto player = Scene::GetObjectPtr<Player>("Player");

    // プレイヤーが死んでいなかったら
    if(player->IsDead() == false) {
        auto mdl = player->GetComponent<ComponentModel>();

        if(player->GetIsKicking() == false && player->GetIsReloading() == false) {
            if(mdl->GetPlayAnimationName() != "kick") {
                mdl->PlayAnimation("kick");
                player->SetIsKicking(true);
            }
        }
        else {
            // is_kickingがtrue中にアニメーションが終わるとフラグをfalseに
            if(mdl->IsPlaying() == false) {
                player->SetIsKicking(false);
                Scene::ReleaseObject(SharedThis());
            }
        }
    }
}

// GUI
void PlayerKick::GUI()
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
    }
    ImGui::End();
}

// 接触処理
void PlayerKick::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    // 当たりで移動させる
    __super::OnHit(hitInfo);
}

// iskicking_の状態を取得
bool PlayerKick::GetIsKicking_() const
{
    return is_kicking_;
}
