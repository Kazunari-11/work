//---------------------------------------------------------------------------
//! @file   ProcTiming.h
//! @brief  処理タイミング
//---------------------------------------------------------------------------
#pragma once
#include "Priority.h"
#include <System/Signals.h>
#include <System/Cereal.h>

//! タイミング
enum struct ProcTiming : u32
{
    PreUpdate = 0,
    Update,
    LateUpdate,
    PrePhysics,
    PostUpdate,

    PreDraw,
    Draw,
    LateDraw,
    PostDraw,

    Shadow,
    Gbuffer,
    HDR,
    Filter,

    NUM,
};

//! プライオリティ設定
constexpr int TIMING(ProcTiming p)
{
    return static_cast<int>(p);
}

//! プライオリティ設定(Macro)
#define OBJTIMING(p) TIMING(ProcTiming::##p)

std::string GetProcTimingName(ProcTiming proc);

class Object;
class Component;

USING_PTR(Object);
USING_PTR(Component);

template <class T>
class Callable
{
public:
    Callable() {}

    Callable(const std::string& name) { SetName(name); }

    virtual ~Callable() {}

    virtual void Exec(T) = 0;

    virtual void SetName(const std::string& str) { name_ = str; }

    virtual const std::string& GetName() { return name_; }

protected:
    std::string name_{};
};

//CEREAL_REGISTER_TYPE( Callable<void> );
//CEREAL_REGISTER_TYPE( Callable<float> );

#define ProcFunc(name, x)                                                                                              \
    class Callable##name : public Callable<void>                                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
        Callable##name()                                                                                               \
            : Callable<void>(#name)                                                                                    \
        {                                                                                                              \
        }                                                                                                              \
        void Exec x;                                                                                                   \
                                                                                                                       \
    private:                                                                                                           \
        CEREAL_SAVELOAD(arc, ver) { arc(CEREAL_NVP(name_)); }                                                          \
    };                                                                                                                 \
    std::shared_ptr<Callable##name> name = std::make_shared<Callable##name>();                                         \
    CEREAL_REGISTER_TYPE(Callable##name);                                                                              \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(Callable<void>, Callable##name)

#define ProcFuncUpdate(name, x)                                                                                        \
    class Callable##name : public Callable<float>                                                                      \
    {                                                                                                                  \
    public:                                                                                                            \
        Callable##name()                                                                                               \
            : Callable<float>(#name)                                                                                   \
        {                                                                                                              \
        }                                                                                                              \
        void Exec x;                                                                                                   \
                                                                                                                       \
    private:                                                                                                           \
        CEREAL_SAVELOAD(arc, ver) { arc(CEREAL_NVP(name_)); }                                                          \
    };                                                                                                                 \
    std::shared_ptr<Callable##name> name = std::make_shared<Callable##name>();                                         \
    CEREAL_REGISTER_TYPE(Callable##name);                                                                              \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(Callable<float>, Callable##name)

//! スロット
template <class T>
struct SlotProc
{
    friend class Scene;
    friend class Object;
    friend class Component;

public:
    bool IsDirty() const { return dirty_; }

    void ResetDirty() { dirty_ = false; }

    const ProcTiming GetTiming() const { return timing_; }

    const Priority GetPriority() const { return priority_; }

    auto GetFunc() { return func_; }

    const auto GetName() const { return name_; }

    const bool IsUpdate() const { return is_update_; }

    const bool IsDraw() const { return is_draw_; }

    void SetProc(std::string name, ProcTiming timing, Priority prio, std::function<void(T)> func)
    {
        name_     = name;
        dirty_    = true;
        timing_   = timing;
        priority_ = prio;
        proc_     = func;

        is_update_ = false;
        is_draw_   = false;
        switch(timing) {
        default:
            break;

        case ProcTiming::PreUpdate:
        case ProcTiming::Update:
        case ProcTiming::LateUpdate:
        case ProcTiming::PostUpdate:
            is_update_ = true;
            break;

        case ProcTiming::PreDraw:
        case ProcTiming::Draw:
        case ProcTiming::LateDraw:
        case ProcTiming::PostDraw:
            is_draw_ = true;
            break;
        }
    }

    void SetProc(std::shared_ptr<Callable<T>> func, ProcTiming timing, Priority prio)
    {
        name_     = func->GetName();
        dirty_    = true;
        timing_   = timing;
        priority_ = prio;
        func_     = func;

        proc_ = nullptr;

        is_update_ = false;
        is_draw_   = false;
        switch(timing) {
        default:
            break;

        case ProcTiming::PreUpdate:
        case ProcTiming::Update:
        case ProcTiming::LateUpdate:
        case ProcTiming::PostUpdate:
            is_update_ = true;
            break;

        case ProcTiming::PreDraw:
        case ProcTiming::Draw:
        case ProcTiming::LateDraw:
        case ProcTiming::PostDraw:
            is_draw_ = true;
            break;
        }
    }

    std::function<void(T)>& GetProc() { return proc_; }

private:
    std::string         name_{};
    ProcTiming          timing_    = ProcTiming::Draw;
    Priority            priority_  = Priority::NORMAL;
    bool                is_update_ = false;   //!< Updateにかかわるもの
    bool                is_draw_   = false;   //!< 表示しないでOFFになる
    sigslot::connection connect_{};
    bool                dirty_ = true;

    std::function<void(T)> proc_;

    std::shared_ptr<Callable<T>> func_;

private:
    //--------------------------------------------------------------------
    //! @name Cereal処理
    //--------------------------------------------------------------------
    //@{
    CEREAL_SAVELOAD(arc, ver)
    {
        dirty_ = true;           //セーブするときにdirty_つける
        arc(CEREAL_NVP(name_),   // name
            CEREAL_NVP(timing_),
            CEREAL_NVP(priority_),
            CEREAL_NVP(dirty_),
            CEREAL_NVP(func_));
        // connect_ は再構築させる
    }

    //@}
};

template <class T>
using SlotProcs = std::unordered_map<std::string, SlotProc<T>>;
