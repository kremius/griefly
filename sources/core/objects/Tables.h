#pragma once

#include "Movable.h"

class Table: public IMovable
{
public:
    DECLARE_SAVED(Table, IMovable);
    DECLARE_GET_TYPE_ITEM(Table);
    Table(quint32 id);
    virtual void AttackBy(IdPtr<Item> item) override;
    virtual void Delete() override;
    void NotifyNeighborhood(bool is_in_existence);
    void UpdateSprite(quint32 ignored_table);
    virtual void AfterWorldCreation() override;
protected:
    QString KV_SAVEBLE(material_);
private:
    static int CheckTable(IdPtr<IOnMapBase> container, quint32 ignored_table);
}
ADD_TO_TYPELIST(Table);



class MetalTable: public Table
{
public:
    DECLARE_SAVED(MetalTable, Table);
    DECLARE_GET_TYPE_ITEM(MetalTable);
    MetalTable(quint32 id);
}
ADD_TO_TYPELIST(MetalTable);
