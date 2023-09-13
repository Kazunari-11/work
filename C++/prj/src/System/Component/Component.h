//---------------------------------------------------------------------------
//! @file   Component.h
//! @brief  コンポーネント ベースクラス
//---------------------------------------------------------------------------
#pragma once

#include <System/Priority.h>
#include <System/ProcTiming.h>
#include <System/Status.h>

#include <functional>
#include <sigslot/signal.hpp>
#include <vector>
#include <cassert>

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

#include <xtr1common>

// ポインター宣言
USING_PTR(Component);
USING_PTR(Object);

//
//===========================================================================
//! クラス用型情報 ClassComponentType<T>
//===========================================================================
template <class T>
class ClassComponentType : public Type
{
public:
    using Type::Type;   // 継承コンストラクタ

    virtual void* createComponentPtr(ObjectPtr obj)
    {
        std::shared_ptr<T> cmp = std::make_shared<T>();

        // すでに存在している場合はnullptrを返す
        Component::RegisterToObject(cmp, obj);

        tmp = cmp;
        return reinterpret_cast<void*>(&tmp);
    }

    std::weak_ptr<T> tmp;
};

//---------------------------------------------------------------------------
//! Componentクラス内ヘッダー宣言 基底クラス用
//! publicで BP_COMPONENT_BASE_TYPE(クラス名) で宣言します
//!
//! @code
//!     class A
//!     {
//!     public:
//!         BP_COMPONENT_BASE_TYPE(A)
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_BASE_TYPE(CLASS)                                                                                  \
    using MyClass = CLASS; /*! 自クラスの型 */                                                                   \
                                                                                                                       \
    /*! 型情報 */                                                                                                   \
    static ClassComponentType<CLASS> TypeInfo;                                                                         \
                                                                                                                       \
    /*! 型情報を取得 */                                                                                          \
    virtual const Type* typeInfo() const                                                                               \
    {                                                                                                                  \
        return &TypeInfo;                                                                                              \
    }

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_COMPONENT_TYPE(クラス名, 親クラス名) で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_COMPONENT_TYPE(B, A);
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_TYPE(CLASS, PARENT_CLASS)                                                                         \
    using Super = PARENT_CLASS; /* 親クラスの型 */                                                               \
    BP_COMPONENT_BASE_TYPE(CLASS);

//---------------------------------------------------------------------------
//! クラス内cpp宣言
//! グローバル変数で BP_OBJECT_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_OBJECT_IMPL(B, u8"クラスB")
//! @endcode
//---------------------------------------------------------------------------
#define BP_COMPONENT_BASE_IMPL(CLASS, DESC_NAME)                                                                       \
    /*! 型情報の実体 */                                                                                          \
    ClassComponentType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, nullptr);

#define BP_COMPONENT_IMPL(CLASS, DESC_NAME)                                                                            \
    /*! 型情報の実体 */                                                                                          \
    ClassComponentType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, &Super::TypeInfo);

//---------------------------------------------------------------------------
//! @brief コンポーネント
class Component : public std::enable_shared_from_this<Component>
{
    friend class Object;
    friend class Scene;

public:
    BP_COMPONENT_BASE_TYPE(Component);

    //	Component( const Component& )			 = delete;
    //	Component& operator=( const Component& ) = delete;

    virtual ~Component()
    {
        for(auto& t : update_timings_) {
            auto& p = t.second;
            if(p.connect_.valid())
                p.connect_.disconnect();

            p.proc_ = nullptr;
        }
        update_timings_.clear();

        for(auto& t : proc_timings_) {
            auto& p = t.second;
            if(p.connect_.valid())
                p.connect_.disconnect();

            p.proc_ = nullptr;
        }
        proc_timings_.clear();
    }

    Object*         GetOwner();            //!< オーナー(従属しているオブジェクト)の取得
    const Object*   GetOwner() const;      //!< オーナー(従属しているオブジェクト)の取得
    ObjectPtr       GetOwnerPtr();         //!< オーナー(従属しているオブジェクト)の取得(SharedPtr)
    const ObjectPtr GetOwnerPtr() const;   //!< オーナー(従属しているオブジェクト)の取得(SharedPtr)

    virtual void Init();                         //!< 初期化
    virtual void Update(float delta_time);       //!< アップデート
    virtual void LateUpdate(float delta_time);   //!< 遅いアップデート
    virtual void Draw();                         //!< 描画
    virtual void LateDraw();                     //!< 遅い描画
    virtual void Exit();                         //!< 終了
    virtual void GUI();                          //!< GUI表示

    virtual void PreUpdate();       //!< 更新前処理
    virtual void PostUpdate();      //!< 更新後処理
    virtual void PreDraw();         //!< 描画前処理
    virtual void PostDraw();        //!< 描画後処理
    virtual void PrePhysics();      //!< Physics前処理
    virtual void InitSerialize();   //!< シリアライズでもどらないユーザー処理関数などを設定

    void SetPriority(ProcTiming timing, Priority priority);

    //----------------------------------------------------------
    //! @name  コンポーネントオブジェクト登録メソッド
    //----------------------------------------------------------
    //@{
    static void RegisterToObject(ComponentPtr cmp, ObjectPtr obj);

    //! 処理を取得
    template <class T>
    SlotProc<T>& GetProc(std::string proc_name, ProcTiming timing)
    {
        if constexpr(std::is_same<T, float>{}) {
            auto itr = update_timings_.find(proc_name);
            if(itr != update_timings_.end())
                return itr->second;

            update_timings_[proc_name]         = SlotProc<T>();
            update_timings_[proc_name].name_   = proc_name;
            update_timings_[proc_name].timing_ = timing;
            return update_timings_[proc_name];
        }
        else {
            auto itr = proc_timings_.find(proc_name);
            if(itr != proc_timings_.end())
                return itr->second;

            proc_timings_[proc_name]         = SlotProc<T>();
            proc_timings_[proc_name].name_   = proc_name;
            proc_timings_[proc_name].timing_ = timing;
            return proc_timings_[proc_name];
        }
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, std::nullptr_t> = nullptr>
    SlotProc<T>& SetProc(std::string            proc_name,
                         std::function<void(T)> func,
                         ProcTiming             timing = ProcTiming::Update,
                         Priority               prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<T>(proc_name, timing);
        proc.SetProc(proc_name, timing, prio, func);
        return proc;
    }

    template <typename T, std::enable_if_t<std::is_void_v<T>, std::nullptr_t> = nullptr>
    SlotProc<T>& SetProc(std::string            proc_name,
                         std::function<void(T)> func,
                         ProcTiming             timing = ProcTiming::Update,
                         Priority               prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<T>(proc_name, timing);
        proc.SetProc(proc_name, timing, prio, func);
        return proc;
    }

    SlotProc<void>& SetProc(std::string           proc_name,
                            std::function<void()> func,
                            ProcTiming            timing = ProcTiming::Draw,
                            Priority              prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<void>(proc_name, timing);
        proc.SetProc(proc_name, timing, prio, func);
        return proc;
    }

    SlotProc<float>& SetProc(std::string                proc_name,
                             std::function<void(float)> func,
                             ProcTiming                 timing = ProcTiming::Update,
                             Priority                   prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<float>(proc_name, timing);
        proc.SetProc(proc_name, timing, prio, func);
        return proc;
    }

    void ResetProc(std::string proc_name)
    {
        {
            auto itr = update_timings_.find(proc_name);
            if(itr != update_timings_.end()) {
                auto& proc = itr->second;
                if(proc.connect_.valid())
                    proc.connect_.disconnect();

                proc.ResetDirty();
                return;
            }
        }
        {
            auto itr = proc_timings_.find(proc_name);
            if(itr != proc_timings_.end()) {
                auto& proc = itr->second;
                if(proc.connect_.valid())
                    proc.connect_.disconnect();

                proc.ResetDirty();
            }
        }
    }

    enum struct StatusBit : u64
    {
        Alive = 0,      //!< 生存状態
        ChangePrio,     //!< プライオリティの変更中
        ShowGUI,        //!< GUI表示中
        Initialized,    //!< 初期化終了
        NoUpdate,       //!< Updateしない
        NoDraw,         //!< Drawしない
        DisablePause,   //!< ポーズ不可
        IsPause,        //!< ポーズ中
        SameType,       //!< 同じタイプのコンポーネント可能
        Exited,         //!< 正しく終了が呼ばれている
        Serialized,     //!< シリアライズ済み.
    };

    void SetStatus(StatusBit b, bool on);   //!< ステータスの設定
    bool GetStatus(StatusBit b);            //!< ステータスの取得

#if 0
    virtual void ContactComponents( ComponentPtr other );
    void ContactCallBack();
#endif

    Component();
    void Construct(ObjectPtr owner);

protected:
    ObjectPtr        owner_ = nullptr;   //!< オーナー
    SlotProcs<float> update_timings_;    //!< 登録処理(update)
    SlotProcs<void>  proc_timings_;      //!< 登録処理

    float update_delta_time_ = 0.0f;   //!< update以外で使用できるように

private:
    Status<StatusBit> status_;   //!< コンポーネント状態

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{

    //! @brief セーブロード
    //! @param arc アーカイバ
    //! @param ver バージョン
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(owner_),            //< オーナー
            CEREAL_NVP(update_timings_),   //< アップデートタイミング
            CEREAL_NVP(proc_timings_),     //< プロセスタイミング
            CEREAL_NVP(status_.get())      //< ステータス
        );
    }

    //@}
};
