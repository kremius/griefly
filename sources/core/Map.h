#pragma once

#include <cstddef>

#include <list>
#include <vector>

#include "Constheader.h"
#include "objects/Tile.h"
#include "atmos/Atmos.h"

#include "Interfaces.h"

class MapMaster;
class LOSfinder
{
public:
    LOSfinder(IMapMaster* map);
    std::list<PosPoint>* CalculateVisisble(std::list<PosPoint>* retval, int posx, int posy, int posz = 0);
private:
    int pos2corner(int pos);
    int corner2pos(int corner);
    int sign(int value);
    bool check_corner(PosPoint p);
    PosPoint corner_point2point(PosPoint p);
    bool is_transparent(PosPoint p);
    bool bresen_x(PosPoint source, PosPoint target);
    bool bresen_y(PosPoint source, PosPoint target);
    bool ray_trace(PosPoint source, PosPoint target);
    void mark_tiles_of_corner_as_visible(
            std::list<PosPoint>* retlist,
            PosPoint at,
            PosPoint center,
            char visibility[]);
    void Clear();
    std::list<PosPoint> worklist;

    IMapMaster* map_;
};

class SyncRandom;
class MapMaster : public IMapMaster
{
public:
    MapMaster(SyncRandom* sync_random, TextPainter* texts);
    ~MapMaster();

    typedef IdPtr<CubeTile> SqType;

    virtual IAtmosphere& GetAtmosphere() override;

    virtual std::vector<std::vector<std::vector<SqType>>>& GetSquares() override;
    virtual const std::vector<std::vector<std::vector<SqType>>>& GetSquares() const override;

    virtual int GetWidth() const override;
    virtual int GetHeight() const override;
    virtual int GetDepth() const override;

    virtual std::list<PosPoint>* GetVisiblePoints() override;

    virtual void ResizeMap(int x, int y, int z) override;
    virtual void FillAtmosphere() override;

    virtual void Represent() override;

    virtual bool IsTileVisible(quint32 tile_id) override;
    virtual bool IsTransparent(int posx, int posy, int posz = 0) override;

    virtual void CalculateVisisble(std::list<PosPoint>* retval, int posx, int posy, int posz = 0) override;

    virtual bool CheckBorders(const int* x, const int* y, const int* z) const override;
private:
    LOSfinder losf_;
    IAtmosphere* atmosphere_;
    std::vector<std::vector<std::vector<SqType>>> squares_;
    std::list<PosPoint>* visible_points_;
};
