//---------------------------------------------------------------------------
//! @file   TargetPoint.h
//! @brief  プレイヤーが目指す地点
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class TargetPoint
//! @brief プレイヤーが目指す地点
class TargetPoint : public Object
{
private:
    float3      rot_              = {0.0f, 0.0f, 0.0f};   //!< 回転
    bool        is_hit_           = false;                //!< プレイヤーが接触したかのフラグ
    float       stay_timer_       = 0.0f;                 //!< プレイヤーが留まった時間
    const float STAY_TIME_        = 25.0f;                //!< プレイヤーが留まらないといけない時間
    bool        stay_creal_       = false;                //!< 滞在完了したかのフラグ
    int         info_font_handle_ = -1;                   //!< 説明の文字用フォント

    //! テクスチャ
    struct Material
    {
        std::shared_ptr<Texture> albedo_;
        std::shared_ptr<Texture> normal_;
        std::shared_ptr<Texture> metalness_;
    };
    std::vector<Material> materials_;

public:
    //! @brief コンストラクタ
    TargetPoint();

    //! @brief デストラクタ
    ~TargetPoint();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief GUI
    void GUI() override;

    //! @brief 接触処理
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

    //! @brief 描画処理
    void PostDraw() override;

    //! @brief 当たっているかのフラグの取得
    //! @detail is_hit_を取得
    //! @return 当たっているフラグ
    bool GetIsHit() const;

    //! @brief 留まることを完了したかの取得
    //! @detail stay_creal_を取得
    //! @return 留まりを完了したかのフラグ
    bool GetStayCreal() const;

    //! @brief 留まっている時間の取得
    //! @detail stay_timer_を取得
    //! @return 留まっている時間のタイマー
    float GetStayTimer() const;

private:
    //! @brief モデルをセットする
    void SetModel();

    //! @brief ゲージを描画
    void DrawGauge() const;
};
