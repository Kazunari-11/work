#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Utils/HelperLib.h>
#include <System/Cereal.h>

USING_PTR(ComponentSpringArm);

class ComponentSpringArm : public Component
{
    friend class Object;

public:
    BP_COMPONENT_TYPE(ComponentSpringArm, Component);

    ComponentSpringArm() {}

    virtual void Init() override;                    //!< 初期化
    virtual void Update(float delta) override;       //!< 更新
    virtual void LateUpdate(float delta) override;   //!< 更新
    virtual void Draw() override;                    //!< デバッグ描画
    virtual void Exit() override;                    //!< 終了処理
    virtual void GUI() override;                     //!< GUI処理

    //! @brief スプリングアームの先の位置の取得
    //! @return 位置の取得
    matrix GetPutOnMatrix() const;

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct SpringArmBit : u32
    {
        Initialized,      //!< 初期化済み
        EnableTimeLine,   //!< TimelineComponentを利用して移動する
    };

    void SetSpringArmStatus(SpringArmBit bit, bool on) { spring_arm_status_.set(bit, on); }

    bool GetSpringArmStatus(SpringArmBit bit) { return spring_arm_status_.is(bit); }

    void SetSpringArmObject(ObjectPtr object);

    void SetSpringArmObject(std::string_view name);

    //! @brief アームの向きを3軸回転で向ける方式
    //! @param rot { X軸, Y軸, Z軸 } 回転量
    ComponentSpringArmPtr SetSpringArmRotate(float3 rot)
    {
        spring_arm_rotate_ = rot;
        return std::dynamic_pointer_cast<ComponentSpringArm>(shared_from_this());
    }

    //! @brief アームの向きをベクトルで向ける方式
    //! @param vec 向ける方向
    ComponentSpringArmPtr SetSpringArmVector(float3 vec)
    {
        float trans[3]{};
        float rot[3]{};
        float scale[3]{};
        auto  mat = HelperLib::Math::CreateMatrixByFrontVector(vec);
        DecomposeMatrixToComponents(mat.f32_128_0, trans, rot, scale);
        spring_arm_rotate_ = float3(rot[0], rot[1], rot[2]);
        return std::dynamic_pointer_cast<ComponentSpringArm>(shared_from_this());
    }

    ComponentSpringArmPtr SetSpringArmOffset(float3 offset)
    {
        spring_arm_offset_ = offset;
        return std::dynamic_pointer_cast<ComponentSpringArm>(shared_from_this());
    }

    ComponentSpringArmPtr SetSpringArmLength(float len)
    {
        spring_arm_length_ = len;
        return std::dynamic_pointer_cast<ComponentSpringArm>(shared_from_this());
    }

    ObjectWeakPtr GetSpringArmObject() { return object_; }

    ComponentSpringArmPtr SetSpringArmStrong(float strong)
    {
        spring_arm_strong_ = strong;
        return std::dynamic_pointer_cast<ComponentSpringArm>(shared_from_this());
    }

    ComponentSpringArmPtr SetSpringArmReturn(float ret)
    {
        spring_arm_strong_ = ret;
        return std::dynamic_pointer_cast<ComponentSpringArm>(shared_from_this());
    }

private:
    Status<SpringArmBit> spring_arm_status_;   //!< 状態

    float3 spring_arm_rotate_{10, 0, 0};
    float3 spring_arm_offset_{0, 3, 0};

    float spring_arm_length_ = 20.0f;

    ObjectWeakPtr object_;
    std::string   object_name_{};

    int         object_node_index_ = 0;
    std::string object_node_name_{};

    // 以後スプリング部
    float spring_arm_strong_ = 1.00f;
    float spring_arm_return_ = 0.01f;

    float spring_arm_length_now_ = 20.0f;
    float spring_arm_vecspd_     = 0.0f;
    float force_up_              = 0.0f;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブ
    // @param arc アーカイバ
    // @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));                                 //< オーナー
        arc(cereal::make_nvp("spring_arm_status", spring_arm_status_.get()));   //< カメラステート
        arc(CEREAL_NVP(spring_arm_rotate_));
        arc(CEREAL_NVP(spring_arm_offset_));
        arc(CEREAL_NVP(spring_arm_length_));
        arc(CEREAL_NVP(object_name_));
        arc(CEREAL_NVP(object_node_index_));
        arc(CEREAL_NVP(object_node_name_));

        arc(CEREAL_NVP(spring_arm_strong_));
        arc(CEREAL_NVP(spring_arm_return_));
        arc(CEREAL_NVP(spring_arm_length_now_));
        arc(CEREAL_NVP(spring_arm_vecspd_));
        arc(CEREAL_NVP(force_up_));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentSpringArm)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentSpringArm)
