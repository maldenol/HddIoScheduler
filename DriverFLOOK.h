#ifndef KRSPZ_DRIVERFLOOK_H
#define KRSPZ_DRIVERFLOOK_H

#include "Driver.h"

class KrDriverFLOOK final : public KrDriver
{
private:
    virtual void AddIORequest(const KrIORequest& IORequest) override;
    virtual void RemoveIORequest(const KrIORequest& IORequest) override;
    virtual std::vector<KrIORequest> GetIORequests() const override;

    virtual void NextIORequest() override;

    std::vector<KrIORequest> IORequestQueueLeft;
    std::vector<KrIORequest> IORequestQueueRight;
    bool bUsingQueueLeft = false;
    bool bMovingOut = true;
};

#endif //KRSPZ_DRIVERFLOOK_H
