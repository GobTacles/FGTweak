// skyrim notification hook, see https://github.com/VersuchDrei/NotificationLogSSE
#include "PCH.h"

void fg_notification_hook_install();

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

            this->_notifications.emplace_front(a_message);
        }

        [[nodiscard]] auto GetMessages() const noexcept -> std::vector<std::string> {
            const auto _ = std::unique_lock(this->_lock);
            return std::vector(this->_notifications.begin(), this->_notifications.end());
        }

    private:
        Proxy() = default;

        static constexpr std::size_t MAX_BUFFER_SIZE = 128;
        mutable std::mutex _lock;
        std::list<std::string> _notifications;
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

void fg_notification_hook_install()
{
    SKSE::AllocTrampoline(1u << 4);

    // const auto papyrus = SKSE::GetPapyrusInterface();
    // papyrus->Register(NotificationLogger::Papyrus::Register);

    NotificationLogger::Hooks::Install();
}

