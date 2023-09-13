//---------------------------------------------------------------------------
//! @file   ComponentCollisionModel.h
//! @brief  コリジョンコンポーネント
//---------------------------------------------------------------------------
#pragma once
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>
#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

USING_PTR(ComponentCollisionModel);

//! @brief コリジョンコンポーネントクラス
class ComponentCollisionModel
    : public ComponentCollision
    , public IMatrix<ComponentCollisionModel>
{
public:
    BP_COMPONENT_TYPE(ComponentCollisionModel, ComponentCollision);

    ComponentCollisionModel()
    {
        collision_type_  = CollisionType::MODEL;
        collision_mass_  = -1;   //< 移動しないように
        collision_group_ = CollisionGroup::GROUND;
    }

    virtual void Init() override;
    virtual void Update(float delta) override;
    virtual void PostUpdate() override;
    virtual void Draw() override;
    virtual void Exit() override;

    virtual void GUI() override;   //!< GUI

    // 現在のモデルにアタッチします
    void AttachToModel(bool update = false);

    HitInfo IsHit(ComponentCollisionPtr col) override;

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override { return collision_transform_; }

    const matrix& GetMatrix() const override { return collision_transform_; }

    virtual ComponentCollisionModelPtr SharedThis() override
    {
        return std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this());
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override { return old_transform_; }

    //@}

    //! @brief マップに対する正確な移動量を割り出す
    //! @param vec 位置
    //! @param vec 動きたい移動量
    //! @return 実際動ける量
    float3 checkMovement(float3 pos, float3 vec, float force = 1.0f);

#if 1   // CompoentCollisionからの移行

    inline ComponentCollisionModelPtr SetName(std::string_view name)
    {
        name_ = name;
        return std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this());
    }

    inline ComponentCollisionModelPtr SetHitCollisionGroup(u32 hit_group)
    {
        collision_hit_ = hit_group;
        return std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this());
    }

    inline ComponentCollisionModelPtr SetCollisionGroup(CollisionGroup grp)
    {
        collision_group_ = grp;
        return std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this());
    }

    inline ComponentCollisionModelPtr SetMass(float mass)
    {
        collision_mass_ = mass;
        return std::dynamic_pointer_cast<ComponentCollisionModel>(shared_from_this());
    }
#endif

private:
    bool                                  update_    = false;
    int                                   ref_model_ = -1;
    MV1_REF_POLYGONLIST                   ref_poly_{};      //!< ポリゴンデータ
    std::vector<MV1_COLL_RESULT_POLY_DIM> hit_poly_dims_;   //!< 結果代入用ポリゴン配列

    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_));
        arc(CEREAL_NVP(update_));
        arc(cereal::make_nvp("ComponentCollision", cereal::base_class<ComponentCollision>(this)));
        AttachToModel(update_);
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentCollisionModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ComponentCollision, ComponentCollisionModel)
