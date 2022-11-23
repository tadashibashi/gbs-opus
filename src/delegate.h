#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <vector>

namespace gbs_opus {

    /**
     * @class
     * A delegate is an event observer that can invoke callbacks of subscribed listeners when an event has occured.
     * Inspired by JavaScript listeners and C# delegates, "Listeners" are actually function pointers the user passes 
     * into the Delegate, which gets wrapped and stored inside.
     * No subclassing of a Listener class is necessary, which is the main difference from the Observer pattern.
     * Both member and global function callbacks are supported.
     *
     * @details
     * In practice, to add a callback, you will pass a global function pointer to AddListener, or another overload
     * where you can pass an object with a corresponding member function pointer.
     * Function signature must match that of the Delegate.
     * Listeners may be removed by a call to RemoveListener and passing the same global function pointer or object +
     * member function combination.
     *
     * When invoking a Delegate, simply call Invoke, or the operator() and pass the arguments.
     * These arguments are copied for each callback, so it is wise to use small objects like pointers/refs,
     * primitives or small structs.
     *
     * @tparam Args - argument types that the Delegate requires for each of its callback listeners.
     *
     */
    template <typename T> class delegate;

    template <typename Ret, typename... Args>
    class delegate<Ret(Args...)>
    {
    private:

        /// Helper class that wraps a Delegate's subscribed listener callbacks.
        /// Enables identification of the wrapped callback via both global function pointer
        /// or object pointer + its member function pointer.
        class func_wrapper {
            friend class delegate;
        public:
            template <typename T>
            func_wrapper(T *object, Ret (T:: *func)(Args...));
            explicit func_wrapper(Ret (*func)(Args...));
            explicit func_wrapper(std::function<Ret(Args...)> *func);
            func_wrapper() = delete;

            template <typename T>
            bool sig_matches(T *obj, Ret (T:: *func_ptr)(Args...)) const;
            bool sig_matches(Ret(*func_ptr)(Args...)) const;
            bool sig_matches(std::function<Ret(Args...)> *func) const;

            Ret operator()(Args ...args) const;
        private:
            void *m_object;         // object to call the function on, if a member func is stored; null when none.
            std::any m_func_ptr; // stores the fn pointer id; we need std::any since member fn ptrs have no portable type.
            std::function<Ret(Args...)> m_func; // the callable function
            bool m_to_remove;        // flag the marks if this wrapper should be removed from Delegate
        };
    public:
        delegate() : m_functions(), m_remove_now() { }
        delegate(delegate &&moved);
        delegate &operator = (delegate &&moved);

        delegate(delegate &) = delete;
        delegate &operator=(delegate &) = delete;

    public:
        // Adds a member function listener for a particular object
        template<typename T>
        void add_listener(T *object, Ret (T:: *func)(Args...));

        // Adds a global function listener
        void add_listener(Ret (*func)(Args...));

        // Adds a pointer to an std::function object
        void add_listener(std::function<Ret(Args...)> *func);

        // Removes an object + member function listener.
        template<typename T>
        void remove_listener(T *object, Ret (T:: *func)(Args...));

        /// Removes a global function listener.
        void remove_listener(void (*func)(Args...));

        /// Removes an std::function pointer listener.
        void remove_listener(std::function<Ret(Args...)> *func);

        /// Removes all previously added listeners.
        /// Faster than individually removing each listener.
        void clear();

        // ===== Calling the delegate =================================================================================
        /// Attempts to fire the callback, ignoring the return value.
        /// Instead of the return value, a bool will be returned: 
        /// true: if any callback was called; false: if no callbacks were called, due to being empty.
        bool try_invoke(Args... args);

        /// Fires the callback to each subscribed listener; returns the result of the last one.
        /// If there are no listeners, a RuntimeException will be thrown.
        /// Please check if (!Empty()), or if (Delegate &) before calling.
        Ret invoke(Args... args);

        /// Fires the callback to each subscribed listener; returns the result of the last one
        /// If there are no listeners, a RuntimeException will be thrown.
        /// Please check if (!Empty()), or if (Delegate &) before calling.
        Ret operator()(Args... args);


        /// Gets the number of listeners currently attached to this Delegate.
        size_t size() const { return m_functions.size(); }

        /// Checks if no listeners are attached to this Delegate.
        bool empty() const { return m_functions.empty(); }

        /// checks if any listeners are attached to this Delegate.
        explicit operator bool() { return !m_functions.empty(); }

        /// Processes removals from RemoveListener functions. Automatically called at the beginning
        /// of Invoke and TryInvoke.
        void process_removals();
    private:


        // List of listeners
        std::vector<func_wrapper> m_functions;

        // Flag indicated whether removals need to be processed.
        bool m_remove_now;
    };

}

#include "delegate.inl"