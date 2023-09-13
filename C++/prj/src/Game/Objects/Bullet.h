//---------------------------------------------------------------------------
//! @file   Bullet.h
//! @brief  弾
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class Bullet
//! @brief 弾
class Bullet : public Object
{
private:
    float3    rot_         = {0.0f, 0.0f, 0.0f};   //!< 回転
    float     sp_          = 0.0f;                 //!< 移動速度  現状使わない
    bool      is_hit_      = false;                //!< 何かに当たったか
    float     radius_      = 0.0f;                 //!< 弾の半径
    float3    vec_         = {0.0f, 0.0f, 0.0f};   //!< 進む方向
    float     timer_       = 0.0f;                 //!< 弾が消えるまでのタイマー
    const int DELETE_TIME_ = 120;                  //!< 消える時間

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
    Bullet();
    //! @brief デストラクタ
    ~Bullet();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update(float delta) override;

    //! @brief 描画処理
    void Draw() override;

    //! @brief 接触処理
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

    //! @brief 位置、方向をセット
    //! @detail use_skill_を設定 デフォルトはtrue
    //! @param pos セットする位置 vec 方向セット
    void SetPositionAndDirection(float3 pos, float3 vec);

private:
    //! @brief モデルをセットする
    void SetModel();
};
