//---------------------------------------------------------------------------
//! @file   Gun.h
//! @brief  プレイヤーが持つ銃
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class Gun
//! @brief プレイヤーが持つ銃
class Gun : public Object
{
private:
    //! テクスチャ
    struct Material
    {
        std::shared_ptr<Texture> albedo_;
        std::shared_ptr<Texture> normal_;
        std::shared_ptr<Texture> metalness_;
        std::shared_ptr<Texture> tex_ao_;
    };
    std::vector<Material> materials_;

public:
    //! @brief コンストラクタ
    Gun();

    //! @brief デストラクタ
    ~Gun();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief GUI
    void GUI() override;

    //! @brief 描画処理
    void Draw() override;

private:
    //! @brief モデルをセットする
    void SetModel(ObjectPtr obj);
};
