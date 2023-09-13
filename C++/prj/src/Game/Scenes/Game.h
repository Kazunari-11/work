//---------------------------------------------------------------------------
//! @file   Game.h
//! @brief  ゲームシーン
//---------------------------------------------------------------------------
#pragma once
#include <System/Scene.h>

//! @class Game
//! @brief ゲームシーン
class Game : public Scene::Base
{
public:
    BP_CLASS_TYPE(Game, Scene::Base);

    //! シーン名称
    std::string Name() override { return u8"ゲーム"; }

    bool Init() override;                //!< 初期化
    void Update(float delta) override;   //!< 更新
    void PostDraw() override;            //!< 描画
    void Exit() override;                //!< 終了
    void GUI() override;                 //!< GUI表示
    void RandomSpawn(float3 pos);        //!< ランダムで敵をスポーン

    enum EnemyType   //!< 敵の種類
    {
        Normal,
        Fast,
        Tough,
        Bomb
    };

    void SetSpawn(EnemyType, float3 pos);   //!< 選択した敵をスポーン

private:
    float3 position_               = float3(0.0f, 0.0f, 0.0f);   //!< 位置
    bool   is_hit_target[3]        = {false, false, false};      //!< ターゲットに当たったか
    bool   is_goal_                = false;                      //!< プレイヤーがゴールしたか
    int    sky_model_              = -1;                         //!< スカイドーム
    int    alpha_                  = 0;                          //!< 黒四角の透明度
    int    info_font_handle_       = -1;                         //!< 説明の文字用フォント
    int    result_font_handle_     = -1;                         //<! クリアやデス文字用フォント
    int    typewriter_font_handle_ = -1;                         //<! タイプライターフォント
};
