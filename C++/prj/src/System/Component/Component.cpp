//---------------------------------------------------------------------------
//! @file   Component.h
//! @brief  コンポーネント ベースクラス
//---------------------------------------------------------------------------
#include "Component.h"
#include <System/Object.h>
#include <System/Scene.h>

BP_COMPONENT_BASE_IMPL(Component, u8"Componentクラス");

//! @brief   オーナーの取得
//! @details 従属しているオブジェクトを取得します
//! @return  オーナーオブジェクト
Object* Component::GetOwner()
{
    return owner_.get();
}

//! @brief オーナーの取得
//! @details 従属しているオブジェクトを取得します
//! @return オーナーオブジェクト
const Object* Component::GetOwner() const
{
    return owner_.get();
}

//! @brief オーナーの取得
//! @details 従属しているオブジェクトを取得します(shared_ptr型)
//! @return オーナーオブジェクト
ObjectPtr Component::GetOwnerPtr()
{
    return owner_;
}

//! @brief オーナーの取得
//! @details 従属しているオブジェクトを取得します(shared_ptr型)
//! @return オーナーオブジェクト
const ObjectPtr Component::GetOwnerPtr() const
{
    return owner_;
}

//! @brief コンストラクタ
//! @param owner オーナー
Component::Component()
    : owner_(nullptr)
{
}

void Component::Construct(ObjectPtr owner)
{
    owner_ = owner;
}

//! @brief 初期化処理
void Component::Init()
{
    SetStatus(StatusBit::Serialized, false);
    SetStatus(StatusBit::ShowGUI, true);
    SetStatus(StatusBit::Initialized, true);
}

//! @brief 更新処理
//! @param delta_time 1フレームの時間
void Component::Update(float delta_time)
{
    update_delta_time_ = delta_time;
}

//! @brief 更新処理
//! @param delta_time 1フレームの時間
void Component::LateUpdate(float delta_time)
{
    update_delta_time_ = delta_time;
}

//! @brief 描画処理
void Component::Draw()
{
}

//! @brief 描画処理
void Component::LateDraw()
{
}

//! @brief 終了処理
void Component::Exit()
{
    status_.off(Component::StatusBit::Alive);
    status_.on(Component::StatusBit::Exited);

    for(auto& timing : update_timings_) {
        auto& p = timing.second;
        if(p.connect_.valid())
            p.connect_.disconnect();

        p.proc_ = nullptr;
    }

    for(auto& timing : proc_timings_) {
        auto& p = timing.second;
        if(p.connect_.valid())
            p.connect_.disconnect();

        p.proc_ = nullptr;
    }
}

//! @brief GUI処理
void Component::GUI()
{
}

//! @brief 更新前処理
void Component::PreUpdate()
{
}

//! @brief 更新後処理
void Component::PostUpdate()
{
}

//! @brief 描画前処理
void Component::PreDraw()
{
}

//! @brief 描画後処理
void Component::PostDraw()
{
}

//! @brief Physics前処理
void Component::PrePhysics()
{
}

void Component::InitSerialize()
{
    SetStatus(StatusBit::Serialized, true);
}

void Component::SetPriority(ProcTiming timing, Priority priority)
{
    Scene::GetCurrentScene()->SetPriority(shared_from_this(), timing, priority);
}

void Component::RegisterToObject(ComponentPtr cmp, ObjectPtr obj)
{
    // 同じタイプを許容しない
    if(!cmp->GetStatus(StatusBit::SameType)) {
        auto cmps = obj->GetComponents();
        for(auto c : cmps) {
            if(c->typeInfo()->className() == cmp->typeInfo()->className())
                return;
        }
    }

    cmp->Construct(obj);

    obj->GetComponents().push_back(cmp);
}

//! @brief ステータスの設定
//! @param b ステータスビット
//! @param on 有効/無効
void Component::SetStatus(StatusBit b, bool on)
{
    on ? status_.on(b) : status_.off(b);
}

//! @brief ステータスの取得
//! @param b ステータスビット
//! @retval true : 有効
//! @retval false: 無効
bool Component::GetStatus(StatusBit b)
{
    return status_.is(b);
}

//----------------------------------------------------------------------------
// Doxygen Componentマニュアル
//----------------------------------------------------------------------------
//! @page section_component Componentについて
//! @li 単純な機能を持ったパーツです
//! @li Objectのつけて利用できます
//! @li 基本同じタイプのコンポーネントは付けれません。(「ComponentTransform」を2つとかは付けれません)
//!
//! @li コンポーネントの利用方法
//! @code
//!　
//!   obj->AddComponent<ComponentModel>();  //< 「ComponentModel」を objにて利用する
//!
//!   // ※objはObjectタイプで、先に次のように作成しておく必要があります【例】
//!   auto obj = Scene::CreateObject<Object>();
//!　
//! @endcode
//!
//! @li 使用しなくなったコンポーネントの削除
//! @code
//!　
//!   obj->RemoveComponent<ComponentModel>();  //< 「ComponentModel」を objにて削除します
//!　
//! @endcode
//!
