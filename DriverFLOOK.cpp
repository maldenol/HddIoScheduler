#include "DriverFLOOK.h"

#include <algorithm>
#include <iostream>

void KrDriverFLOOK::AddIORequest(const KrIORequest& IORequest)
{
    // Get the inactive queue and add the request to it
    std::vector<KrIORequest>& IORequestQueue = bUsingQueueLeft ? IORequestQueueRight : IORequestQueueLeft;
    IORequestQueue.push_back(IORequest);
    // Then sort the queue
    std::sort(IORequestQueue.begin(), IORequestQueue.end());
}

void KrDriverFLOOK::RemoveIORequest(const KrIORequest& IORequest)
{
    std::vector<KrIORequest>::iterator Iter = std::remove(IORequestQueueLeft.begin(), IORequestQueueLeft.end(), IORequest);
    if (Iter != IORequestQueueLeft.end())
    {
        IORequestQueueLeft.erase(Iter, IORequestQueueLeft.end());
    }
    else
    {
        Iter = std::remove(IORequestQueueRight.begin(), IORequestQueueRight.end(), IORequest);
        IORequestQueueRight.erase(Iter, IORequestQueueRight.end());
    }
}

std::vector<KrIORequest> KrDriverFLOOK::GetIORequests() const
{
    std::vector<KrIORequest> Result;
    Result.reserve(IORequestQueueLeft.size() + IORequestQueueRight.size());
    Result.insert(Result.begin(), IORequestQueueLeft.begin(), IORequestQueueLeft.end());
    Result.insert(Result.begin(), IORequestQueueRight.begin(), IORequestQueueRight.end());
    return Result;
}

void KrDriverFLOOK::NextIORequest()
{
    if (IORequestQueueLeft.empty() && IORequestQueueRight.empty())
    {
        SetCurrentIORequest(nullptr);
        return;
    }

    // If the active is empty
    if (bUsingQueueLeft && IORequestQueueLeft.empty() || !bUsingQueueLeft && IORequestQueueRight.empty())
    {
        // Swap queues
        bUsingQueueLeft = !bUsingQueueLeft;
    }
    // And get the active one
    std::vector<KrIORequest>& IORequestQueue = bUsingQueueLeft ? IORequestQueueLeft : IORequestQueueRight;

    KrIORequest Result;
    bool bResultIsSet = false;

    // Determine start and end indexes for the loop based on the move direction
    size_t Index, End;
    if (bMovingOut)
    {
        Index = 0;
        End = IORequestQueue.size();
    }
    else
    {
        Index = IORequestQueue.size() - 1;
        End = -1;
    }
    while (Index != End)
    {
        const KrIORequest& IORequest = IORequestQueue[Index];
        const unsigned Track = GetTrackBySector(IORequest.Sector);

        // If requested track is the current one
        if (Track == GetCurrentTrack())
        {
            Result = IORequest;
            bResultIsSet = true;
            IORequestQueue.erase(IORequestQueue.begin() + Index);

            break;
        }
        // If requested track lays in the move direction
        else if ((Track > GetCurrentTrack()) == bMovingOut)
        {
            Result = IORequest;
            bResultIsSet = true;
            IORequestQueue.erase(IORequestQueue.begin() + Index);

            break;
        }

        if (bMovingOut)
        {
            ++Index;
        }
        else
        {
            --Index;
        }
    }

    // If unable move in the current direction
    if (!bResultIsSet)
    {
        // Flip the move direction and try one more time
        bMovingOut = !bMovingOut;
        NextIORequest();
    }
    else
    {
        SetCurrentIORequest(&Result);

        std::cout << "DRIVER: Using FLOOK strategy\n";
        std::cout << " Direction: " << (bMovingOut ? "OUT" : "IN") << "\n";
        std::cout << " Current buffer: (" << GetTrackBySector(GetCurrentIORequest()->Sector) << ":" << GetCurrentIORequest()->Sector << ")\n";
        std::cout << " Current queue: " << (bUsingQueueLeft ? "LEFT" : "RIGHT") << "\n";
        std::cout << " Left buffer queue: [";
        for (const KrIORequest& IORequest : IORequestQueueLeft)
        {
            std::cout << " (" << GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")";
        }
        std::cout << " ]\n";
        std::cout << " Right buffer queue: [";
        for (const KrIORequest& IORequest : IORequestQueueRight)
        {
            std::cout << " (" << GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")";
        }
        std::cout << " ]\n";
    }
}
