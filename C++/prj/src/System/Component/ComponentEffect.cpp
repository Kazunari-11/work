//---------------------------------------------------------------------------
//! @file   ComponentEffect.h
//! @brief  モデルコンポーネント
//---------------------------------------------------------------------------
#include <System/Scene.h>
#include <System/Component/ComponentEffect.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include <unordered_map>

BP_COMPONENT_IMPL(ComponentEffect, u8"Effectコンポーネント");

std::unordered_map<std::string, int> ComponentEffect::exist_effects_resource_{};
int                                  ComponentEffect::component_effect_count = 0;

//! @brief Effekseerエフェクトロード
//! @param path エフェクトファイル(.efkefc)
void ComponentEffect::Load(std::string_view path)
{
    effect_status_.off(EffectBit::ErrorFileNotFound);

    path_ = path;

    // ファイルが存在しているか?
    if(!HelperLib::File::CheckFileExistence(path_)) {
        // ロードできなかった
        effect_status_.on(EffectBit::ErrorFileNotFound);
        effect_status_.off(EffectBit::Initialized);
        return;
    }

    auto resource = exist_effects_resource_.find(path_);
    if(resource != exist_effects_resource_.end()) {
        // リソースが存在する場合
        effect_handle_ = resource->second;

        // 初期化済み
        effect_status_.on(EffectBit::Initialized);
        return;
    }

    int result = LoadEffekseerEffect(HelperLib::String::ToWString(path_).data());
    if(result == -1) {
        // ロードできなかった
        effect_status_.on(EffectBit::ErrorFileNotFound);
        return;
    }

    effect_handle_ = result;
    // リソースの登録
    exist_effects_resource_[path_] = effect_handle_;

    // 初期化済み、未スタートにしておく
    effect_status_.on(EffectBit::Initialized);
}

//! @brief モデル更新
//! @param delta 1フレームの秒数
void ComponentEffect::Update(float delta)
{
    // モデルが存在しているならばTransform設定を行う
    if(IsValid() && IsPlaying()) {
        // アニメーションがあり再生している?
        effect_time_ += (delta * effect_speed_);

        auto mat  = effect_transform_;
        auto trns = GetOwner()->GetComponent<ComponentTransform>();
        if(trns)
            mat = mul(mat, trns->GetMatrix());

        // Drawはここで抑えておく
        if(GetStatus(Component::StatusBit::NoDraw)) {
            //システムして「すべて描くモード」が使用されているため消すことができない。
            //よってスケールを0にして表示するようにして消したように見せかける
            SetScalePlayingEffekseer3DEffect(effect_play_handle_, 0, 0, 0);
            return;
        }

        float* matz = (float*)mat.f32_128_0;
        float  pos[3], rot[3], scale[3];
        DecomposeMatrixToComponents(matz, pos, rot, scale);

        SetPosPlayingEffekseer3DEffect(effect_play_handle_, pos[0], pos[1], pos[2]);
        SetScalePlayingEffekseer3DEffect(effect_play_handle_, scale[0], scale[1], scale[2]);
        SetRotationPlayingEffekseer3DEffect(effect_play_handle_, rot[0], rot[1], rot[2]);
    }
}

//! @brief モデル描画
void ComponentEffect::Draw()
{
    if(!effect_status_.is(EffectBit::Initialized))
        return;

    if(Scene::IsPause() || effect_status_.is(EffectBit::Paused)) {
        SetSpeedPlayingEffekseer3DEffect(effect_play_handle_, 0.0f);
    }
    else {
        SetSpeedPlayingEffekseer3DEffect(effect_play_handle_, effect_speed_);
    }

    bool playing = IsEffekseer3DEffectPlaying(effect_play_handle_) == 0 ? !effect_status_.is(EffectBit::Paused) : false;
    effect_status_.set(EffectBit::Playing, playing);
    if(!playing && effect_status_.is(EffectBit::Loop)) {
        Play(true);
    }
}

//! @brief 終了処理
void ComponentEffect::Exit()
{
    __super::Exit();
    // ここで解放必要なし
}

//! @brief GUI処理
void ComponentEffect::GUI()
{
    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();

        // モデルコンポーネント表示
        if(ImGui::TreeNode("Effect")) {
            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            // ロード完了チェックフラグ
            bool loaded = IsValid();

            ImGui::BeginDisabled(true);   // UI上の編集不可(ReadOnly)
            {
                if(loaded)
                    ImGui::Checkbox(u8"【LoadOK】", &loaded);
                else
                    ImGui::TextColored({1, 0, 0, 1}, u8"【LoadNG】");
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            // ファイル名
            char file_name[1024];
            sprintf_s(file_name, "%s", path_.c_str());
            if(ImGui::InputText(u8"File", file_name, 1024)) {
                path_ = file_name;
                effect_status_.off(EffectBit::Initialized);
                Load(path_);
            }
            ImGui::Separator();

            if(IsValid()) {
                ImGui::CheckboxFlags("Loop", &effect_status_.get(), 1 << (int)EffectBit::Loop);

                if(IsPaused()) {
                    if(ImGui::Button("Resume")) {
                        PlayPause(false);
                    }
                }
                else {
                    if(!IsPlaying()) {
                        if(ImGui::Button("Play")) {
                            Play();
                        }
                    }
                    else {
                        if(ImGui::Button("Pause")) {
                            PlayPause();
                        }
                    }
                }
                ImGui::Separator();
            }

            // アニメーション名
            if(IsPlaying()) {
                ImGui::TextColored({0.5, 1, 0.5, 1}, u8"再生中");
                ImGui::Text(u8"[%3.2f]%s", effect_time_, GetEffectName().data());
                ImGui::Separator();
            }

            // モデル姿勢
            if(ImGui::TreeNode(u8"エフェクト姿勢")) {
                ImGui::DragFloat4(u8"Ｘ軸", effect_transform_.f32_128_0, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::DragFloat4(u8"Ｙ軸", effect_transform_.f32_128_1, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::DragFloat4(u8"Ｚ軸", effect_transform_.f32_128_2, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::DragFloat4(u8"座標", effect_transform_.f32_128_3, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::Separator();
                ImGui::TreePop();
            }

            // 姿勢を TRSで変更できるように設定
            float* mat = effect_transform_.f32_128_0;
            float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
            DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3(u8"座標(T)", matrixTranslation, 0.01f, -100000.00f, 100000.0f, "%.2f");
            ImGui::DragFloat3(u8"回転(R)", matrixRotation, 0.1f, -360.0f, 360.0f, "%.2f");
            ImGui::DragFloat3(u8"サイズ(S)", matrixScale, 0.01f, 0.00f, 1000.0f, "%.2f");
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ComponentEffect::Play(bool loop)
{
    // 前のエフェクトは止める
    StopEffekseer3DEffect(effect_play_handle_);

    effect_status_.set(EffectBit::Loop, loop);

    effect_play_handle_ = PlayEffekseer3DEffect(effect_handle_);
}

void ComponentEffect::SetPlaySpeed(float speed)
{
    effect_speed_ = speed;
    SetSpeedPlayingEffekseer3DEffect(effect_play_handle_, effect_speed_);
}

float ComponentEffect::GetPlaySpeed()
{
    return effect_speed_;
}

bool ComponentEffect::IsPlaying()
{
    // まだ初期化できてない場合はfalse
    if(!IsValid())
        return false;

    return effect_status_.is(EffectBit::Playing);
}

void ComponentEffect::PlayPause(bool active)
{
    effect_status_.set(EffectBit::Paused, active);
    if(active) {
        // ポーズはスピード0.0で代用
        SetSpeedPlayingEffekseer3DEffect(effect_play_handle_, 0.0f);
    }
    else {
        // 通常スピードに戻す
        SetSpeedPlayingEffekseer3DEffect(effect_play_handle_, effect_speed_);
    }
}

bool ComponentEffect::IsPaused()
{
    if(effect_status_.is(EffectBit::Paused))
        return true;

    return false;
}

bool ComponentEffect::IsValid()
{
    return effect_status_.is(EffectBit::Initialized);
}

const std::string_view ComponentEffect::GetEffectName()
{
    return HelperLib::File::GetOnlyFileNameWithoutExtension(path_);
}

const float ComponentEffect::GetEffectTime()
{
    if(IsPlaying())
        return effect_time_;

    return 0.0f;
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置

const matrix ComponentEffect::GetWorldMatrix() const
{
    return mul(GetMatrix(), GetOwner()->GetWorldMatrix());
}

//! @brief 1フレーム前のワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置

const matrix ComponentEffect::GetOldWorldMatrix() const
{
    return mul(GetMatrix(), GetOwner()->GetOldWorldMatrix());
}
