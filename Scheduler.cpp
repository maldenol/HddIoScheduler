#include "Scheduler.h"

#include "BufferCache.h"
#include "Driver.h"

#include <iostream>

void KrScheduler::SetBufferCache(KrBufferCache* const InBufferCache)
{
    BufferCache = InBufferCache;
}

void KrScheduler::SetDriver(KrDriver* const InDriver)
{
    Driver = InDriver;
}

bool KrScheduler::Tick()
{
    std::cout << "SCHEDULER: " << SystemTime << "us (NEXT ITERATION)\n";

    if (!UpdateCurrentUserProcess())
    {
        if (NextDriverInterruptionSystemTime == 0)
        {
            std::cout << "SCHEDULER: All user processes finished, flushing buffer cache\n";

            if (BufferCache->Flush())
            {
                std::cout << "\n";
                return true;
            }

            std::cout << "SCHEDULER: Buffer cache flushed, exiting\n\n";
            return false;
        }

        if (SystemTime >= NextDriverInterruptionSystemTime)
        {
            SpendTimeInDriverInterruption();
        }
        else
        {
            const unsigned TimeUntilDriverInterruption = NextDriverInterruptionSystemTime - SystemTime;
            std::cout << "SCHEDULER: Nothing to do for " << TimeUntilDriverInterruption << "us\n\n";
            SystemTime += TimeUntilDriverInterruption;
        }

        return true;
    }

    KrUserProcess& CurrentUserProcess = UserProcesses[CurrentUserProcessIndex];
    std::cout << "SCHEDULER: " << "Running user process \"" << CurrentUserProcess.Name << "\"";

    KrIORequest& IORequest = CurrentUserProcess.IORequests.front();
    if (IORequest.State == KrIORequestState::BeforeIO)
    {
        std::cout << " in user mode\n";

        if (IORequest.OperationType == KrIOOperationType::Write)
        {
            SpendTime(ProcessingBeforeWriteTime, "user");
        }

        std::cout << "SCHEDULER: User process \"" << CurrentUserProcess.Name << "\"";
        std::cout << " invoked " << (IORequest.OperationType == KrIOOperationType::Read ? "read" : "write") << "()";
        std::cout << " for buffer (" << Driver->GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")\n";

        IORequest.State = KrIORequestState::IOSysCall;
    }
    else if (IORequest.State == KrIORequestState::IOSysCall)
    {
        std::cout << " in kernel mode\n";

        const unsigned TimeSpent = IORequest.OperationType == KrIOOperationType::Read ? SysCallReadTime : SysCallWriteTime;
        SpendTime(TimeSpent, "kernel");

        IORequest.UserProcessName = CurrentUserProcess.Name;
        const bool bIONotBlocked = BufferCache->RequestBuffer(IORequest);
        if (bIONotBlocked)
        {
            IORequest.State = KrIORequestState::IONotBlocked;
            WakeUp(CurrentUserProcess);
        }
        else
        {
            IORequest.State = KrIORequestState::IOBlocked;
            std::cout << "SCHEDULER: Block user process \"" << CurrentUserProcess.Name << "\"\n";
        }
    }
    else if (IORequest.State == KrIORequestState::IONotBlocked)
    {
        std::cout << " in user mode\n";

        if (IORequest.OperationType == KrIOOperationType::Read)
        {
            SpendTime(ProcessingAfterReadTime, "user");
        }

        CurrentUserProcess.IORequests.erase(CurrentUserProcess.IORequests.begin());

        if (CurrentUserProcess.IORequests.empty())
        {
            std::cout << "SCHEDULER: User process \"" << CurrentUserProcess.Name << "\" exited\n";
            UserProcesses.erase(UserProcesses.begin() + CurrentUserProcessIndex);
            if (CurrentUserProcessIndex == UserProcesses.size())
            {
                CurrentUserProcessIndex = 0;
            }
        }
    }

    std::cout << "\n";
    return true;
}

void KrScheduler::Enqueue(const KrUserProcess& UserProcess)
{
    UserProcesses.push_back(UserProcess);
    std::cout << "SCHEDULER: Enqueue user process \"" << UserProcess.Name << "\": [";
    for (const KrIORequest& IORequest : UserProcess.IORequests)
    {
        std::cout << " ";
        std::cout << (IORequest.OperationType == KrIOOperationType::Read ? "R" : "W");
        std::cout << IORequest.Sector;
    }
    std::cout << " ]\n";
}

KrUserProcess* KrScheduler::GetUserProcessByName(const std::string& UserProcessName)
{
    for (KrUserProcess& UserProcess : UserProcesses)
    {
        if (UserProcess.Name == UserProcessName)
        {
            return &UserProcess;
        }
    }

    return nullptr;
}

void KrScheduler::WakeUp(KrUserProcess& UserProcess)
{
    KrIORequest& IORequest = UserProcess.IORequests.front();

    if (IORequest.State == KrIORequestState::IOBlocked)
    {
        IORequest.State = KrIORequestState::IONotBlocked;
        std::cout << "SCHEDULER: Wake up user process \"" << UserProcess.Name << "\"\n";
    }

    if (IORequest.OperationType == KrIOOperationType::Write)
    {
        std::cout << "SCHEDULER: User process \"" << UserProcess.Name << "\"";
        std::cout << " modified buffer (" << Driver->GetTrackBySector(IORequest.Sector) << ":" << IORequest.Sector << ")\n";

        BufferCache->ModifyBuffer(IORequest.Sector);
    }
}

void KrScheduler::RegisterDriverInterruption(const unsigned TimeUntilDriverInterruption)
{
    NextDriverInterruptionSystemTime = SystemTime + UserProcessTimeAfterDriverInterruption + TimeUntilDriverInterruption;
    std::cout << "SCHEDULER: Next driver interruption at " << NextDriverInterruptionSystemTime << "us\n";
}

void KrScheduler::PrintSettings() const
{
    std::cout << "\tSysCallReadTime " << SysCallReadTime << "\n";
    std::cout << "\tSysCallWriteTime " << SysCallWriteTime << "\n";
    std::cout << "\tDriverInterruptionTime " << DriverInterruptionTime << "\n";
    std::cout << "\tProcessingAfterReadTime " << ProcessingAfterReadTime << "\n";
    std::cout << "\tProcessingBeforeWriteTime " << ProcessingBeforeWriteTime << "\n";
}

bool KrScheduler::UpdateCurrentUserProcess()
{
    if (UserProcesses.empty())
    {
        return false;
    }

    size_t Index = CurrentUserProcessIndex;
    do
    {
        const std::vector<KrIORequest>& IORequests = UserProcesses[Index].IORequests;
        if (!IORequests.empty() && IORequests.front().State != KrIORequestState::IOBlocked)
        {
            CurrentUserProcessIndex = Index;
            return true;
        }

        ++Index;
        if (Index == UserProcesses.size())
        {
            Index = 0;
        }
    }
    while (Index != CurrentUserProcessIndex);

    return false;
}

void KrScheduler::SpendTime(unsigned Time, const std::string& Mode)
{
    while (Time > 0)
    {
        UserProcessTimeAfterDriverInterruption = 0;

        if (NextDriverInterruptionSystemTime == 0)
        {
            SpendTimeInUserProcess(Time, Mode);
            return;
        }

        const unsigned TimeUntilDriverInterruption = NextDriverInterruptionSystemTime - SystemTime;
        if (TimeUntilDriverInterruption >= Time)
        {
            SpendTimeInUserProcess(Time, Mode);
            return;
        }

        SpendTimeInUserProcess(TimeUntilDriverInterruption, Mode);
        Time -= TimeUntilDriverInterruption;

        UserProcessTimeAfterDriverInterruption = Time;

        SpendTimeInDriverInterruption();
    }
}

void KrScheduler::SpendTimeInUserProcess(const unsigned Time, const std::string& Mode)
{
    SystemTime += Time;

    const KrUserProcess& CurrentUserProcess = UserProcesses[CurrentUserProcessIndex];
    std::cout << "... User process \"" << CurrentUserProcess.Name << "\" spent " << Time << "us in " << Mode << " mode\n";
}

void KrScheduler::SpendTimeInDriverInterruption()
{
    NextDriverInterruptionSystemTime = 0;

    std::cout << "\n<<< Begin driver interruption at " << SystemTime << "us\n";

    SystemTime += DriverInterruptionTime;

    Driver->HandleInterruption();

    std::cout << "... Driver interruption spent " << DriverInterruptionTime << "us\n";

    std::cout << ">>> End driver interruption\n\n";
}
