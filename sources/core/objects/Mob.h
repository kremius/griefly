#pragma once

#include "MessageReceiver.h"

#include "Interface.h"

class IMob: public IMessageReceiver
{
public:
    DECLARE_SAVED(IMob, IMessageReceiver);
    DECLARE_GET_TYPE_ITEM(imob)
    IMob(quint32 id);
    virtual void AfterWorldCreation() override;
    virtual void Delete();
    virtual void cautOverMind();

    virtual void InitGUI(){}
    virtual void DeinitGUI(){}
    virtual void GenerateInterfaceForFrame();
    virtual void ProcessMessage(const Message2& msg) override;

    virtual InterfaceBase* GetInterface() { return nullptr; }

    virtual void CalculateVisible(std::list<PosPoint>* visible_list) { ; }
};

ADD_TO_TYPELIST(IMob);
