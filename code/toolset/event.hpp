#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace tlr
{

template<typename T>
class FunctionWrapper;

template<typename R, typename... Args>
class FunctionWrapper<R(Args...)>
{
public:
    FunctionWrapper() = default;

    FunctionWrapper(std::function<R(Args...)>&& func) : _func(std::move(func)) {}

    bool operator==(const FunctionWrapper<R(Args...)>& other) const
    {
        return _func.target_type() == other._func.target_type();
    }

    template<typename... ArgsAlias>
    R operator()(ArgsAlias&&... args) const
    {
        return _func(std::forward<ArgsAlias>(args)...);
    }

    std::size_t hash() const
    {
        return _func.target_type().hash_code();
    }

private:
    std::function<R(Args...)> _func;
};

} // namespace tlr

namespace std
{

template<typename R, typename... Args>
struct hash<tlr::FunctionWrapper<R(Args...)>>
{
    std::size_t operator()(const tlr::FunctionWrapper<R(Args...)>& wrapper) const
    {
        return wrapper.hash();
    }
};

} // namespace std

namespace tlr
{

template <typename... Args>
class Event
{
public:
    Event() = default;

    void operator+=(std::function<void(Args...)>&& listener)
    {
        _listeners.insert(FunctionWrapper<void(Args...)>(std::move(listener)));
    }

    void operator-=(std::function<void(Args...)>&& listener)
    {
        _listeners.erase(FunctionWrapper<void(Args...)>(std::move(listener)));
    }

    template<typename... ArgsAlias>
    void Raise(ArgsAlias&&... args)
    {
        for (auto& listener : _listeners)
        {
            listener(std::forward<ArgsAlias>(args)...);
        }
    }

private:
    std::unordered_set<FunctionWrapper<void(Args...)>> _listeners;
};

} // namespace tlr
