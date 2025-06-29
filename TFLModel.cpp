#include "TFLModel.h"


bool BranchConnect::operator==(const BranchConnect &rhs) const
{
    return idx == rhs.idx && branch == rhs.branch;
}
