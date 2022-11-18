#ifndef GBS_OPUS_SYSTEM_H
#define GBS_OPUS_SYSTEM_H
#include <string>

namespace gbs_opus
{
    class updatable
    {
    public:
        explicit updatable(int priority = 50) : m_priority(priority) { }
        virtual void update() = 0;
        void set_update_priority(int priority) { m_priority = priority; }

        [[nodiscard]]
        int update_priority() const { return m_priority; }
    private:
        int m_priority;
    };

    class input_processor
    {
    public:
        explicit input_processor(int priority = 50) : m_priority(priority) { }
        virtual void process_input() = 0;
        void set_input_priority(int priority) { m_priority = priority; }

        [[nodiscard]]
        int input_priority() const { return m_priority; }
    private:
        int m_priority;
    };

    class renderable
    {
    public:
        explicit renderable(int priority = 50) : m_priority(priority) { }
        virtual void render() = 0;
        void set_render_priority(int priority) { m_priority = priority; }

        [[nodiscard]]
        int render_priority() const { return m_priority; }
    private:
        int m_priority;
    };

    class system
    {
    public:
        explicit system(std::string name) : m_name(std::move(name)) { }
        virtual ~system() = default;

        virtual bool init() = 0;
        virtual bool close() = 0;

        [[nodiscard]]
        const std::string &name() const { return m_name; }
    private:
        std::string m_name;
    };
}

#endif //GBS_OPUS_SYSTEM_H
