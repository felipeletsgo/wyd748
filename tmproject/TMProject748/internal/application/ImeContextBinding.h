#pragma once

// Owns the IME context that the client creates for its main window and keeps
// the context that was associated before it. C-26: the previous context was
// stored in window extra bytes that the window class does not reserve, so the
// store failed and shutdown associated a null context instead of restoring it.
// Api supplies Create(), Associate(window, context) -> previous, Destroy(context).
namespace ime_context
{
template<class Window, class Context>
class Binding
{
public:
    template<class Api>
    bool Install(Api& api, Window window)
    {
        if (m_installed)
            return true;
        const Context created = api.Create();
        if (!created)
            return false;
        m_previous = api.Associate(window, created);
        m_created = created;
        m_window = window;
        m_installed = true;
        return true;
    }

    // Restores the previous context and destroys only the context created here.
    template<class Api>
    void Restore(Api& api)
    {
        if (!m_installed)
            return;
        api.Associate(m_window, m_previous);
        api.Destroy(m_created);
        m_installed = false;
        m_created = Context{};
        m_previous = Context{};
        m_window = Window{};
    }

    bool Installed() const { return m_installed; }
    Context Previous() const { return m_previous; }

private:
    Window m_window{};
    Context m_created{};
    Context m_previous{};
    bool m_installed = false;
};
}
