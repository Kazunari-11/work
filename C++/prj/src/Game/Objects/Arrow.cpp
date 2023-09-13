//---------------------------------------------------------------------------
//! @file   Arrow.cpp
//! @brief  目標地点を示す矢印
//---------------------------------------------------------------------------
#include <Game/Objects/Arrow.h>
#include <Game/Objects/Player.h>
#include <System/Scene.h>
#include <Game/Objects/TargetPoint.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentAttachModel.h>

// コンストラクタ
Arrow::Arrow()
{
}

// デストラクタ
Arrow::~Arrow()
{
}

// 初期化
bool Arrow::Init()
{
    __super::Init();

    SetName("Item");

    // オブジェクトにモデルをつける
    auto model = AddComponent<ComponentModel>();
    model->Load("data/Game/Arrow/Arrow.mv1");
    model->SetRotationAxisXYZ({0, 0, 90});
    model->SetScaleAxisXYZ(0.8f);
    model->SetTranslate({0.0f, 5.0f, 0.0f});

    auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
    // モデルのシェーダーを変更
    model->setOverrideShader(nullptr, shader_ps_);

    return true;
};

// 更新処理
void Arrow::Update([[maybe_unused]] float delta)
{
    auto player = Scene::GetObjectPtr<Player>("Player");
    // プレイヤーの上部へ
    SetTranslate({player->GetTranslate().x, player->GetTranslate().y + 10, player->GetTranslate().z});

    // ターゲットの方向へ向ける
    RotationToTarget();
}

// GUI
void Arrow::GUI()
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
    }
    ImGui::End();
}

// ターゲットの方向へ向ける
void Arrow::RotationToTarget()
{
    auto targetpoint = Scene::GetObjectPtr<TargetPoint>("TargetPoint");
    auto target      = targetpoint->GetTranslate();
    auto pos         = GetTranslate();

    float3 move = (target - pos);
    move        = normalize(move);
    float x     = -move.x;
    float z     = -move.z;
    float theta = atan2(x, z) * RadToDeg;
    auto  mdl   = GetComponent<ComponentModel>();
    mdl->SetRotationAxisXYZ({GetRotationAxisXYZ().x + 90.0f, theta, GetRotationAxisXYZ().z});
}
