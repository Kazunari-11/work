//---------------------------------------------------------------------------
//! @file   Object.h
//! @brief  オブジェクト ベースクラス
//---------------------------------------------------------------------------
#pragma once

#include "Priority.h"
#include "Status.h"

#include <System/Component/Component.h>
#include <System/Component/ComponentCollision.h>
#include <System/Component/ComponentTransform.h>

#include <System/Utils/HelperLib.h>
#include <System/Cereal.h>

#include <string>
#include <memory>

//---------------------------------------------------------------------------
// ポインター宣言
//---------------------------------------------------------------------------

USING_PTR(Object);
USING_PTR(Component);

//
//===========================================================================
//! クラス用型情報 ClassObjectType<T>
//===========================================================================
template <class T>
class ClassObjectType : public Type
{
public:
    using Type::Type;   // 継承コンストラクタ

    virtual void* createObjectPtr()
    {
        std::shared_ptr<T> tmp = std::make_shared<T>();
        Object::RegisterCurrentScene(tmp);

        obj = tmp;
        return reinterpret_cast<void*>(&obj);
    }

    std::weak_ptr<T> obj;
};

//---------------------------------------------------------------------------
//! Objectクラス内ヘッダー宣言 基底クラス用
//! publicで BP_BASE_TYPE(クラス名) で宣言します
//!
//! @code
//!     class A
//!     {
//!     public:
//!         BP_OBJECT_BASE_TYPE(A)
//! @endcode
//---------------------------------------------------------------------------
#define BP_OBJECT_BASE_TYPE(CLASS)                                                                                     \
    using MyClass = CLASS; /*! 自クラスの型 */                                                                   \
                                                                                                                       \
    /*! 型情報 */                                                                                                   \
    static ClassObjectType<CLASS> TypeInfo;                                                                            \
                                                                                                                       \
    /*! 型情報を取得 */                                                                                          \
    virtual const Type* typeInfo() const                                                                               \
    {                                                                                                                  \
        return &TypeInfo;                                                                                              \
    }

//	/*! インスタンスを作成(クラスをnewしてポインタを返す)*/ \
//	static std::shared_ptr<CLASS> createObjectPtr();

//---------------------------------------------------------------------------
//! クラス内ヘッダー宣言
//! publicで BP_BASE_TYPE(クラス名, 親クラス名) で宣言します
//!
//! @code
//!     class B : public A
//!     {
//!     public:
//!         BP_OBJECT_TYPE(B, A);
//! @endcode
//---------------------------------------------------------------------------
#define BP_OBJECT_TYPE(CLASS, PARENT_CLASS)                                                                            \
    using Super = PARENT_CLASS; /* 親クラスの型 */                                                               \
    BP_OBJECT_BASE_TYPE(CLASS);

//---------------------------------------------------------------------------
//! クラス内cpp宣言
//! グローバル変数で BP_OBJECT_IMPL(クラス名, u8任意の名前文字列) で宣言します
//! @code
//! BP_OBJECT_IMPL(B, u8"クラスB")
//! @endcode
//---------------------------------------------------------------------------
#define BP_OBJECT_BASE_IMPL(CLASS, DESC_NAME)                                                                          \
    /*! 型情報の実体 */                                                                                          \
    ClassObjectType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, nullptr);

#define BP_OBJECT_IMPL(CLASS, DESC_NAME)                                                                               \
    /*! 型情報の実体 */                                                                                          \
    ClassObjectType<CLASS> CLASS::TypeInfo(#CLASS, sizeof(CLASS), DESC_NAME, &Super::TypeInfo);

//---------------------------------------------------------------------------
//! オブジェクトクラス
//---------------------------------------------------------------------------
class Object
    : public std::enable_shared_from_this<Object>
    , public IMatrix<Object>
{
    friend class Scene;

public:
    BP_OBJECT_BASE_TYPE(Object);

    Object();            //!< コンストラクタ
    virtual ~Object();   //!< デストラクタ

    virtual bool Init();                    //!< 初期化
    virtual void Update(float delta);       //!< 更新
    virtual void LateUpdate(float delta);   //!< 遅い更新
    virtual void Draw();                    //!< 描画
    virtual void LateDraw();                //!< 遅い描画
    virtual void Exit();                    //!< 終了
    virtual void GUI();                     //!< GUI表示

    virtual void PreUpdate();    //!< 更新前処理
    virtual void PostUpdate();   //!< 更新後処理
    virtual void PreDraw();      //!< 描画前処理
    virtual void PostDraw();     //!< 描画後処理
    virtual void PrePhysics();   //!< 物理シミュレーション前処理

    virtual void InitSerialize();   //!< シリアライズでもどらないユーザー処理関数などを設定

    void UseWarp();
    //----------------------------------------------------------
    //! @name  オブジェクトシーン登録メソッド
    //----------------------------------------------------------
    //@{
    static void RegisterCurrentScene(ObjectPtr obj);

    //@}
    //----------------------------------------------------------
    //! @name  オブジェクト名
    //----------------------------------------------------------
    //@{

    //! 名前の設定
    auto SetName(const std::string& name, bool use_construct = false)
    {
        name_         = setUniqueName(name);
        name_default_ = name;

        if(!use_construct)
            return shared_from_this();

        return std::shared_ptr<Object>(nullptr);
    }

    std::string_view GetName() const;          //!< 名前の取得
    std::string_view GetNameDefault() const;   //!< 名前の取得

    //@}
    //----------------------------------------------------------
    //! @name  オブジェクトステータス
    //----------------------------------------------------------
    //@{

    //! オブジェクトステータスビット
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
        Exited,         //!< 終了呼び出し済み.
        Serialized,     //!< シリアライズ済み.
        CalledGUI,      //!< GUIが正しく呼ばれた.
        Located,        //!< 配置されている.
    };

    void SetStatus(StatusBit b, bool on);   //!< ステータスの設定
    bool GetStatus(StatusBit b);            //!< ステータスの取得

    //! 存在するオブジェクト数
    static size_t ExistObjectCount();

    //@}
    //--------------------------------------------------------------------
    //! @name コンポーネントに対する命令
    //--------------------------------------------------------------------
    //@{

    //! コンポーネント追加
    //! @tparam [in] class T コンポーネントタイプ
    //! @tparam [in] Args コンポーネント初期化パラメータ
    //! @return 追加されたコンポーネント
    template <class T, class... Args>
    std::shared_ptr<T> AddComponent(Args... args);

    //! コンポーネント取得
    //! @tparam T コンポーネントタイプ
    //! @return 追加されたコンポーネント
    template <class T>
    std::shared_ptr<T> GetComponent();

    //! コンポーネント取得
    //! @tparam T コンポーネントタイプ
    //! @return 追加されたコンポーネント
    template <class T>
    std::shared_ptr<T> GetComponent() const;

    template <class T>
    std::vector<std::shared_ptr<T>> GetComponents();

    template <class T>
    std::vector<std::shared_ptr<T>> GetComponents() const;

    //! コンポーネント削除
    //! @tparam [in] class T コンポーネントタイプ
    template <class T>
    void RemoveComponent();

    //! コンポーネント削除
    //! @param [in] component 削除するコンポーネント
    void RemoveComponent(ComponentPtr component);

    //! コンポーネントをすべて削除する
    void RemoveAllComponents();

    //! 全ての処理を削除する
    void RemoveAllProcesses();

    //----------------------------------------------------------------
    // @name コンポーネントコールバック関係
    //----------------------------------------------------------------
    //@{

    //! @brief コンポーネントのヒットコールバック
    //! @param hitInfo ヒット情報
    virtual void OnHit([[maybe_unused]] const ComponentCollision::HitInfo& hitInfo)
    {
        if(auto cmp = GetComponent<ComponentTransform>()) {
            cmp->AddTranslate(hitInfo.push_);
        }
        // 地面に当たっている時
        // @todo 重力加速も初期化する
#pragma warning(disable : 26813)
        // ここは&で参照すべき所ではない
        if(hitInfo.hit_collision_->GetCollisionGroup() == ComponentCollision::CollisionGroup::GROUND) {
            hitInfo.collision_->SetCollisionStatus(ComponentCollision::CollisionBit::IsGround, true);
        }
#pragma warning(default : 26813)
    }

    //@}
    //----------------------------------------------------------------
    // @name 処理優先関係
    //----------------------------------------------------------------
    //@{
    template <class T>
    SlotProc<T>& GetProc(const std::string& proc_name, ProcTiming timing)
    {
        if constexpr(std::is_same<T, float>{}) {
            auto itr = update_timings_.find(proc_name);
            if(itr != update_timings_.end())
                return itr->second;

            update_timings_[proc_name]         = SlotProc<T>();
            update_timings_[proc_name].name_   = proc_name;
            update_timings_[proc_name].timing_ = timing;
            update_timings_[proc_name].dirty_  = true;
            return update_timings_[proc_name];
        }
        else {
            auto itr = proc_timings_.find(proc_name);
            if(itr != proc_timings_.end())
                return itr->second;

            proc_timings_[proc_name]         = SlotProc<T>();
            proc_timings_[proc_name].name_   = proc_name;
            proc_timings_[proc_name].timing_ = timing;
            proc_timings_[proc_name].dirty_  = true;
            return proc_timings_[proc_name];
        }
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, std::nullptr_t> = nullptr>
    SlotProc<T>& SetProc(const std::string& proc_name, std::function<void(T)> func, ProcTiming timing, Priority prio)
    {
        auto& proc = GetProc<T>(proc_name, timing);
        if(proc_name != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() || proc.IsDirty()) {
            proc.SetProc(proc_name, timing, prio, func);
        }
        return proc;
    }

    template <typename T, std::enable_if_t<std::is_void_v<T>, std::nullptr_t> = nullptr>
    SlotProc<T>& SetProc(const std::string&     proc_name,
                         std::function<void(T)> func,
                         ProcTiming             timing = ProcTiming::Update,
                         Priority               prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<T>(proc_name, timing);
        if(proc_name != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() || proc.IsDirty()) {
            proc.SetProc(proc_name, timing, prio, func);
        }
        return proc;
    }

    /**
     * @brief           プロセス設定
     * @param proc_name プロセス名
     * @param func      Update処理
     * @param timing    タイミング
     * @param prio      処理優先
     * @return          プロセス
    */
    SlotProc<void>& SetProc(const std::string& proc_name,
                            //std::function<void()> func,
                            std::function<void()> func,
                            ProcTiming            timing = ProcTiming::Draw,
                            Priority              prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<void>(proc_name, timing);
        if(proc_name != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() || proc.IsDirty()) {
            proc.SetProc(proc_name, timing, prio, func);
        }
        return proc;
    }

    /**
     * @brief           プロセス設定
     * @param proc_name プロセス名
     * @param func      処理
     * @param timing    タイミング
     * @param prio      処理優先
     * @return          プロセス
    */
    SlotProc<float>& SetProc(const std::string&         proc_name,
                             std::function<void(float)> func,
                             ProcTiming                 timing = ProcTiming::Update,
                             Priority                   prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<float>(proc_name, timing);
        if(proc_name != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() || proc.IsDirty()) {
            proc.SetProc(proc_name, timing, prio, func);
        }
        return proc;
    }

#if 0
	template<class T>
	SlotProc<T>&
	SetProc( std::shared_ptr<Callable<T>> func, ProcTiming timing = ProcTiming::Update, Priority prio = Priority::NORMAL )
	{
		auto& proc = GetProc<T>( func->GetName(), timing );
		if( func->GetName() != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() || proc.IsDirty() )
		{
			proc.SetProc( func, timing, prio );
		}
		return proc;
	}
#endif

#if 1
    SlotProc<float>& SetProc(std::shared_ptr<Callable<float>> func,
                             ProcTiming                       timing = ProcTiming::Update,
                             Priority                         prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<float>(func->GetName(), timing);
        if(func->GetName() != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() ||
           proc.IsDirty()) {
            proc.SetProc(func, timing, prio);
        }
        return proc;
    }

    SlotProc<void>& SetProc(std::shared_ptr<Callable<void>> func,
                            ProcTiming                      timing = ProcTiming::Draw,
                            Priority                        prio   = Priority::NORMAL)
    {
        auto& proc = GetProc<void>(func->GetName(), timing);
        if(func->GetName() != proc.GetName() || timing != proc.GetTiming() || prio != proc.GetPriority() ||
           proc.IsDirty()) {
            proc.SetProc(func, timing, prio);
        }
        return proc;
    }
#endif

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

    //@}

    //----------------------------------------------------------------------
    //! @name IMatrixインターフェースの利用するための定義
    //----------------------------------------------------------------------
    //@{
    //! @brief TransformのMatrix情報を取得します
    //! @return ComponentTransform の Matrix
    matrix& Matrix() override;

    const matrix& GetMatrix() const override;

    ObjectPtr SharedThis()
    {
        return shared_from_this();
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetWorldMatrix() const override
    {
        return GetMatrix();
    }

    //! @brief ワールドMatrixの取得
    //! @return 他のコンポーネントも含めた位置
    virtual const matrix GetOldWorldMatrix() const override;

    //@}
    //----------------------------------------------------------------------
    //! @name システム処理系
    //----------------------------------------------------------------------
    //@{

    //! 全コンポーネントの取得
    //! @return コンポーネントそのものを受け取る
    //! @attention auto& で受け取ってください。(autoで受け取ると別物になります【C++17】)
    ComponentPtrVec& GetComponents()
    {
        return components_;
    }

    //! 使用していないコンポーネントの削除
    void ModifyComponents();

    //! 使用している名前リストを消去する
    //! ステージ移行などでオブジェクトが消去されたときに呼ぶ
    static void ClearObjectNames();

    //@}

    void SetGravity(float3 g)
    {
        // Physicsにて影響を受けるgravity
        // JOLTでは使用しない
        gravity_ = g;
    }

    int GetVersion();

    virtual bool Save(std::string_view filename = "");

    virtual bool Load(std::string_view filename = "");
    //@}

protected:
    std::string       name_{};           //!< オブジェクト名
    std::string       name_default_{};   //!< 番号なしのオブジェクト名
    Status<StatusBit> status_{};         //!< ステータス
    ComponentPtrVec   components_;       //!< コンポーネント
    SlotProcs<float>  update_timings_;   //!< 登録処理
    SlotProcs<void>   proc_timings_;     //!< 登録処理
    float3            gravity_;

    // コンポーネントリークチェック用
    ComponentWeakPtrVec leak_components_;

    std::string setUniqueName(const std::string& name);

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        arc(CEREAL_NVP(name_));
        arc(CEREAL_NVP(name_default_),
            CEREAL_NVP(status_.get()),
            CEREAL_NVP(update_timings_),
            CEREAL_NVP(proc_timings_));

        arc(CEREAL_NVP(components_));
        if(ver >= 1)
            arc(CEREAL_NVP(gravity_));

        status_.off(StatusBit::Serialized);
    }

    //@}
};

//---------------------------------------------------------------------------
// Object Cerealバージョン
//---------------------------------------------------------------------------
CEREAL_CLASS_VERSION(Object, 1);

template <class T, class... Args>
std::shared_ptr<T> Object::AddComponent(Args... args)
{
    assert(this != nullptr &&
           "実行しているオブジェクト(this)"
           "がありません。「再試行」をおして、「呼び出し履歴」からどこでemptyになったのかを確認してください。");

    // 同じタイプを許容しない場合
    // 同じものが存在する場合はエラーとする
    auto cmp = GetComponent<T>();
    if(cmp != nullptr) {
        if(!cmp->status_.is(Component::StatusBit::SameType))
            assert(!"このComponentは同じタイプを許容しません");
    }

    auto ptr = new T();

    // ここでエラーが出る場合はコンポーネントクラスに次の関数を作成します
    // void Construct( ObjectPtr owner )
    ptr->Construct(shared_from_this(), std::forward<Args>(args)...);

    std::shared_ptr<T> component = std::shared_ptr<T>(ptr);
    // std::shared_ptr<T> comp = std::make_shared<T>(shared_from_this(), std::forward<Args>(args)...);
    // comp->Init();
    components_.push_back(component);

    return component;
}

//! @brief コンポーネント取得
//! @tparam T コンポーネントタイプ
//! @return 追加されたコンポーネント
template <class T>
std::shared_ptr<T> Object::GetComponent()
{
    assert(this != nullptr &&
           "実行しているオブジェクト(this)"
           "がありません。「再試行」をおして、「呼び出し履歴」からどこでemptyになったのかを確認してください。");

    for(auto& component : components_) {
        auto cast = std::dynamic_pointer_cast<T>(component);
        if(cast)
            return cast;
    }

    return nullptr;
}

//! @brief コンポーネント取得
//! @tparam T コンポーネントタイプ
//! @return 追加されたコンポーネント
template <class T>
std::shared_ptr<T> Object::GetComponent() const
{
    assert(this != nullptr &&
           "実行しているオブジェクト(this)"
           "がありません。「再試行」をおして、「呼び出し履歴」からどこでemptyになったのかを確認してください。");

    for(auto& component : components_) {
        auto cast = std::dynamic_pointer_cast<T>(component);
        if(cast)
            return cast;
    }

    return nullptr;
}

template <class T>
std::vector<std::shared_ptr<T>> Object::GetComponents()
{
    assert(this != nullptr &&
           "実行しているオブジェクト(this)"
           "がありません。「再試行」をおして、「呼び出し履歴」からどこでemptyになったのかを確認してください。");

    std::vector<std::shared_ptr<T>> cmps;

    for(auto& component : components_) {
        auto cmp = std::dynamic_pointer_cast<T>(component);
        if(cmp)
            cmps.push_back(cmp);
    }

    return cmps;
}

template <class T>
std::vector<std::shared_ptr<T>> Object::GetComponents() const
{
    assert(this != nullptr &&
           "実行しているオブジェクト(this)"
           "がありません。「再試行」をおして、「呼び出し履歴」からどこでemptyになったのかを確認してください。");

    std::vector<std::shared_ptr<T>> cmps;

    for(auto& component : components_) {
        auto cmp = std::dynamic_pointer_cast<T>(component);
        if(cmp)
            cmps.push_back(cmp);
    }

    return cmps;
}

template <typename _Type>
void Object::RemoveComponent()
{
    assert(this != nullptr &&
           "実行しているオブジェクト(this)"
           "がありません。「再試行」をおして、「呼び出し履歴」からどこでemptyになったのかを確認してください。");

    for(int i = components_.size() - 1; i >= 0; --i) {
        auto& c = components_[i];

        if(std::dynamic_pointer_cast<_Type>(c)) {
            c->Exit();
            break;
        }
    }
}
