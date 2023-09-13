#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Utils/HelperLib.h>
#include <System/Cereal.h>

USING_PTR(ComponentAttachModel);

class ComponentAttachModel : public Component
{
    friend class Object;

public:
    BP_COMPONENT_TYPE(ComponentAttachModel, Component);

    ComponentAttachModel() {}

    virtual void Init() override;                //!< 初期化
    virtual void Update(float delta) override;   //!< 更新
    virtual void PostUpdate() override;          //!< 更新
    virtual void Draw() override;                //!< デバッグ描画
    virtual void Exit() override;                //!< 終了処理
    virtual void GUI() override;                 //!< GUI処理

    //! @brief アタッチするノードの位置の取得
    //! @return 位置の取得
    matrix GetPutOnMatrix() const;

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct AttachModelBit : u32
    {
        Initialized,   //!< 初期化済み
    };

    void SetAttachModelStatus(AttachModelBit bit, bool on) { attach_model_status_.set(bit, on); }

    bool GetAttachModelStatus(AttachModelBit bit) { return attach_model_status_.is(bit); }

    void SetAttachObject(ObjectPtr object, std::string_view node = "");

    void SetAttachObject(std::string_view name, std::string_view node = "");

    void SetAttachRotate(float3 rot) { attach_model_rotate_ = rot; }

    void SetAttachOffset(float3 offset) { attach_model_offset_ = offset; }

    ObjectWeakPtr GetAttachObject() { return object_; }

private:
    Status<AttachModelBit> attach_model_status_;   //!< 状態

    float3 attach_model_rotate_{0, 0, 0};
    float3 attach_model_offset_{0, 0, 0};

    ObjectWeakPtr object_;
    std::string   object_name_{};

    int         object_node_index_ = 0;
    std::string object_node_name_{};

    bool node_manipulate_ = false;

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
        arc(cereal::make_nvp("owner", owner_));                                     //< オーナー
        arc(cereal::make_nvp("attach_model_status", attach_model_status_.get()));   //< カメラステート
        arc(CEREAL_NVP(attach_model_rotate_));
        arc(CEREAL_NVP(attach_model_offset_));
        arc(CEREAL_NVP(object_name_));
        arc(CEREAL_NVP(object_node_index_));
        arc(CEREAL_NVP(object_node_name_));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentAttachModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentAttachModel)
