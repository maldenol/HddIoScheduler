#include "BufferCache.h"

#include "Driver.h"

#include <iostream>

void KrBufferCache::SetDriver(KrDriver* const InDriver)
{
    Driver = InDriver;
}

bool KrBufferCache::RequestBuffer(const KrIORequest& IORequest)
{
    std::cout << "CACHE: Requested " << (IORequest.OperationType == KrIOOperationType::Read ? "read" : "write");
    std::cout << " for buffer (" << Driver->GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")\n";

    KrBuffer Buffer;
    Buffer.Sector = IORequest.Sector;

    std::vector<KrBuffer>::iterator BufferInSegmentLeft = std::find(SegmentLeft.begin(), SegmentLeft.end(), Buffer);
    const bool bCacheHitLeft = BufferInSegmentLeft != SegmentLeft.end();
    const bool bCacheHitRight = !bCacheHitLeft && (std::find(SegmentRight.begin(), SegmentRight.end(), Buffer) != SegmentRight.end());

    const bool bCacheHit = bCacheHitLeft || bCacheHitRight;
    std::cout << "CACHE: Buffer (" << Driver->GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")";
    std::cout << " " << (bCacheHit ? "found" : "not found") << "\n";

    if (bCacheHit)
    {
        if (bCacheHitLeft)
        {
            if (SegmentRight.size() < SegmentRightBufferNum)
            {
                const KrBuffer& MovedBuffer = *BufferInSegmentLeft;
                SegmentRight.insert(SegmentRight.begin(), MovedBuffer);
                SegmentLeft.erase(BufferInSegmentLeft);
            }
            else
            {
                const KrBuffer MovedBuffer = *BufferInSegmentLeft;
                SegmentLeft.erase(BufferInSegmentLeft);
                SegmentLeft.insert(SegmentLeft.begin(), MovedBuffer);
            }
        }

        PrintBuffer();

        return true;
    }
    else
    {
        KrIORequest IOReadRequest = IORequest;
        IOReadRequest.bReadFirstly = IOReadRequest.OperationType == KrIOOperationType::Write;

        if (SegmentLeft.size() < BufferNum - SegmentRightBufferNum)
        {
            std::cout << "CACHE: Getting free buffer\n";
        }
        else
        {
            const KrBuffer& RemovedBuffer = SegmentLeft.back();
            std::cout << "CACHE: Removing buffer (" << Driver->GetTrackBySector(RemovedBuffer.Sector) << ":" << RemovedBuffer.Sector << ") from cache\n";

            if (RemovedBuffer.bDirty)
            {
                KrIORequest IOWriteRequest;
                IOWriteRequest.Sector = RemovedBuffer.Sector;
                IOWriteRequest.OperationType = KrIOOperationType::Write;
                IOWriteRequest.bReadFirstly = false;
                Driver->Request(IOWriteRequest);
            }

            SegmentLeft.erase(SegmentLeft.end() - 1);
        }

        PrintBuffer();

        std::cout << "CACHE: Requesting driver read\n";
        Driver->Request(IOReadRequest);
    }

    return false;
}

void KrBufferCache::ModifyBuffer(const unsigned Sector)
{
    MarkDirty(Sector, true);
}

bool KrBufferCache::Flush()
{
    std::cout << "CACHE: Flushing buffers\n";

    if (SegmentLeft.empty() && SegmentRight.empty())
    {
        std::cout << "CACHE: Already flushed\n";
        return false;
    }

    KrDriver* const InnerDriver = Driver;
    auto FlushSegment = [InnerDriver](std::vector<KrBuffer>& Segment)
    {
        for (const KrBuffer& Buffer : Segment)
        {
            if (Buffer.bDirty)
            {
                KrIORequest IOWriteRequest;
                IOWriteRequest.Sector = Buffer.Sector;
                IOWriteRequest.OperationType = KrIOOperationType::Write;
                IOWriteRequest.bReadFirstly = false;
                InnerDriver->Request(IOWriteRequest);
            }
        }

        Segment.clear();
    };

    FlushSegment(SegmentLeft);
    FlushSegment(SegmentRight);

    std::cout << "CACHE: Successfully flushed\n";
    return true;
}

void KrBufferCache::OnReadBuffer(const unsigned Sector)
{
    KrBuffer Buffer;
    Buffer.Sector = Sector;

    if (std::find(SegmentLeft.begin(), SegmentLeft.end(), Buffer) == SegmentLeft.end()
        && std::find(SegmentRight.begin(), SegmentRight.end(), Buffer) == SegmentRight.end())
    {
        SegmentLeft.insert(SegmentLeft.begin(), Buffer);
        std::cout << "CACHE: Buffer (" << Driver->GetTrackBySector(Buffer.Sector) << ":" << Buffer.Sector << ") added to cache\n";
    }

    PrintBuffer();
}

void KrBufferCache::OnWriteBuffer(const unsigned Sector)
{
    MarkDirty(Sector, false);
}

bool KrBufferCache::Contains(const unsigned Sector) const
{
    KrBuffer Buffer;
    Buffer.Sector = Sector;
    return std::find(SegmentLeft.begin(), SegmentLeft.end(), Buffer) != SegmentLeft.end()
        || std::find(SegmentRight.begin(), SegmentRight.end(), Buffer) != SegmentRight.end();
}

void KrBufferCache::PrintBuffer() const
{
    std::cout << "CACHE: Using LRU strategy\n";
    std::cout << " Left segment: [";
    for (const KrBuffer& Buffer : SegmentLeft)
    {
        std::cout << " (" << Driver->GetTrackBySector(Buffer.Sector) << ":" << Buffer.Sector << ")";
    }
    std::cout << " ]\n";
    std::cout << " Right segment: [";
    for (const KrBuffer& Buffer : SegmentRight)
    {
        std::cout << " (" << Driver->GetTrackBySector(Buffer.Sector) << ":" << Buffer.Sector << ")";
    }
    std::cout << " ]\n";
}

void KrBufferCache::PrintSettings() const
{
    std::cout << "\tBufferNum " << BufferNum << "\n";
    std::cout << "\tSegmentRightBufferNum " << SegmentRightBufferNum << "\n";
}

void KrBufferCache::MarkDirty(const unsigned Sector, const bool bDirty)
{
    auto MarkDirtyInSegment = [Sector, bDirty](std::vector<KrBuffer>& Segment) -> bool
    {
        for (const KrBuffer& Buffer : Segment)
        {
            if (Buffer.Sector == Sector)
            {
                KrBuffer NewBuffer = Buffer;
                NewBuffer.bDirty = bDirty;
                std::find(Segment.begin(), Segment.end(), Buffer)->bDirty = bDirty;
                return true;
            }
        }
        return false;
    };

    if (!MarkDirtyInSegment(SegmentLeft))
    {
        MarkDirtyInSegment(SegmentRight);
    }
}