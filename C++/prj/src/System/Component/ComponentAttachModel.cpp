#include <System/Component/ComponentAttachModel.h>
#include <System/Component/ComponentTransform.h>
#include <System/Component/ComponentModel.h>
#include <System/Object.h>
#include <System/Scene.h>
#include <System/ImGui.h>

BP_COMPONENT_IMPL(ComponentAttachModel, u8"AttachModel機能クラス");

//---------------------------------------------------------
//! 初期化
//---------------------------------------------------------
void ComponentAttachModel::Init()
{
    __super::Init();

    // PreUpdateの最後で自分の位置を設定する(ターゲットの移動の後)
    Scene::GetCurrentScene()->SetPriority(shared_from_this(), ProcTiming::PreUpdate, Priority::LOWEST);
    // モデルの姿勢が完了した後実行したい
    Scene::GetCurrentScene()->SetPriority(shared_from_this(), ProcTiming::PostUpdate, Priority::LOWEST);

    SetAttachModelStatus(AttachModelBit::Initialized, true);
}

//---------------------------------------------------------
//! 更新
//---------------------------------------------------------
void ComponentAttachModel::Update([[maybe_unused]] float delta)
{
#if 0
	auto owner = GetOwner();
	if( owner )
	{
		auto target = object_.lock();
		if( target == nullptr )
		{
			if( object_name_.empty() )
				return;

			SetAttachObject( object_name_ );
		}

		owner->SetMatrix( GetPutOnMatrix() );

		if( !owner->GetStatus( Object::StatusBit::Located ) )
		{
			owner->UseWarp();
		}
	}
#endif
}

void ComponentAttachModel::PostUpdate()
{
    auto owner = GetOwner();
    if(owner) {
        auto target = object_.lock();
        if(target == nullptr) {
            if(object_name_.empty())
                return;

            SetAttachObject(object_name_, object_node_name_);
        }

        owner->SetMatrix(GetPutOnMatrix());

        if(!owner->GetStatus(Object::StatusBit::Located)) {
            owner->UseWarp();
        }
    }
}

//! @brief スプリングアームの先からのマトリクス取得
//! @return マトリクス
matrix ComponentAttachModel::GetPutOnMatrix() const
{
    float  trans[3] = {attach_model_offset_.x, attach_model_offset_.y, attach_model_offset_.z};
    float  rot[3]   = {attach_model_rotate_.x, attach_model_rotate_.y, attach_model_rotate_.z};
    float  scale[3] = {1.0f, 1.0f, 1.0f};
    matrix rmat;
    ImGuizmo::RecomposeMatrixFromComponents(trans, rot, scale, rmat.f32_128_0);

    matrix mat = matrix::identity();

    if(auto object = object_.lock()) {
        mat = object->GetMatrix();

        if(auto target_model = object->GetComponent<ComponentModel>()) {
            auto model_mat = target_model->GetMatrix();
            int  no        = target_model->GetNodeIndex(object_node_name_);
            if(no >= 0) {
                mat = target_model->GetNodeMatrix(no);
            }
        }
    }
    mat = mul(rmat, mat);

    return mat;
}

void ComponentAttachModel::SetAttachObject(ObjectPtr object, std::string_view node)
{
    object_           = object;
    object_name_      = object->GetName();
    object_node_name_ = node;
    if(auto model = object->GetComponent<ComponentModel>())
        object_node_index_ = model->GetNodeIndex(node);

    GetOwner()->SetStatus(Object::StatusBit::Located, false);
}

void ComponentAttachModel::SetAttachObject(std::string_view name, std::string_view node)
{
    if(auto obj = Scene::GetObjectPtr<Object>(name))
        SetAttachObject(obj, node);
}

//---------------------------------------------------------
//! デバッグ表示
//---------------------------------------------------------
void ComponentAttachModel::Draw()
{
    if(node_manipulate_) {
        if(auto object = object_.lock()) {
            if(auto target_model = object->GetComponent<ComponentModel>()) {
                int no = target_model->GetNodeIndex(object_node_name_);
                if(no >= 0) {
                    matrix mat = target_model->GetNodeMatrix(no);
                    ShowGizmo((float*)mat.f32_128_0,
                              ImGuizmo::OPERATION::TRANSLATE,
                              ImGuizmo::LOCAL,
                              reinterpret_cast<uint64_t>(object.get()));
                }
            }
        }
    }
}

//---------------------------------------------------------
//! カメラ終了処理
//---------------------------------------------------------
void ComponentAttachModel::Exit()
{
    __super::Exit();
}

//---------------------------------------------------------
//! GUI処理
//---------------------------------------------------------
void ComponentAttachModel::GUI()
{
    assert(GetOwner());
    auto obj_name = GetOwner()->GetName();

    ImGui::Begin(obj_name.data());
    {
        node_manipulate_ = false;

        ImGui::Separator();
        if(ImGui::TreeNode(u8"AttachModel")) {
            node_manipulate_ = true;

            if(ImGui::Button(u8"削除")) {
                GetOwner()->RemoveComponent(shared_from_this());
            }

            u32* bit = &attach_model_status_.get();
            u32  val = *bit;
            ImGui::CheckboxFlags(u8"初期化済", &val, 1 << (int)AttachModelBit::Initialized);

            if(ImGui::BeginCombo("AttachObject", object_name_.data())) {
                auto objs = Scene::GetObjectsPtr<Object>();
                for(int i = 0; i < objs.size(); i++) {
                    auto        obj         = objs[i];
                    std::string object_name = std::string(obj->GetName());

                    bool is_selected = (object_name_ == object_name);
                    if(ImGui::Selectable(object_name.data(), is_selected)) {
                        object_name_ = object_name;
                        SetAttachObject(object_name_);
                    }
                }
                ImGui::EndCombo();
            }

            if(!object_name_.empty()) {
                auto obj = Scene::GetObjectPtr<Object>(object_name_);
                if(auto model = obj->GetComponent<ComponentModel>()) {
                    auto items = model->GetNodesNamePChar();

                    if(ImGui::Combo("Node", &object_node_index_, items.data(), (int)items.size())) {
                        // 切り替えたとき
                        object_node_name_ = items[object_node_index_];
                    }
                }
            }

            ImGui::DragFloat3(u8"AttachModel回転", (float*)&attach_model_rotate_, 0.1f, -10000.0f, 10000.0f, "%.1f");
            ImGui::DragFloat3(u8"AttachModelオフセット",
                              (float*)&attach_model_offset_,
                              0.1f,
                              -10000.0f,
                              10000.0f,
                              "%.1f");

            ImGui::TreePop();
        }
    }
    ImGui::End();
}
