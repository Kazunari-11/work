//---------------------------------------------------------------------------
//! @file   ComponentEffect.h
//! @brief  エフェクトコンポーネント
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Object.h>

#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

USING_PTR(ComponentEffect);

//! @brief モデルコンポーネントクラス
class ComponentEffect final
    : public Component
    , public IMatrix<ComponentEffect>
{
public:
    BP_COMPONENT_TYPE(ComponentEffect, Component);

    ComponentEffect() { component_effect_count++; }

    ~ComponentEffect()
    {
        component_effect_count--;
        if(component_effect_count == 0) {
            // TODO リソースを削除する
        }
    }

    void Construct(ObjectPtr owner) { __super::Construct(owner); }

    void Construct(ObjectPtr owner, std::string_view path)
    {
        __super::Construct(owner);
        Load(path);
    }

    virtual void Update(float delta) override;   //!< 更新
    virtual void Draw() override;                //!< 描画
    virtual void Exit() override;                //!< 終了
    virtual void GUI() override;                 //!< GUI

    //------------------------------------------------------------------------
    // @name エフェクト操作
    //------------------------------------------------------------------------
    //@{

    //! @brief モデルロード
    //! @param path モデル名
    void Load(std::string_view path);

    //! @brief 再生
    //! @param name エフェクトファイル名
    //! @param loop ループするかどうか
    void Play(bool loop = false);

    //! @brief 再生スピードの設定
    //! @param speed 再生スピード
    void SetPlaySpeed(float speed);

    //! @brief 再生スピードの取得
    //! @return 再生スピード
    float GetPlaySpeed();

    //! @brief 再生中かどうか
    //! @return 再生中か
    bool IsPlaying();

    //! @brief ポーズ処理と解除処理
    //! @param active ポーズするかどうか
    void PlayPause(bool pause = true);

    //! @brief ポーズ中かどうか
    //! @retval true : ポーズ中
    bool IsPaused();

    //! @brief 利用可能かどうか
    //! @retval true : 利用可能
    bool IsValid();

    //! @brief 再生アニメーション名
    //! @return アニメーション名
    const std::string_view GetEffectName();

    //! @brief 再生経過時間の取得
    //! @return 再生経過時間
    const float GetEffectTime();

    //@}

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct EffectBit : u64
    {
        Initialized,         //!< 初期化済み
        ErrorFileNotFound,   //!< ファイル読み込みエラー
        Playing,             //!< エフェクト再生中
        Paused,              //!< エフェクトポーズ中
        Loop,                //!< ループ再生指定
    };

    bool IsValid() const { return effect_status_.is(EffectBit::Initialized); }   //!< モデルが読み込まれているか?

    //---------------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //---------------------------------------------------------------------------
    //@{

    matrix& Matrix() override { return effect_transform_; }   //!< マトリクス取得

    const matrix& GetMatrix() const override { return effect_transform_; }   //!< マトリクス取得

    ComponentEffectPtr SharedThis() { return std::dynamic_pointer_cast<ComponentEffect>(shared_from_this()); }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override;

    //@}

private:
    //! モデル用のトランスフォーム
    matrix effect_transform_ = matrix::scale(1.0f);

    Status<EffectBit> effect_status_;   //!< 状態
    std::string       path_{};          //!< 読み込みエフェクト名

    //! @brief エフェクト
    static std::unordered_map<std::string, int> exist_effects_resource_;

    // エフェクトハンドル(リソース)
    int effect_handle_ = -1;

    // エフェクトハンドル(再生中)
    int effect_play_handle_ = -1;

    //! エフェクト再生時間
    float effect_time_ = 0.0f;

    //! エフェクト再生スピード
    float effect_speed_ = 1.0f;

    //! 存在するエフェクトコンポーネント
    static int component_effect_count;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_),
            cereal::make_nvp("effect_transform", effect_transform_),
            cereal::make_nvp("path", path_),
            cereal::make_nvp("model_status", effect_status_.get()),
            cereal::make_nvp("exist_effects_resource", exist_effects_resource_));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));

        // エフェクトリソースは再度ロードをしなおす
        for(auto effect : exist_effects_resource_) {
            // ロードしてリソース登録する
            auto wname = HelperLib::String::ToWString(effect.first);

            exist_effects_resource_[effect.first] = LoadEffekseerEffect(wname.data());
        }

        if(!path_.empty())
            Load(path_);
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentEffect)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentEffect)
