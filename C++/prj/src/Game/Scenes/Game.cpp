//---------------------------------------------------------------------------
//! @file   Game.cpp
//! @brief  ゲームシーン
//---------------------------------------------------------------------------
#include <Game/Scenes/Game.h>
#include <Game/Objects/Camera.h>
#include <Game/Objects/Bullet.h>
#include <Game/Objects/Player.h>
#include <Game/Objects/Gun.h>
#include <Game/Objects/Arrow.h>
#include <Game/Objects/NormalEnemy.h>
#include <Game/Objects/BombEnemy.h>
#include <Game/Objects/FastEnemy.h>
#include <Game/Objects/ToughEnemy.h>
#include <Game/Objects/TargetPoint.h>
#include <Game/Objects/Goal.h>
#include <Game/Scenes/Title.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentCamera.h>
#include <Game/Objects/Filter.h>
#include <Game/Shadowmap.h>

BP_CLASS_IMPL(Game, u8"Game");

namespace
{
// NPC沸き場所
float3 Spawn_pos[] = {
  // スポーン後の曲がり角すぐ
    {-150.0f,  0.0f, -100.0f},

 // 修理1中の曲がり角後ろ
    {-310.0f, 17.0f,   95.0f},
 // 修理1中の屋根上
    {-310.0f, 75.0f,   10.0f},

 // 階段上ガレージ前広場
    {  80.0f, 17.0f,  -10.0f},
 // コンテナ付近曲がり角
    { 170.0f, 20.0f, -110.0f},

 // 修理部品周り
    { 490.0f, 26.0f,  -40.0f},

 // 修理２中入口
    { 320.0f, 20.0f, -170.0f},
 // 修理２中タンク裏
    { 475.0f, 20.0f, -300.0f},
 // 修理２中建物裏
    { 600.0f, 20.0f, -270.0f},
 // 修理２中コンテナ上奇襲
    { 500.0f, 80.0f,   80.0f},

 // ２から３への通りの真ん中
    { 255.0f, 20.0f,   80.0f},

 // 修理3中建物小裏
    { 440.0f, 20.0f,  265.0f},
 // 修理3中建物大隙間
    { 145.0f, 20.0f,  420.0f},
 // 修理3中小階段パレット上
    { 190.0f, 32.0f,  275.0f},

 // 脱出時向かいの通路
    { 250.0f, 23.0f,   10.0f},
};
}   // namespace

// 初期化
bool Game::Init()
{
    for(int i = 0; i < 3; i++) {
        is_hit_target[i] = false;
    }

    // ステージ
    {
        auto obj = Scene::CreateObject<Object>()->SetName("Ground");

        auto shader_ps_ = std::make_shared<ShaderPs>("data/Shader/ps_gray");

        // FPSサンプルマップ
        obj->AddComponent<ComponentModel>("data/Game/GameMap/Map.mv1")
            ->SetScaleAxisXYZ({10, 10, 10})
            // モデルのシェーダーを変更
            ->setOverrideShader(nullptr, shader_ps_);

        obj->AddComponent<ComponentCollisionModel>()->AttachToModel();
    }

    // シャドウ
    {
        auto obj = Scene::CreateObject<Shadowmap>();
    }

    auto fil = Scene::CreateObject<Filter>();

    //----------------------------------------------------------------------------------
    // Player
    //----------------------------------------------------------------------------------
    auto player = Scene::CreateObject<Player>();
    player->SetName("Player")->SetTranslate({-357, -1, 182});
    player->SetSpeed(0.8f);

    auto arrow = Scene::CreateObject<Arrow>()->SetName("Arrow");

    Camera::Create(player)->SetName("PlayerCamera");

    // 1つ目のターゲットポイント
    auto targetpoint_1 = Scene::CreateObject<TargetPoint>();
    targetpoint_1->SetName("TargetPoint");
    // 左下階段上
    targetpoint_1->SetTranslate({-367, 21, -20});

    // 最初からいる敵
    for(int i = 0; i < 3; i++) {
        SetSpawn(Game::Normal, Spawn_pos[0]);
    }

    sky_model_ = MV1LoadModel("data/Game/SkyDome/dome11.mqoz");

    alpha_ = 0;

    is_goal_ = false;

    info_font_handle_       = LoadFontDataToHandle("data/Game/Font/InfoFont.dft");
    result_font_handle_     = LoadFontDataToHandle("data/Game/Font/ResultFont.dft");
    typewriter_font_handle_ = LoadFontDataToHandle("data/Game/Font/TypewriterFont.dft");

    //----------------------------------------------------------------------------------
    // Gun
    //----------------------------------------------------------------------------------
    auto gun = Scene::CreateObject<Gun>();

    return true;
}

// 更新処理
void Game::Update([[maybe_unused]] float delta)
{
    auto player = Scene::GetObjectPtr<Player>("Player");
    auto target = Scene::GetObjectPtr<TargetPoint>("TargetPoint");

    if(player->IsDead() == true) {
        alpha_ += 1;
        // 真っ暗になったらシーンチェンジ
        if(255 <= alpha_) {
            Scene::Change(Scene::GetScene<Title>());
        }
    }

    if(is_hit_target[0] == false) {
        // 修理中の敵スポーン
        if(target->GetStayTimer() >= 5.0f && target->GetStayTimer() <= 5.02f) {
            for(int i = 0; i < 2; i++) {
                SetSpawn(Game::Normal, Spawn_pos[1]);
            }
        }
        // 屋根上奇襲
        if(target->GetStayTimer() >= 10.0f && target->GetStayTimer() <= 10.03f) {
            SetSpawn(Game::Normal, Spawn_pos[2]);
        }
        if(target->GetStayTimer() >= 15.0f && target->GetStayTimer() <= 15.03f) {
            for(int i = 0; i < 2; i++) {
                SetSpawn(Game::Normal, Spawn_pos[1]);
            }
        }

        if(target->GetStayCreal() == true) {
            is_hit_target[0] = true;
            Scene::ReleaseObject(target);
            // 2つ目のターゲットポイントを作成
            auto target_obj = Scene::CreateObject<TargetPoint>();
            //Spawn();
            target_obj->SetName("TargetPoint");
            // 右上入って左奥
            target_obj->SetTranslate({490.0f, 26.0f, -40.0f});

            // 移動中に邪魔をする敵
            // 2か所から3体ずつ敵が出てくる
            for(int i = 0; i < 2; i++) {
                for(int j = 0; j < 3; j++) {
                    RandomSpawn(Spawn_pos[3 + i]);
                    RandomSpawn(Spawn_pos[5]);
                }
            }
        }
    }
    // 2つ目のターゲットポイントに当たったら
    else if(is_hit_target[0] == true && is_hit_target[1] == false) {
        if(target->GetStayTimer() >= 6.0f && target->GetStayTimer() <= 6.02f) {
            // 3か所から2体ずつ敵が出てくる
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 2; j++) {
                    RandomSpawn(Spawn_pos[6 + i]);
                }
            }
        }
        // 屋根上奇襲
        else if(target->GetStayTimer() >= 12.0f && target->GetStayTimer() <= 12.03f) {
            SetSpawn(Game::Normal, Spawn_pos[9]);
        }
        else if(target->GetStayTimer() >= 18.0f && target->GetStayTimer() <= 18.03f) {
            // 3か所から2体ずつ敵が出てくる
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 2; j++) {
                    RandomSpawn(Spawn_pos[6 + i]);
                }
            }
        }

        if(target->GetStayCreal() == true) {
            is_hit_target[1] = true;
            Scene::ReleaseObject(target);
            // 3つ目のターゲットポイントを作成
            auto target_obj = Scene::CreateObject<TargetPoint>();
            target_obj->SetName("TargetPoint");
            // 左上入って奥
            target_obj->SetTranslate({295, 26, 360});

            // 移動中に邪魔をする敵
            for(int i = 0; i < 3; i++) {
                RandomSpawn(Spawn_pos[10]);
            }
        }
    }
    // 3つ目のターゲットポイントに当たったら
    else if(is_hit_target[0] == true && is_hit_target[1] == true && is_hit_target[2] == false) {
        if(target->GetStayTimer() >= 6.0f && target->GetStayTimer() <= 6.03f) {
            // 3か所から2体ずつ敵が出てくる
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 2; j++) {
                    RandomSpawn(Spawn_pos[11 + i]);
                }
            }
        }
        else if(target->GetStayTimer() >= 12.0f && target->GetStayTimer() <= 12.03f) {
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 2; j++) {
                    RandomSpawn(Spawn_pos[11 + i]);
                }
            }
        }
        else if(target->GetStayTimer() >= 18.0f && target->GetStayTimer() <= 18.03f) {
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 2; j++) {
                    RandomSpawn(Spawn_pos[11 + i]);
                }
            }
        }

        if(target->GetStayCreal() == true) {
            Scene::ReleaseObject(target);
            Scene::ReleaseObject("Arrow");
            is_hit_target[2] = true;

            // 脱出を促す大量の敵
            for(int i = 0; i < 9; i++) {
                RandomSpawn(Spawn_pos[14]);
            }
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 4; j++) {
                    RandomSpawn(Spawn_pos[11 + i]);
                }
            }

            // ゴール地点を作成
            auto goal = Scene::CreateObject<Goal>()->SetName("Goal")->SetTranslate({422, 20, 190});
        }
    }

    // 目標地点3つに到達したらゴール地点を作成
    if(is_hit_target[0] == true && is_hit_target[1] == true && is_hit_target[2] == true && is_goal_ == false) {
        auto goal = Scene::GetObjectPtr<Goal>("Goal");
        if(goal->GetIsHit() == true) {
            is_goal_ = true;
            Scene::ReleaseObject(goal);
        }
    }

    // ゴールしたら
    if(is_goal_ == true) {
        player->SetIsClear(true);

        // enterを押すとゲーム画面へ
        if(IsKeyOn(KEY_INPUT_RETURN)) {
            Scene::Change(Scene::GetScene<Title>());
        }
    }
}

// 描画
void Game::PostDraw()
{
    // ここのライトを入れたままにするとちょっと暗くなったりする
    SetUseLighting(FALSE);
    //	空モデルの描画
    MV1DrawModel(sky_model_);

    // ライトありで描画
    SetUseLighting(TRUE);

    // 左上の灰色
    DrawFillBox(0, 0, 300, 40, GetColor(75, 75, 75));

    unsigned int color = 0;

    if(is_hit_target[0] == false) {
        color = RED;
    }
    if(is_hit_target[0] == true && is_hit_target[1] == false) {
        color = BLUE;
    }
    if(is_hit_target[0] == true && is_hit_target[1] == true && is_hit_target[2] == false) {
        color = GREEN;
    }

    if(is_hit_target[0] == true && is_hit_target[1] == true && is_hit_target[2] == true) {
        DrawStringToHandle(70, 10, "にげろ", RED, info_font_handle_);

        // クリア時文字表示
        if(is_goal_ == true) {
            DrawFillBox(0, 0, WINDOW_W, WINDOW_H, BLACK);

            DrawStringToHandle(WINDOW_W / 2 - 250, int(WINDOW_H / 3.5), "CLEAR", RED, result_font_handle_);

            DrawStringToHandle(50, WINDOW_H - 200, "Press enter return to title", WHITE, typewriter_font_handle_);
        }
    }
    else {
        DrawStringToHandle(50, 10, "目標地点を目指せ", color, info_font_handle_);
    }

    auto player = Scene::GetObjectPtr<Player>("Player");

    // 死亡時文字表示
    if(player->IsDead() == true) {
        // 徐々に暗く
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_);
        DrawFillBox(0, 0, WINDOW_W, WINDOW_H, BLACK);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        DrawStringToHandle(WINDOW_W / 2 - 350, WINDOW_H / 2 - 30, "YOU DEAD", RED, result_font_handle_);
    }
}

// 終了
void Game::Exit()
{
    DeleteFontToHandle(info_font_handle_);
    DeleteFontToHandle(result_font_handle_);
    DeleteFontToHandle(typewriter_font_handle_);
}

// GUI
void Game::GUI()
{
}

// ランダムで敵をスポーン　
void Game::RandomSpawn(float3 pos)
{
    int type = GetRand(3);
    if(type == 0) {
        auto enemy = Scene::CreateObject<NormalEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate(pos);
        enemy->SetSpeed(0.80f);
    }
    else if(type == 1) {
        auto enemy = Scene::CreateObject<ToughEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate(pos);
        enemy->SetSpeed(0.60f);
    }
    else if(type == 2) {
        auto enemy = Scene::CreateObject<FastEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate(pos);
        enemy->SetSpeed(1.2f);
    }
    else if(type == 3) {
        auto enemy = Scene::CreateObject<BombEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate(pos);
        enemy->SetSpeed(0.60f);
    }
}

// 敵をスポーン
void Game::SetSpawn(EnemyType enemytype, float3 pos)
{
    if(enemytype == Normal) {
        auto enemy = Scene::CreateObject<NormalEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate({pos.x + GetRand(10) - 5, pos.y, pos.z + GetRand(10) - 5});
        enemy->SetSpeed(0.80f);
    }
    else if(enemytype == Tough) {
        auto enemy = Scene::CreateObject<ToughEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate({pos.x + GetRand(30) - 15, pos.y, pos.z + GetRand(30) - 15});
        enemy->SetSpeed(0.60f);
    }
    else if(enemytype == Fast) {
        auto enemy = Scene::CreateObject<FastEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate({pos.x + GetRand(30) - 15, pos.y, pos.z + GetRand(30) - 15});
        enemy->SetSpeed(1.3f);
    }
    else if(enemytype == Bomb) {
        auto enemy = Scene::CreateObject<BombEnemy>();
        enemy->SetName("Enemy");
        enemy->SetTranslate({pos.x + GetRand(30) - 15, pos.y, pos.z + GetRand(30) - 15});
        enemy->SetSpeed(0.80f);
    }
}
