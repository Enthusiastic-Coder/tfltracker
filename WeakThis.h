#pragma once
#include <memory>

template <typename T>
class WeakThis
{
public:
    WeakThis(T* t) : _livingObject(new int(0)), _class(t){}
    WeakThis(const WeakThis& rhs) = delete;
    WeakThis& operator=(const WeakThis& rhs) = delete;

    class WeakThisPtr
    {
    public:

        WeakThisPtr(T* t, std::weak_ptr<int> w) :_t(t), _w(w) {}

        operator bool() const
        {
            return _w.lock() != nullptr;
        }

        T* operator->() const
        {
            return _t;
        }

    private:
        T* _t;
        std::weak_ptr<int> _w;
    };

    WeakThisPtr operator()() const
    {
        return {_class, _livingObject};
    }

private:
    std::shared_ptr<int> _livingObject;
    T* _class;
};