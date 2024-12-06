#ifndef KRSPZ_DRIVERLOOK_H
#define KRSPZ_DRIVERLOOK_H

#include "Driver.h"

class KrDriverLOOK final : public KrDriver
{
public:
    virtual void PrintSettings() const override;

private:
    virtual void AddIORequest(const KrIORequest& IORequest) override;
    virtual void RemoveIORequest(const KrIORequest& IORequest) override;
    virtual std::vector<KrIORequest> GetIORequestQueue() const override;

    virtual void NextIORequest() override;

    unsigned MaxConsecutiveAccessToTrackNum = 2;

    std::vector<KrIORequest> IORequestQueue;
    bool bMovingOut = true;
    unsigned CurrentConsecutiveAccessToTrackNum = 0;
};

#endif //KRSPZ_DRIVERLOOK_H
