// 2026-04 fgsmodlists.com FGTweak

#include "PCH.h"
#include "fg_logutil.h" // myprint, myprint_init

class cFGTweakMain
{
public:
    cFGTweakMain() {}
    ~cFGTweakMain() {}

    const std::string sVersionInfo = ".v01";

    // kDataLoaded
    void OnDataLoaded()
    {
        // NOTE: RE::ConsoleLog::GetSingleton()->Print wont work before kDataLoaded
        myprint("FGTweak{}: OnDataLoaded",sVersionInfo);
    }
    
// ***** skse events 

    // MessagingInterface listener : input=hotkeys, kDataLoaded
    void OnMsgInterfaceMsg (SKSE::MessagingInterface::Message *message)
    {
        // if (message->type == SKSE::MessagingInterface::kInputLoaded)
            // RE::BSInputDeviceManager::GetSingleton()->AddEventSink(this);
        if (message->type == SKSE::MessagingInterface::kDataLoaded) OnDataLoaded();
        if (message->type == SKSE::MessagingInterface::kPostPostLoad) OnPostPostLoad();
    }

	// kPostPostLoad
    void OnPostPostLoad ()
    {
    } 
    
// ***** utils

    template <typename T>
    const T* my_error_if_null (const char* name, const T* p) { if (!p) myprint("{}=null",name); return p; };

// ***** registry

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
        myprint_init();
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
