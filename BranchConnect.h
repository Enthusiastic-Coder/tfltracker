#pragma once

class Branch;
struct BranchConnect
{
    Branch* branch = nullptr;
    int idx=0;
    bool operator==(const BranchConnect& rhs) const;
};

#include "Branch.h"

using Branches = std::vector<std::shared_ptr<Branch>>;

