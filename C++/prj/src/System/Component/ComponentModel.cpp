//---------------------------------------------------------------------------
//! @file   ComponentModel.h
//! @brief  モデルコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>
#include "Game/Shadowmap.h"

BP_COMPONENT_IMPL(ComponentModel, u8"Model機能クラス");

//! @brief モデルロード
//! @param path ロードするモデル(.MV1/.MQO/.Xなど)
void ComponentModel::Load(std::string_view path)
{
    model_status_.off(ModelBit::ErrorFileNotFound);
    model_status_.on(ModelBit::SetFileAlready);

    path_ = path;

    // ファイルが存在しているか?
    if(!HelperLib::File::CheckFileExistence(path_)) {
        // ロードできなかった
        model_status_.on(ModelBit::ErrorFileNotFound);
        model_status_.off(ModelBit::Initialized);
        nodes_name_.clear();
        return;
    }

    bool result = model_->load(path_);
    if(!result) {
        // ロードできなかった
        model_status_.on(ModelBit::ErrorFileNotFound);
    }
    else {
        // ロード済み
        model_status_.on(ModelBit::Initialized);
    }

    // ノード名を初期化しておく
    nodes_name_.clear();
}

//! @brief モデル更新
//! @param delta 1フレームの秒数
void ComponentModel::Update(float delta)
{
    // モデルが存在しているならばTransform設定を行う
    if(IsValid()) {
        // アニメーションがあり再生している?
        if(animation_ && animation_->isValid() && animation_->isPlaying()) {
            animation_->update(delta);
            animation_time_ += delta;
        }

        auto mat  = model_transform_;
        auto trns = GetOwner()->GetComponent<ComponentTransform>();
        if(trns) {
            mat = mul(mat, trns->GetMatrix());
        }

        if(model_) {
            // ワールド行列を設定
            model_->setWorldMatrix(mat);

            // モデル更新
            model_->update(delta);
        }
    }
}

//! @brief モデル描画
void ComponentModel::Draw()
{
    if(!model_status_.is(ModelBit::Initialized))
        return;

    // Drawはここで抑えておく
    if(GetStatus(Component::StatusBit::NoDraw))
        return;

    if(model_ == nullptr)
        return;

    auto drawShadow = [&]() {
        // ワールド行列を設定(コリジョン移動分)
        model_->setWorldMatrix(GetWorldMatrix());

        // シェーダーを利用するかどうかを設定
        model_->useShader(UseShader());

        // モデル描画
        model_->render(nullptr, overrided_shader_ps_.get());
    };

    auto drawScene = [&]() {
        // ワールド行列を設定(コリジョン移動分)
        model_->setWorldMatrix(GetWorldMatrix());

        // シェーダーを利用するかどうかを設定
        model_->useShader(UseShader());

        // テクスチャとしてシャドウカラーテクスチャを設定
        SetTexture(11, Shadowmap::shadowTexture());

        // モデル描画
        model_->render(nullptr, overrided_shader_ps_.get());

        SetTexture(11, nullptr);
    };

    // 描画登録
    SetProc("Model::Shadow", drawShadow, ProcTiming::Shadow, Priority::NORMAL);
    SetProc("Model::HDR", drawScene, ProcTiming::HDR, Priority::NORMAL);
}

//! @brief 終了処理
void ComponentModel::Exit()
{
    __super::Exit();
    // 解放必要なし
}

//! @brief GUI処理
void ComponentModel::GUI()
{
    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();

        if(!ImGui::IsWindowFocused())
            node_manipulate_ = false;

        // モデルコンポーネント表示
        if(ImGui::TreeNode("Model")) {
            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            // シェーダー利用設定
            bool shader = UseShader();
            if(ImGui::Checkbox(u8"UseShader", &shader)) {
                model_status_.set(ModelBit::UseShader, shader);
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

            // モデルファイル名
            char file_name[1024];
            sprintf_s(file_name, "%s", path_.c_str());
            if(ImGui::InputText(u8"File", file_name, 1024)) {
                path_ = file_name;
                model_status_.off(ModelBit::Initialized);
                Load(path_);
            }
            ImGui::Separator();

            auto GUIAnimation = [this]() {
                constexpr int item_count = 4;
                auto flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable |   //
                             ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_ScrollY;

                bool change = false;

                if(animations_desc_.empty()) {
                    if(ImGui::Button(u8"アニメーション追加"))
                        animations_desc_.push_back({});
                }
                else {
                    if(ImGui::TreeNode(u8"アニメーション")) {
                        if(ImGui::BeginTable("table1", item_count, flags)) {
                            for(int row = 0; row < animations_desc_.size();) {
                                auto& desc = animations_desc_[row];
                                ImGui::TableNextRow();

                                {
                                    ImGui::SetNextItemWidth(30);
                                    ImGui::TableSetColumnIndex(0);
                                    std::string id = std::string(u8"削除##anm_row") + std::to_string(row);
                                    if(ImGui::Button(id.c_str())) {
                                        animations_desc_.erase(animations_desc_.begin() + row);
                                        continue;
                                    }
                                    ImGui::SameLine();

                                    ImGui::SetNextItemWidth(100);
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::PushID(row * item_count + 0);   // assign unique id
                                    change |=
                                        ImGui::InputText("##v", &desc.name_, ImGuiInputTextFlags_EnterReturnsTrue);
                                    ImGui::PopID();

                                    ImGui::SetNextItemWidth(500);
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::PushID(row * item_count + 1);   // assign unique id
                                    change |= ImGui::InputText("##v", &desc.file_path_);
                                    ImGui::PopID();

                                    ImGui::SetNextItemWidth(80);
                                    ImGui::TableSetColumnIndex(2);
                                    ImGui::PushID(row * item_count + 2);   // assign unique id
                                    change |= ImGui::InputInt("##v", (int*)&desc.animation_index_);
                                    ImGui::PopID();

                                    ImGui::SetNextItemWidth(80);
                                    ImGui::TableSetColumnIndex(3);
                                    ImGui::PushID(row * item_count + 3);   // assign unique id
                                    change |= ImGui::InputFloat("##v", &desc.animation_speed_);
                                    ImGui::PopID();
                                }
                                row++;
                            }
                            ImGui::EndTable();
                        }
                        if(ImGui::Button(u8"アニメーション追加"))
                            animations_desc_.push_back({});

                        ImGui::TreePop();
                    }
                }

                // 再登録
                if(change) {
                    SetAnimation(animations_desc_);
                }
            };

            GUIAnimation();

            // アニメーション名
            if(IsPlaying()) {
                ImGui::TextColored({0.5, 1, 0.5, 1}, u8"アニメーション再生中");
                ImGui::Text(u8"[%3.2f]%s", animation_time_, GetPlayAnimationName().data());
                ImGui::Separator();
            }

            // モデル姿勢
            if(ImGui::TreeNode(u8"モデル姿勢")) {
                ImGui::DragFloat4(u8"Ｘ軸", model_transform_.f32_128_0, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::DragFloat4(u8"Ｙ軸", model_transform_.f32_128_1, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::DragFloat4(u8"Ｚ軸", model_transform_.f32_128_2, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::DragFloat4(u8"座標", model_transform_.f32_128_3, 0.01f, -10000.0f, 10000.0f, "%.2f");
                ImGui::Separator();
                ImGui::TreePop();
            }

            // 姿勢を TRSで変更できるように設定
            float* mat = model_transform_.f32_128_0;
            float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
            DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
            ImGui::DragFloat3(u8"座標(T)", matrixTranslation, 0.01f, -100000.00f, 100000.0f, "%.2f");
            ImGui::DragFloat3(u8"回転(R)", matrixRotation, 0.1f, -360.0f, 360.0f, "%.2f");
            ImGui::DragFloat3(u8"サイズ(S)", matrixScale, 0.01f, 0.00f, 1000.0f, "%.2f");
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

            if(ImGui::TreeNode("Model:Nodes")) {
                node_manipulate_ = false;
                auto list        = GetNodesNamePChar();
                ImGui::ListBox(u8"ノード名", &select_node_index_, list.data(), (int)list.size(), 10);
                if(select_node_index_ < list.size())
                    node_manipulate_ = true;

                ImGui::TreePop();
            }
            else {
                node_manipulate_ = false;
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();

    if(node_manipulate_) {
        matrix matx = MV1GetFrameLocalWorldMatrix(GetModel(), select_node_index_);
        auto   trns = GetOwner()->GetComponent<ComponentTransform>();
        //matx		= mul( GetMatrix(), matx );
        //matx		= mul( trns->GetMatrix(), matx );

        float* mat_float = (float*)matx.f32_128_0;
        ShowGizmo(mat_float, gizmo_operation_, gizmo_mode_, reinterpret_cast<uint64_t>(this));
#if 0
		// Gizmoを表示するためのMatrixをDxLibから取得
		auto camera_view = GetCameraViewMatrix();
		auto camera_proj = GetCameraProjectionMatrix();

		// Gizmoの表示を設定する
		ImGuizmo::SetOrthographic( false );
		ImGuizmo::SetDrawlist( ImGui::GetBackgroundDrawList() );

		// 画面サイズを取得する
		RECT rect;
		RECT crect;
		GetWindowRect( GetMainWindowHandle(), &rect );
		GetClientRect( GetMainWindowHandle(), &crect );

		// Gizmoを画面に合わせて処理する
		float windowWidth  = (float)rect.right - rect.left;
		float windowHeight = (float)rect.bottom - rect.top;
		float windowBar	   = windowHeight - crect.bottom;
		ImGuizmo::SetRect( (float)rect.left,
						   (float)rect.top + windowBar / 2,
						   windowWidth,
						   windowHeight - windowBar / 2 );

		// Manipulateを表示する
		static bool	 useSnap		 = false;
		static float snap[ 3 ]		 = { 1.0f, 1.0f, 1.0f };
		static float bounds[]		 = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
		static float boundsSnap[]	 = { 0.1f, 0.1f, 0.1f };
		static bool	 boundSizing	 = true;
		static bool	 boundSizingSnap = false;
		ImGuizmo::AllowAxisFlip( false );	 //< これがないとGizmoが反転してしまう

		{
			ImGuizmo::SetID( 10 );
			ImGuizmo::Manipulate( (const float*)&camera_view,
								  (const float*)&camera_proj,
								  gizmo_operation_,
								  gizmo_mode_,
								  mat_float,
								  NULL,
								  false ? &snap[ 0 ] : NULL,
								  boundSizing ? bounds : NULL,
								  boundSizingSnap ? boundsSnap : NULL );
		}
#endif
    }
}

ComponentModelPtr ComponentModel::SetAnimation(const std::vector<Animation::Desc> anims)
{
    if(model_) {
        // エディター用アニメーション設定用
        animations_desc_ = anims;

        // アニメーションクラスの作成
        animation_ = std::make_shared<Animation>(anims.data(), anims.size());

        //  モデルにアニメーションを設定
        model_->bindAnimation(animation_.get());
    }

    return std::dynamic_pointer_cast<ComponentModel>(shared_from_this());
}

void ComponentModel::PlayAnimationNoSame(std::string_view name, bool loop, float blend_time, float start_time)
{
    if(current_animation_name_ == name)
        return;

    PlayAnimation(name, loop, blend_time, start_time);
}

void ComponentModel::PlayAnimation(std::string_view name, bool loop, float blend_time, float start_time)
{
    if(animation_) {
        animation_->play(name, loop, blend_time, start_time);
        current_animation_name_ = name;
        animation_time_         = 0.0f;
    }
}

bool ComponentModel::IsPlaying()
{
    if(animation_)
        return animation_->isPlaying();

    return false;
}

void ComponentModel::PlayPause(bool active)
{
    if(animation_)
        animation_->pause(active);
}

bool ComponentModel::IsPaused()
{
    if(animation_)
        return animation_->isPaused();

    return false;
}

bool ComponentModel::IsAnimationValid()
{
    if(animation_)
        return animation_->isValid();

    return false;
}

const std::string_view ComponentModel::GetPlayAnimationName()
{
    if(IsPlaying()) {
        return current_animation_name_;
    }

    // 何も再生されていない
    return "";
}

const float ComponentModel::GetAnimationTime()
{
    if(IsPlaying())
        return animation_time_;

    return 0.0f;
}

std::vector<std::string_view> ComponentModel::GetNodesName()
{
    //std::vector<std::string_view> view{};
    if(nodes_name_.empty()) {
        int num = MV1GetFrameNum(GetModel());
        if(num <= 0)
            return nodes_name_;

        nodes_name_.reserve(num);
        //view.reserve( num );
        for(int i = 0; i < num; i++) {
            nodes_name_.emplace_back(MV1GetFrameName(GetModel(), i));
            //view.emplace_back( nodes_name_.back() );
        }
    }

    //return view;
    return nodes_name_;
}

std::vector<const char*> ComponentModel::GetNodesNamePChar()
{
    std::vector<const char*> listbox;
    auto                     names = GetNodesName();

    listbox.reserve(names.size());

    for(auto& name : names) {
        listbox.emplace_back(name.data());
    }

    return listbox;
}

int ComponentModel::GetNodeIndex(std::string_view name)
{
    auto names = GetNodesName();
    for(int i = 0; i < names.size(); i++) {
        if(names[i] == name) {
            return i;
        }
    }
    // 存在せず
    return -1;
}

float3 ComponentModel::GetNodePosition(std::string_view name)
{
    int index = GetNodeIndex(name);
    if(index >= 0) {
        return GetNodePosition(index);
    }
    // 存在せず(初期位置)
    return {0, 0, 0};
}

float3 ComponentModel::GetNodePosition(int no)
{
    auto mat = MV1GetFrameLocalWorldMatrix(GetModel(), no);
    return {mat.m[3][0], mat.m[3][1], mat.m[3][2]};
}

matrix ComponentModel::GetNodeMatrix(std::string_view name)
{
    int index = GetNodeIndex(name);
    if(index >= 0) {
        return GetNodeMatrix(index);
    }
    // 存在せず(初期位置)
    return matrix::identity();
}

matrix ComponentModel::GetNodeMatrix(int no)
{
    return MV1GetFrameLocalWorldMatrix(GetModel(), no);
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置

const matrix ComponentModel::GetWorldMatrix() const
{
    return mul(GetMatrix(), GetOwner()->GetWorldMatrix());
}

//! @brief 1フレーム前のワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置

const matrix ComponentModel::GetOldWorldMatrix() const
{
    return mul(GetMatrix(), GetOwner()->GetOldWorldMatrix());
}
