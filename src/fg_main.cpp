// 2026-04 fgsmodlists.com FGTweak
#include "PCH.h"
#include "fg_win.h"
#include "fg_log.h" // logger.info
#include "fg_str_util.h" // str

// TODO: check pagefile > 20gb
// TODO: warn if overlay apps are running
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

    cFGTweakMain* GetEventSink () { return this; }

    const std::string sVersionInfo = ".v0.1.0";
	const uint32_t SCANCODE_test = 65; // f1=59.. f7=65  f11=87 

    // kDataLoaded
    void OnDataLoaded()
    {
        // NOTE: RE::ConsoleLog::GetSingleton()->Print wont work before kDataLoaded
        LoadSettings();
        
        // PageFile warning
        constexpr uint64_t gb = 1ull * 1024 * 1024 * 1024; // 20 GB
        constexpr uint64_t pagefile_min = 20ull * gb;
        std::optional<fg_memory_info> mi = win_get_memory_info();
        std::string meminfo = "unknown";
        if (mi) 
        {
            meminfo = std::format("physical memory: {} GB, page file: {} GB",
                mi->physical_memory/gb,
                mi->page_file_size/gb
            );
        }

        logger.info("OnDataLoaded, version={} meminfo={}",sVersionInfo,meminfo);
        
        if (!mi || mi->page_file_size < pagefile_min)
        {
            std::string msg = std::format("[FGTweak] PageFile Warning:\r\nPlease set the Windows PageFile to at least 20 GB\r\n{}\r\nWrite !pagefile in our discord for instructions",
                meminfo);
            RE::DebugMessageBox(msg.c_str());
        }
    }
    
// ***** config/.ini file

    CSimpleIniA iniFile;
    void LoadSettings() {
        iniFile.LoadFile(L"Data/SKSE/Plugins/FGTweak.ini");
        // const char *key_value = iniFile.GetValue("MyKeyName", "MyDefaultValue");
    }

// ***** skse events

    // MessagingInterface listener : input=hotkeys, kDataLoaded
    void OnMsgInterfaceMsg (SKSE::MessagingInterface::Message *message)
    {
        if (message->type == SKSE::MessagingInterface::kInputLoaded)
        {
            logger.info("kInputLoaded"); 
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink(this);
        }
        if (message->type == SKSE::MessagingInterface::kPostLoad)       { logger.info("kPostLoad"); }
        if (message->type == SKSE::MessagingInterface::kPostPostLoad)   { logger.info("kPostPostLoad"); OnPostPostLoad(); }
        if (message->type == SKSE::MessagingInterface::kPreLoadGame)    { logger.info("kPreLoadGame"); }
        if (message->type == SKSE::MessagingInterface::kPostLoadGame)   { logger.info("kPostLoadGame"); }
        if (message->type == SKSE::MessagingInterface::kSaveGame)       { logger.info("kSaveGame"); }
        if (message->type == SKSE::MessagingInterface::kDeleteGame)     { logger.info("kDeleteGame"); }
        if (message->type == SKSE::MessagingInterface::kNewGame)        { logger.info("kNewGame"); }
        if (message->type == SKSE::MessagingInterface::kDataLoaded)     { logger.info("kDataLoaded"); OnDataLoaded(); }
    }

	// kPostPostLoad
    void OnPostPostLoad ()
    {
    }
    
    // hotkeys
    void OnButtonDown(RE::ButtonEvent* button)
    {
        auto dxScanCode = button->GetIDCode();
        // logger.info("OnButtonDown {}", dxScanCode);
        if (dxScanCode == SCANCODE_test) OnKey_Test();
    }

    std::string name_shard = "Shard of Lorkhan"; // e.g. OnPlayerActivateItem / GetBaseObject()->GetName()
    
    void OnPlayerActivateItem (std::string itemName)
    {
        if (!is_player_in_rol()) return;
        if (itemName == name_shard) logger.info("OnPlayerActivateItem '{}'",itemName);
    }
    
    void OnCrosshairRefEvent (const SKSE::CrosshairRefEvent* e)
    {
        if (!is_player_in_rol()) return;

        // WARNING: BaseObject is the "class" (like all bandits) rather than the "instance" (this particular bandit)
        // WARNING GetActorBase() returns the base of the actor, not the instance.
        // Actor/Character: What you encounter in game. Bandit Thug in Valtheim Towers, etc.
        // Actorbase: The base that all Bandit Thugs in the game pull from.

		if (auto o = e->crosshairRef) {
            // if (auto* p = o->GetBaseObject()) { ... }
            // logger.info("OnCrosshairRefEvent GetName={} GetFormID={}",o->GetName(),o->GetFormID());
		}
    }

    std::set<std::string> ignore_menu {
        "Loading Menu","Mist Menu","Fader Menu","LoadWaitSpinner","BTPS Ovelay Menu","BTPS Menu",
        "TrueHUD","Main Menu","oxygenMeter2","Cursor Menu","Console",
    };

    void OnMenuOpenClose (std::string menuName,bool opening)
    {
        if (ignore_menu.contains(menuName)) return;
        // bool is_racemenu = menuName == "RaceSex Menu";
        // bool is_msgbox = menuName == "MessageBoxMenu";
        logger.info("MenuOpenClose menuName=\"{}\" opening={}", menuName, opening);
    }
    

// ***** setup reminder / cage

    const RE::NiPoint3 ROL_spawn_pos{5575.0f,-17411.0f,4683.0f}; // z+140 = in the air during jump so player can fall down to the ground vs height
    const float dist_ROL_area = 15000.0f; // detect if we are in starting area at all (i havent found a dimension or cell id yet) : seen d=7500 on other side from start
    const float dist_ROL_cage = 200.0f; // 2 steps ~ 120
    std::set<std::string> last_quests;

    void OnKey_Test ()
    {
        logger.info("OnKey_Test");
        RE::Actor* actor = get_player_actor();       
        if (!actor) return;
        RE::NiAVObject* niav = actor->Get3D2();

        if (RE::BGSScene* s = actor->GetCurrentScene())
        {
            logger.info("scene=valid GetFormEditorID={} GetObjectTypeName={} GetName={}",s->GetFormEditorID(),s->GetObjectTypeName(),s->GetName());
        } else { logger.info("scene=null"); }
        
        logger.info("niav={}",niav?"valid":"null");
        if (niav)
        {
            auto p0 = ROL_spawn_pos;
            RE::NiPoint3 pos = niav->worldBound.center;
            float d = pos.GetDistance(p0);
            bool in_rol = d < dist_ROL_area;
            bool in_cage = d < dist_ROL_cage;
            logger.info("bound={} d={} in_rol={} in_cage={}",str(niav->worldBound),d,in_rol,in_cage); 
            // ROL charcreate during: 5576.5,-17423.7,4517.7,r=127.7 scene=null
            // ROL charcreate after:  5580.3,-17413.1,4575.9,r=1 (firstperson)
            // ROL charcreate after:  5580.5,-17421.6,4563.7,r=75.7 (3rdperson)
            // ROL walk 2 steps    :  5582.7,-17542.4,4548.7,r=74.5 (3rdperson) , x+-2 y+-120 z+-15
            if (in_rol && !in_cage)
            {
                actor->SetPosition(p0, true);
                
                std::string msg_long = "please finish setup\r\nbefore moving around\r\nsee the guide on the website, step 5";
                std::string msg_short = "please finish setup before moving";

                // Show a "Debug Notification" (displays in the top-left corner of the game)
                RE::DebugNotification(msg_short.c_str());

                // Popup a "Debug MessageBox" (with an OK button)
                RE::DebugMessageBox(msg_long.c_str());
            }
        }
        
        if (auto* dataHandler = RE::TESDataHandler::GetSingleton())
        {
            auto& quests = dataHandler->GetFormArray<RE::TESQuest>();
            std::set<std::string> cur_quests; // last_quests
            for (auto* q : quests) {
                if (!q || !q->IsActive()) continue;
                std::string qname = q->GetName();
                cur_quests.emplace(qname);
                if (last_quests.contains(qname)) continue; // only show newly active quests
                if (last_quests.empty()) continue; // dont spam hundreds the first time

                // if (!q->IsRunning()) continue;
                bool inHUD = q->data.flags.all(RE::QuestFlag::kDisplayedInHUD);
                bool isMain = q->data.questType == RE::QUEST_DATA::Type::kMainQuest;
                bool isMisc = q->data.questType == RE::QUEST_DATA::Type::kMiscellaneous;
                bool isSide = q->data.questType == RE::QUEST_DATA::Type::kSideQuest;
                logger.info("+quest name={} id={} IsCompleted={} inHUD={} type={}",qname,q->GetFormEditorID(),q->IsCompleted(),inHUD,isMain?"main":isMisc?"misc":isSide?"side":"other");
            }
            last_quests = cur_quests;
            logger.info("num_quests_active={}",last_quests.size());
        }
    }

// ***** actor utils

    bool is_player_in_rol ()
    {
        // TODO: cache in bool once we have a 100msec step function
        RE::Actor* actor = get_player_actor();       
        if (!actor) return false;
        RE::NiAVObject* niav = actor->Get3D2();
        if (!niav) return false;
        RE::NiPoint3 pos = niav->worldBound.center;
        float d = pos.GetDistance(ROL_spawn_pos);
        return d < dist_ROL_area;
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
                if (button->IsDown()) OnButtonDown(button); // IsDown = IsPressed && HeldDuration=0
            }
        }
		return res;
	}

    // CrosshairRefEvent
	RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* e, RE::BSTEventSource<SKSE::CrosshairRefEvent>*)
	{
        OnCrosshairRefEvent(e);
		return RE::BSEventNotifyControl::kContinue;
	}

    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event,
                                          RE::BSTEventSource<RE::TESActivateEvent>*) override {
        if (event && event->actionRef && event->actionRef->GetFormID() == 0x14) {
            std::string activated = event->objectActivated->GetBaseObject()->GetName();
            OnPlayerActivateItem(activated);
            // NOTE: returning kStop does NOT prevent activation
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    // Log information about Menu open/close events that happen in the game
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
        std::string menuName = str(event->menuName);
        OnMenuOpenClose(menuName,event->opening);
        return RE::BSEventNotifyControl::kContinue;
    }

// ***** utils

    template <typename T>
    const T* my_error_if_null (const char* name, const T* p) { if (!p) logger.info("{}=null",name); return p; };

// ***** papyrus 

    static std::string Papyrus_GetVersion(RE::StaticFunctionTag*) { return "FGTweak.v01"; }
    static bool MyRegisterPapyrus(RE::BSScript::IVirtualMachine *vm)
    {
        vm->RegisterFunction("GetVersion","FGTweak",Papyrus_GetVersion);
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
    bool OnPluginLoad(const SKSE::LoadInterface *skse)
    {
        logger.logger_init();
        SKSE::Init(skse);
        
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

        if (r.skse.papyrus) r.skse.papyrus->Register(MyRegisterPapyrus);
        
	    if (auto* src = SKSE::GetCrosshairRefEventSource()) src->AddEventSink(GetEventSink()); else logger.info("GetCrosshairRefEventSource=null?");

	    if (auto* src = RE::ScriptEventSourceHolder::GetSingleton())
        {
            src->AddEventSink<RE::TESActivateEvent>(GetEventSink()); 
        } else { logger.info("ScriptEventSourceHolder=null?"); }
        
        if (auto* src = RE::UI::GetSingleton()) src->AddEventSink<RE::MenuOpenCloseEvent>(GetEventSink());

	    r.skse.message->RegisterListener([](SKSE::MessagingInterface::Message *message) {
            gFGTweakMain.OnMsgInterfaceMsg(message);
        });
        return true;
    }
} gFGTweakMain;

// loader skse plugin entry point
SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    return gFGTweakMain.OnPluginLoad(skse);
}
