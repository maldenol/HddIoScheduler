#ifndef KRSPZ_SCHEDULER_H
#define KRSPZ_SCHEDULER_H

#include "UserProcess.h"

#include <vector>

class KrBufferCache;
class KrDriver;

class KrScheduler final
{
public:
    void SetBufferCache(KrBufferCache* const InBufferCache);
    void SetDriver(KrDriver* const InDriver);

    bool Tick();

    void Enqueue(const KrUserProcess& UserProcess);
    KrUserProcess* GetUserProcessByName(const std::string& UserProcessName);

    void WakeUp(KrUserProcess& UserProcess);
    void RegisterDriverInterruption(const unsigned TimeUntilDriverInterruption);

    void PrintSettings() const;

private:
    bool UpdateCurrentUserProcess();

    void SpendTime(unsigned Time, const std::string& Mode);
    void SpendTimeInUserProcess(const unsigned Time, const std::string& Mode);
    void SpendTimeInDriverInterruption();

    unsigned SysCallReadTime = 150;
    unsigned SysCallWriteTime = 150;
    unsigned DriverInterruptionTime = 50;
    unsigned ProcessingAfterReadTime = 7000;
    unsigned ProcessingBeforeWriteTime = 7000;

    unsigned SystemTime = 0;
    unsigned UserProcessTimeAfterDriverInterruption = 0;
    unsigned NextDriverInterruptionSystemTime = 0;

    std::vector<KrUserProcess> UserProcesses;
    size_t CurrentUserProcessIndex = 0;

    KrBufferCache* BufferCache = nullptr;
    KrDriver* Driver = nullptr;
};

#endif //KRSPZ_SCHEDULER_H