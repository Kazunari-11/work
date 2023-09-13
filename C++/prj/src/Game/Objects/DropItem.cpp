//---------------------------------------------------------------------------
//! @file   DropItem.cpp
//! @brief  ドロップアイテム
//---------------------------------------------------------------------------
#include <Game/Objects/DropItem.h>
#include <Game/Objects/Player.h>
#include <System/Scene.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentAttachModel.h>

// コンストラクタ
DropItem::DropItem()
{
}

// デストラクタ
DropItem::~DropItem()
{
}

// 初期化
bool DropItem::Init()
{
    is_hit_ = false;
    __super::Init();

    SetName("Item");

    //モデルをセットする
    SetModel();

    auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
    auto model      = GetComponent<ComponentModel>();
    // モデルのシェーダーを変更
    model->setOverrideShader(nullptr, shader_ps_);

    auto col = AddComponent<ComponentCollisionCapsule>();
    col->SetRadius(4);
    col->UseGravity();

    return true;
};

// 更新処理
void DropItem::Update([[maybe_unused]] float delta)
{
    SetRotationAxisXYZ({GetRotationAxisXYZ().x, deletetimer_, GetRotationAxisXYZ().z});
    deletetimer_ += 1;

    if(DELETE_TIME_ < deletetimer_) {
        Scene::ReleaseObject(SharedThis());
    }
}

// GUI
void DropItem::GUI()
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
    }
    ImGui::End();
}

// 描画
void DropItem::Draw()
{
}

// 接触処理
void DropItem::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
    auto owner = hitInfo.hit_collision_->GetOwnerPtr();
    if(owner->GetNameDefault() == "Player") {
        auto player = Scene::GetObjectPtr<Player>("Player");
        player->GetAmmo();

        Scene::ReleaseObject(SharedThis());
    }

    // 当たりで移動させる
    __super::OnHit(hitInfo);
}

// モデルをセットする
void DropItem::SetModel()
{
    if(auto model = AddComponent<ComponentModel>()) {
        model->Load("data/Game/DropItem/Sci-fi Container.mv1");
        model->SetRotationAxisXYZ({0, 0, 0});
        model->SetScaleAxisXYZ(5.0f);
        model->SetTranslate({0.0f, 3.0f, 0.0f});

        // テクスチャ
        {
            std::string path = "data/Game/DropItem/Textures/";

            Material mat{};
            mat.albedo_    = std::make_shared<Texture>(path + "Sci-fi_Box_AlbedoTransparency.png");
            mat.normal_    = std::make_shared<Texture>(path + "Sci-fi_Box_Normal_DirectX.png");
            mat.roughness_ = std::make_shared<Texture>(path + "Sci-fi_Box_MetallicSmoothness.png");
            mat.metalness_ = std::make_shared<Texture>(path + "Sci-fi_Box_MetallicSmoothness.png");
            mat.tex_ao_    = std::make_shared<Texture>(path + "Sci-fi_Box_AO.png");

            materials_.push_back(mat);
        }

        // モデルに設定されているテクスチャを上書き
        model->SetProc(
            "ModelDraw",
            [model, this] {
                if(auto model_box = model->GetModelClass()) {
                    auto& mat = materials_[0];
                    model_box->overrideTexture(Model::TextureType::Diffuse, mat.albedo_);
                    model_box->overrideTexture(Model::TextureType::Normal, mat.normal_);
                    model_box->overrideTexture(Model::TextureType::Roughness, mat.roughness_);
                    model_box->overrideTexture(Model::TextureType::Metalness, mat.metalness_);
                    model_box->overrideTexture(Model::TextureType::Albedo, mat.tex_ao_);

                    model_box->renderByMesh(0);
                }
            },
            ProcTiming::Draw);

        auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
        // モデルのシェーダーを変更
        model->setOverrideShader(nullptr, shader_ps_);
    }
}
