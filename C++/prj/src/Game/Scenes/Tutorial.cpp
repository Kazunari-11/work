//---------------------------------------------------------------------------
//! @file   Tutorial.cpp
//! @brief  チュートリアル
//---------------------------------------------------------------------------
#include "Precompile.h"
#include <Game/Scenes/Tutorial.h>
#include <Game/Scenes/Game.h>
#include <Game/Objects/Player.h>
#include <Game/Objects/NormalEnemy.h>
#include <Game/Objects/Bullet.h>
#include <Game/Objects/Camera.h>
#include <Game/Objects/TargetPoint.h>
#include <Game/Objects/Gun.h>
#include <Game/Objects/Arrow.h>
#include <Game/Objects/Goal.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCamera.h>
#include <System/Component/ComponentTargetTracking.h>
#include <Game/Objects/Filter.h>
#include <Game/Shadowmap.h>

BP_CLASS_IMPL(Tutorial, u8"チュートリアル");

// 初期化
bool Tutorial::Init()
{
    // ステージ
    {
        auto obj        = Scene::CreateObject<Object>()->SetName("Ground");
        auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray");
        obj->AddComponent<ComponentModel>("data/Game/TutorialMap/Stage/Stage00.mv1")
            // モデルのシェーダーを変更
            ->setOverrideShader(nullptr, shader_ps_);

        obj->AddComponent<ComponentCollisionModel>()->AttachToModel();
    }

    // シャドウ
    {
        auto obj = Scene::CreateObject<Shadowmap>();
    }

    auto fil = Scene::CreateObject<Filter>();

    font_handle_ = LoadFontDataToHandle("data/Game/Font/InfoFont.dft");

    //----------------------------------------------------------------------------------
    // Player
    //----------------------------------------------------------------------------------
    // Player / 位置(30,10,0)
    auto player = Scene::CreateObject<Player>();
    player->SetName("Player")->SetTranslate({30, 10, 0});

    player->SetSpeed(0.8f);
    player->SetBulletNum(100000);
    player->SetTutorial(true);

    auto arrow = Scene::CreateObject<Arrow>()->SetName("Arrow");

    //----------------------------------------------------------------------------------
    // カメラ
    //----------------------------------------------------------------------------------
    auto cam = Camera::Create(player)->SetName("PlayerCamera");

    //----------------------------------------------------------------------------------
    // Gun
    //----------------------------------------------------------------------------------
    auto gun = Scene::CreateObject<Gun>();

    //----------------------------------------------------------------------------------
    // Enemy
    //----------------------------------------------------------------------------------
    auto enemy = Scene::CreateObject<NormalEnemy>();
    enemy->SetName("NormalEnemy");
    enemy->SetTranslate({30, 10, -100});
    enemy->SetSpeed(0.45f);
    enemy->SetTutorial(true);

    // ターゲットポイント
    auto targetpoint_1 = Scene::CreateObject<TargetPoint>();
    targetpoint_1->SetName("TargetPoint");
    targetpoint_1->SetTranslate({67, 6, -100});

    sky_model_ = MV1LoadModel("data/Game/SkyDome/dome11.mqoz");

    return true;
}

// 更新処理
void Tutorial::Update([[maybe_unused]] float delta)
{
    // enterを押すとゲーム画面へ
    if(IsKeyOn(KEY_INPUT_RETURN)) {
        Scene::Change(Scene::GetScene<Game>());
    }

    auto player = Scene::GetObjectPtr<Player>("Player");
}

// 描画
void Tutorial::PostDraw()
{
    // ここのライトを入れたままにするとちょっと暗くなったりする
    SetUseLighting(FALSE);
    //	空モデルの描画
    MV1DrawModel(sky_model_);
    // ライトありで描画
    SetUseLighting(TRUE);

    DrawFillBox(WINDOW_W, 0, WINDOW_W - 400, 260, GetColor(75, 75, 75));

    constexpr int CH_OPERATION_POS_X = 900;
    constexpr int CH_OPERATION_POS_Y = 10;
    int           ch_operation_x     = CH_OPERATION_POS_X;
    int           ch_operation_y     = CH_OPERATION_POS_Y;

    DrawStringToHandle(ch_operation_x, ch_operation_y, "WASD: 移動", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 25, "マウス: カメラ移動", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 50, "左クリック: ショット", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 75, "右クリック: ターゲッティング", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 100, "スペース: スキル使用", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 125, "スキル使用中にスペース", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 150, "スキル解除", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 175, "R: リロード", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 200, "ホイールボタン: キック", WHITE, font_handle_);
    DrawStringToHandle(ch_operation_x, ch_operation_y + 225, "Enterキーでゲームスタート", WHITE, font_handle_);

    // 左上の灰色
    DrawFillBox(0, 0, 300, 40, GetColor(75, 75, 75));

    DrawStringToHandle(20, 5, "チュートリアル中", WHITE, font_handle_);
}

// 終了処理
void Tutorial::Exit()
{
    DeleteFontToHandle(font_handle_);
}

// GUI
void Tutorial::GUI()
{
}
