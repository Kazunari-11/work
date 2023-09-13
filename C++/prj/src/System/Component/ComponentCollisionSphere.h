//---------------------------------------------------------------------------
//! @file   ComponentCollisionSphere.h
//! @brief  コリジョンコンポーネント
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>
#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

USING_PTR(ComponentCollisionSphere);

//! @brief コリジョンコンポーネントクラス
class ComponentCollisionSphere
    : public ComponentCollision
    , public IMatrix<ComponentCollisionSphere>
{
public:
    BP_COMPONENT_TYPE(ComponentCollisionSphere, ComponentCollision);

    ComponentCollisionSphere() { collision_type_ = CollisionType::SPHERE; }

    virtual void Init() override;
    virtual void Update(float delta) override;
    virtual void PostUpdate() override;
    virtual void Draw() override;
    virtual void Exit() override;

    virtual void GUI() override;   //!< GUI

    //! @brief 半径の設定
    //! @param radius 半径
    ComponentCollisionSpherePtr SetRadius(float radius);

    //! @brief 半径の取得
    //! @return 半径
    float GetRadius() const;

    HitInfo IsHit(ComponentCollisionPtr col) override;

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override { return collision_transform_; }

    const matrix& GetMatrix() const override { return collision_transform_; }

    virtual ComponentCollisionSpherePtr SharedThis() override
    {
        return std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this());
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override { return old_transform_; }

    //@}
#if 1   // CompoentCollisionからの移行

    inline ComponentCollisionSpherePtr SetName(std::string_view name)
    {
        name_ = name;
        return std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this());
    }

    inline ComponentCollisionSpherePtr SetHitCollisionGroup(u32 hit_group)
    {
        collision_hit_ = hit_group;
        return std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this());
    }

    inline ComponentCollisionSpherePtr SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
        return std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this());
    }

    inline ComponentCollisionSpherePtr SetMass(float mass)
    {
        collision_mass_ = mass;
        return std::dynamic_pointer_cast<ComponentCollisionSphere>(shared_from_this());
    }
#endif

protected:
#ifdef USE_JOLT_PHYSICS
    bool set_size_ = false;
#endif                      //USE_JOLT_PHYSICS
    float radius_ = 1.0f;   //!< 半径

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));
        arc(cereal::make_nvp("radius", radius_));
        arc(cereal::make_nvp("ComponentCollision", cereal::base_class<ComponentCollision>(this)));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollisionSphere)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ComponentCollision, ComponentCollisionSphere)
