// 2026-04 fgsmodlists.com FGTweak
#include "PCH.h"
#include "fg_log.h" // logger.info
#include "fg_str_util.h" // str

class cFGTweakMain :
	public RE::BSTEventSink<RE::InputEvent*>, // hotkey
	public RE::BSTEventSink<SKSE::CrosshairRefEvent> // crosshair over object
{
public:
    FGLogger logger;
    cFGTweakMain() {}
    ~cFGTweakMain() {}

    cFGTweakMain* GetEventSink () { return this; }

    const std::string sVersionInfo = ".v01";
	const uint32_t SCANCODE_test = 65; // f1=59.. f7=65  f11=87 

    // kDataLoaded
    void OnDataLoaded()
    {
        // NOTE: RE::ConsoleLog::GetSingleton()->Print wont work before kDataLoaded
        logger.info("OnDataLoaded, version={}",sVersionInfo);
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

    void OnKey_Test ()
    {
        logger.info("OnKey_Test");
        RE::Actor* actor = myActorPlayer();       
        if (!actor) return;
        RE::NiAVObject* niav = actor->Get3D2();

        if (RE::BGSScene* s = actor->GetCurrentScene())
        {
            logger.info("scene=valid GetFormEditorID={} GetObjectTypeName={} GetName={}",s->GetFormEditorID(),s->GetObjectTypeName(),s->GetName());
        } else { logger.info("scene=null"); }
        
        logger.info("niav={}",niav?"valid":"null");
        if (niav)
        {
            logger.info("bound={}",str(niav->worldBound)); 
            // ROL charcreate during: 5576.5,-17423.7,4517.7,r=127.7 scene=null
            // ROL charcreate after:  5580.3,-17413.1,4575.9,r=1 (firstperson)
            // ROL charcreate after:  5580.5,-17421.6,4563.7,r=75.7 (3rdperson)
            // ROL walk 2 steps    :  5582.7,-17542.4,4548.7,r=74.5 (3rdperson) , x+-2 y+-120 z+-15
        }
    }

// ***** actor utils
    
    RE::Actor* myActorPlayer () {
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
        // WARNING GetActorBase() returns the base of the actor, not the instance.
        // Actor/Character: What you encounter in game. Bandit Thug in Valtheim Towers, etc.
        // Actorbase: The base that all Bandit Thugs in the game pull from.

        #if 0
        RE::TESObjectREFR* res = e ? e->crosshairRef.get() : nullptr;
		if (auto o = e->crosshairRef) {
            if (auto* p = o->GetBaseObject()) { // TESBoundObject WARNING: BaseObject is the "class" (like all bandits) rather than the "instance" (this particular bandit)
                res = p;
                // logger.info("CrosshairRefEvent GetName={} GetFormID={}",p->GetName(),p->GetFormID());
            }
		}
        #endif
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
        
	    if (auto* src = SKSE::GetCrosshairRefEventSource()) src->AddEventSink(GetEventSink()); else logger.info("GetCrosshairRefEventSource=null?0");

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
