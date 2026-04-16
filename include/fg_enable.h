// central enable defines so we can easily disable something at compile time
#pragma once

// #define ENABLE_PAPYRUS_VERSION

#define ENABLE_PAGE_FILE_CHECK

#define ENABLE_WIN_LIST_PROCESSES // needed for overlay check
#define ENABLE_OVERLAY_CHECK
// #define ENABLE_OVERLAY_MSGBOX // can be too annoying

// #define ENABLE_NOTIFICATION_HOOK
// #define ENABLE_QUEST_LIST

// #define SETUP_GUIDE_MESSAGE_DELAYED // actually blocks some of the setup scripts when delayed until its closed
// NOTE: so show setup message immediately and add line "Please close this message box while waiting so the setup can run."
