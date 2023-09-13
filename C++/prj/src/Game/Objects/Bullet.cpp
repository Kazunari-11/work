//---------------------------------------------------------------------------
//! @file   Bullet.cpp
//! @brief  弾
//---------------------------------------------------------------------------
#include <Game/Objects/Bullet.h>
#include <System/Scene.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentModel.h>
#include <System/Utils/HelperLib.h>
#include <Game/Objects/Player.h>
#include <Game/Objects/Gun.h>

// コンストラクタ
Bullet::Bullet()
{
}

// デストラクタ
Bullet::~Bullet()
{
}

// 初期化
bool Bullet::Init()
{
    __super::Init();

    sp_     = 250.0f;
    radius_ = 1.0f;

    SetName("Bullet");
    auto col = AddComponent<ComponentCollisionSphere>()->SetRadius(radius_);
    col->SetCollisionGroup(ComponentCollision::CollisionGroup::WEAPON);

    // 敵と地面に当たるように
    col->SetHitCollisionGroup((u32)ComponentCollision::CollisionGroup::ENEMY |
                              (u32)ComponentCollision::CollisionGroup::GROUND);

    // モデルをセットする
    SetModel();

    return true;
}

// 更新処理
void Bullet::Update(float delta)
{
    auto player = Scene::GetObjectPtr<Player>("Player");

    if(player->IsUseSkill() == false) {
        // 前に動く
        AddTranslate(vec_ * sp_ * delta);

        // 時間経過後消去
        timer_++;
        if(DELETE_TIME_ < timer_) {
            Scene::ReleaseObject(SharedThis());
        }
    }
}

// 描画処理
void Bullet::Draw()
{
}

// 位置、方向設定
void Bullet::SetPositionAndDirection(float3 pos, float3 vec)
{
    vec_ = normalize(vec);
    SetMatrix(HelperLib::Math::CreateMatrixByFrontVector(vec));
    SetTranslate(pos);
}

// 接触処理
void Bullet::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    // 次のownerのオブジェクトと当たった!
    auto owner  = hitInfo.hit_collision_->GetOwnerPtr();
    auto player = Scene::GetObjectPtr<Player>("Player");

    // スキルが使われていない時だけ当たり判定をする
    if(player->IsUseSkill() == false) {
        if(owner->GetNameDefault() == "Enemy") {
            Scene::ReleaseObject(SharedThis());
        }
        if(owner->GetNameDefault() == "Ground") {
            Scene::ReleaseObject(SharedThis());
        }
    }
}

// モデルをセットする
void Bullet::SetModel()
{
    if(auto model = AddComponent<ComponentModel>()) {
        model->Load("data/Game/Bullet/model.mv1");

        // テクスチャ
        {
            std::string path = "data/Game/Bullet/";

            Material mat{};
            mat.albedo_    = std::make_shared<Texture>(path + "Iron_basecolor.png");
            mat.normal_    = std::make_shared<Texture>(path + "Iron_normal.png");
            mat.roughness_ = std::make_shared<Texture>(path + "Iron_roughness.png");
            mat.metalness_ = std::make_shared<Texture>(path + "Iron_metallic.png");

            materials_.push_back(mat);
        }

        auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
        // モデルのシェーダーを変更
        model->setOverrideShader(nullptr, shader_ps_);

        model->SetScaleAxisXYZ({0.4f, 0.4f, 0.4f});
        model->SetRotationAxisXYZ({90.0f, 0.0f, 0.0f});
        model->SetTranslate({GetTranslate().x, GetTranslate().y, GetTranslate().z - 1.0f});

        if(auto m = model->GetModelClass()) {
            auto& mat = materials_[0];
            m->overrideTexture(Model::TextureType::Diffuse, mat.albedo_);
            m->overrideTexture(Model::TextureType::Normal, mat.normal_);
            m->overrideTexture(Model::TextureType::Roughness, mat.roughness_);
            m->overrideTexture(Model::TextureType::Metalness, mat.metalness_);
        }
    }
}