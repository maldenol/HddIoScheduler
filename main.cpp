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
    KrScheduler Scheduler;
    KrBufferCache BufferCache;
//    KrDriverFIFO Driver;
//    KrDriverLOOK Driver;
    KrDriverFLOOK Driver;

    Scheduler.SetBufferCache(&BufferCache);
    Scheduler.SetDriver(&Driver);
    BufferCache.SetDriver(&Driver);
    Driver.SetScheduler(&Scheduler);
    Driver.SetBufferCache(&BufferCache);

    std::cout << "Settings:\n";
    Scheduler.PrintSettings();
    BufferCache.PrintSettings();
    Driver.PrintSettings();
    std::cout << "\n";

//    KrUserProcess UserProcess0;
//    UserProcess0.Name = "yyy";
//    UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Read});
//    Scheduler.Enqueue(UserProcess0);
//
//    KrUserProcess UserProcess1;
//    UserProcess1.Name = "qqq";
//    UserProcess1.IORequests.push_back(KrIORequest{110, KrIOOperationType::Read});
//    Scheduler.Enqueue(UserProcess1);
//
//    KrUserProcess UserProcess2;
//    UserProcess2.Name = "eee";
//    UserProcess2.IORequests.push_back(KrIORequest{1500, KrIOOperationType::Read});
//    Scheduler.Enqueue(UserProcess2);

//    KrUserProcess UserProcess0;
//    UserProcess0.Name = "yyy";
//    UserProcess0.IORequests.push_back(KrIORequest{100, KrIOOperationType::Write});
//    UserProcess0.IORequests.push_back(KrIORequest{110, KrIOOperationType::Read});
//    UserProcess0.IORequests.push_back(KrIORequest{120, KrIOOperationType::Read});
//    UserProcess0.IORequests.push_back(KrIORequest{130, KrIOOperationType::Read});
//    UserProcess0.IORequests.push_back(KrIORequest{140, KrIOOperationType::Read});
//    UserProcess0.IORequests.push_back(KrIORequest{150, KrIOOperationType::Read});
//    UserProcess0.IORequests.push_back(KrIORequest{160, KrIOOperationType::Read});
//    UserProcess0.IORequests.push_back(KrIORequest{170, KrIOOperationType::Write});
//    Scheduler.Enqueue(UserProcess0);

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
    }

    {
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
    }

    {
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

    return 0;
}
