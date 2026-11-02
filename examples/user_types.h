#pragma once

#include <string>

#include <jm/json_fwd.h>

struct Config
{
    std::string host;
    int port = 0;
};
JM_DECLARE_JSON(Config)
