#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentModel.h>
#include <System/Cereal.h>

USING_PTR(ComponentTargetTracking);

class ComponentTargetTracking : public Component
{
public:
    BP_COMPONENT_TYPE(ComponentTargetTracking, Component);

    ComponentTargetTracking() {}

    virtual void Init() override;         //!< 初期化
    virtual void PreUpdate() override;    //!< 向きの初期化
    virtual void PostUpdate() override;   //!< 向きの更新
    virtual void Draw() override;         //!< 向きのマトリクス表示(デバッグ用)
    virtual void Exit() override;         //!< 終了処理
    virtual void GUI() override;          //!< GUI処理

    //! @brief 追跡オブジェクトの設定
    //! @param obj 追跡したいオブジェクト
    void SetTargetObjectPtr(ObjectWeakPtr obj);

    //! @brief 追跡ポイントの設定
    //! @param target 向きたいポイント
    void SetTargetDirection(float3 target);

    //! @brief トラッキングを行うノード名
    //! @param name ノード名
    void SetTrackingNode(const std::string& name);

    //! @brief トラッキングを行うノード名
    //! @param name ノード名
    void SetTrackingNode(int tracking_node_index);

    //! @brief 左右の角度制限
    //! @param left_right 左右の角度
    void SetTrackingLimitLeftRight(float2 left_right);

    //! @brief 上下の角度制限
    //! @param left_right 上下の角度
    void SetTrackingLimitUpDown(float2 up_down);

    //! @brief 前ベクトルのセット
    //! @param vec 前ベクトル
    void SetFrontVector(float3 vec);

    //---------------------------------------------------------------------------
    //! トラッキングステータス
    //---------------------------------------------------------------------------
    enum struct TrackingBit : u32
    {
        Initialized,      //!< 初期化済み
        LimitAxisY,       //!< Y軸制限を有効にする
        LimitAxisX,       //!< X軸制限を有効にする
        ObjectTracking,   //!< オブジェクトをトラッキングする
    };

    void SetTrackingStatus(TrackingBit bit, bool on) { tracking_status_.set(bit, on); }

    bool GetTrackingStatus(TrackingBit bit) { return tracking_status_.is(bit); }

private:
    Status<TrackingBit> tracking_status_;           //!< 状態
    std::string         tracked_node_;              //!< 追跡させるノード名
    int                 tracked_node_index_ = -1;   //!< ノードインデックス
    float3              front_vector_       = {0, 0, 1};

    float3 look_at_  = {0, 0, 0};   //!< 注視点
    float2 limit_lr_ = {70, 70};
    float2 limit_ud_ = {50, 50};

    matrix tracking_matrix_ = matrix::identity();

    ObjectWeakPtr         tracking_object_{};   //!< 追跡オブジェクト
    ComponentModelWeakPtr owner_model_{};

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
        arc(cereal::make_nvp("owner", owner_));                             //< オーナー
        arc(cereal::make_nvp("tracking_status", tracking_status_.get()));   //< カメラステート
        arc(CEREAL_NVP(tracked_node_), CEREAL_NVP(tracked_node_index_));
        arc(CEREAL_NVP(front_vector_));
        arc(CEREAL_NVP(tracking_object_), CEREAL_NVP(look_at_));   //< カメラ位置とターゲット
        arc(CEREAL_NVP(limit_lr_), CEREAL_NVP(limit_ud_));

        SetTrackingNode(tracked_node_);
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentTargetTracking)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentTargetTracking)
