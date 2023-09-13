//---------------------------------------------------------------------------
//! @file   ComponentTransform.cpp
//! @brief  トランスフォームコンポーネント(座標と姿勢と大きさ)
//---------------------------------------------------------------------------
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>

#include <ImGuizmo/ImGuizmo.h>

BP_COMPONENT_IMPL(ComponentTransform, u8"Transform機能クラス");

Component* ComponentTransform::select_component_ = nullptr;

bool ShowGizmo(float* matrix, ImGuizmo::OPERATION ope, ImGuizmo::MODE mode, uint64_t id)
{
    // Gizmoを表示するためのMatrixをDxLibから取得
    auto camera_view = GetCameraViewMatrix();
    auto camera_proj = GetCameraProjectionMatrix();

    // Gizmoの表示を設定する
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

    // 画面サイズを取得する
    RECT rect;
    RECT crect;
    GetWindowRect(GetMainWindowHandle(), &rect);
    GetClientRect(GetMainWindowHandle(), &crect);

    // Gizmoを画面に合わせて処理する
    float windowWidth  = (float)rect.right - rect.left;
    float windowHeight = (float)rect.bottom - rect.top;
    float windowBar    = windowHeight - crect.bottom;
    ImGuizmo::SetRect((float)rect.left, (float)rect.top + windowBar / 2, windowWidth, windowHeight - windowBar / 2);

    // Manipulateを表示する
    static bool  useSnap         = false;
    static float snap[3]         = {1.0f, 1.0f, 1.0f};
    static float bounds[]        = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
    static float boundsSnap[]    = {0.1f, 0.1f, 0.1f};
    static bool  boundSizing     = false;
    static bool  boundSizingSnap = false;
    ImGuizmo::SetID((int)id);
    ImGuizmo::AllowAxisFlip(false);   //< これがないとGizmoが反転してしまう

    return ImGuizmo::Manipulate((const float*)&camera_view,
                                (const float*)&camera_proj,
                                ope,
                                mode,
                                (float*)matrix,
                                NULL,
                                false ? &snap[0] : NULL,
                                boundSizing ? bounds : NULL,
                                boundSizingSnap ? boundsSnap : NULL);
}

// ImGuizmoのMatrixからEulerに変換する際にいきなり軸に180度変換がかかってしまうため修正
void DecomposeMatrixToComponents(const float* matx, float* translation, float* rotation, float* scale)
{
    matrix mat = *(matrix*)matx;

    scale[0] = length(mat.axisX());
    scale[1] = length(mat.axisY());
    scale[2] = length(mat.axisZ());

    constexpr float threshold = 0.001f;
    constexpr float RAD2DEG   = (180.f / DX_PI_F);
    if(fabs(mat._23 - 1.0) < threshold) {
        rotation[0] = DX_PI_F / 2.0f;
        rotation[1] = 0;
        rotation[2] = atan2f(mat._12, mat._11);
    }
    else if(fabs(mat._23 + 1.0) < threshold) {
        rotation[0] = -DX_PI_F / 2.0f;
        rotation[1] = 0;
        rotation[2] = atan2f(mat._12, mat._11);
    }
    else {
        rotation[0] = asin(mat._23);
        rotation[1] = atan2(-mat._13, mat._33);
        rotation[2] = atan2(-mat._21, mat._22);
    }

    rotation[0] *= RAD2DEG;
    rotation[1] *= RAD2DEG;
    rotation[2] *= RAD2DEG;

    translation[0] = mat.translate().x;
    translation[1] = mat.translate().y;
    translation[2] = mat.translate().z;
}

void ComponentTransform::PostUpdate()
{
    __super::PostUpdate();

    old_transform_ = GetWorldMatrix();
}

//! @brief GUI処理
void ComponentTransform::GUI()
{
    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    if(is_guizmo_) {
        // Gizmo表示
        ShowGizmo(GetMatrixFloat(), gizmo_operation_, gizmo_mode_, reinterpret_cast<uint64_t>(this));

        // キーにより、Manipulateの処理を変更する
        // TODO : 一旦UE4に合わせておくが、のちにEditor.iniで設定できるようにする
        // W = Translate / E = Rotate / R = Scale (Same UE5)
        if(!ImGui::IsAnyItemActive()) {
            if(ImGui::IsKeyPressed('W'))
                gizmo_operation_ = ImGuizmo::TRANSLATE;
            if(ImGui::IsKeyPressed('E'))
                gizmo_operation_ = ImGuizmo::ROTATE;
            if(ImGui::IsKeyPressed('R'))
                gizmo_operation_ = ImGuizmo::SCALE;
        }
    }

    // GUI描画
    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        is_guizmo_ = false;
        if(ImGui::TreeNode("Transform")) {
            ImGui::DragFloat4(u8"Ｘ軸", VectorAxisXFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"Ｙ軸", VectorAxisYFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"Ｚ軸", VectorAxisZFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::DragFloat4(u8"座標", TranslateFloat(), 0.01f, -10000.0f, 10000.0f, "%.2f");
            ImGui::Separator();
            ImGui::TreePop();
        }

        if(ImGui::IsWindowFocused()) {
            select_component_ = this;
        }
        if(select_component_ == this)
            is_guizmo_ = true;

        // ギズモの処理選択
        if(ImGui::RadioButton(u8"座標", gizmo_operation_ == ImGuizmo::TRANSLATE))
            gizmo_operation_ = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if(ImGui::RadioButton(u8"回転", gizmo_operation_ == ImGuizmo::ROTATE))
            gizmo_operation_ = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if(ImGui::RadioButton(u8"サイズ", gizmo_operation_ == ImGuizmo::SCALE))
            gizmo_operation_ = ImGuizmo::SCALE;
        ImGui::SameLine();
        if(ImGui::RadioButton(u8"全部", gizmo_operation_ == ImGuizmo::UNIVERSAL))
            gizmo_operation_ = ImGuizmo::UNIVERSAL;

        if(gizmo_operation_ != ImGuizmo::SCALE) {
            if(ImGui::RadioButton("Local", gizmo_mode_ == ImGuizmo::LOCAL))
                gizmo_mode_ = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if(ImGui::RadioButton("World", gizmo_mode_ == ImGuizmo::WORLD))
                gizmo_mode_ = ImGuizmo::WORLD;
        }

        // TRSにてマトリクスを再度作成する
        bool   update = false;
        float* mat    = GetMatrixFloat();
        float  matrixTranslation[3], matrixRotation[3], matrixScale[3];
        DecomposeMatrixToComponents(mat, matrixTranslation, matrixRotation, matrixScale);
        if(ImGui::DragFloat3(u8"座標(T)", matrixTranslation)) {
            //GetOwner()->SetStatus( Object::StatusBit::Located, false );
            update = true;
        }
        ImGui::DragFloat3(u8"回転(R)", matrixRotation);
        ImGui::DragFloat3(u8"サイズ(S)", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, mat);

        if(update)
            PostUpdate();
    }
    ImGui::End();
}
