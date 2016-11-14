#pragma once

#include <array>
#include <vector>

#include "AtmosHolder.h"
#include "../Constheader.h"
#include "Interfaces.h"

class SyncRandom;
class IMapMaster;
class AtmosGrid;
class TextPainter;

class Atmosphere : public IAtmosphere
{
public:
    Atmosphere(SyncRandom* random, IMapMaster* map, TextPainter* texts);
    virtual void Resize(quint32 x, quint32 y, quint32 z) override;
    virtual void Process() override;
    virtual void ProcessMove() override;

    virtual void SetFlags(quint32 x, quint32 y, quint32 z, Flags flags) override;
    virtual void LoadGrid() override;
private:
    AtmosGrid* grid_;

    qint64 grid_processing_ns_;
    qint64 movement_processing_ns_;

    void ProcessTileMove(int x, int y, int z);

    int x_size_;
    int y_size_;
    int z_size_;

    SyncRandom* random_;
    IMapMaster* map_;
    TextPainter* texts_;
};
