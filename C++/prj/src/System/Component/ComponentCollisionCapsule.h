//---------------------------------------------------------------------------
//! @file   ComponentCollisionCapsule.h
//! @brief  コリジョンコンポーネント
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>
#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

USING_PTR(ComponentCollisionCapsule);

//! @brief コリジョンコンポーネントクラス
class ComponentCollisionCapsule
    : public ComponentCollision
    , public IMatrix<ComponentCollisionCapsule>
{
public:
    BP_COMPONENT_TYPE(ComponentCollisionCapsule, ComponentCollision);

    ComponentCollisionCapsule() { collision_type_ = CollisionType::CAPSULE; }

    virtual void Init() override;
    virtual void Update(float delta) override;
    virtual void PrePhysics() override;
    virtual void PostUpdate() override;
    virtual void Draw() override;
    virtual void Exit() override;

    virtual void GUI() override;   //!< GUI

    //! @brief 半径の設定
    //! @param radius 半径
    ComponentCollisionCapsulePtr SetRadius(float radius);

    //! @brief 半径の取得
    //! @return 半径
    float GetRadius() const;

    //! @brief カプセルの高さ設定
    //! @param height 高さ (radius*2 より大きい必要があります)
    //! @return 自分のSharedPtr
    ComponentCollisionCapsulePtr SetHeight(float height);

    //! @brief カプセルの高さを取得
    //! @return カプセルの高さ
    float GetHeight();

    HitInfo IsHit(ComponentCollisionPtr col) override;

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override { return collision_transform_; }

    const matrix& GetMatrix() const override { return collision_transform_; }

    virtual ComponentCollisionCapsulePtr SharedThis() override
    {
        return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override { return old_transform_; }

    //@}

    void setGravity(float g) { gravity_ = {0, g, 0}; }

    float gravity() { return gravity_.y; }

#if 1   // CompoentCollisionからの移行

    inline ComponentCollisionCapsulePtr SetName(std::string_view name)
    {
        name_ = name;
        return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
    }

    inline ComponentCollisionCapsulePtr SetHitCollisionGroup(u32 hit_group)
    {
        collision_hit_ = hit_group;
        return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
    }

    inline ComponentCollisionCapsulePtr SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
        return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
    }

    inline ComponentCollisionCapsulePtr SetMass(float mass)
    {
        collision_mass_ = mass;
        return std::dynamic_pointer_cast<ComponentCollisionCapsule>(shared_from_this());
    }
#endif

private:
    float radius_ = 1.0f;   //!< 半径
    float height_ = 3.0f;   //!< 高さ
#ifdef USE_JOLT_PHYSICS
    bool set_size_ = false;

    std::shared_ptr<physics::Character> character_{};   //!< キャラクターコントローラー

    class CapsuleListener : public physics::Character::ContactListener
    {
        void onContactAdded([[maybe_unused]] const physics::Character*            character,
                            [[maybe_unused]] u64                                  other_body_id,
                            [[maybe_unused]] const float3&                        contact_position,
                            [[maybe_unused]] const float3&                        contact_normal,
                            [[maybe_unused]] physics::Character::ContactSettings& result) override
        {
            // ノーマルの方向にて現在の重力加速を抑える
            float d = dot(-contact_normal, {0, 1, 0});
            if(GetComponent())
                GetComponent()->setGravity(GetComponent()->gravity() * (1 - d));
        }

    public:
        void SetComponent(ComponentCollisionCapsulePtr comp) { component_ = comp; }

        ComponentCollisionCapsulePtr GetComponent() { return component_; }

    protected:
        ComponentCollisionCapsulePtr component_ = nullptr;
    };

    CapsuleListener listener_;
    // 重力加速度
    float3 gravity_ = 0.0f;
#endif   //USE_JOLT_PHYSICS
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));
        arc(cereal::make_nvp("radius", radius_));
        arc(cereal::make_nvp("height", height_));
        arc(cereal::make_nvp("ComponentCollision", cereal::base_class<ComponentCollision>(this)));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollisionCapsule)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ComponentCollision, ComponentCollisionCapsule)
