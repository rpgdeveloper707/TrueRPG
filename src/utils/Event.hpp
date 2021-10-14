#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>

template<typename T, typename M>
struct Pair
{
    T &first;
    M second;

    Pair(T &f, M s) : first(f), second(s) { }
};

template<typename ...Args>
class AbstarctEventHandler
{
public:
    AbstarctEventHandler() {}
    virtual ~AbstarctEventHandler() = default;
    virtual void call(Args ...args) = 0;

    bool operator==(const AbstarctEventHandler<Args...> &o) const
    {
        return isEqual(o);
    }

    bool operator!=(const AbstarctEventHandler<Args...> &o) const
    {
        return !(*this == o);
    }
protected:
    virtual bool isEqual(const AbstarctEventHandler<Args...> &o) const = 0;
};

template<typename ...Args>
class FunctionEventHandler : public AbstarctEventHandler<Args...>
{
private:
    using function = void(*)(Args...);

    function _func;
public:
    FunctionEventHandler(function f) : AbstarctEventHandler<Args...>(), _func(f) 
    {
        assert(f);
    }

    void call(Args ...args) override
    {
        _func(args...);
    }
protected:
    bool isEqual(const AbstarctEventHandler<Args...> &o) const override
    {
        const FunctionEventHandler<Args...> *other = dynamic_cast<const FunctionEventHandler<Args...> *>(&o);
        return (other != nullptr && other->_func == _func);
    }
};

template<class C, typename ...Args>
class MethodEventHandler : public AbstarctEventHandler<Args...>
{
private:
    using Method = void(C::*)(Args...);

    C &_class;
    Method _method;
public: 
    MethodEventHandler(C &c, Method m) : AbstarctEventHandler<Args...>(), _class(c), _method(m) 
    {
        assert(m);
    }

    void call(Args ...args) override
    {
        (_class.*_method)(args...);
    }
protected:
    bool isEqual(const AbstarctEventHandler<Args...> &o) const override
    {
        const MethodEventHandler<C, Args...> *other = dynamic_cast<const MethodEventHandler<C, Args...> *>(&o);
        return (other != nullptr && &other->_class == &_class && other->_method == _method);
    }
};

template<typename ...Args>
class IEvent
{
public:
    
    void operator+=(std::shared_ptr<AbstarctEventHandler<Args...>> handler)
    {
        add(handler);
    }

    void operator-=(std::shared_ptr<AbstarctEventHandler<Args...>> handler)
    {
        remove(handler);
    }

    void operator+=(void(*f)(Args...))
    {
        add(f);
    }

    void operator-=(void(*f)(Args...))
    {
        remove(f);
    }

protected:
    IEvent() {};

    virtual void add(std::shared_ptr<AbstarctEventHandler<Args...>> handler) = 0;
    virtual void remove(std::shared_ptr<AbstarctEventHandler<Args...>> handler) = 0;

    virtual void add(void(*f)(Args...)) = 0;
    virtual void remove(void(*f)(Args...)) = 0;
};

template<typename ...Args>
class Event : public IEvent<Args...>
{
private:
    class EventHandlerList
    {
    private:
        using HandlerPtr = std::shared_ptr<AbstarctEventHandler<Args...>>;
        using HandlerIt = typename std::vector<HandlerPtr>::const_iterator;

        std::vector<HandlerPtr> _handlers;
    public:
        EventHandlerList() : _handlers() { }

        void add(HandlerPtr handler)
        {
            if (findHandler(handler) == _handlers.end())
            {
                _handlers.emplace_back(handler);
            }
        }

        void remove(HandlerPtr handler)
        {
            auto it = findHandler(handler);

            if (it != _handlers.end())
            {
                _handlers.erase(it);
            }
        }

        void call(Args... args)
        {
            for (auto &handle : _handlers)
            {
                if (handle)
                    handle->call(args...);
            }
        }
    private:
        inline HandlerIt findHandler(HandlerPtr &handler) const
        {
            return std::find_if(_handlers.cbegin(), _handlers.cend(), 
                                    [&handler](const HandlerPtr oneHandler)
                                    {
                                        return (*oneHandler == *handler);
                                    });
        }
    };

    EventHandlerList _handlerList;
public:
    Event() : IEvent<Args...>(), _handlerList() { }

    void operator()(Args... args)
    {
        _handlerList.call(args...);
    }
protected:
    void add(std::shared_ptr<AbstarctEventHandler<Args...>> handler) override
    {
        _handlerList.add(handler);
    }

    void remove(std::shared_ptr<AbstarctEventHandler<Args...>> handler) override
    {
        _handlerList.remove(handler);
    }

    void add(void(*f)(Args...)) override
    {
        _handlerList.add(std::make_shared<FunctionEventHandler<Args...>>(f));
    }

    void remove(void(*f)(Args...)) override
    {
        _handlerList.remove(std::make_shared<FunctionEventHandler<Args...>>(f));
    }
};

template<class C, typename ...Args>
std::shared_ptr<MethodEventHandler<C, Args...>> createEventHandler(C &object, void(C::*method)(Args...))
{
    return std::make_shared<MethodEventHandler<C, Args...>>(object, method);
}

#endif