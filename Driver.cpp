#include "Driver.h"

#include "BufferCache.h"
#include "Scheduler.h"

#include <iostream>

void KrDriver::SetScheduler(KrScheduler* InScheduler)
{
    Scheduler = InScheduler;
}

void KrDriver::SetBufferCache(KrBufferCache* const InBufferCache)
{
    BufferCache = InBufferCache;
}

void KrDriver::Request(const KrIORequest& IORequest)
{
    // If there are IO operations for the requested sector
    // no need to read the sector from the disk
    std::vector<KrIORequest> IORequests = GetIORequestQueue();
    if (const KrIORequest* const InnerCurrentIORequest = GetCurrentIORequest())
    {
        IORequests.push_back(*InnerCurrentIORequest);
    }
    for (const KrIORequest& OtherIORequest : IORequests)
    {
        if (OtherIORequest.Sector == IORequest.Sector)
        {
            const unsigned Track = GetTrackBySector(IORequest.Sector);
            std::cout << "DRIVER: Already requested IO (read)";
            std::cout << " for buffer (" << Track << ":" << IORequest.Sector << ")\n";

            KrIORequest IORequestCopy = IORequest;
            IORequestCopy.bReadFirstly = false;
            AddIORequest(IORequestCopy);

            return;
        }
    }

    AddIORequest(IORequest);

    const bool bRead = IORequest.OperationType == KrIOOperationType::Read || IORequest.bReadFirstly;
    const unsigned Track = GetTrackBySector(IORequest.Sector);
    std::cout << "DRIVER: Requested IO (" << (bRead ? "read" : "write") << ")";
    std::cout << " for buffer (" << Track << ":" << IORequest.Sector << ")\n";

    ExecuteNextIORequest();
}

unsigned KrDriver::GetTrackBySector(const unsigned Sector) const
{
    return Sector / SectorPerTrackNum;
}

void KrDriver::OnInterruption()
{
    bMoveRequested = false;

    const KrIORequest IORequest = *GetCurrentIORequest(); // getting by value cause a reference can become invalid

    const unsigned Track = GetTrackBySector(IORequest.Sector);
    CurrentTrack = Track;

    const bool bWasRead = IORequest.OperationType == KrIOOperationType::Read || IORequest.bReadFirstly;
    std::cout << "DRIVER: Completed IO (" << (bWasRead ? "read" : "write") << ")";
    std::cout << " for buffer (" << Track << ":" << IORequest.Sector << ")\n";

    // Notifying the buffer cache about the IO operation completion
    if (bWasRead)
    {
        BufferCache->OnReadBuffer(IORequest.Sector);
    }
    else
    {
        BufferCache->OnWriteBuffer(IORequest.Sector);
    }

    // Waking up the user process if it is still running
    if (KrUserProcess* UserProcess = Scheduler->GetUserProcessByName(IORequest.UserProcessName))
    {
        Scheduler->WakeUp(*UserProcess);
    }

    // Waking up other user processes that have requested IO operation for the same sector
    for (const KrIORequest& OtherIORequest : GetIORequestQueue())
    {
        if (OtherIORequest.Sector == IORequest.Sector)
        {
            if (KrUserProcess* OtherUserProcess = Scheduler->GetUserProcessByName(OtherIORequest.UserProcessName))
            {
                Scheduler->WakeUp(*OtherUserProcess);
                RemoveIORequest(OtherIORequest);
            }
        }
    }

    ExecuteNextIORequest();
}

void KrDriver::PrintSettings() const
{
    std::cout << "\tTrackNum " << TrackNum << "\n";
    std::cout << "\tSectorPerTrackNum " << SectorPerTrackNum << "\n";
    std::cout << "\tHeadMoveSingleTrackTime " << HeadMoveSingleTrackTime << "\n";
    std::cout << "\tHeadRewindTime " << HeadRewindTime << "\n";
    std::cout << "\tRotationDelayTime " << RotationDelayTime << "\n";
    std::cout << "\tSectorAccessTime " << SectorAccessTime << "\n";
}

void KrDriver::SetCurrentIORequest(const KrIORequest* const InCurrentIORequest)
{
    bCurrentIORequestSet = InCurrentIORequest;
    if (InCurrentIORequest)
    {
        CurrentIORequest = *InCurrentIORequest;
    }
}

const KrIORequest* KrDriver::GetCurrentIORequest() const
{
    if (bCurrentIORequestSet)
    {
        return &CurrentIORequest;
    }
    return nullptr;
}

unsigned KrDriver::GetCurrentTrack() const
{
    return CurrentTrack;
}

void KrDriver::ExecuteNextIORequest()
{
    if (bMoveRequested)
    {
        return;
    }

    // Get next IO request
    NextIORequest();
    const KrIORequest* const IORequest = GetCurrentIORequest();

    // If there are no IO requests
    if (!IORequest)
    {
        std::cout << "DRIVER: Nothing to do\n";
        return;
    }

    bMoveRequested = true;

    // Requested track
    const unsigned Track = GetTrackBySector(IORequest->Sector);

    // Direct move
    const unsigned CurrentDeltaTrack = std::abs((int)Track - (int)CurrentTrack);
    const unsigned HeadMoveDirectTime = CurrentDeltaTrack * HeadMoveSingleTrackTime;

    // Move with rewind
    const unsigned EdgeTrack = (CurrentTrack > TrackNum / 2) ? TrackNum : 0;
    const unsigned EdgeDeltaTrack = std::abs((int)Track - (int)EdgeTrack);
    const unsigned HeadMoveWithRewindTime = (CurrentDeltaTrack ? HeadRewindTime : 0) + EdgeDeltaTrack * HeadMoveSingleTrackTime;

    // Smallest move time
    const unsigned SmallestHeadMoveTime = std::min(HeadMoveDirectTime, HeadMoveWithRewindTime);

    std::cout << "DRIVER: Moving from track " << CurrentTrack << " to " << Track << " in " << SmallestHeadMoveTime << "us\n";
    std::cout << " Direct move time: " << HeadMoveDirectTime << "us\n";
    std::cout << " Move time with rewind: " << HeadMoveWithRewindTime << "us\n";

    const unsigned IOOperationTime = SmallestHeadMoveTime + RotationDelayTime + SectorAccessTime;
    std::cout << "DRIVER: Sector access in " << IOOperationTime << "us\n";

    // "Plan" an interruption
    Scheduler->RegisterDriverInterruption(IOOperationTime);
}
