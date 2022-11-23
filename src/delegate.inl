#include "delegate.h"

#include <algorithm>
#include <typeinfo>
#include <utility>

namespace gbs_opus
{
    // ===== delegate Implementation ==================================================================================
    template<typename Ret, typename... Args>
    delegate<Ret(Args...)>::delegate(delegate &&moved) :
            m_functions(std::move(moved.m_functions)), m_remove_now(moved.m_remove_now)
    {

    }

    template<typename Ret, typename...Args>
    delegate<Ret(Args...)> &delegate<Ret(Args...)>::operator = (delegate &&moved)
    {
        m_functions = std::move(moved.m_functions);
        m_remove_now = moved.m_remove_now;

        return *this;
    }

    template<typename Ret, typename... Args>
    void delegate<Ret(Args...)>::clear()
    {
        for (auto &f : m_functions)
            f.m_to_remove = true;

        m_remove_now = true;
    }

    template<typename Ret, typename... Args>
    Ret delegate<Ret(Args...)>::invoke(Args... args)
    {
        if (m_functions.empty())
            throw std::runtime_error("Called Invoke on an empty delegate: delegate must contain at least one callback.");

        process_removals();
        for (auto it = m_functions.begin(), end = m_functions.end() - 1; it != end; ++it)
            (*it)(args...);

        return (*(m_functions.end() - 1))(args...);
    }

    template<typename Ret, typename... Args>
    bool delegate<Ret(Args...)>::try_invoke(Args... args)
    {
        process_removals();
        for (auto &func : m_functions)
            func(args...);
        return !m_functions.empty();
    }

    template<typename Ret, typename... Args>
    Ret delegate<Ret(Args...)>::operator()(Args... args)
    {
        return invoke(args...);
    }

    template<typename Ret, typename... Args>
    template<typename T>
    void delegate<Ret(Args...)>::remove_listener(T *object, Ret (T:: *func)(Args...))
    {
        // Find the listener
        auto it = std::find_if(m_functions.begin(), m_functions.end(),
                               [object, func](auto &f)
                               {
                                   return f.sig_matches(object, func);
                               });

        // If it exists, remove it
        if (it != m_functions.end())
        {
            it->m_to_remove = true;
            m_remove_now = true;
        }
        else
        {
            throw std::runtime_error("There was no matching callback in the delegate.");
        }
    }

    template<typename Ret, typename... Args>
    void delegate<Ret(Args...)>::remove_listener(void (*func)(Args...))
    {
        auto it = std::find_if(m_functions.begin(), m_functions.end(),
                               [func](const auto &f)
                               {
                                   return f.sig_matches(func);
                               });

        if (it != m_functions.end())
        {
            it->m_to_remove = true;
            m_remove_now = true;
        }
        else
        {
            throw std::runtime_error("There was no matching callback in the delegate.");
        }
    }

    template<typename Ret, typename... Args>
    void delegate<Ret(Args...)>::remove_listener(std::function<Ret(Args...)> *func)
    {
        auto it = std::find_if(m_functions.begin(), m_functions.end(),
                               [func](const auto &f)
                               {
                                   return f.sig_matches(func);
                               });

        if (it != m_functions.end())
        {
            it->m_to_remove = true;
            m_remove_now = true;
        }
        else
        {
            throw std::runtime_error("There was no matching callback in the delegate.");
        }
    }

    template<typename Ret, typename... Args>
    template<typename T>
    void delegate<Ret(Args...)>::add_listener(T *object, Ret (T:: *func)(Args...))
    {
        if (!object)
            throw std::runtime_error("Couldn't add listener to delegate–object was null");
        if (!func)
            throw std::runtime_error("Couldn't add listener to delegate–callback function ptr was null");

        m_functions.emplace_back(std::move(func_wrapper(object, func)));
    }

    template<typename Ret, typename... Args>
    void delegate<Ret(Args...)>::add_listener(Ret (*func)(Args...))
    {
        if (!func)
            throw std::runtime_error("Couldn't ad listener to delegate–function ptr was null");

        m_functions.emplace_back(std::move(func_wrapper(func)));
    }

    template<typename Ret, typename... Args>
    void delegate<Ret(Args...)>::add_listener(std::function<Ret(Args...)> *func)
    {
        if (!func)
            throw std::runtime_error("Couldn't add listener to delegate: callback function ptr was null");
        if (!(*func))
            throw std::runtime_error("Couldn't add listener to delegate: callback function does not have a target");

        m_functions.emplace_back(std::move(func_wrapper(func)));
    }

    template<typename Ret, typename... Args>
    void delegate<Ret(Args...)>::process_removals()
    {
        // Only perform removals if flag was set
        if (m_remove_now)
        {
            // Erase all function wrappers with "toRemove" flag set
            m_functions.erase(
                    std::remove_if(m_functions.begin(), m_functions.end(),
                                   [](const func_wrapper &f) {
                                       return f.m_to_remove;
                                   }),
                    m_functions.end());

            // Unset flag
            m_remove_now = false;
        }
    }


    // ===== delegate<Ret, Args...>::FunctionWrapper Implementation ========================================================

    template<typename Ret, typename... Args>
    template<typename T>
    delegate<Ret(Args...)>::func_wrapper::func_wrapper(T *object, Ret(T:: *func)(Args...)) :
            m_object(object), m_func_ptr(func),
            m_func([func, object](Args... args)->Ret
                     {
                         (object->*func)(args...);
                     }), m_to_remove()
    { }

    template<typename Ret, typename... Args>
    delegate<Ret(Args...)>::func_wrapper::func_wrapper(Ret (*func)(Args...)) :
            m_object(nullptr), m_func_ptr(func), m_func(func), m_to_remove()
    { }

    template<typename Ret, typename... Args>
    delegate<Ret(Args...)>::func_wrapper::func_wrapper(std::function<Ret(Args...)> *func) :
            m_object(nullptr), m_func_ptr(func), m_func(*func), m_to_remove()
    {}

    template<typename Ret, typename... Args>
    Ret delegate<Ret(Args...)>::func_wrapper::operator()(Args ...args) const
    {
        if (!m_func)
            throw std::runtime_error("func_wrapper::operator(): missing functor target. "
                                     "Functions stored in delegate must have a target");
        m_func(args...);
    }

    template<typename Ret, typename... Args>
    template <typename T>
    bool delegate<Ret(Args...)>::func_wrapper::sig_matches(T *object, Ret (T:: *func_ptr)(Args...)) const
    {
        return ((void *)object == m_object &&
                typeid(func_ptr) == m_func_ptr.type() &&
                func_ptr == std::any_cast<Ret(T:: *)(Args...)>(m_func_ptr));
    }

    template<typename Ret, typename... Args>
    bool delegate<Ret(Args...)>::func_wrapper::sig_matches(Ret(*func_ptr)(Args...)) const
    {
        return (m_object == nullptr &&
                std::any_cast<Ret(*)(Args...)>(func_ptr) == m_func_ptr);
    }

    template<typename Ret, typename... Args>
    bool delegate<Ret(Args...)>::func_wrapper::sig_matches(std::function<Ret(Args...)> *func) const
    {
        return m_object == nullptr &&
               std::any_cast<std::function<Ret(Args...)> *>(m_func_ptr) == func;
    }
}