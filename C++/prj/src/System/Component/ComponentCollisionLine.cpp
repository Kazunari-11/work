//---------------------------------------------------------------------------
//! @file   ComponentCollisionLine.h
//! @brief  Sphereコリジョンコンポーネント
//---------------------------------------------------------------------------
#include <System/Component/ComponentCollisionLine.h>
#include <System/Component/ComponentCollisionCapsule.h>
#include <System/Component/ComponentCollisionSphere.h>
#include <System/Component/ComponentCollisionModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentModel.h>
#include <System/Object.h>
#include <System/Scene.h>

BP_COMPONENT_IMPL(ComponentCollisionLine, u8"SphereCollision機能クラス");

void ComponentCollisionLine::Init()
{
    __super::Init();
}

void ComponentCollisionLine::Update(float delta)
{
    __super::Update(delta);
}

// ここで当たり処理後の処理を行います
void ComponentCollisionLine::PostUpdate()
{
    __super::PostUpdate();
    old_transform_ = GetWorldMatrix();
}

void ComponentCollisionLine::Draw()
{
    if(!Scene::IsEdit() && !collision_status_.is(CollisionBit::ShowInGame))
        return;

    __super::Draw();

    auto trans = mul(inverse(collision_transform_), GetWorldMatrix());

    SetUseLighting(FALSE);
    SetLightEnable(FALSE);

    auto   line = GetWorldLine();
    VECTOR pos1 = cast(line[0]);
    VECTOR pos2 = cast(line[1]);

    printfDx("{%2.1f, %2.1f, %2.1f}", pos1.x, pos1.y, pos1.z);
    printfDx("{%2.1f, %2.1f, %2.1f}", pos2.x, pos2.y, pos2.z);

    DrawLine3D(pos1, pos2, GetColor(255, 0, 0));
    SetLightEnable(TRUE);
    SetUseLighting(TRUE);
}

void ComponentCollisionLine::Exit()
{
    __super::Exit();
}

//! @brief GUI処理
void ComponentCollisionLine::GUI()
{
    // 基底クラスのGUIはそのままは使用しない
    //__super::GUI();

    // オーナーの取得
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        ImGui::Separator();
        auto ui_name = std::string("Collision Line:") + std::to_string(collision_id_);
        if(ImGui::TreeNode(ui_name.c_str())) {
            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            // コリジョン情報を出す
            GUICollisionData();

            std::string colname = u8"COL:" + std::to_string(collision_id_) + "/ ";

            auto   line = GetLine();
            float3 pos1 = line[0];
            float3 pos2 = line[1];

            bool change = false;
            change |= ImGui::DragFloat3((colname + u8" Start").data(), (float*)&pos1, 0.1f);
            change |= ImGui::DragFloat3((colname + u8" End").data(), (float*)&pos2, 0.1f);

            if(HelperLib::Math::NearlyEqual(pos1.x, pos2.x) && HelperLib::Math::NearlyEqual(pos1.y, pos2.y) &&
               HelperLib::Math::NearlyEqual(pos1.z, pos2.z)) {
                pos2 += {0, 0, 0.01};
            }

            // StartかEndが変更されていたら再設定する
            if(change)
                SetLine(pos1, pos2);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

ComponentCollisionLinePtr ComponentCollisionLine::SetLine(float3 start, float3 end)
{
    auto mat = HelperLib::Math::CreateMatrixByFrontVector(end - start, {0, 1, 0}, true);
    SetMatrix(mat);
    SetTranslate(start);

    line_scale_ = length(start - end);

    return SharedThis();
}

std::array<float3, 2> ComponentCollisionLine::GetLine() const
{
    std::array<float3, 2> line;

    line[0] = GetTranslate();
    line[1] = GetTranslate() + GetMatrix().axisZ() * line_scale_;

    return line;
}

std::array<float3, 2> ComponentCollisionLine::GetWorldLine() const
{
    auto line = GetLine();

    auto trans = mul(inverse(collision_transform_), GetWorldMatrix());

    line[0] = mul(float4(line[0], 1), trans).xyz;
    line[1] = mul(float4(line[1], 1), trans).xyz;

    return line;
}

//! @brief 当たっているかを調べる
//! @param col 相手のコリジョン
//! @return HitInfoを返す
ComponentCollisionLine::HitInfo ComponentCollisionLine::IsHit(ComponentCollisionPtr col)
{
    HitInfo info;

    switch(col->GetCollisionType()) {
    case ComponentCollision::CollisionType::LINE:
        // @todo HitCheck_Line_Sphere()
        break;
    case ComponentCollision::CollisionType::TRIANGLE:
        // @todo HitCheck_Sphere_Triangle()
        break;
    case ComponentCollision::CollisionType::SPHERE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionSphere>(col));
        break;
    case ComponentCollision::CollisionType::CAPSULE:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionCapsule>(col));
        break;
    case ComponentCollision::CollisionType::MODEL:
        return isHit(std::dynamic_pointer_cast<ComponentCollisionLine>(shared_from_this()),
                     std::dynamic_pointer_cast<ComponentCollisionModel>(col));
        break;
    }

    return info;
}

//! @brief ワールドMatrixの取得
//! @return 他のコンポーネントも含めた位置
const matrix ComponentCollisionLine::GetWorldMatrix() const
{
    matrix transform = collision_transform_;

    auto obj = GetOwner();
    if(attach_node_ >= 0) {
        auto mdl = obj->GetComponent<ComponentModel>();
        if(mdl) {
            transform = mul(transform, attach_node_matrix_);
        }
    }
    else {
        matrix mat = obj->GetMatrix();
        transform  = mul(transform, mat);
    }

    return transform;
}
