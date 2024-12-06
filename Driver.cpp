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
    for (const KrIORequest& OtherIORequest : GetIORequests())
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

    MoveNextTrack();
}

unsigned KrDriver::GetTrackBySector(const unsigned Sector) const
{
    return Sector / SectorPerTrackNum;
}

void KrDriver::HandleInterruption()
{
    bMoveRequested = false;

    const KrIORequest IORequest = *GetCurrentIORequest();

    const unsigned Track = GetTrackBySector(IORequest.Sector);
    CurrentTrack = Track;

    const bool bWasRead = IORequest.OperationType == KrIOOperationType::Read || IORequest.bReadFirstly;
    std::cout << "DRIVER: Completed IO (" << (bWasRead ? "read" : "write") << ")";
    std::cout << " for buffer (" << Track << ":" << IORequest.Sector << ")\n";

    if (bWasRead)
    {
        BufferCache->OnReadBuffer(IORequest.Sector);
    }
    else
    {
        BufferCache->OnWriteBuffer(IORequest.Sector);
    }

    KrUserProcess* UserProcess = Scheduler->GetUserProcessByName(IORequest.UserProcessName);

    SetCurrentIORequest(nullptr);

    if (UserProcess)
    {
        Scheduler->WakeUp(*UserProcess);
    }

    std::vector<KrIORequest> IORequests = GetIORequests();
    for (size_t Index = 0; Index < IORequests.size(); ++Index)
    {
        KrIORequest& OtherIORequest = IORequests[Index];

        if (OtherIORequest.Sector == IORequest.Sector)
        {
            if (KrUserProcess* OtherUserProcess = Scheduler->GetUserProcessByName(OtherIORequest.UserProcessName))
            {
                Scheduler->WakeUp(*OtherUserProcess);
                RemoveIORequest(OtherIORequest);
            }
        }
    }

    MoveNextTrack();
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

void KrDriver::MoveNextTrack()
{
    if (bMoveRequested)
    {
        return;
    }

    const KrIORequest* IORequest = nullptr;
    do
    {
        NextIORequest();
        IORequest = GetCurrentIORequest();
    }
    while (IORequest && (IORequest->OperationType == KrIOOperationType::Read) && BufferCache->Contains(IORequest->Sector));

    if (!IORequest)
    {
        std::cout << "DRIVER: Nothing to do\n";
        return;
    }

    bMoveRequested = true;

    const unsigned Track = GetTrackBySector(IORequest->Sector);

    const unsigned CurrentDeltaTrack = std::abs((int)Track - (int)CurrentTrack);
    const unsigned HeadMoveDirectTime = CurrentDeltaTrack * HeadMoveSingleTrackTime;

    const unsigned EdgeTrack = (CurrentTrack > TrackNum / 2) ? TrackNum : 0;
    const unsigned EdgeDeltaTrack = std::abs((int)Track - (int)EdgeTrack);
    const unsigned HeadMoveWithRewindTime = (CurrentDeltaTrack ? HeadRewindTime : 0) + EdgeDeltaTrack * HeadMoveSingleTrackTime;

    const unsigned SmallestHeadMoveTime = std::min(HeadMoveDirectTime, HeadMoveWithRewindTime);

    std::cout << "DRIVER: Moving from track " << CurrentTrack << " to " << Track << " in " << SmallestHeadMoveTime << "us\n";
    std::cout << " Direct move time: " << HeadMoveDirectTime << "us\n";
    std::cout << " Move time with rewind: " << HeadMoveWithRewindTime << "us\n";

    const unsigned IOOperationTime = SmallestHeadMoveTime + RotationDelayTime + SectorAccessTime;
    std::cout << "DRIVER: Sector access in " << IOOperationTime << "us\n";

    Scheduler->RegisterDriverInterruption(IOOperationTime);
}
