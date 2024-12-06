#ifndef KRSPZ_BUFFERCACHE_H
#define KRSPZ_BUFFERCACHE_H

#include "Buffer.h"
#include "IORequest.h"

#include <vector>

class KrDriver;

class KrBufferCache final
{
public:
    void SetDriver(KrDriver* const InDriver);

    /* Request buffer access, returns whether a cache hit happened */
    bool RequestBuffer(const KrIORequest& IORequest);
    void ModifyBuffer(const unsigned Sector);

    bool Flush();

    /* On buffer read from the disk */
    void OnReadBuffer(const unsigned Sector);
    /* On buffer written to the disk */
    void OnWriteBuffer(const unsigned Sector);

    void PrintBuffer() const;

    void PrintSettings() const;

private:
    void MarkDirty(const unsigned Sector, const bool bDirty);

    size_t BufferNum = 7;
    size_t SegmentRightBufferNum = 3;

    std::vector<KrBuffer> SegmentLeft;
    std::vector<KrBuffer> SegmentRight;

    KrDriver* Driver = nullptr;
};

#endif //KRSPZ_BUFFERCACHE_H
