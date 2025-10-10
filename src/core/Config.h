#pragma once

// Engine Information
#define ENGINE_NAME "BlockEngine"
#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 1
#define ENGINE_VERSION_PATCH 0
#define ENGINE_VERSION_STRING "0.1.0"

#define ENGINE_ORGANIZATION "Zalthen-dev"
#define ENGINE_WEBSITE "https://github.com/Zalthen-dev/BlockEngine"

// Version Helper
#define ENGINE_VERSION_NUMBER                                                  \
    ((ENGINE_VERSION_MAJOR * 10000) + (ENGINE_VERSION_MINOR * 100) +           \
     ENGINE_VERSION_PATCH)

#define ENGINE_MAKE_VERSION_STRING(major, minor, patch)                        \
    #major "." #minor "." #patch

// Window Title Helper
#define ENGINE_MAKE_WINDOW_TITLE(prefix) prefix " v" ENGINE_VERSION_STRING