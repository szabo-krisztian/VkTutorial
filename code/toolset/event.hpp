/*
 *   TODO: This event system is not complete, it needs to be refined.
 * 
 *   The FunctionWrapper class is needed to allow std::function objects to be used in
 *   unordered containers such as std::unordered_set. This class provides custom hashing
 *   and equality comparison between std::function objects, ensuring that only one instance
 *   of each unique function type is stored in the container. std::functions can be expensive
 *   to copy, that is why you are seeing only rvalue parameters. 
 */

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

    R operator()(Args... args) const
    {
        return _func(std::forward<Args>(args)...);
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

    void Raise(Args... args)
    {
        for (auto& listener : _listeners)
        {
            listener(std::forward<Args>(args)...);
        }
    }

private:
    std::unordered_set<FunctionWrapper<void(Args...)>> _listeners;
};

} // namespace tlr
