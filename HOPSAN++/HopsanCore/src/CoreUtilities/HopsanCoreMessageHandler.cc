/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCoreMessageHandler.cc
//! @author <peter.nordin@liu.se>
//! @date   2010-03-03
//!
//! @brief Contains the Classes for hopsancore -> main program message exchange
//!
//$Id$
#include "CoreUtilities/HopsanCoreMessageHandler.h"

#ifdef USETBB
#include "mutex.h"
#endif

using namespace std;
using namespace hopsan;

HopsanCoreMessageHandler::HopsanCoreMessageHandler()
{
    mMaxQueueSize = 10000;
#ifdef USETBB
    mpMutex = new tbb::mutex;
#endif
}

HopsanCoreMessageHandler::~HopsanCoreMessageHandler()
{
#ifdef USETBB
    delete mpMutex;
#endif
}

void HopsanCoreMessageHandler::addMessage(const int type, const string preFix, const string message, const string tag, const int debuglevel)
{
#ifdef USETBB
    mpMutex->lock();
#endif
    HopsanCoreMessage msg;
    msg.type = type;
    msg.debuglevel = debuglevel;
    msg.message = preFix + message;
    msg.tag = tag;
    mMessageQueue.push(msg);
    if (mMessageQueue.size() > mMaxQueueSize)
    {
        //If the que is to long delete old unhandled messages
        mMessageQueue.pop();
    }
#ifdef USETBB
    mpMutex->unlock();
#endif
}


void HopsanCoreMessageHandler::addInfoMessage(const string message, const string tag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::INFO, "Info: ", message, tag, dbglevel);
}

void HopsanCoreMessageHandler::addWarningMessage(const string message, const string tag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::WARNING, "Warning: ", message, tag, dbglevel);
}

void HopsanCoreMessageHandler::addErrorMessage(const string message, const string tag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::ERROR, "Error: ", message, tag, dbglevel);
}

void HopsanCoreMessageHandler::addDebugMessage(const string message, const string tag, const int dbglevel)
{
    addMessage(HopsanCoreMessage::DEBUG, "Debug: ", message, tag, dbglevel);
}


HopsanCoreMessage HopsanCoreMessageHandler::getMessage()
{
#ifdef USETBB
    mpMutex->lock();
#endif
    HopsanCoreMessage msg;
    if (mMessageQueue.size() > 0)
    {
        msg = mMessageQueue.front();
        mMessageQueue.pop();
    }
    else
    {
        msg.type = HopsanCoreMessage::ERROR;
        msg.debuglevel = 0;
        msg.message = "Error: You requested a message even though the message queue is empty";
    }
#ifdef USETBB
    mpMutex->unlock();
#endif
    return msg;
}

size_t HopsanCoreMessageHandler::getNumWaitingMessages()
{
#ifdef USETBB
    mpMutex->lock();
    size_t num = mMessageQueue.size();
    mpMutex->unlock();
    return num;
#else
    return = mMessageQueue.size();
#endif
}

HopsanCoreMessageHandler hopsan::gCoreMessageHandler;
DLLIMPORTEXPORT HopsanCoreMessageHandler* hopsan::getCoreMessageHandlerPtr()
{
    return &gCoreMessageHandler;
}
