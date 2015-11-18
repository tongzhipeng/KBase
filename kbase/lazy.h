/*
 @ 0xCCCCCCCC
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef KBASE_LAZY_H_
#define KBASE_LAZY_H_

#include <functional>
#include <memory>
#include <mutex>

namespace kbase {

// Lazy<T> manages a single instance of type T, which will be lazily created on the
// first time it is accessed.
// The instance of type T is created either by using its default constructor,
// or a specified Creator. The constructor will only ever be called once, even in a
// multiple-thread environment. However, access of the internal instance is not
// self-thread, you need to have guaranteed thread-safey on your own.
template<typename T>
class Lazy {
public:
    // Creator is used when you have demand on controlling the construction of the
    // value. Each valid Creator should return a raw pointer to a T object.
    // Lazy stashes an instance of Creator during construction, and invokes it when
    // the lazy value is accessed in the first time, and then takes over the
    // ownership of the raw pointer which is returned from the Creator.
    typedef std::function<T*()> Creator;

    // Initializes the value with its default constructor.
    Lazy()
        : ctor_([]() { return new T(); })
    {}

    // Initializes the value by calling crator.
    explicit Lazy(Creator creator)
        : ctor_(creator)
    {}

    Lazy(const Lazy&) = delete;

    Lazy(Lazy&&) = delete;

    Lazy& operator=(const Lazy&) = delete;

    Lazy& operator=(Lazy&&) = delete;

    ~Lazy() = default;

    T& value()
    {
        std::call_once(flag_, &Lazy::Initialize, this);
        return *value_.get();
    }

    bool value_created() const
    {
        return static_cast<bool>(value_);
    }

private:
    void Initialize()
    {
        value_.reset(ctor_());
    }

private:
    std::unique_ptr<T> value_;
    Creator ctor_;
    std::once_flag flag_;
};

}   // namespace kbase

#endif  // KBASE_LAZY_H_