// 2026-04 fgsmodlists.com FGTweak
#include "PCH.h"
#include "fg_win.h"
#include "fg_log.h" // logger.info
#include "fg_str_util.h" // str
#include "fg_notification.h"
#include "fg_enable.h"

// note: check pagefile > 20gb
// note: warn if overlay apps are running
// idea : maybe we can just list running quests and detect mcm recorder script running?
// idea : movement speed debuff, player.setav speedmult 1

class cFGTweakMain :
	public RE::BSTEventSink<RE::InputEvent*>, // hotkey
	public RE::BSTEventSink<SKSE::CrosshairRefEvent>, // crosshair over object
    public RE::BSTEventSink<RE::TESActivateEvent>, // activate item
    public RE::BSTEventSink<RE::MenuOpenCloseEvent> // menu
{
public:
    FGLogger logger;
    cFGTweakMain() {}
    ~cFGTweakMain() {}

    cFGTweakMain* get_event_sink () { return this; }

    const std::string sVersionInfo = ".v0.1.0";
	const uint32_t SCANCODE_test = 65; // f1=59.. f7=65  f11=87 

// ***** start late1 (data loaded)

    // kDataLoaded
    void on_data_loaded()
    {
        // NOTE: logger/RE::ConsoleLog::GetSingleton()->Print wont work before kDataLoaded
        load_settings();
        

        step_loop_start();
    }

// ***** start late2 (main menu)

    bool on_main_menu_check_enabled = true;
    void on_main_menu_check ()
    {
        if (!on_main_menu_check_enabled) return;
        on_main_menu_check_enabled = false; // only once
        
        // PageFile warning
        constexpr uint64_t gb = 1ull * 1024 * 1024 * 1024; // 20 GB
        constexpr uint64_t pagefile_min = 20ull * gb;
        std::optional<fg_memory_info> mi = win_get_memory_info();
        std::string meminfo = mi ? std::format("physical memory: {} GB, page file: {} GB",
                mi->physical_memory/gb,
                mi->page_file_size/gb
            ) : "unknown";

        logger.info("on_main_menu_check, version={} meminfo={}",sVersionInfo,meminfo);
        
        if (!mi || mi->page_file_size < pagefile_min)
        {
            std::string msg = std::format("[FGTweak] PageFile Warning:\r\nPlease set the Windows PageFile to at least 20 GB\r\n{}\r\nWrite !pagefile in our discord for instructions",
                meminfo);
            RE::DebugMessageBox(msg.c_str());
        }
    }

// ***** overlay_warning

    #ifdef ENABLE_OVERLAY_CHECK
    // overwlay warnings
    std::set<std::string> ow_discord = { "CoreUIComponents.dll" }; // TODO: can this come from other apps too ?
    std::set<std::string> ow_steam = { "GameOverlayRenderer64.dll","GameOverlayRenderer.dll" };
    // steam_api64.dll gameoverlayrenderer64.dll steamclient64.dll skse64_1_6_1170.dll usvfs_x64.dll(mo2)
    // steam, discord, medal, GeForce Experience, Overwolf, MSI Afterburner.
    
    bool has_proc (std::optional<std::set<std::string>> v_proc,std::set<std::string> v_search)
    {
        if (v_proc) for (auto path : *v_proc)
        {
            std::string haystack = str_lower(path);
            for (auto s : v_search) if (haystack.find(str_lower(s)) != std::string::npos) return true;
        }
        return false;
    }
    #endif

    std::optional<std::set<std::string>> ow_v_proc;
    bool overlay_steam      = false;
    bool overlay_discord    = false;
    bool overlay_any        = false;

    void overlay_warning_init ()
    {
        #ifdef ENABLE_OVERLAY_CHECK
        static bool done = false;
        if (done) return; else done = true; // only once
        ow_v_proc = win_list_processes();
        overlay_steam = has_proc(ow_v_proc,ow_steam);
        overlay_discord = has_proc(ow_v_proc,ow_discord);
        logger.info("overlay_warning_init, #processes={} overlays: steam={} discord={}",ow_v_proc?std::to_string(ow_v_proc->size()):"(none)",overlay_steam,overlay_discord);
        
        overlay_any = overlay_steam | overlay_discord;
        overlay_any |= has_proc(ow_v_proc,{"medal","Experience","Overwolf","Afterburner"});
        #endif
    }

    void overlay_warning_notification ()
    {
        #ifdef ENABLE_OVERLAY_CHECK
        overlay_warning_init();
        if (!overlay_any) return;
        RE::DebugNotification("if you experience crashes, try disabling");
        RE::DebugNotification("overlays like steam, discord, medal,");
        RE::DebugNotification("GeForce Experience, Overwolf, MSI Afterburner");
        #endif
    }

    void overlay_warning_msg_boxes ()
    {
        #ifdef ENABLE_OVERLAY_CHECK
        static bool done = false;
        if (done) return; else done = true; // only once per skyrim start
        overlay_warning_init();
        auto& v_proc = ow_v_proc;
        if (v_proc) for (auto s : *v_proc)
        {
            s = s.substr(s.find_last_of('\\') + 1); // remove anything before last backslash
            logger.info("+proc '{}'",s);
        }

        if (overlay_steam) {
            std::string msg = std::format("[FGTweak] Overlay Warning: Steam\r\nplease make sure 'steam overlay while ingame'\r\nis disabled in steam library (rclick skyrim: setting : general)");
            RE::DebugMessageBox(msg.c_str());
        }

        if (overlay_discord) {
            std::string msg = std::format("[FGTweak] Overlay Warning: Discord\r\nif you have discord we recommend\r\ndisabling 'game overlay' in the settings");
            RE::DebugMessageBox(msg.c_str());
        }

        //  medal, GeForce Experience, Overwolf, MSI Afterburner.
        auto ow_warn_aux = [this](std::string n) {
            std::string msg = std::format("[FGTweak] Overlay Warning: {}\r\nplease make sure you have disabled overlay apps like {}",n,n);
            RE::DebugMessageBox(msg.c_str());
        };
        if (has_proc(v_proc,{"medal"})          ) ow_warn_aux("Medal.TV");
        if (has_proc(v_proc,{"Experience"})     ) ow_warn_aux("GeForce Experience");
        if (has_proc(v_proc,{"Overwolf"})       ) ow_warn_aux("Overwolf");
        if (has_proc(v_proc,{"Afterburner"})    ) ow_warn_aux("MSI Afterburner");
        #endif
    }
    
// ***** step

    bool enabled_setup_help = true;
    size_t c_step = 0;
    bool need_init = true;
    int c_setup_teleport = 3;
    int setup_help_teleport_message_countdown = 0;

    void on_game_start (std::uint32_t imsg) // called once for new and twice for load: pre+post
    {
        enabled_setup_help = true;
        // if (imsg == SKSE::MessagingInterface::kNewGame) overlay_warning_msg_boxes();
        if (imsg != SKSE::MessagingInterface::kPreLoadGame) overlay_warning_notification();
        if (imsg != SKSE::MessagingInterface::kPostLoadGame)
        {
            need_init = true;
        }
    }

    void step_200msec()
    {
        if (!has_game_start()) return; // still early in skyrim main menu
        if (last_game_start_was_pre_load()) return; // wait until load is finished
        ++c_step;
        [[maybe_unused]] bool b_1s  = (c_step % (5*1)) == 0;
        [[maybe_unused]] bool b_5s  = (c_step % (5*5)) == 0;
        [[maybe_unused]] bool b_10s = (c_step % (5*10)) == 0;

        if (need_init)
        {
            need_init = false;
        }

        if (enabled_setup_help)
        {
            update_player_pos();
            if (!_has_player_pos) return; // still early
            if (_has_player_pos && !_is_player_in_rol) { logger.info("not in starting area -> disabling setup_help"); enabled_setup_help = false; return; }

            if (last_game_start_was_new() && !_is_player_at_spawn && c_setup_teleport > 0)
            {
                c_setup_teleport--;
                logger.info("setup helper teleport back to spawn, remaining={}",c_setup_teleport);

                RE::Actor* actor = get_player_actor();     
                if (actor) actor->SetPosition(ROL_spawn_pos, true);
                setup_help_teleport_message_countdown = 2;
            }

            if (setup_help_teleport_message_countdown > 0)
            {
                if (--setup_help_teleport_message_countdown == 0)
                {
                    std::string msg_long = "please finish setup\r\nbefore moving around\r\nsee the guide on the website, step 5";
                    std::string msg_short = "please finish setup before moving";

                    // Show a "Debug Notification" (displays in the top-left corner of the game)
                    RE::DebugNotification(msg_short.c_str());

                    // Popup a "Debug MessageBox" (with an OK button)
                    RE::DebugMessageBox(msg_long.c_str());
                }
            }

            if (b_10s) {
                std::optional<std::string> n = fg_notification_get_last();
                logger.info("step 10s, last_notification={}",n?*n:"(none)");
            }
        }
    }

// ***** config/.ini file

    CSimpleIniA iniFile; // see SimpleIni.h : vcpkg https://github.com/brofield/simpleini
    void load_settings() {
        iniFile.LoadFile(L"Data/SKSE/Plugins/FGTweak.ini"); // NOTE i think while running, .ini files are visible under Data/SKSE/..
        // const char *key_value = iniFile.GetValue("MyKeyName", "MyDefaultValue");
    }

// ***** skse events

    std::optional<std::uint32_t> game_start_imsg;
    bool has_game_start () { return game_start_imsg?true:false; }
    bool last_game_start_was_new  () { return has_game_start() && *game_start_imsg == SKSE::MessagingInterface::kNewGame; }
    bool last_game_start_was_load () { return has_game_start() && (*game_start_imsg == SKSE::MessagingInterface::kPreLoadGame || *game_start_imsg == SKSE::MessagingInterface::kPostLoadGame); }
    bool last_game_start_was_pre_load () { return has_game_start() && *game_start_imsg == SKSE::MessagingInterface::kPreLoadGame; }
    void notify_game_start (std::uint32_t imsg) { game_start_imsg = imsg; on_game_start(imsg); }

    // MessagingInterface listener : input=hotkeys, kDataLoaded
    void on_msg_interface_msg (SKSE::MessagingInterface::Message *message)
    {
        if (message->type == SKSE::MessagingInterface::kInputLoaded)
        {
            logger.info("kInputLoaded"); 
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(this); // hotkeys
        }
        if (message->type == SKSE::MessagingInterface::kPostLoad)       { logger.info("kPostLoad"); }
        if (message->type == SKSE::MessagingInterface::kPostPostLoad)   { logger.info("kPostPostLoad"); on_post_post_load(); }
        if (message->type == SKSE::MessagingInterface::kPreLoadGame)    { logger.info("kPreLoadGame"); notify_game_start(message->type); }
        if (message->type == SKSE::MessagingInterface::kPostLoadGame)   { logger.info("kPostLoadGame"); notify_game_start(message->type); }
        if (message->type == SKSE::MessagingInterface::kSaveGame)       { logger.info("kSaveGame"); }
        if (message->type == SKSE::MessagingInterface::kDeleteGame)     { logger.info("kDeleteGame"); }
        if (message->type == SKSE::MessagingInterface::kNewGame)        { logger.info("kNewGame"); notify_game_start(message->type); }
        if (message->type == SKSE::MessagingInterface::kDataLoaded)     { logger.info("kDataLoaded"); on_data_loaded(); }
    }

	// kPostPostLoad
    void on_post_post_load ()
    {
    }
    
    // hotkeys
    void on_button_down(RE::ButtonEvent* button)
    {
        if (!enabled_setup_help) return;
        auto dxScanCode = button->GetIDCode();
        // logger.info("on_button_down {}", dxScanCode);
        if (dxScanCode == SCANCODE_test) on_hotkey_test();
    }

    std::string name_shard = "Shard of Lorkhan"; // e.g. on_activate_event / GetBaseObject()->GetName()
    
    void on_activate_event (std::string itemName)
    {
        if (!enabled_setup_help) return;
        if (!is_player_in_rol()) return;
        if (itemName == name_shard) logger.info("on_activate_event '{}'",itemName);
    }
    
    void on_crosshair_event (const SKSE::CrosshairRefEvent* e)
    {
        if (!enabled_setup_help) return;
        if (!is_player_in_rol()) return;

        // WARNING: BaseObject is the "class" (like all bandits) rather than the "instance" (this particular bandit)
        // WARNING GetActorBase() returns the base of the actor, not the instance.
        // Actor/Character: What you encounter in game. Bandit Thug in Valtheim Towers, etc.
        // Actorbase: The base that all Bandit Thugs in the game pull from.

		if (auto o = e->crosshairRef) {
            // if (auto* p = o->GetBaseObject()) { ... }
            // logger.info("on_crosshair_event GetName={} GetFormID={}",o->GetName(),o->GetFormID());
		}
    }

    std::set<std::string> ignore_menu {
        "Loading Menu","Mist Menu","Fader Menu","LoadWaitSpinner","BTPS Ovelay Menu","BTPS Menu",
        "TrueHUD","Main Menu","oxygenMeter2","Cursor Menu","Console",
    };

    void on_main_menu (bool opening)
    {
        if (opening) on_main_menu_check();
    }

    void on_menu_open_close (std::string menuName,bool opening)
    {
        if (menuName == "Main Menu") on_main_menu(opening);
        if (!enabled_setup_help) return;
        if (ignore_menu.contains(menuName)) return;
        // bool is_racemenu = menuName == "RaceSex Menu";
        // bool is_msgbox = menuName == "MessageBoxMenu";
        logger.info("MenuOpenClose menuName=\"{}\" opening={}", menuName, opening);
    }
    

// ***** setup reminder / cage

    std::set<std::string> last_quests;

    void on_hotkey_test () // F7
    {
        // logger.info("on_hotkey_test");
        RE::Actor* actor = get_player_actor();       
        if (!actor) return;
        RE::NiAVObject* niav = actor->Get3D2();

        if (RE::BGSScene* s = actor->GetCurrentScene())
        {
            logger.info("scene=valid GetFormEditorID={} GetObjectTypeName={} GetName={}",s->GetFormEditorID(),s->GetObjectTypeName(),s->GetName());
        } else {
            // logger.info("scene=null"); 
        }
        
        // logger.info("niav={}",niav?"valid":"null");
        if (niav)
        {
            auto p0 = ROL_spawn_pos;
            RE::NiPoint3 pos = niav->worldBound.center;
            float d = pos.GetDistance(p0);
            update_player_pos();
            bool in_rol = _is_player_in_rol;
            bool in_cage = _is_player_at_spawn;
            logger.info("bound={} d={} in_rol={} in_cage={}",str(niav->worldBound),d,in_rol,in_cage); 
            // ROL charcreate during: 5576.5,-17423.7,4517.7,r=127.7 scene=null
            // ROL charcreate after:  5580.3,-17413.1,4575.9,r=1 (firstperson)
            // ROL charcreate after:  5580.5,-17421.6,4563.7,r=75.7 (3rdperson)
            // ROL walk 2 steps    :  5582.7,-17542.4,4548.7,r=74.5 (3rdperson) , x+-2 y+-120 z+-15
        }
        
        #ifdef ENABLE_QUEST_LIST
        if (auto* dataHandler = RE::TESDataHandler::GetSingleton())
        {
            auto& quests = dataHandler->GetFormArray<RE::TESQuest>();
            bool last_q_was_empty = last_quests.empty();
            size_t c_q_printed = 0;
            size_t c_q_max_print = 50;
            for (auto* q : quests) {
                if (!q || !q->IsEnabled()) continue; // IsEnabled,IsRunning
                std::string qname = q->GetName();
                if (last_quests.contains(qname)) continue; // only show newly active quests
                last_quests.emplace(qname);
                if (last_q_was_empty) continue; // dont spam hundreds the first time


                if (c_q_printed >= c_q_max_print) continue; // dont spam dozens
                ++c_q_printed;
                // if (!q->IsRunning()) continue;
                bool inHUD = q->data.flags.all(RE::QuestFlag::kDisplayedInHUD);
                bool isMain = q->data.questType == RE::QUEST_DATA::Type::kMainQuest;
                bool isMisc = q->data.questType == RE::QUEST_DATA::Type::kMiscellaneous;
                bool isSide = q->data.questType == RE::QUEST_DATA::Type::kSideQuest;
                logger.info("+quest name={} id={} IsCompleted={} inHUD={} type={}",qname,q->GetFormEditorID(),q->IsCompleted(),inHUD,isMain?"main":isMisc?"misc":isSide?"side":"other");
                if (c_q_printed == c_q_max_print) logger.info("+quest...");
            }
            logger.info("num_quests_active={}",last_quests.size());
        }
        #endif

        std::optional<std::string> n = fg_notification_get_last();
        if (n) logger.info("last_notification={}",*n);
        // Unforgiving Devices updated
        // Restraining followers...
        // Restrained 17 followers!
        // [McmRecorder] 0011_T.N.G..json (11/51)
        // [McmRecorder] 0012_Scrappies Matchmaker.json (12/51)
    }

// ***** actor utils

    // TODO: cache in bool once we have a 100msec step function
    bool is_player_in_rol () { update_player_pos(); return _is_player_in_rol; }

    bool _has_player_pos = false;
    bool _is_player_in_rol = false;
    bool _is_player_at_spawn = false;
    
    const RE::NiPoint3 ROL_spawn_pos{5575.0f,-17411.0f,4683.0f}; // z+140 = in the air during jump so player can fall down to the ground vs height
    const float dist_ROL_area = 15000.0f; // detect if we are in starting area at all (i havent found a dimension or cell id yet) : seen d=7500 on other side from start
    const float dist_ROL_spawn = 200.0f; // 2 steps ~ 120

    bool update_player_pos ()
    {
        RE::Actor* actor = get_player_actor();       
        if (!actor) return false;
        RE::NiAVObject* niav = actor->Get3D2();
        if (!niav) return false;
        RE::NiPoint3 pos = niav->worldBound.center;
        float d = pos.GetDistance(ROL_spawn_pos);
        _is_player_in_rol = d < dist_ROL_area;
        _is_player_at_spawn = d < dist_ROL_spawn;
        if (!_has_player_pos)
        {
            logger.info("update_player_pos first: d={} rol={} spawn={} pos={}",d,_is_player_in_rol,_is_player_at_spawn,str(pos));
            _has_player_pos = true;
        }
        return _is_player_in_rol;
    }
    
    RE::Actor* get_player_actor () {
        RE::PlayerCharacter* p = RE::PlayerCharacter::GetSingleton();
        return p ? p->As<RE::Actor>() : nullptr;
    }

// ***** EventSink api
    
    // InputEvent
	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>* eSrc)
	{
        RE::BSEventNotifyControl res = RE::BSEventNotifyControl::kContinue;
        if (res == RE::BSEventNotifyControl::kContinue && eventPtr)
        {
            auto* e = *eventPtr;
            if (e && e->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
                auto* button = e->AsButtonEvent();
                if (button->IsDown()) on_button_down(button); // IsDown = IsPressed && HeldDuration=0
            }
        }
		return res;
	}

    // CrosshairRefEvent
	RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* e, RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
	{
        on_crosshair_event(e);
		return RE::BSEventNotifyControl::kContinue;
	}

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event,
                                          RE::BSTEventSource<RE::TESActivateEvent>*) override {
        if (event && event->actionRef && event->actionRef->GetFormID() == 0x14) {
            std::string activated = event->objectActivated->GetBaseObject()->GetName();
            on_activate_event(activated);
            // NOTE: returning kStop does NOT prevent activation
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    // Log information about Menu open/close events that happen in the game
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
        std::string menuName = str(event->menuName);
        on_menu_open_close(menuName,event->opening);
        return RE::BSEventNotifyControl::kContinue;
    }

// ***** step thread

    std::jthread stepThread;
    std::atomic_bool stepQueued{ false };

    void step_loop_start()
    {
        auto* task = SKSE::GetTaskInterface();
        if (!task) {
            logger.info("TaskInterface=null");
            return;
        }

        stepThread = std::jthread([this](std::stop_token stopToken) {
            using namespace std::chrono_literals;

            while (!stopToken.stop_requested()) {
                std::this_thread::sleep_for(200ms);
                if (stopToken.stop_requested()) { break; }

                // Prevent piling up tasks if the previous step has not run yet.
                if (stepQueued.exchange(true)) { continue;}

                SKSE::GetTaskInterface()->AddTask([this]() { stepQueued = false; step_200msec(); });
            }
        });
    }

    void step_loop_stop()
    {
        if (stepThread.joinable()) {
            stepThread.request_stop();
            stepThread.join();
        }
    }

// ***** utils

    template <typename T>
    const T* my_error_if_null (const char* name, const T* p) { if (!p) logger.info("{}=null",name); return p; };

// ***** papyrus 

    static std::string papyrus_GetVersion(RE::StaticFunctionTag*) { return "FGTweak.v01"; }
    static bool my_register_papyrus(RE::BSScript::IVirtualMachine *vm)
    {
        vm->RegisterFunction("GetVersion","FGTweak",papyrus_GetVersion);
        return true;
    }

// ***** registry

    // TODO: move to fg_registry.h
    class cFGRegistry { public:
        struct SKSEi { // skse interfaces
            const SKSE::ScaleformInterface*             scaleform       = nullptr; // kScaleform
            const SKSE::PapyrusInterface*               papyrus         = nullptr; // kPapyrus
            const SKSE::SerializationInterface*         serialization   = nullptr; // kSerialization
            const SKSE::TaskInterface*                  task            = nullptr; // kTask
            const SKSE::TrampolineInterface*            trampoline      = nullptr; // kTrampoline
            const SKSE::MessagingInterface*             message         = nullptr; //
            const SKSE::ObjectInterface*                object          = nullptr; //
            const SKSEDelayFunctorManager*              delay           = nullptr; //
            const SKSEObjectRegistry*                   oregistry       = nullptr; //
            const SKSEPersistentObjectStorage*          persist         = nullptr; //
        } skse;
    };
    static inline cFGRegistry _registry;
    static cFGRegistry& get_registry () { return _registry; }

// ***** SKSE api 

    // loader skse plugin entry point
    bool on_plugin_load(const SKSE::LoadInterface *skse)
    {
        logger.logger_init();
        SKSE::Init(skse);

        fg_notification_hook_install();
        
        auto& r = get_registry();
        r.skse.scaleform        = my_error_if_null("skse.scaleform"     ,SKSE::GetScaleformInterface()); // ScaleformInterface*
	    r.skse.papyrus          = my_error_if_null("skse.papyrus"       ,SKSE::GetPapyrusInterface()); // PapyrusInterface*
	    r.skse.serialization    = my_error_if_null("skse.serialization" ,SKSE::GetSerializationInterface()); // SerializationInterface*
	    r.skse.task             = my_error_if_null("skse.task"          ,SKSE::GetTaskInterface()); // TaskInterface*
	    r.skse.trampoline       = my_error_if_null("skse.trampoline"    ,SKSE::GetTrampolineInterface()); // TrampolineInterface*
        r.skse.message          = my_error_if_null("skse.message"       ,SKSE::GetMessagingInterface()); // MessagingInterface*
        r.skse.object           = my_error_if_null("skse.object"        ,SKSE::GetObjectInterface()); // ObjectInterface*
        r.skse.delay            = my_error_if_null("skse.delay"         ,SKSE::GetDelayFunctorManager()); // SKSEDelayFunctorManager*
        r.skse.oregistry        = my_error_if_null("skse.oregistry"     ,SKSE::GetObjectRegistry()); // SKSEObjectRegistry*
        r.skse.persist          = my_error_if_null("skse.persist"       ,SKSE::GetPersistentObjectStorage()); // SKSEPersistentObjectStorage*

        if (r.skse.papyrus) r.skse.papyrus->Register(my_register_papyrus);
        
	    if (auto* src = SKSE::GetCrosshairRefEventSource()) src->AddEventSink(get_event_sink()); else logger.info("GetCrosshairRefEventSource=null?");

	    if (auto* src = RE::ScriptEventSourceHolder::GetSingleton())
        {
            src->AddEventSink<RE::TESActivateEvent>(get_event_sink()); 
        } else { logger.info("ScriptEventSourceHolder=null?"); }
        
        if (auto* src = RE::UI::GetSingleton()) src->AddEventSink<RE::MenuOpenCloseEvent>(get_event_sink());

	    r.skse.message->RegisterListener([](SKSE::MessagingInterface::Message *message) {
            gFGTweakMain.on_msg_interface_msg(message);
        });
        return true;
    }
} gFGTweakMain;

// loader skse plugin entry point
SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    return gFGTweakMain.on_plugin_load(skse);
}
