//---------------------------------------------------------------------------
//! @file   TargetPoint.cpp
//! @brief  プレイヤーが目指す地点
//---------------------------------------------------------------------------
#include <Game/Objects/TargetPoint.h>
#include <Game/Objects/Player.h>
#include <System/Scene.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentEffect.h>

// コンストラクタ
TargetPoint::TargetPoint()
{
}

// デストラクタ
TargetPoint::~TargetPoint()
{
}

// 初期化
bool TargetPoint::Init()
{
    __super::Init();
    is_hit_           = false;
    info_font_handle_ = LoadFontDataToHandle("data/Game/Font/InfoFont.dft");

    // コリジョン(カプセル)
    auto col = AddComponent<ComponentCollisionSphere>();
    col->SetTranslate({0, 0, 0});
    col->SetRadius(5.0);

    // モデルをセットする
    SetModel();
    auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
    auto model      = GetComponent<ComponentModel>();
    // モデルのシェーダーを変更
    model->setOverrideShader(nullptr, shader_ps_);

    auto efct = AddComponent<ComponentEffect>();
    // エフェクト
    efct->Load("data/Game/Effects/goalhint.efkefc");
    efct->SetScaleAxisXYZ(5.5f);
    efct->SetTranslate({0.0f, -5.0f, 0.0f});
    efct->SetPlaySpeed(0.5f);
    efct->Play(true);

    return true;
};

// 更新処理
void TargetPoint::Update([[maybe_unused]] float delta)
{
    auto mdl = GetComponent<ComponentModel>();

    auto player = Scene::GetObjectPtr<Player>("Player");

    auto target = player->GetTranslate();

    is_hit_ = false;

    // スキル使われていないかつ死んでいない
    bool normal_alive = player->IsUseSkill() == false && player->IsDead() == false;

    if(normal_alive) {
        // プレイヤーと自身の距離
        float3 ptm = player->GetTranslate() - GetTranslate();
        // 近づくと修理を始める
        constexpr int LENG = 15;
        if(length(ptm).x < LENG) {
            is_hit_ = true;
            stay_timer_ += 0.03f;
        }

        if(STAY_TIME_ <= stay_timer_) {
            stay_creal_ = true;
            stay_timer_ = STAY_TIME_;
        }
    }
}

void TargetPoint::GUI()
{
    __super::GUI();

    // GUI描画
    ImGui::Begin(GetName().data());
    {
        ImGui::Separator();
        ImGui::DragFloat(u8"stay_timer", &stay_timer_, 1, 0, 100, "%2.2f");
    }
    ImGui::End();
}

// 接触処理
void TargetPoint::OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
{
}

// 描画処理
void TargetPoint::PostDraw()
{
    // プレイヤーと接触中
    if(is_hit_ == true) {
        //ゲージを描画
        DrawGauge();
    }
}

// is_hit_の状態を取得
bool TargetPoint::GetIsHit() const
{
    return is_hit_;
}

// stay_crealの状態取得
bool TargetPoint::GetStayCreal() const
{
    return stay_creal_;
}

// stay_timerの取得
float TargetPoint::GetStayTimer() const
{
    return stay_timer_;
}

// モデルをセットする
void TargetPoint::SetModel()
{
    if(auto model = AddComponent<ComponentModel>()) {
        model->Load("data/Game/Generator/model.mv1");
        model->SetTranslate({0.0f, -5.0f, 0.0f});
        model->SetScaleAxisXYZ({0.01f, 0.01, 0.01f});

        // テクスチャ
        {
            std::string path = "data/Game/Generator/";

            Material mat{};
            mat.albedo_    = std::make_shared<Texture>(path + "SciFi Generator Dif1024.png");
            mat.normal_    = std::make_shared<Texture>(path + "SciFi Generator-Nor1024.png");
            mat.metalness_ = std::make_shared<Texture>(path + "SciFi Generator Emit1024.png");

            materials_.push_back(mat);
        }

        // モデルに設定されているテクスチャを上書き
        model->SetProc(
            "ModelDraw",
            [model, this] {
                // この部分をDrawタイミングで使用する
                if(auto model_box = model->GetModelClass()) {
                    auto& mat = materials_[0];
                    model_box->overrideTexture(Model::TextureType::Diffuse, mat.albedo_);
                    model_box->overrideTexture(Model::TextureType::Normal, mat.normal_);
                    model_box->overrideTexture(Model::TextureType::Metalness, mat.metalness_);

                    model_box->renderByMesh(0);
                }
            },
            ProcTiming::Draw);

        auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray_vertex_diffuse");
        // モデルのシェーダーを変更
        model->setOverrideShader(nullptr, shader_ps_);
    }
}

//ゲージを描画
void TargetPoint::DrawGauge() const
{
    // ゲージの表示する位置
    constexpr int BOX_POS_X = 350;
    constexpr int BOX_POS_Y = 50;
    constexpr int SPACE     = 1;
    constexpr int TIME_MAX  = 25;
    // 下地
    DrawFillBox(BOX_POS_X, BOX_POS_Y - 35, BOX_POS_X + TIME_MAX * 20 + SPACE, BOX_POS_Y + 30, GetColor(75, 75, 75));
    // 内
    DrawFillBox(BOX_POS_X, BOX_POS_Y, int(BOX_POS_X + stay_timer_ * 20), BOX_POS_Y + 30, RED);
    // 外枠
    DrawBox(BOX_POS_X - SPACE,
            BOX_POS_Y - SPACE,
            BOX_POS_X + TIME_MAX * 20 + SPACE,
            BOX_POS_Y + 30 + SPACE,
            WHITE,
            false);

    // 文字表示
    DrawStringToHandle(BOX_POS_X + 200, BOX_POS_Y - 30, "修理中", WHITE, info_font_handle_);
}
