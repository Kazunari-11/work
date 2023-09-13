//---------------------------------------------------------------------------
//! @file   Player.h
//! @brief  プレイヤー
//---------------------------------------------------------------------------
#pragma once
#include <System/Object.h>

//! @class Player
//! @brief プレイヤークラス
class Player : public Object
{
private:
    float       sp_                  = 0.0f;     //!< 移動速度
    float       hp_                  = 0.0f;     //!< 体力
    bool        is_use_skill_        = false;    //!< 時止めを使ってるか
    bool        is_dead_             = false;    //!< 死んでいるかどうか
    float       skill_timer_         = 0.0f;     //!< スキル使用時間カウント
    int         skill_cooltime_      = 0;        //!< スキル使用後クールタイム
    int         guns_mag_num_        = 0;        //!< 残弾数
    int         have_bullet_num_     = 0;        //!< 弾薬所持数
    int         bullet_timer_        = 0;        //!< 弾発射レート利用可能
    int         gun_image_           = -1;       //!< 銃画像
    bool        damage_flag_         = false;    //!< ダメージを受けているかのフラグ
    bool        is_push_mouse_right_ = false;    //!< マウスの右ボタンを押しているか
    bool        is_tutorial_         = false;    //!< チュートリアル中かどうかのフラグ
    bool        is_reloading_        = false;    //!< リロード中かどうか
    bool        is_kicking_          = false;    //!< キック中かどうか
    bool        is_clear_            = false;    //!< クリア状態
    int         shot_se_             = -1;       //!< 発砲SE
    int         no_ammo_se_          = -1;       //!< 弾切れSE
    int         death_se_            = -1;       //!< 死亡SE
    int         skill_activ_se_      = -1;       //!< スキル発動SE
    int         skill_release_se     = -1;       //!< スキル解除SE
    int         skill_empty_se_      = -1;       //!< スキルゲージ空SE
    int         skill_notuse_se_     = -1;       //!< スキル使用不可時SE
    const float HP_MAX_              = 600.0f;   //!< 体力最大値
    const float SKILL_TIMER_MAX_     = 300.0f;   //!< スキル使用時間カウント上限
    const int   SKILL_COOLTIME_MAX_  = 300;      //!< スキル使用後クールタイム上限
    int         font_handle_         = -1;       //!< フォントデータ保存用変数

    //! 描画色変更情報
    struct FadeInfo
    {
        float4 center_position_;   //!< [xyz_] オブジェクトの中心位置
        float  nega_radius;        //!< 色反転の円の半径
        float  gray_radius;        //!< グレーの円の半径
    };
    int cb_fade_info_ = -1;   //!< [DxLib] モデル情報定数バッファハンドル

public:
    //! @brief コンストラクタ
    Player();
    //! @brief デストラクタ
    ~Player();

    //! @brief 初期化
    //! @return 初期化が終わったか
    bool Init() override;

    //! @brief 更新処理
    void Update([[maybe_unused]] float delta) override;

    //! @brief 描画処理
    void PostDraw();

    //! @brief GUI
    void GUI() override;

    //! @brief 接触処理
    void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo) override;

    //! @brief 終了処理
    void Exit() override;

public:
    //! @brief グレーの円の半径を取得
    //! @return グレーの円の半径
    float GetGrayRadius() const
    {
        void* p = GetBufferShaderConstantBuffer(cb_fade_info_);
        {
            auto* info = reinterpret_cast<FadeInfo*>(p);

            return info->gray_radius;
        }
    }

    //! @brief 色反転の円の半径を取得
    //! @return 色反転の円の半径
    float GetNegaRadius() const
    {
        void* p = GetBufferShaderConstantBuffer(cb_fade_info_);
        {
            auto* info = reinterpret_cast<FadeInfo*>(p);

            return info->nega_radius;
        }
    }

    //! @brief 移動スピードを設定
    //! @param s 移動速度
    //! @return 移動速度
    float SetSpeed(float s) { return sp_ = s; }

    //! @brief HP取得
    //! @return 体力
    float GetHp() const { return hp_; }

    //! @brief スキル使用状況の取得
    //! @return スキルフラグ
    bool IsUseSkill() const { return is_use_skill_; }

    //! @brief 生死の取得
    //! @return 生死フラグ
    bool IsDead() const { return is_dead_; }

    //! @brief 所持弾数セット
    //! @param n 持たせたい弾数
    //! @return 弾薬所持数
    int SetBulletNum(int n) { return have_bullet_num_ = n; }

    //! @brief フラグの状態変化
    //! @detail マウスの右ボタンを押している間is_push_mouse_right_をtrueに 押していないと falseに
    void PushMouseRight();

    //! @brief フラグの取得
    //! @detail is_push_mouse_right_を取得
    //! @return マウス右ボタン押下フラグ
    bool GetIsPushMouseR() const;

    //! @brief チュートリアル状態にするか
    //! @param flag on / off のフラグ
    //! @return チュートリアルフラグ
    bool SetTutorial(bool flag) { return is_tutorial_ = flag; }

    //! @brief 弾数を追加
    void GetAmmo();

    //! @brief 銃のリロード
    //! @detail マガジン内の弾を保持したままリロードする。予備弾薬が無い、マガジン容量がMAXなら実行できない
    void MagReload();

    //! @brief プレイヤーのダメージ
    //! @param damage ダメージ量
    void Damage(float damage);

    //! @brief フラグの取得
    //! @detail is_reloading_を取得
    //! @return リロード中か判定フラグ
    bool GetIsReloading() const;

    //! @brief フラグの取得
    //! @detail is_kicking_を取得
    //! @return キック中か判定フラグ
    bool GetIsKicking() const;

    //! @brief フラグをセット
    //! @detail is_kicking_を設定
    //! @param flag on / off のフラグ
    void SetIsKicking(bool flag);

    //! @brief フラグをセット
    //! @detail use_skill_を設定 デフォルトはtrue
    //! @param flag on / off のフラグ
    void SetIsUseSkill(bool flag = true);

    //! @brief フラグをセット
    //! @detail is_clearを設定 デフォルトはtrue
    //! @param flag on / off のフラグ
    void SetIsClear(bool flag = false);

    //! @brief フラグの取得
    //! @detail is_clearを取得
    //! @return クリアしたかの判定フラグ
    bool GetIsClear() const;

private:
    //! @brief HPゲージの描画
    void DrawHPGauge() const;

    //! @brief スキルゲージの描画
    void DrawSkillGauge() const;

    //! @brief 所持、マガジン弾数表示
    void DrawBulletInfo() const;

    //! @brief カメラの向いている方向に前進
    //! @param mat 各ベクトルを保存する
    void MoveCameraFront(matrix& mat);

    //! @brief WASDで移動
    //! @param mat 各ベクトルを保存する
    //! @param move ベクトル保存変数
    void MoveWASD(matrix& mat, float3& move);

    //! @brief 白黒、色反転の半径を設定
    void UseGrayNegaR();

    //! @brief モデルを進行方向に向ける
    //! @param move ベクトル保存変数
    void RotatoMoveDir(float3& move);

    //! @brief ボリュームを設定しSEを再生
    //! @param se_handle seのハンドル
    //! @param volume ボリューム
    void PlaySE(int se_handle, int volume = 255) const;

    //! @brief リロード
    //! @detail 弾を装填、アニメーションを変更
    void Reload(float3& move);

    //! @brief ショット
    //! @detail 左マウスボタンで弾を作成
    void Shot();

    //! @brief 移動処理
    //! @param mat 各ベクトルを保存する
    //! @param move ベクトル保存変数
    //! @detail プレイヤーを移動させる、アニメーションを変更
    void Move(float3& move, float delta);

    //! @brief キック
    //! @detail PlayerKickを作成
    void Kick();

    //! @brief スキル
    //! @detail スキル発動後、スキル容量を減らし、0になると強制解除もしくは任意解除
    void Skill();

    //! @brief 死亡
    //! @detail アニメーションを変更し、終わるとそこで固定
    void Dead();
};