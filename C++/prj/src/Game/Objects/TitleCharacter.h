//---------------------------------------------------------------------------
//! @file   TitleCharacter.h
//! @brief  タイトルキャラクター
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>
#include <System/Component/ComponentCamera.h>

//! @class TitleCharacter
//! @brief タイトルキャラクター
class TitleCharacter : public Object
{
private:
    float3 eye_pos_      = {0.0f, 0.0f, -0.0f};   //!< カメラポジション
    float3 look_at_      = {0.0f, 0.0f, -0.0f};   //!< 注視点
    float  alpha_        = 255.0f;                //!< 黒四角の透明度
    bool   key_flag      = false;                 //!< 入力フラグ
    int    shot_se_      = -1;                    //!< 発砲SE
    int    death_se_     = -1;                    //!< 死亡SE
    int    cartridge_se_ = -1;                    //!< 薬莢SE
    int    se_count_     = 0;                     //!< cartridge_se_のタイミングカウント

public:
    //! @brief コンストラクタ
    TitleCharacter();

    //! @brief デストラクタ
    ~TitleCharacter();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief 描画処理
    void Draw();

    //! @brief ボリュームを設定しSEを再生
    //! @param se_handle seのハンドル
    //! @param volume ボリューム
    void PlaySE(int se_handle, int volume = 255) const;
};
