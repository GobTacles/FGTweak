#pragma once
#include "fg_enable.h"

size_t fg_notification_get_total();
std::optional<std::string> fg_notification_get_last();
std::vector<std::string> fg_notification_get_cached();
void fg_notification_hook_install();
