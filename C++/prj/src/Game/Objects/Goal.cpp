//---------------------------------------------------------------------------
//! @file   Goal.cpp
//! @brief  ゴール地点
//---------------------------------------------------------------------------
#include <Game/Objects/Goal.h>
#include <Game/Objects/Player.h>
#include <System/Scene.h>
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentEffect.h>

// コンストラクタ
Goal::Goal()
{
}

// デストラクタ
Goal::~Goal()
{
}

// 初期化
bool Goal::Init()
{
    is_hit_ = false;
    __super::Init();

    // コリジョン(カプセル)
    auto col = AddComponent<ComponentCollisionSphere>();
    col->SetTranslate({0, 0, 0});
    col->SetRadius(5.0);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::ETC);
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::GROUND);
    col->UseGravity();

    auto efct = AddComponent<ComponentEffect>();
    // エフェクト
    efct->Load("data/Game/Effects/goalhint.efkefc");
    efct->SetScaleAxisXYZ(5.5f);
    efct->SetTranslate({0.0f, -3.0f, 0.0f});
    efct->SetPlaySpeed(0.5f);
    efct->Play(true);

    return true;
};

// 更新処理
void Goal::Update([[maybe_unused]] float delta)
{
    // object playerをポインターで取得できる
    auto player = Scene::GetObjectPtr<Player>("Player");

    auto target = player->GetTranslate();

    is_hit_ = false;

    // スキル使われていないかつ死んでいない
    bool normal_alive = player->IsUseSkill() == false && player->IsDead() == false;

    if(normal_alive) {
        // プレイヤーと自身の距離
        float3 ptm = player->GetTranslate() - GetTranslate();

        // 近づくとゴール
        constexpr int LENG = 20;
        if(length(ptm).x < LENG) {
            is_hit_ = true;
        }
    }
}

// GUI
void Goal::GUI()
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
void Goal::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    // 当たりで移動させる
    __super::OnHit(hitInfo);
}

// 描画処理
void Goal::PostDraw()
{
}

// is_hit_の状態を取得
bool Goal::GetIsHit() const
{
    return is_hit_;
}