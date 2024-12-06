#ifndef KRSPZ_IOREQUEST_H
#define KRSPZ_IOREQUEST_H

#include <string>

struct KrUserProcess;

enum class KrIOOperationType
{
    Read,
    Write,
};

enum class KrIORequestState
{
    BeforeIO,
    IOSysCall,
    IOBlocked,
    IONotBlocked,
};

struct KrIORequest final
{
    unsigned Sector;
    KrIOOperationType OperationType;

    /* Whether the sector should be read before writing */
    bool bReadFirstly = false;
    KrIORequestState State = KrIORequestState::BeforeIO;
    std::string UserProcessName;

    bool operator==(const KrIORequest& Other) const
    {
        return Sector == Other.Sector
            && OperationType == Other.OperationType
            && bReadFirstly == Other.bReadFirstly;
    }

    bool operator<(const KrIORequest& Other) const
    {
        return Sector < Other.Sector;
    }
};

#endif //KRSPZ_IOREQUEST_H
