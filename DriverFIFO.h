#ifndef KRSPZ_DRIVERFIFO_H
#define KRSPZ_DRIVERFIFO_H

#include "Driver.h"

class KrDriverFIFO final : public KrDriver
{
private:
    virtual void AddIORequest(const KrIORequest& IORequest) override;
    virtual void RemoveIORequest(const KrIORequest& IORequest) override;
    virtual std::vector<KrIORequest> GetIORequests() const override;

    virtual void NextIORequest() override;

    std::vector<KrIORequest> IORequestQueue;
};

#endif //KRSPZ_DRIVERFIFO_H
