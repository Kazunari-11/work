//---------------------------------------------------------------------------
//! @file   DropItem.h
//! @brief  ドロップアイテム
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class DropItem
//! @brief ドロップアイテム
class DropItem : public Object
{
private:
    bool      is_hit_      = false;   //!< プレイヤーが接触したかのフラグ
    int       deletetimer_ = 0;       //!< 消えるまでのタイマー
    const int DELETE_TIME_ = 1200;    //!< 消える時間

    //! テクスチャ
    struct Material
    {
        std::shared_ptr<Texture> albedo_;
        std::shared_ptr<Texture> normal_;
        std::shared_ptr<Texture> roughness_;
        std::shared_ptr<Texture> metalness_;
        std::shared_ptr<Texture> tex_ao_;
    };
    std::vector<Material> materials_;

public:
    //! @brief コンストラクタ
    DropItem();

    //! @brief デストラクタ
    ~DropItem();

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
    void Draw() override;

private:
    //! @brief モデルをセットする
    void SetModel();
};
