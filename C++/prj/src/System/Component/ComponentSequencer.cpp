#include <System/Component/ComponentSequencer.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentEffect.h>
#include <System/Object.h>
#include <System/Scene.h>
#include <System/ImGui.h>

// CPPもここで加えておく
#include "../imgui/misc/cpp/imgui_stdlib.cpp"

#include <numeric>
#include <algorithm>

void SequenceObject::SetAnimationFromFrame(int frame)
{
    animation_name_  = "";
    animation_loop_  = false;
    animation_frame_ = 0;
    for(int i = static_cast<int>(animation_keys_.size()) - 1; i >= 0; --i) {
        int key = animation_keys_[i];
        if(key <= frame) {
            float past = static_cast<float>(frame - key);

            if(animation_name_ != animation_values_[i].name_)
                animation_change_ = true;

            animation_name_  = animation_values_[i].name_;
            animation_frame_ = static_cast<int>(past);
            animation_loop_  = animation_values_[i].loop_;
            break;
        }
    }
}

void SequenceObject::SetEffectFromFrame(int frame)
{
    effect_loop_  = false;
    effect_frame_ = 0;
    for(int i = static_cast<int>(effect_keys_.size()) - 1; i >= 0; --i) {
        int key = effect_keys_[i];
        if(key <= frame) {
            float past = static_cast<float>(frame - key);

            if(effect_number_ != i)
                effect_change_ = true;

            effect_number_ = i;
            effect_frame_  = static_cast<int>(past);
            effect_loop_   = effect_values_[i].loop_;
            break;
        }
        if(i == 0 && static_cast<int>(effect_keys_[i]) > frame) {
            effect_number_ = -1;
            break;
        }
    }
}

matrix SequenceObject::GetTransformFromFrame(int frame)
{
    auto calc_value = [](int                   frame,
                         std::vector<uint32_t> keys,
                         std::vector<float3>   values,
                         float3                default_value = {0, 0, 0}) -> float3 {
        int    old   = -1;
        float3 value = default_value;
        for(int i = 0; i < keys.size(); ++i) {
            int key = static_cast<int>(keys[i]);
            if(key >= frame) {
                float base = static_cast<float>(key - old);
                float par  = static_cast<float>(frame - old) / base;

                value = value * (1.0f - par) + values[i] * par;
                break;
            }
            else {
                old   = key;
                value = values[i];
            }
        }

        return value;
    };

    float3 position = calc_value(frame, position_keys_, position_values_);
    float3 rotation = calc_value(frame, rotation_keys_, rotation_values_);
    float3 scale    = calc_value(frame, scale_keys_, scale_values_, {1, 1, 1});

    matrix mat;
    ImGuizmo::RecomposeMatrixFromComponents((float*)&position, (float*)&rotation, (float*)&scale, (float*)&mat);
    return mat;
}

void SequenceObject::ShowGuizmo()
{
    if(show_guizmo_) {
        if(ShowGizmo(object_matrix_.f32_128_0, guizmo_operation_, guizmo_mode_, reinterpret_cast<uint64_t>(this))) {
            float trns[3];
            float rot[3];
            float scale[3];
            DecomposeMatrixToComponents(object_matrix_.f32_128_0, trns, rot, scale);

            switch(guizmo_operation_) {
            case ImGuizmo::OPERATION::TRANSLATE:
                position_values_[guizmo_index_] = float3(trns[0], trns[1], trns[2]);
                break;
            case ImGuizmo::OPERATION::ROTATE:
                rotation_values_[guizmo_index_] = float3(rot[0], rot[1], rot[2]);
                break;
            case ImGuizmo::OPERATION::SCALE:
                scale_values_[guizmo_index_] = float3(scale[0], scale[1], scale[2]);
                break;
            default:
                break;
            }
        }
        show_guizmo_ = false;
    }
}

namespace
{
template <class T>
void SortVectors(std::vector<uint32_t>& v1, std::vector<T>& v2)
{
    size_t                n = v1.size();
    std::vector<uint32_t> p(n), v1x(n);
    std::vector<T>        v2x(n);
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(), [&](int a, int b) { return v1[a] < v1[b]; });
    for(int i = 0; i < n; i++) {
        v1x[i] = v1[p[i]];
        v2x[i] = v2[p[i]];
    }
    v1 = v1x;
    v2 = v2x;
}
}   // namespace

void SequenceObject::Sort()
{
    SortVectors<float3>(position_keys_, position_values_);
    SortVectors<float3>(rotation_keys_, rotation_values_);
    SortVectors<float3>(scale_keys_, scale_values_);
}

void SequenceObject::Update(bool playing, float frame)
{
    if(playing) {
        int i_frame = static_cast<int>(frame);
        SetAnimationFromFrame(i_frame);
        SetEffectFromFrame(i_frame);

        object_matrix_ = GetTransformFromFrame(i_frame);
        if(auto obj = Scene::GetObjectPtr<Object>(name_)) {
            obj->SetMatrix(object_matrix_);
            if(auto component = obj->GetComponent<ComponentModel>()) {
                component->PlayAnimationNoSame(animation_name_, animation_loop_);
                if(fabs(frame - animation_frame_old_) > 1.0f && !component->IsPlaying())
                    component->PlayAnimation(animation_name_, false, 0.0f, animation_frame_old_ / 60.0f);

                //component->Update( ( frame - animation_frame_old_ ) / 60.0f );
                animation_frame_old_ = i_frame;
            }

            if(auto component = obj->GetComponent<ComponentEffect>()) {
                if(effect_change_) {
                    component->Play(effect_loop_);
                    effect_change_ = false;
                }
#if 0
				if( fabs( frame - effect_frame_old_ ) > 1.0f && !component->IsPlaying() )
					component->PlayPause( true );
#endif
                component->Update((frame - effect_frame_old_) / 60.0f);
                effect_frame_old_ = i_frame;
            }
        }
    }

    ShowGuizmo();
    Sort();
}

void SequenceObject::GUI(uint32_t start, uint32_t end)
{
    if(ImGui::BeginNeoGroup(GetName().data(), &open_)) {
        ImGui::Dummy({0, 8});
        ImGui::Separator();

        ImGui::InputText(GetNameLabel().data(), &name_);

        ImGui::Spacing();

        std::string pos_name = "Position##pos" + std::to_string(reinterpret_cast<uint64_t>(this));
        if(ImGui::BeginNeoTimeline<float3>(pos_name.data(),
                                           position_keys_,
                                           position_values_,
                                           nullptr,
                                           ImGuiNeoSequencerFlags_AllowLengthChanging)) {
            int index = ImGui::GetNeoKeySelected();
            if(index >= 0 && position_keys_.size() > index) {
                guizmo_index_           = index;
                std::string frame_table = "Frame##frame-pos" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::DragScalar(frame_table.data(), ImGuiDataType_U32, &position_keys_[index], 1, &start, &end);

                if(ImGui::DragFloat3("Position", (float*)&position_values_[index], 0.1f)) {
                    SetTranslate(position_values_[index]);
                }

                guizmo_operation_ = ImGuizmo::OPERATION::TRANSLATE;
                guizmo_mode_      = ImGuizmo::MODE::WORLD;
                show_guizmo_      = true;
                object_matrix_    = GetTransformFromFrame(position_keys_[index]);
            }
            ImGui::EndNeoTimeLine();
        }

        std::string rot_name = "Rotation##rot" + std::to_string(reinterpret_cast<uint64_t>(this));
        if(ImGui::BeginNeoTimeline<float3>(rot_name.data(),
                                           rotation_keys_,
                                           rotation_values_,
                                           nullptr,
                                           ImGuiNeoSequencerFlags_AllowLengthChanging)) {
            int index = ImGui::GetNeoKeySelected();
            if(index >= 0 && rotation_keys_.size() > index) {
                guizmo_index_           = index;
                std::string frame_table = "Frame##frame-rot" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::DragScalar(frame_table.data(), ImGuiDataType_U32, &rotation_keys_[index], 1, &start, &end);

                if(ImGui::DragFloat3("Rotation", (float*)&rotation_values_[index], 0.1f)) {
                    SetRotationAxisXYZ(rotation_values_[index]);
                }

                guizmo_operation_ = ImGuizmo::OPERATION::ROTATE;
                guizmo_mode_      = ImGuizmo::MODE::LOCAL;
                show_guizmo_      = true;
                object_matrix_    = GetTransformFromFrame(rotation_keys_[index]);
            }
            ImGui::EndNeoTimeLine();
        }

        std::string scale_name = "Scale##scale" + std::to_string(reinterpret_cast<uint64_t>(this));
        if(ImGui::BeginNeoTimeline<float3>(scale_name.data(),
                                           scale_keys_,
                                           scale_values_,
                                           nullptr,
                                           ImGuiNeoSequencerFlags_AllowLengthChanging)) {
            int index = ImGui::GetNeoKeySelected();
            if(index >= 0 && scale_keys_.size() > index) {
                guizmo_index_           = index;
                std::string frame_table = "Frame##frame-scale" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::DragScalar(frame_table.data(), ImGuiDataType_U32, &scale_keys_[index], 1, &start, &end);

                if(ImGui::DragFloat3("Scale", (float*)&scale_values_[index], 0.1f)) {
                    SetScaleAxisXYZ(scale_values_[index]);
                }

                guizmo_operation_ = ImGuizmo::OPERATION::SCALE;
                guizmo_mode_      = ImGuizmo::MODE::LOCAL;
                show_guizmo_      = true;
                object_matrix_    = GetTransformFromFrame(scale_keys_[index]);
            }
            ImGui::EndNeoTimeLine();
        }

        std::string anm_name = "Animation##anm" + std::to_string(reinterpret_cast<uint64_t>(this));
        if(ImGui::BeginNeoTimeline<AnimObject>(anm_name.data(),
                                               animation_keys_,
                                               animation_values_,
                                               nullptr,
                                               ImGuiNeoSequencerFlags_AllowLengthChanging)) {
            int index = ImGui::GetNeoKeySelected();
            if(index >= 0 && animation_keys_.size() > index) {
                guizmo_index_          = index;
                std::string frame_name = "Frame##frame-anm" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::DragScalar(frame_name.data(), ImGuiDataType_U32, &animation_keys_[index], 1, &start, &end);
                std::string check_name = "Loop##checkbox-anm" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::Checkbox(check_name.data(), &animation_values_[index].loop_);
                ImGui::InputText("Animation", &animation_values_[index].name_);

                show_guizmo_ = false;
            }
            ImGui::EndNeoTimeLine();
        }

        std::string eff_name = "Effect##effect" + std::to_string(reinterpret_cast<uint64_t>(this));
        if(ImGui::BeginNeoTimeline<AnimObject>(eff_name.data(),
                                               effect_keys_,
                                               effect_values_,
                                               nullptr,
                                               ImGuiNeoSequencerFlags_AllowLengthChanging)) {
            int index = ImGui::GetNeoKeySelected();
            if(index >= 0 && effect_keys_.size() > index) {
                guizmo_index_          = index;
                std::string frame_name = "Frame##frame-eff" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::DragScalar(frame_name.data(), ImGuiDataType_U32, &effect_keys_[index], 1, &start, &end);
                std::string check_name = "Loop##checkbox-eff" + std::to_string(reinterpret_cast<uint64_t>(this));
                ImGui::Checkbox(check_name.data(), &effect_values_[index].loop_);
                //ImGui::InputText( "Effect", &effect_values_[ index ].name_ );

                show_guizmo_ = false;
            }
            ImGui::EndNeoTimeLine();
        }

        ImGui::EndNeoGroup();
    }
}

BP_COMPONENT_IMPL(ComponentSequencer, u8"Sequencer機能クラス");

void ComponentSequencer::Init()
{
    __super::Init();
}

void ComponentSequencer::Update(float delta_seconds)
{
    __super::Update(delta_seconds);

    bool step = false;

    ImGui::Begin("Sequencer##__SequencerID__");
    {
        uint32_t frame = static_cast<int>(current_frame_);
        if(ImGui::BeginNeoSequencer("Sequencer",
                                    &frame,
                                    &start_frame_,
                                    &end_frame_,
                                    {0, 0},
                                    ImGuiNeoSequencerFlags_AllowLengthChanging)) {
            if(frame != static_cast<uint32_t>(current_frame_)) {
                current_frame_ = (float)frame;
                step           = true;
            }
            if(!GetSequencerStatus(SequencerBit::Playing)) {
                if(ImGui::Button("Play")) {
                    Play();
                }
            }
            else {
                // プレイ中
                if(ImGui::Button("Stop")) {
                    Stop();
                }
            }

            if(ImGui::Button(u8"オブジェクト追加")) {
                objects_.push_back(std::make_shared<SequenceObject>());
            }

            for(int i = (int)objects_.size() - 1; i >= 0; --i) {
                auto& seq_obj = objects_[i];
                if(ImGui::IsSelectedNeoGroup(seq_obj->GetName().data())) {
                    ImGui::SameLine();
                    if(ImGui::Button(u8"オブジェクト削除")) {
                        objects_.erase(objects_.begin() + i);
                    }
                }
            }
#if 0
			std::string start_frame_name = "StartFrame##startframe" + std::to_string( reinterpret_cast<uint64_t>( this ) );
			ImGui::DragScalar( start_frame_name.data(), ImGuiDataType_U32, &SequenceObject::start_frame, 1, 0, &SequenceObject::end_frame );
			std::string end_frame_name = "EndFrame##endframe" + std::to_string( reinterpret_cast<uint64_t>( this ) );
			ImGui::DragScalar( end_frame_name.data(), ImGuiDataType_U32, &SequenceObject::end_frame, 1, &SequenceObject::start_frame );
#endif
            for(auto& seq_obj : objects_)
                seq_obj->GUI(start_frame_, end_frame_);

            ImGui::EndNeoSequencer();
        }
    }
    ImGui::End();

    // プレイ中
    if(GetSequencerStatus(SequencerBit::Playing)) {
        current_frame_ += delta_seconds / (1.0f / 60.0f);
        if(current_frame_ >= (float)end_frame_) {
            if(GetSequencerStatus(SequencerBit::Loop)) {
                current_frame_ = 0;
            }
            else {
                SetSequencerStatus(SequencerBit::Playing, false);
            }
        }
    }

    for(auto& seq_obj : objects_) {
        seq_obj->Update(GetSequencerStatus(SequencerBit::Playing) || step, current_frame_);
    }
}

void ComponentSequencer::PostUpdate()
{
}

//---------------------------------------------------------
//! デバッグ表示
//---------------------------------------------------------
void ComponentSequencer::Draw()
{
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentSequencer::Exit()
{
    __super::Exit();
}

//---------------------------------------------------------
//! GUI処理
//---------------------------------------------------------
void ComponentSequencer::GUI()
{
}

void ComponentSequencer::Play(int start_frame, bool loop)
{
    SetSequencerStatus(SequencerBit::Playing, true);
    SetSequencerStatus(SequencerBit::Loop, loop);
    current_frame_ = (float)start_frame;
}

void ComponentSequencer::Stop()
{
    SetSequencerStatus(SequencerBit::Playing, false);
}

bool ComponentSequencer::IsPlaying() const
{
    return GetSequencerStatus(SequencerBit::Playing);
}
