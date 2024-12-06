#include "DriverLOOK.h"

#include <algorithm>
#include <iostream>

void KrDriverLOOK::PrintSettings() const
{
    KrDriver::PrintSettings();

    std::cout << "\tMaxConsecutiveAccessToTrackNum " << MaxConsecutiveAccessToTrackNum << "\n";
}

void KrDriverLOOK::AddIORequest(const KrIORequest& IORequest)
{
    // And request to the queue
    IORequestQueue.push_back(IORequest);
    // And sort the queue
    std::sort(IORequestQueue.begin(), IORequestQueue.end());
}

void KrDriverLOOK::RemoveIORequest(const KrIORequest& IORequest)
{
    IORequestQueue.erase(std::remove(IORequestQueue.begin(), IORequestQueue.end(), IORequest), IORequestQueue.end());
}

std::vector<KrIORequest> KrDriverLOOK::GetIORequestQueue() const
{
    return IORequestQueue;
}

void KrDriverLOOK::NextIORequest()
{
    if (IORequestQueue.empty())
    {
        SetCurrentIORequest(nullptr);
        return;
    }

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
            // If able to access the same track one more time
            if (CurrentConsecutiveAccessToTrackNum < MaxConsecutiveAccessToTrackNum)
            {
                ++CurrentConsecutiveAccessToTrackNum;

                Result = IORequest;
                bResultIsSet = true;
                IORequestQueue.erase(IORequestQueue.begin() + Index);

                break;
            }
        }
        // If requested track lays in the move direction
        else if ((Track > GetCurrentTrack()) == bMovingOut)
        {
            // Clear the consecutive access to track counter
            CurrentConsecutiveAccessToTrackNum = 0;

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
        // Flip the move direction, clear the consecutive access to track counter and try one more time
        bMovingOut = !bMovingOut;
        CurrentConsecutiveAccessToTrackNum = 0;
        NextIORequest();
    }
    else
    {
        SetCurrentIORequest(&Result);

        std::cout << "DRIVER: Using LOOK strategy\n";
        std::cout << " Direction: " << (bMovingOut ? "OUT" : "IN") << "\n";
        std::cout << " Current buffer: (" << GetTrackBySector(GetCurrentIORequest()->Sector) << ":" << GetCurrentIORequest()->Sector << ")\n";
        std::cout << " Buffer queue: [";
        for (const KrIORequest& IORequest : IORequestQueue)
        {
            std::cout << " (" << GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")";
        }
        std::cout << " ]\n";
    }
}
