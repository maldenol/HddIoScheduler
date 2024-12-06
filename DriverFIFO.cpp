#include "DriverFIFO.h"

#include <iostream>

void KrDriverFIFO::AddIORequest(const KrIORequest& IORequest)
{
    IORequestQueue.push_back(IORequest);
}

void KrDriverFIFO::RemoveIORequest(const KrIORequest& IORequest)
{
    IORequestQueue.erase(std::remove(IORequestQueue.begin(), IORequestQueue.end(), IORequest), IORequestQueue.end());
}

std::vector<KrIORequest> KrDriverFIFO::GetIORequests() const
{
    return IORequestQueue;
}

void KrDriverFIFO::NextIORequest()
{
    if (IORequestQueue.empty())
    {
        SetCurrentIORequest(nullptr);
        return;
    }

    const KrIORequest Result = IORequestQueue.front();
    IORequestQueue.erase(IORequestQueue.begin());

    std::cout << "DRIVER: Using FIFO strategy\n";
    std::cout << " Current buffer: (" << GetTrackBySector(Result.Sector) << ":" << Result.Sector << ")\n";
    std::cout << " Buffer queue: [";
    for (const KrIORequest& IORequest : IORequestQueue)
    {
        std::cout << " (" << GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")";
    }
    std::cout << " ]\n";

    SetCurrentIORequest(&Result);
}
