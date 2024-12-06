#ifndef KRSPZ_DRIVER_H
#define KRSPZ_DRIVER_H

#include "IORequest.h"

#include <vector>

class KrBufferCache;
class KrScheduler;

class KrDriver
{
public:
    virtual ~KrDriver() = default;

    void SetScheduler(KrScheduler* InScheduler);
    void SetBufferCache(KrBufferCache* const InBufferCache);

    /* Request disk IO operation */
    void Request(const KrIORequest& IORequest);

    unsigned GetTrackBySector(const unsigned Sector) const;

    /* On disk driver interruption */
    void OnInterruption();

    virtual void PrintSettings() const;

protected:
    virtual void AddIORequest(const KrIORequest& IORequest) = 0;
    virtual void RemoveIORequest(const KrIORequest& IORequest) = 0;
    virtual std::vector<KrIORequest> GetIORequestQueue() const = 0;

    virtual void NextIORequest() = 0;

    void SetCurrentIORequest(const KrIORequest* const InCurrentIORequest);
    const KrIORequest* GetCurrentIORequest() const;

    unsigned GetCurrentTrack() const;

private:
    unsigned TrackNum = 10;
    unsigned SectorPerTrackNum = 500;
    unsigned HeadMoveSingleTrackTime = 500;
    unsigned HeadRewindTime = 10;
    unsigned RotationDelayTime = 4000;
    unsigned SectorAccessTime = 16;

    /* Executes next IO request if not executing yet */
    void ExecuteNextIORequest();

    unsigned CurrentTrack = 0;

    KrIORequest CurrentIORequest;
    bool bCurrentIORequestSet = false;
    bool bMoveRequested = false;

    KrScheduler* Scheduler = nullptr;
    KrBufferCache* BufferCache = nullptr;
};

#endif //KRSPZ_DRIVER_H
