/*
 * Системнe програмнe забезпечення
 * Курсова робота
 * Алгоритми планування введення-виведення для жорсткого диска та управління буферним кешем
 * Мальований Денис Олегович, ІО-13
 * Варіант 4 (1315 % 4 + 1 = 4)
 * LRU з двома сегментами, FIFO, LOOK, FLOOK
 */

#include "BufferCache.h"
#include "DriverFIFO.h"
#include "DriverFLOOK.h"
#include "DriverLOOK.h"
#include "Scheduler.h"

#include <iostream>

int main()
{
    int ExampleIndex;
    std::cout << "Enter example index (0-8):";
    std::cin >> ExampleIndex;

    int DiskDriverStrategy;
    std::cout << "Enter disk driver strategy (1-3):";
    std::cin >> DiskDriverStrategy;

    std::cout << "\n";

    KrScheduler Scheduler;
    KrBufferCache BufferCache;
    KrDriver* Driver = nullptr;
    if (DiskDriverStrategy == 1)
    {
        Driver = new KrDriverFIFO{};
    }
    else if (DiskDriverStrategy == 2)
    {
        Driver = new KrDriverLOOK{};
    }
    else if (DiskDriverStrategy == 3)
    {
        Driver = new KrDriverFLOOK{};
    }
    else
    {
        std::cout << "CONSOLE: Unsupported strategy index\n";
        return 0;
    }

    std::cout << "CONSOLE: Change MaxConsecutiveAccessToTrackNum in the header file\n\n";
    std::cout << "CONSOLE: Change BufferNum and SegmentRightBufferNum in the header file\n\n";

    Scheduler.SetBufferCache(&BufferCache);
    Scheduler.SetDriver(Driver);
    BufferCache.SetDriver(Driver);
    Driver->SetScheduler(&Scheduler);
    Driver->SetBufferCache(&BufferCache);

    std::cout << "Settings:\n";
    Scheduler.PrintSettings();
    BufferCache.PrintSettings();
    Driver->PrintSettings();
    std::cout << "\n";

    // Example #1
    if (ExampleIndex == 1)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess0);
    }

    // Example #2
    if (ExampleIndex == 2)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess0);
    }

    // Example #3
    if (ExampleIndex == 3)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess0);

        KrUserProcess UserProcess1;
        UserProcess1.Name = "qqq";
        UserProcess1.IORequests.push_back(KrIORequest{1000, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess1);
    }

    // Example #4
    if (ExampleIndex == 4)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess0);

        KrUserProcess UserProcess1;
        UserProcess1.Name = "qqq";
        UserProcess1.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess1);
    }

    // Example #5
    if (ExampleIndex == 5)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Write});
        UserProcess0.IORequests.push_back(KrIORequest{110, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{120, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{130, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{140, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{150, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{160, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{170, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess0);
    }

    // Example #6
    if (ExampleIndex == 6)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess0);

        KrUserProcess UserProcess1;
        UserProcess1.Name = "qqq";
        UserProcess1.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess1);
    }

    // Example #7 & #8
    if (ExampleIndex == 7 || ExampleIndex == 8)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess0);

        KrUserProcess UserProcess1;
        UserProcess1.Name = "qqq";
        UserProcess1.IORequests.push_back(KrIORequest{110, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess1);

        KrUserProcess UserProcess2;
        UserProcess2.Name = "eee";
        UserProcess2.IORequests.push_back(KrIORequest{1500, KrIOOperationType::Read});
        Scheduler.Enqueue(UserProcess2);
    }

    // Own example
    if (ExampleIndex == 0)
    {
        KrUserProcess UserProcess0;
        UserProcess0.Name = "yyy";
        UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{110, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{500, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{510, KrIOOperationType::Read});
        UserProcess0.IORequests.push_back(KrIORequest{1000, KrIOOperationType::Write});
        UserProcess0.IORequests.push_back(KrIORequest{1010, KrIOOperationType::Write});
        UserProcess0.IORequests.push_back(KrIORequest{1500, KrIOOperationType::Write});
        UserProcess0.IORequests.push_back(KrIORequest{1510, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess0);

        KrUserProcess UserProcess1;
        UserProcess1.Name = "qqq";
        UserProcess1.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        UserProcess1.IORequests.push_back(KrIORequest{110, KrIOOperationType::Read});
        UserProcess1.IORequests.push_back(KrIORequest{500, KrIOOperationType::Write});
        UserProcess1.IORequests.push_back(KrIORequest{510, KrIOOperationType::Write});
        UserProcess1.IORequests.push_back(KrIORequest{1000, KrIOOperationType::Read});
        UserProcess1.IORequests.push_back(KrIORequest{1010, KrIOOperationType::Read});
        UserProcess1.IORequests.push_back(KrIORequest{1500, KrIOOperationType::Write});
        UserProcess1.IORequests.push_back(KrIORequest{1510, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess1);

        KrUserProcess UserProcess2;
        UserProcess2.Name = "eee";
        UserProcess2.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
        UserProcess2.IORequests.push_back(KrIORequest{110, KrIOOperationType::Write});
        UserProcess2.IORequests.push_back(KrIORequest{500, KrIOOperationType::Read});
        UserProcess2.IORequests.push_back(KrIORequest{510, KrIOOperationType::Write});
        UserProcess2.IORequests.push_back(KrIORequest{1000, KrIOOperationType::Read});
        UserProcess2.IORequests.push_back(KrIORequest{1010, KrIOOperationType::Write});
        UserProcess2.IORequests.push_back(KrIORequest{1500, KrIOOperationType::Read});
        UserProcess2.IORequests.push_back(KrIORequest{1510, KrIOOperationType::Write});
        Scheduler.Enqueue(UserProcess2);
    }

    std::cout << "\n";

    while (Scheduler.Tick());

    delete Driver;

    return 0;
}
