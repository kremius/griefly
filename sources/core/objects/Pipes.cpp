#include "Pipes.h"

#include "../Helpers.h"
#include "representation/Chat.h"
#include "../Game.h"

#include "ElectricTools.h"

PipeBase::PipeBase(quint32 id) : IMovable(id)
{
    anchored = true;
    v_level = 1;

    SetSprite("icons/pipes.dmi");
    //SetState("intact");

    name = "Please do not create me";

    AddDefaultValues(&atmos_holder_);
}

void PipeBase::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<AtmosTool> at = item)
    {
        GetGame().GetChat().PostTextFor(AtmosTool::GetInfo(atmos_holder_), at->GetOwner());
        return;
    }
}

void PipeBase::ConnectHelper(IdPtr<PipeBase>& connection, Dir dir)
{
    if (!connection.IsValid())
    {
        GetNeighbour(dir)->ForEach(
        [&](IdPtr<IOnMapBase> obj)
        {
            if (connection.IsValid())
            {
                return;
            }
            if (IdPtr<PipeBase> pipe = obj)
            {
                if (pipe->Connect(helpers::revert_dir(dir), GetId()))
                {
                    connection = pipe;
                }
            }
        });
    }
}

void PipeBase::ProcessHelper(IdPtr<PipeBase>& connection, Dir dir)
{
    if (connection.IsValid())
    {
        if (connection->CanTransferGas(helpers::revert_dir(dir)))
        {
            connection->GetAtmosHolder()->Connect(GetAtmosHolder());
        }
    }
    else
    {
        if (IdPtr<CubeTile> cube = owner)
        {
            cube->GetAtmosHolder()->Connect(GetAtmosHolder());
        }
    }
}


Pipe::Pipe(quint32 id) : PipeBase(id)
{
    SetState("intact");

    name = "Pipe";
}

bool Pipe::Connect(Dir dir, IdPtr<PipeBase> pipe)
{
    Dir head;
    Dir tail;
    GetTailAndHead(GetDir(), &head, &tail);

    if (   (dir == head)
        && (!head_.IsValid()))
    {
        head_ = pipe;
        return true;
    }
    if (   (dir == tail)
        && (!tail_.IsValid()))
    {
        tail_ = pipe;
        return true;
    }

    return false;
}

void Pipe::AfterWorldCreation()
{
    PipeBase::AfterWorldCreation();
    SetFreq(1);

    Dir head;
    Dir tail;
    GetTailAndHead(GetDir(), &head, &tail);

    ConnectHelper(head_, head);
    ConnectHelper(tail_, tail);
}

void Pipe::Process()
{
    Dir head;
    Dir tail;
    GetTailAndHead(GetDir(), &head, &tail);

    ProcessHelper(head_, head);
    ProcessHelper(tail_, tail);
}

void Pipe::GetTailAndHead(Dir dir, Dir* head, Dir* tail)
{
    static int DIRS_DATA[10][2]
        = {
        {D_LEFT, D_RIGHT}, // D_LEFT
        {D_RIGHT, D_LEFT}, // D_RIGHT
        {D_UP, D_DOWN}, // D_UP
        {D_DOWN, D_UP}, // D_DOWN
        {D_ZUP, D_ZDOWN}, // D_ZUP
        {D_ZDOWN, D_ZUP}, // D_ZDOWN
        {D_RIGHT, D_DOWN}, // D_SOUTHEAST
        {D_DOWN, D_LEFT}, // D_SOUTHWEST
        {D_UP, D_RIGHT}, // D_NORTHEAST
        {D_LEFT, D_UP} // D_NORTHWEST
        };
    *head = DIRS_DATA[dir][0];
    *tail = DIRS_DATA[dir][1];
}

Manifold::Manifold(quint32 id) : PipeBase(id)
{
    SetState("manifold");
    name = "Manifold";
}

bool Manifold::Connect(Dir dir, IdPtr<PipeBase> pipe)
{
    Dir tail;
    Dir left;
    Dir right;
    GetConnectionsDirs(GetDir(), &tail, &left, &right);

    if (   (dir == tail)
        && (!tail_.IsValid()))
    {
        tail_ = pipe;
        return true;
    }

    if (   (dir == left)
        && (!left_.IsValid()))
    {
        left_ = pipe;
        return true;
    }

    if (   (dir == right)
        && (!right_.IsValid()))
    {
        right_ = pipe;
        return true;
    }

    return false;
}

void Manifold::AfterWorldCreation()
{
    PipeBase::AfterWorldCreation();

    SetFreq(1);

    Dir tail;
    Dir left;
    Dir right;
    GetConnectionsDirs(GetDir(), &tail, &left, &right);

    ConnectHelper(tail_, tail);
    ConnectHelper(left_, left);
    ConnectHelper(right_, right);
}

bool Manifold::CanTransferGas(Dir dir) const
{
    return true;
}

void Manifold::Process()
{
    Dir tail;
    Dir left;
    Dir right;
    GetConnectionsDirs(GetDir(), &tail, &left, &right);

    ProcessHelper(tail_, tail);
    ProcessHelper(left_, left);
    ProcessHelper(right_, right);
}

void Manifold::GetConnectionsDirs(Dir dir, Dir *tail, Dir *left, Dir *right)
{
    *tail = helpers::revert_dir(dir);
    if (dir == D_DOWN)
    {
        *left = D_RIGHT;
        *right = D_LEFT;
    }
    else if (dir == D_UP)
    {
        *left = D_LEFT;
        *right = D_RIGHT;
    }
    else if (dir == D_RIGHT)
    {
        *left = D_UP;
        *right = D_DOWN;
    }
    else
    {
        *left = D_DOWN;
        *right = D_UP;
    }
}

Vent::Vent(quint32 id) : PipeBase(id)
{
    SetSprite("icons/vent_pump.dmi");

    SetHidden(false);

    v_level = 3;

    name = "Vent";
}

bool Vent::Connect(Dir dir, IdPtr<PipeBase> pipe)
{
    if (dir != GetDir())
    {
        return false;
    }
    if (tail_.IsValid())
    {
        return false;
    }

    tail_ = pipe;
    return true;
}

void Vent::AfterWorldCreation()
{
    PipeBase::AfterWorldCreation();

    SetFreq(1);
    ConnectHelper(tail_, GetDir());
}

void Vent::Process()
{
    ProcessHelper(tail_, GetDir());
    if (IdPtr<CubeTile> cube = owner)
    {
        cube->GetAtmosHolder()->Connect(GetAtmosHolder());
    }
}

void Vent::SetHidden(bool hidden)
{
    if (hidden)
    {
        SetState("hoff");
    }
    else
    {
        SetState("off");
    }
}

Valve::Valve(quint32 id) : Pipe(id)
{
    SetSprite("icons/digital_valve.dmi");
    SetState("valve0");
    closed_ = true;
}

bool Valve::CanTransferGas(Dir dir) const
{
    if (closed_)
    {
        return false;
    }
    return true;
}

void Valve::Process()
{
    if (closed_)
    {
        return;\
    }
    Pipe::Process();
}

void Valve::AttackBy(IdPtr<Item> item)
{
    if (closed_)
    {
        closed_ = false;
        SetState("valve1");
    }
    else
    {
        closed_ = true;
        SetState("valve0");
    }
}

Connector::Connector(quint32 id) : PipeBase(id)
{
    SetState("connector");
    v_level = 3;

    name = "Connector";
}

void Connector::ConnectToGasTank(IdPtr<GasTank> tank)
{
    tank_ = tank;
}

void Connector::DisconnectFromGasTank()
{
    tank_ = 0;
}

bool Connector::Connect(Dir dir, IdPtr<PipeBase> pipe)
{
    if (dir != GetDir())
    {
        return false;
    }
    if (tail_.IsValid())
    {
        return false;
    }

    tail_ = pipe;
    return true;
}

void Connector::AfterWorldCreation()
{
    PipeBase::AfterWorldCreation();
    SetFreq(1);
    ConnectHelper(tail_, GetDir());
    if (auto tank = owner->GetItem<GasTank>())
    {
        ConnectToGasTank(tank);
        tank->anchored = true;
    }
}

void Connector::Process()
{
    ProcessHelper(tail_, GetDir());
    if (tank_)
    {
        tank_->GetAtmosHolder()->Connect(GetAtmosHolder());
    }
}


PipePump::PipePump(quint32 id) : Pipe(id)
{
    SetSprite("icons/pipes2.dmi");
    SetState("pipepump-run");

    pump_pressure_ = 38000;
}

bool PipePump::CanTransferGas(Dir dir) const
{
    if (helpers::revert_dir(dir) == GetDir())
    {
        return true;
    }
    return false;
}

void PipePump::Process()
{
    Dir head;
    Dir tail;
    GetTailAndHead(GetDir(), &head, &tail);
    ProcessHelper(tail_, tail);
    AtmosHolder* head_connection = nullptr;
    if (head_)
    {
        head_connection = head_->GetAtmosHolder();
    }
    if (head_connection == nullptr)
    {
        if (IdPtr<CubeTile> cube = owner)
        {
            head_connection = cube->GetAtmosHolder();
        }
    }
    if (head_connection == nullptr)
    {
        return;
    }

    if (static_cast<int>(head_connection->GetPressure()) >= pump_pressure_)
    {
        return;
    }

    head_connection->Connect(
        GetAtmosHolder(),
        MAX_GAS_LEVEL,
        MAX_GAS_LEVEL / 4,
        MAX_GAS_LEVEL);
}
