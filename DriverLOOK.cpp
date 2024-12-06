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
    IORequestQueue.push_back(IORequest);
    std::sort(IORequestQueue.begin(), IORequestQueue.end());
}

void KrDriverLOOK::RemoveIORequest(const KrIORequest& IORequest)
{
    IORequestQueue.erase(std::remove(IORequestQueue.begin(), IORequestQueue.end(), IORequest), IORequestQueue.end());
}

std::vector<KrIORequest> KrDriverLOOK::GetIORequests() const
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

        if (Track == GetCurrentTrack())
        {
            if (CurrentConsecutiveAccessToTrackNum < MaxConsecutiveAccessToTrackNum)
            {
                ++CurrentConsecutiveAccessToTrackNum;

                Result = IORequest;
                bResultIsSet = true;
                IORequestQueue.erase(IORequestQueue.begin() + Index);

                break;
            }
        }
        else if ((Track > GetCurrentTrack()) == bMovingOut)
        {
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

    if (!bResultIsSet)
    {
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
