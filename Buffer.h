#ifndef KRSPZ_BUFFER_H
#define KRSPZ_BUFFER_H

struct KrBuffer final
{
    unsigned Sector;
    bool bDirty = false;

    bool operator==(const KrBuffer& Other) const
    {
        return Sector == Other.Sector;
    }

    bool operator<(const KrBuffer& Other) const
    {
        return Sector < Other.Sector;
    }
};

#endif //KRSPZ_BUFFER_H
