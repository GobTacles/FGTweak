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
    
// ***** SKSE api 

    // loader skse plugin entry point
    bool OnPluginLoad(const SKSE::LoadInterface *skse)
    {
        myprint_init();
        SKSE::Init(skse);
        
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
