//---------------------------------------------------------------------------
//! @file   ComponentModel.h
//! @brief  モデルコンポーネント
//---------------------------------------------------------------------------
#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Graphics/Animation.h>
#include <System/Object.h>

#include <ImGuizmo/ImGuizmo.h>
#include <DxLib.h>

USING_PTR(ComponentModel);

//! @brief モデルコンポーネントクラス
class ComponentModel
    : public Component
    , public IMatrix<ComponentModel>
{
public:
    BP_COMPONENT_TYPE(ComponentModel, Component);

    ComponentModel()
    {
        // シェーダー利用を標準とします
        model_status_.on(ModelBit::UseShader);

        model_ = std::make_shared<Model>();
    }

    void Construct(ObjectPtr owner) { __super::Construct(owner); }

    void Construct(ObjectPtr owner, std::string_view path)
    {
        __super::Construct(owner);

        // シェーダー利用を標準とします
        model_status_.on(ModelBit::UseShader);

        model_ = std::make_shared<Model>();
        Load(path);
    }

    //! @brief モデルロード
    //! @param path モデル名
    void Load(std::string_view path);

    virtual void Update(float delta) override;   //!< 更新
    virtual void Draw() override;                //!< 描画
    virtual void Exit() override;                //!< 終了
    virtual void GUI() override;                 //!< GUI

    ComponentModelPtr setOverrideShader(const std::shared_ptr<ShaderVs> vs, std::shared_ptr<ShaderPs> ps)
    {
        overrided_shader_ps_ = ps;
        return std::dynamic_pointer_cast<ComponentModel>(shared_from_this());
    }

    //------------------------------------------------------------------------
    // @name アニメーション
    //------------------------------------------------------------------------
    //@{

    //! @brief アニメーションデータ
    //! @param anims アニメーション構造リスト
    ComponentModelPtr SetAnimation(const std::vector<Animation::Desc> anims);

    //! @brief アニメーション再生(同じなら再再生しない)
    //! @param name 再生するアニメーション名
    //! @param loop ループするかどうか(デフォルト:しない)
    //! @param speed 補完秒数(デフォルト:0.2秒)
    //! @param start_time スタートする位置(デフォルト:0.0)
    void
    PlayAnimationNoSame(std::string_view name, bool loop = false, float blend_time = 0.2f, float start_time = 0.0f);

    //! @brief アニメーション再生
    //! @param name 再生するアニメーション名
    //! @param loop ループするかどうか(デフォルト:しない)
    //! @param speed 補完秒数(デフォルト:0.2秒)
    //! @param start_time スタートする位置(デフォルト:0.0)
    void PlayAnimation(std::string_view name, bool loop = false, float blend_time = 0.2f, float start_time = 0.0f);

    //! @brief アニメーション中かどうか
    //! @retval true : アニメーション中
    bool IsPlaying();

    //! @brief ポーズ処理と解除処理
    //! @param active ポーズするかどうか
    void PlayPause(bool active = true);

    //! @brief ポーズ中かどうか
    //! @retval true : ポーズ中
    bool IsPaused();

    //! @brief 利用可能かどうか
    //! @retval true : 利用可能
    bool IsAnimationValid();

    //! @brief 再生アニメーション名
    //! @return アニメーション名
    const std::string_view GetPlayAnimationName();

    //! @brief 再生経過時間の取得
    //! @return 再生経過時間
    const float GetAnimationTime();

    //@}

    //! @brief モデル取得
    //! @return モデル
    std::shared_ptr<Model> GetModelClass() { return model_; }

    //! @brief モデル取得
    //! @return モデル(DxLib)
    int GetModel() { return *(model_.get()); }

    //---------------------------------------------------------------------------
    //! @name ノード関係
    //---------------------------------------------------------------------------
    //@{
    std::vector<std::string_view> GetNodesName();
    std::vector<const char*>      GetNodesNamePChar();
    int                           GetNodeIndex(std::string_view name);

    float3 GetNodePosition(std::string_view name);
    float3 GetNodePosition(int no);

    matrix GetNodeMatrix(std::string_view name);
    matrix GetNodeMatrix(int no);

    //@}

    //---------------------------------------------------------------------------
    //! モデルステータス
    //---------------------------------------------------------------------------
    enum struct ModelBit : u64
    {
        SetFileAlready,      //!< ファイル設定済み
        Initialized,         //!< 初期化済み
        ErrorFileNotFound,   //!< ファイル読み込みエラー
        UseShader,           //!< シェーダーを使用する
    };

    bool IsValid() const { return model_status_.is(ModelBit::Initialized); }   //!< モデルが読み込まれているか?

    bool UseShader() const { return model_status_.is(ModelBit::UseShader); }   //!< シェーダーを利用するか?

    //---------------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //---------------------------------------------------------------------------
    //@{

    matrix& Matrix() override { return model_transform_; }   //!< マトリクス取得

    const matrix& GetMatrix() const override { return model_transform_; }   //!< マトリクス取得

    ComponentModelPtr SharedThis() { return std::dynamic_pointer_cast<ComponentModel>(shared_from_this()); }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override;

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override;

    //@}

private:
    //! モデル用のトランスフォーム
    matrix model_transform_ = matrix::scale(0.1f);

    Status<ModelBit>       model_status_;      //!< 状態
    std::string            path_{};            //!< 読み込みモデル名
    std::shared_ptr<Model> model_ = nullptr;   //!< モデルクラス

    ImGuizmo::OPERATION gizmo_operation_ = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      gizmo_mode_      = ImGuizmo::LOCAL;

    std::vector<std::string_view> nodes_name_;
    int                           select_node_index_ = 0;
    bool                          node_manipulate_   = false;

    //! @brief アニメーション
    std::shared_ptr<Animation> animation_;
    std::string                current_animation_name_;
    float                      animation_time_ = 0.0f;

    std::vector<Animation::Desc> animations_desc_;

    std::shared_ptr<ShaderPs> overrided_shader_ps_;   // 上書きするシェーダー

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(cereal::make_nvp("owner", owner_),
            cereal::make_nvp("model_transform", model_transform_),
            cereal::make_nvp("path", path_),
            cereal::make_nvp("desc", animations_desc_),
            cereal::make_nvp("model_status", model_status_.get()));

        arc(cereal::make_nvp("Component", cereal::base_class<Component>(this)));

        if(!path_.empty()) {
            Load(path_);
            if(!animations_desc_.empty()) {
                SetAnimation(animations_desc_);
            }
        }
    }

    //@}
};

//! @brief 外部Animation::Descのセーブロード
CEREAL_SAVELOAD_OTHER(Animation::Desc, arc, other)
{
    arc(CEREAL_NVP(other.name_),              //!< アニメーション名(任意)
        CEREAL_NVP(other.file_path_),         //!< ファイルパス
        CEREAL_NVP(other.animation_index_),   //!< ファイル内のアニメーション番号
        CEREAL_NVP(other.animation_speed_)    //!< アニメーションの再生速度(default:1.0f)
    );
}

CEREAL_REGISTER_TYPE(ComponentModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentModel)
