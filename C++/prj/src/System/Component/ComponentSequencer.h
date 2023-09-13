#pragma once

#include <System/Component/Component.h>
#include <System/Component/ComponentTransform.h>
#include <System/Utils/HelperLib.h>
#include <System/Cereal.h>

#include <im-neo-sequencer/imgui_neo_sequencer.h>

struct AnimObject
{
    std::string name_;
    bool        loop_;

private:
    CEREAL_SAVELOAD(arc, ver) { arc(CEREAL_NVP(name_), CEREAL_NVP(loop_)); }
};

// sequencer用のオブジェクト
class SequenceObject : public IMatrix<SequenceObject>
{
public:
    virtual matrix& Matrix() { return object_matrix_; }

    //! @brief TransformのMatrix情報を取得します
    //! @return Transform の Matrix
    virtual const matrix& GetMatrix() const { return object_matrix_; }

    virtual std::shared_ptr<SequenceObject> SharedThis() { return nullptr; }

    virtual const matrix GetWorldMatrix() const { return object_matrix_; }

    //! @brief 1フレーム前のワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const { return object_matrix_; }

public:
    std::string GetName()
    {
        // ImGuiにてname_の長さが変化するための対処
        return name_ + "##obj" + std::to_string(reinterpret_cast<uint64_t>(this));
    }

    std::string GetNameLabel() { return "NAME##name" + std::to_string(reinterpret_cast<uint64_t>(this)); }

    bool IsOpen() { return open_; }

    void   SetAnimationFromFrame(int frame);
    void   SetEffectFromFrame(int frame);
    matrix GetTransformFromFrame(int frame);
    void   ShowGuizmo();
    void   Sort();

    void Update(bool playing, float frame);

    void GUI(uint32_t start, uint32_t end);

private:
    bool open_ = true;

    bool                show_guizmo_      = false;
    ImGuizmo::OPERATION guizmo_operation_ = ImGuizmo::OPERATION::TRANSLATE;
    ImGuizmo::MODE      guizmo_mode_      = ImGuizmo::MODE::WORLD;
    int                 guizmo_index_     = -1;

    std::string           name_{};
    std::vector<uint32_t> position_keys_{};
    std::vector<uint32_t> rotation_keys_{};
    std::vector<uint32_t> scale_keys_{};

    std::vector<float3> position_values_{};
    std::vector<float3> rotation_values_{};
    std::vector<float3> scale_values_{};

    std::vector<uint32_t>   animation_keys_{};
    std::vector<AnimObject> animation_values_{};

    std::string animation_name_{};
    int         animation_frame_old_ = -1;
    int         animation_frame_     = -1;
    bool        animation_loop_      = false;
    bool        animation_change_    = false;

    std::vector<uint32_t>   effect_keys_{};
    std::vector<AnimObject> effect_values_{};

    int  effect_number_    = -1;
    int  effect_frame_old_ = -1;
    int  effect_frame_     = -1;
    bool effect_loop_      = false;
    bool effect_change_    = false;

    matrix object_matrix_{
        float4{1, 0, 0, 0},
        float4{0, 1, 0, 0},
        float4{0, 0, 1, 0},
        float4{0, 0, 0, 1},
    };

    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(open_),   //
            CEREAL_NVP(name_),
            CEREAL_NVP(position_keys_),
            CEREAL_NVP(rotation_keys_),
            CEREAL_NVP(scale_keys_),
            CEREAL_NVP(position_values_),
            CEREAL_NVP(rotation_values_),
            CEREAL_NVP(scale_values_),
            CEREAL_NVP(animation_keys_),
            CEREAL_NVP(animation_values_),
            CEREAL_NVP(scale_values_),
            CEREAL_NVP(scale_values_),
            CEREAL_NVP(animation_name_),
            CEREAL_NVP(animation_frame_old_),
            CEREAL_NVP(animation_frame_),
            CEREAL_NVP(animation_change_),
            CEREAL_NVP(animation_loop_),
            CEREAL_NVP(effect_number_),
            CEREAL_NVP(effect_frame_old_),
            CEREAL_NVP(effect_frame_),
            CEREAL_NVP(effect_change_),
            CEREAL_NVP(effect_loop_),
            CEREAL_NVP(object_matrix_));
    }
};

CEREAL_REGISTER_TYPE(SequenceObject)

USING_PTR(ComponentSequencer);

class ComponentSequencer : public Component
{
    friend class Object;

public:
    BP_COMPONENT_TYPE(ComponentSequencer, Component);

    ComponentSequencer() {}

    virtual void Init() override;                //!< 初期化
    virtual void Update(float delta) override;   //!< 更新
    virtual void PostUpdate() override;          //!< 更新
    virtual void Draw() override;                //!< デバッグ描画
    virtual void Exit() override;                //!< 終了処理
    virtual void GUI() override;                 //!< GUI処理

    //! 再生
    void Play(int start_frame = 0, bool loop = false);

    //! ストップ
    void Stop();

    //! プレイ中か?
    bool IsPlaying() const;

    //---------------------------------------------------------------------------
    //! ステータス
    //---------------------------------------------------------------------------
    enum struct SequencerBit : u32
    {
        Initialized,   //!< 初期化済み
        Playing,       //!< プレイ中
        Loop,          //!< ループ指定
    };

    void SetSequencerStatus(SequencerBit bit, bool on) { sequencer_status_.set(bit, on); }

    bool GetSequencerStatus(SequencerBit bit) const { return sequencer_status_.is(bit); }

#if 0
	uint32_t GetStartFrame()
	{
		return start_frame_;
	}
	void SetStartFrame( uint32_t start )
	{
		start_frame_ = start;
	}
	uint32_t GetEndFrame()
	{
		return start_frame_;
	}
	void SetEndFrame( uint32_t end )
	{
		end_frame_ = end;
	}
	float GetCurrentFrame()
	{
		return current_frame_;
	}
	void SetCurrentFrame( float current )
	{
		current_frame_ = current;
	}
#endif

private:
    Status<SequencerBit> sequencer_status_;   //!< 状態

    std::vector<std::shared_ptr<SequenceObject>> objects_;

    uint32_t start_frame_   = 0;
    uint32_t end_frame_     = 60 * 5;
    float    current_frame_ = 0;

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
        arc(cereal::make_nvp("owner", owner_));   //< オーナー
        arc(cereal::make_nvp("sequencer_status", sequencer_status_.get()));
        arc(CEREAL_NVP(start_frame_), CEREAL_NVP(end_frame_), CEREAL_NVP(current_frame_));
        arc(CEREAL_NVP(objects_));
    }

    //@}
};

CEREAL_REGISTER_TYPE(ComponentSequencer)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, ComponentSequencer)
