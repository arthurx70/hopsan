//!
//! @file   HopsanEssentials.h
//! @author <peter.nordin@liu.se>
//! @date   2010-02-19
//!
//! @brief Contains the HopsanEssentials Class
//!
//$Id$

#ifndef HOPSANESSENTIALS_H
#define HOPSANESSENTIALS_H

#include "Node.h"
#include "Port.h"
#include "Component.h"
#include "CoreUtilities/LoadExternal.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"

//! @brief This class gives access to HopsanCore Initialization, externalLib loading and ComponentCreation.
//!
//! This is a singleton class and as such it can only be used in one instance in any main program
//! further it is not capable of deleting it self. You have to do that manually by using "delete some_ptr"
//! or you can let the operating system clean it up when the program exits (If you can live with that).
//! DO NOT create an instance of this object within HopsanCore this instance will not be the same as the one in the main program.
//! If you need to use it, the main program should pass in a pointer.
//!
class DLLIMPORTEXPORT HopsanEssentials
{
private:
    static bool mHasInstance;
    static HopsanEssentials* mpInstance;
    NodeFactory* mpNodeFactory;
    ComponentFactory* mpComponentFactory;
    HopsanCoreMessageHandler* mpMessageHandler;

    void Initialize();
    HopsanEssentials();

public:
    LoadExternal externalLoader;
    HopsanCoreMessage getMessage();
    size_t checkMessage();


    static HopsanEssentials* getInstance();
    ~HopsanEssentials();

    Component* CreateComponent(const std::string &rString);
    ComponentSystem* CreateComponentSystem();
};

#endif // HOPSANESSENTIALS_H
