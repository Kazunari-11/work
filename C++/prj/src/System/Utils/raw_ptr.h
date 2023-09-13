//---------------------------------------------------------------------------
//!	@file	raw_ptr.h
//!	@brief	生ポインタ
//---------------------------------------------------------------------------
#pragma once

// 使用する場合は有効にする
//#define USE_COM

//===========================================================================
//! 生ポインタクラス
//! スマートポインタやCOMポインタから生ポインタに透過的に変換します
//! 関数の引数に用います。所有権を持たないポインタです
//===========================================================================
template <typename T>
class raw_ptr
{
public:
    raw_ptr() = default;

    raw_ptr(const std::unique_ptr<T>& ptr)
        : ptr_(ptr.get())
    {
    }

    raw_ptr(const std::shared_ptr<T>& ptr)
        : ptr_(ptr.get())
    {
    }

#ifdef USE_COM
    raw_ptr(const com_ptr<T>& ptr)
        : ptr_(ptr.Get())
    {
    }
#endif

    raw_ptr(T* ptr)
        : ptr_(ptr)
    {
    }

    bool operator==(const raw_ptr<T>& rhs)
    {
        return ptr_ == rhs.get();
    }

    bool operator!=(const raw_ptr<T>& rhs)
    {
        return ptr_ != rhs.get();
    }

    bool operator==(const T* rhs)
    {
        return ptr_ == rhs;
    }

    bool operator!=(const T* rhs)
    {
        return ptr_ != rhs;
    }

    operator bool() const
    {
        return ptr_ != nullptr;
    }

    T& operator*() const
    {
        return *ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

    T* get() const
    {
        return ptr_;
    }

private:
    T* ptr_ = nullptr;
};
