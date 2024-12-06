#ifndef KRSPZ_USERPROCESS_H
#define KRSPZ_USERPROCESS_H

#include "IORequest.h"

#include <string>
#include <vector>

struct KrUserProcess final
{
    std::string Name;
    std::vector<KrIORequest> IORequests;
};

#endif //KRSPZ_USERPROCESS_H
