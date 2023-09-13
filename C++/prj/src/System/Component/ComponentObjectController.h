#pragma once
#include <System/Scene.h>
#include <System/Component/Component.h>

class ComponentObjectController : public Component
{
public:
    BP_COMPONENT_TYPE(ComponentObjectController, Component);

    void Init() override { __super::Init(); }

    void Update(float delta_time) override
    {
        __super::Update(delta_time);

        // オーナー(自分がAddComponentされたObject)を取得します
        // 処理されるときは必ずOwnerは存在しますので基本的にnullptrチェックは必要ありません
        auto owner = GetOwner();

        // オーナーのMatrix( 位置、回転情報、スケール )を取得しておきます
        matrix mat = owner->GetMatrix();

        // カレントのカメラ(現在映しているカメラ)を取得する
        // カメラが存在すればカメラの向きに合わせて移動する (先のmatを差し替えている)
        if(auto camera = Scene::GetCurrentCamera().lock()) {
            auto vec = camera->GetTarget() - camera->GetPosition();
            vec.y    = 0.0f;   //< y軸は考慮しない
            mat      = HelperLib::Math::CreateMatrixByFrontVector(vec);

            ShowGizmo((float*)&mat, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, 123456);
            SetLightEnable(FALSE);
            SetUseLighting(FALSE);
            DrawLine3D({0, 10, 0}, cast(float3{0, 10, 0} + normalize(vec) * 100.0f), GetColor(255, 0, 0));
            SetUseLighting(TRUE);
            SetLightEnable(TRUE);
        }

        float3 vec{0, 0, 0};
        if(IsKey(key_up_))
            vec = mat.axisZ();

        if(IsKey(key_down_))
            vec = -mat.axisZ();

        if(IsKey(key_right_))
            vec = mat.axisX();

        if(IsKey(key_left_))
            vec = -mat.axisX();

        if((float)length(vec) > 0.0f) {
            owner->AddTranslate(normalize(vec) * speed_);
        }

        if(auto camera = Scene::GetCurrentCamera().lock()) {
            auto cam_owner = camera->GetOwner();
            if(auto arm = cam_owner->GetComponent<ComponentSpringArm>()) {
                // ターゲットがいる場合
                if(auto target = target_.lock()) {
                    // 徐々に敵の方に向ける(1フレーム最大3度)
                    owner->SetRotationToPositionWithLimit(target->GetTranslate(),
                                                          target_cam_side_speed_ * delta_time * 60.0f);

                    // カメラローテーションをロック方向にしておくと
                    // 戻った時に違和感がない
                    cam_ry_ = owner->GetRotationAxisXYZ().y;
                    cam_rx_ = (cam_rx_ + target_cam_up_down) * 0.95f - target_cam_up_down;

                    arm->SetSpringArmRotate({cam_rx_, 0, 0});
                }
                else {
                    if(IsKey(cam_up_)) {
                        cam_rx_ += cam_speed_;
                        if(cam_rx_ > limit_cam_up_)
                            cam_rx_ = limit_cam_up_;
                    }
                    if(IsKey(cam_down_)) {
                        cam_rx_ -= cam_speed_;
                        if(cam_rx_ < limit_cam_down_)
                            cam_rx_ = limit_cam_down_;
                    }
                    if(IsKey(cam_right_)) {
                        cam_ry_ += cam_speed_;
                        if(cam_ry_ > 360.0f)
                            cam_ry_ -= 360.0f;
                    }
                    if(IsKey(cam_left_)) {
                        cam_ry_ -= cam_speed_;
                        if(cam_ry_ < -360.0f)
                            cam_ry_ += 360.0f;
                    }

                    // Y軸はオブジェクトそのものの向きを使う
                    owner->SetRotationAxisXYZ({0, cam_ry_, 0});
                    // 上下はSpringArmで制御しておく
                    arm->SetSpringArmRotate({cam_rx_, 0, 0});
                }
            }
        }
    }

    void SetSpeed(const float speed) { speed_ = speed; }

    const float GetSpeed() const { return speed_; }

    void SetKeys(int up, int down, int left, int right)
    {
        key_up_    = up;
        key_down_  = down;
        key_left_  = left;
        key_right_ = right;
    }

    void SetCameraKeys(int up, int down, int left, int right)
    {
        cam_up_    = up;
        cam_down_  = down;
        cam_left_  = left;
        cam_right_ = right;
    }

    void SetTarget(ObjectPtr target) { target_ = target; }

    void GUI() override
    {
        __super::GUI();

        ImGui::Begin(GetOwner()->GetName().data());
        {
            ImGui::Separator();
            if(ImGui::TreeNode(u8"ComponentObjectController")) {
                // GUI上でオーナーから自分(SampleObjectController)を削除します
                if(ImGui::Button(u8"削除"))
                    GetOwner()->RemoveComponent(shared_from_this());

                ImGui::DragFloat(u8"速度", &speed_, 0.1f);

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

private:
    float speed_     = 1.0f;
    int   key_up_    = KEY_INPUT_W;
    int   key_down_  = KEY_INPUT_S;
    int   key_left_  = KEY_INPUT_A;
    int   key_right_ = KEY_INPUT_D;

    float cam_speed_ = 1.0f;
    int   cam_up_    = KEY_INPUT_UP;
    int   cam_down_  = KEY_INPUT_DOWN;
    int   cam_left_  = KEY_INPUT_LEFT;
    int   cam_right_ = KEY_INPUT_RIGHT;

    float cam_rx_ = 0;
    float cam_ry_ = 0;

    float limit_cam_up_   = 5.0f;     //!< 上を眺める
    float limit_cam_down_ = -45.0f;   //!< 下を見る

    ObjectWeakPtr target_;
    float         target_cam_side_speed_ = 3.0f;    //!< ロックオン時のカーソル移動速度
    float         target_cam_up_down     = 10.0f;   //!< ターゲットを見る際の上下固定

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver) { arc(cereal::make_nvp("Component", cereal::base_class<Component>(this))); }
};

BP_COMPONENT_IMPL(ComponentObjectController, u8"移動コンポーネント");
