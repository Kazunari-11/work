//---------------------------------------------------------------------------
//! @file   Gun.cpp
//! @brief  プレイヤーが持つ銃
//---------------------------------------------------------------------------
#include <Game/Objects/Gun.h>
#include <Game/Objects/Player.h>
#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentAttachModel.h>

// コンストラクタ
Gun::Gun()
{
}

// デストラクタ
Gun::~Gun()
{
}

// 初期化
bool Gun::Init()
{
    __super::Init();

    SetName("Item");

    // 持たせるオブジェクト
    auto obj = Scene::CreateObject<Object>()->SetName("Gun");
    obj->SetTranslate({0, 10, 10});

    //モデルをセットする
    SetModel(obj);

    if(auto attach = obj->AddComponent<ComponentAttachModel>()) {
        auto player = Scene::GetObjectPtr<Player>("Player");

        //プレイヤーの右手にアタッチする
        attach->SetAttachObject(player, "mixamorig:RightHand");
    }
    return true;
};

// 更新処理
void Gun::Update([[maybe_unused]] float delta)
{
}

// GUI
void Gun::GUI()
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
void Gun::Draw()
{
}

//モデルをセットする
void Gun::SetModel(ObjectPtr obj)
{
    // オブジェクトにモデルをつける
    if(auto model = obj->AddComponent<ComponentModel>()) {
        model->Load("data/Game/Gun/Gun.mv1");
        model->SetTranslate({-7.29f, 11.0f, -1.0f});
        model->SetRotationAxisXYZ({0.0, -174.0f, -86.0f});
        model->SetScaleAxisXYZ({0.7f, 0.7f, 0.7f});

        // テクスチャ
        {
            // 銃全体
            Material mat{};
            mat.albedo_    = std::make_shared<Texture>("data/Game/Gun/Gun_BaseColor.png");
            mat.normal_    = std::make_shared<Texture>("data/Game/Gun/Gun_Normal.png");
            mat.metalness_ = std::make_shared<Texture>("data/Game/Gun/Gun_Metallic.png");
            materials_.push_back(mat);
        }

        //モデルに設定されているテクスチャを上書き
        model->SetProc(
            "ModelDraw",
            [model, this] {
                // この部分をDrawタイミングで使用する
                if(auto model_gun = model->GetModelClass()) {
                    {
                        auto& mat = materials_[0];
                        model_gun->overrideTexture(Model::TextureType::Diffuse, mat.albedo_);
                        model_gun->overrideTexture(Model::TextureType::Albedo, mat.albedo_);
                        model_gun->overrideTexture(Model::TextureType::Normal, mat.normal_);
                        model_gun->overrideTexture(Model::TextureType::Metalness, mat.metalness_);

                        model_gun->renderByMesh(0);
                    }
                }
            },
            ProcTiming::Draw);

        auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
        // モデルのシェーダーを変更
        model->setOverrideShader(nullptr, shader_ps_);
    }
}
