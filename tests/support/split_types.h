#pragma once

#include <string>

#include <jm/json_fwd.h>

struct SplitUser
{
    std::string name;
    int score = 0;
};
JM_DECLARE_JSON(SplitUser)
