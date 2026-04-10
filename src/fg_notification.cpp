// skyrim notification hook, see https://github.com/VersuchDrei/NotificationLogSSE
#include "PCH.h"
#include "fg_notification.h"
#include "fg_enable.h"

#ifdef ENABLE_NOTIFICATION_HOOK
namespace NotificationLogger::detail {
    class Proxy {
    public:
        Proxy(const volatile Proxy&) = delete;
        Proxy& operator=(const volatile Proxy&) = delete;

        [[nodiscard]] static Proxy& GetSingleton() noexcept {
            static Proxy singleton;
            return singleton;
        }

        void AddMessage(std::string_view a_message) noexcept {
            const auto _ = std::unique_lock(this->_lock);
            while (this->_notifications.size() >= MAX_BUFFER_SIZE) {
                this->_notifications.pop_back();
            }
            ++_total_notifications;
            this->_notifications.emplace_front(a_message);
        }

        [[nodiscard]] auto GetMessages() const noexcept -> std::vector<std::string> {
            const auto _ = std::unique_lock(this->_lock);
            return std::vector(this->_notifications.begin(), this->_notifications.end());
        }

        [[nodiscard]] auto GetTotalNotifications() const noexcept -> std::size_t {
            const auto _ = std::unique_lock(this->_lock);
            return _total_notifications;
        }

        [[nodiscard]] auto GetLastMessage() const noexcept -> std::optional<std::string> {
            const auto _ = std::unique_lock(this->_lock);
            if (_notifications.empty()) return std::nullopt;
            return _notifications.front();
        }

    private:
        Proxy() = default;

        static constexpr std::size_t MAX_BUFFER_SIZE = 128;
        mutable std::mutex _lock;
        std::list<std::string> _notifications;
        size_t _total_notifications;
    };

    struct CreateHUDDataMessage {
        static void thunk(RE::HUDData::Type a_type, const char* a_message) {
            if (a_message) {
                auto& proxy = Proxy::GetSingleton();
                proxy.AddMessage(a_message);
            }

            func(a_type, a_message);
        }

        inline static REL::Relocation<decltype(thunk)> func;
    };
}  // namespace NotificationLogger::detail

namespace NotificationLogger::Hooks {
    inline void Install() {
        const auto base = RELOCATION_ID(52050, 52933).address();
        const auto target = base + (REL::Module::GetRuntime() != REL::Module::Runtime::AE ? 0x19B : 0x31D);

        auto& trampoline = SKSE::GetTrampoline();
        detail::CreateHUDDataMessage::func = trampoline.write_call<5>(target, detail::CreateHUDDataMessage::thunk);
    }
}  // namespace NotificationLogger::Hooks
#endif

size_t fg_notification_get_total()
{
    #ifdef ENABLE_NOTIFICATION_HOOK
    auto& proxy = NotificationLogger::detail::Proxy::GetSingleton();
    return proxy.GetTotalNotifications();
    #else
    return 0;
    #endif
}

std::vector<std::string> fg_notification_get_cached()
{
    #ifdef ENABLE_NOTIFICATION_HOOK
    auto& proxy = NotificationLogger::detail::Proxy::GetSingleton();
    return proxy.GetMessages();
    #else
    return {};
    #endif
}

std::optional<std::string> fg_notification_get_last()
{
    #ifdef ENABLE_NOTIFICATION_HOOK
    auto& proxy = NotificationLogger::detail::Proxy::GetSingleton();
    return proxy.GetLastMessage();
    #else
    return std::nullopt;
    #endif
}

void fg_notification_hook_install()
{
    #ifdef ENABLE_NOTIFICATION_HOOK
    SKSE::AllocTrampoline(1u << 4);
    NotificationLogger::Hooks::Install();
    #endif
}

