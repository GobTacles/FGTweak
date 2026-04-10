#pragma once

std::optional<std::string> fg_notification_get_last();
std::vector<std::string> fg_notification_get_cached();
void fg_notification_hook_install();

