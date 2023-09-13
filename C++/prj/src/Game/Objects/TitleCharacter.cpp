//---------------------------------------------------------------------------
//! @file   TitleCharacter.cpp
//! @brief  タイトルキャラクター
//---------------------------------------------------------------------------
#include "Precompile.h"
#include <Game/Objects/TitleCharacter.h>
#include <System/Component/ComponentModel.h>
#include <Game/Scenes/Tutorial.h>

TitleCharacter ::TitleCharacter()
{
}

TitleCharacter ::~TitleCharacter()
{
}

// 初期化
bool TitleCharacter ::Init()
{
    __super::Init();

    // モデルコンポーネント(0.08倍)
    AddComponent<ComponentModel>("data/Game/NormalEnemy/model.mv1")
        ->SetScaleAxisXYZ({
            0.08f
    })
        ->SetAnimation({
            // アニメーション
            {"run", "data/Game/NormalEnemy/Anim/Run.mv1", 0, 1.0f},
            {"death", "data/Game/NormalEnemy/Anim/Death.mv1", 0, 1.0f},
        });

    eye_pos_ = {0.0f, 56.0f, -2.0f};
    look_at_ = {0.0f, 20.0f, 211.0f};

    Scene::CreateObject<Object>()->AddComponent<ComponentCamera>()->SetPositionAndTarget(eye_pos_, look_at_);
    shot_se_      = LoadSoundMem("data/Game/SE/Player/shot.mp3");
    death_se_     = LoadSoundMem("data/Game/SE/Enemy/death.mp3");
    cartridge_se_ = LoadSoundMem("data/Game/SE/Player/cartridge.mp3");

    return true;
}

// 更新処理
void TitleCharacter ::Update([[maybe_unused]] float delta)
{
    if(240 <= alpha_) {
        alpha_ -= 0.3f;
    }
    auto mdl = GetComponent<ComponentModel>();

    // キーが押されるまで
    if(key_flag == false) {
        if(mdl->GetPlayAnimationName() != "run") {
            mdl->PlayAnimation("run", true);
        }

        // マウス左ボタンを押したら
        if(IsMouseOn(MOUSE_INPUT_LEFT)) {
            PlaySE(shot_se_, 150);

            se_count_ = 0;
            key_flag  = true;
        }
    }
    else {
        se_count_++;

        // 死亡SEを鳴らす
        if(se_count_ == 30) {
            PlaySE(death_se_, 60);
        }
        // 薬莢SEを鳴らす
        if(se_count_ == 180) {
            PlaySE(cartridge_se_);
        }

        // アニメーションが終わったらだんだん暗くしていく
        if(mdl->IsPlaying() == false) {
            alpha_ += 0.8f;
            // 真っ暗になったらシーンチェンジ
            if(255 <= alpha_) {
                Scene::Change(Scene::GetScene<Tutorial>());
            }
            return;
        }
        if(mdl->GetPlayAnimationName() != "death") {
            mdl->PlayAnimation("death");
        }
    }
}

// 描画処理
void TitleCharacter ::Draw()
{
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)alpha_);
    DrawFillBox(0, 0, WINDOW_W, WINDOW_H, BLACK);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// ボリュームを設定しSEを再生
void TitleCharacter::PlaySE(int se_handle, int volume) const
{
    ChangeVolumeSoundMem(volume, se_handle);
    PlaySoundMem(se_handle, DX_PLAYTYPE_BACK);
}
