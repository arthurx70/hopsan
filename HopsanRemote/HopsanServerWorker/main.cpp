//$Id$

#include <sstream>
#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <signal.h>

#include "zmq.hpp"

#include "Messages.h"
#include "MessageUtilities.h"
#include "ServerMessageUtilities.h"
#include "FileAccess.h"

#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "TicToc.hpp"

#ifdef _WIN32
#include "windows.h"
#endif

using namespace hopsan;
using namespace std;

HopsanEssentials gHopsanCore;

typedef struct
{
    string fullName;
    vector< vector<double> > *pData = 0;
    vector< double > *pTimeData = 0;
    size_t dataLength = 0;
    size_t dataId = 0;
    NodeDataDescription *pDataDescription = 0;
    string unit;
}ModelVariableInfo_t;

// ------------------------------
// Model utilities BEGIN
// ------------------------------
//! @todo Model utilities should not be in this file they should be shared

//! @todo vector for vars bad maybe list or map better
void collectAllModelVariables(ComponentSystem *pSys, vector<ModelVariableInfo_t> &rvMVI, HString systemHierarchy)
{
    // Append this systems time vector
    vector<double> *pTime = pSys->getLogTimeVector();
    ModelVariableInfo_t tmvi;
    tmvi.fullName = (systemHierarchy+"Time").c_str();
    tmvi.unit = "s";
    tmvi.pTimeData = pTime;
    tmvi.dataLength = pSys->getNumActuallyLoggedSamples();
    rvMVI.push_back(tmvi);

    vector<Component *> subComps = pSys->getSubComponents();
    for (size_t c=0; c<subComps.size(); ++c)
    {
        Component *pComp = subComps[c];
        if (pComp->isComponentSystem())
        {
            // Collect results for subsystem
            collectAllModelVariables(static_cast<ComponentSystem*>(pComp), rvMVI, systemHierarchy+pComp->getName()+"$");
        }
        else
        {
            vector<Port*> ports = pComp->getPortPtrVector();
            for (size_t p=0; p<ports.size(); ++p)
            {
                //cout << "port: " << p << " of: " << ports.size() << endl;
                Port *pPort = ports[p];
                if (pPort->isMultiPort())
                {
                    // Ignore multiports, not possible to determine what we want to log anyway
                    continue;
                }

                //! @todo what about time vector
                vector< vector<double> > *pLogData = pPort->getLogDataVectorPtr();

                const vector<NodeDataDescription> *pVars = pPort->getNodeDataDescriptions();
                if (pVars)
                {
                    for (size_t v=0; v<pVars->size(); ++v)
                    {
                        // Only write something if data has been logged (skip ports that are not logged)
                        // We assume that the data vector has been cleared
                        //! @todo check if log on
                        if (pLogData->size() > 0)
                        {
                            const NodeDataDescription *pVarDesc = &(*pVars)[v];
                            ModelVariableInfo_t mvi;
                            mvi.fullName = (systemHierarchy+pComp->getName()+"#"+pPort->getName()+"#"+pVarDesc->name).c_str();
                            //mvi.pDataDescription = pVarDesc;
                            mvi.unit = pVarDesc->unit.c_str();
                            mvi.pData = pLogData;
                            mvi.dataId = v;
                            mvi.dataLength = pSys->getNumActuallyLoggedSamples();

                            rvMVI.push_back(mvi);
                        }
                    }
                }
            }
        }
    }
}

void splitStringOnDelimiter(const std::string &rString, const char delim, std::vector<std::string> &rSplitVector)
{
    rSplitVector.clear();
    string item;
    stringstream ss(rString);
    while(getline(ss, item, delim))
    {
        rSplitVector.push_back(item);
    }
}

bool setParameter(ComponentSystem *pSystem, HString &fullname, const HString &rValue)
{
    if (pSystem)
    {
        size_t d = fullname.find_first_of('$');
        if (d != HString::npos)
        {
            HString sysname = fullname.substr(0, d);
            fullname.erase(0, d+1);
            ComponentSystem *pSubsys = pSystem->getSubComponentSystem(sysname);
            return setParameter(pSubsys, fullname, rValue);
        }
        else
        {
            //! @todo write split function in hstring class
            string item;
            stringstream ss(fullname.c_str());
            vector<string> cpv;
            while(getline(ss, item, '#'))
            {
                cpv.push_back(item);
            }

            if (cpv.size() == 2 || cpv.size() == 3)
            {
                Component *pComp = pSystem->getSubComponent(cpv[0].c_str());
                if (pComp)
                {
                    string parameter;
                    if (cpv.size() == 2)
                    {
                        parameter = cpv[1];
                    }
                    else if (cpv.size() == 3)
                    {
                        // Set component name and restor the (startvalue) name
                        parameter = cpv[1]+"#"+cpv[2];
                    }

                    return pComp->setParameterValue(parameter.c_str(), rValue);
                }
            }
        }
    }
    return false;
}

string getParameter(ComponentSystem *pSystem, HString &fullname)
{
    if (pSystem)
    {
        size_t d = fullname.find_first_of('$');
        if (d != HString::npos)
        {
            HString sysname = fullname.substr(0, d);
            fullname.erase(0, d+1);
            ComponentSystem *pSubsys = pSystem->getSubComponentSystem(sysname);
            return getParameter(pSubsys, fullname);
        }
        else
        {
            //! @todo write split function in hstring class
            string item;
            stringstream ss(fullname.c_str());
            vector<string> cpv;
            while(getline(ss, item, '#'))
            {
                cpv.push_back(item);
            }

            if (cpv.size() == 2 || cpv.size() == 3)
            {
                Component *pComp = pSystem->getSubComponent(cpv[0].c_str());
                if (pComp)
                {
                    string parameter;
                    if (cpv.size() == 2)
                    {
                        parameter = cpv[1];
                    }
                    else if (cpv.size() == 3)
                    {
                        // Set component name and restor the (startvalue) name
                        parameter = cpv[1]+"#"+cpv[2];
                    }

                    HString value;
                    pComp->getParameterValue(parameter.c_str(), value);
                    return value.c_str();
                }
            }
        }
    }
    return "";
}

// ------------------------------
// Model utilities END
// ------------------------------

string gWorkerId;
#define PRINTWORKER "Worker "+gWorkerId+"; "

std::string nowDateTime()
{
    std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buff[64];
    std::strftime(buff, sizeof(buff), "%b %d %H:%M:%S", std::localtime(&now_time));
    return std::string(&buff[0]);
}

void loadComponentLibraries(const std::string &rDir)
{
    FileAccess fa;
    if (fa.enterDir(rDir))
    {
        vector<string> soFiles = fa.findFilesWithSuffix(TO_STR(DLL_EXT));
        for (string f : soFiles)
        {
            cout << PRINTWORKER << nowDateTime() << " Loading library file: " << f << endl;
            gHopsanCore.loadExternalComponentLib(f.c_str());
        }
    }
    else
    {
        cout << PRINTWORKER << nowDateTime() << " Error: Could not enter directory: " << rDir << endl;
    }
}

void sendServerGoodby(zmq::socket_t &rSocket)
{
    zmq::message_t response;
    sendServerStringMessage(rSocket, SW_Finished, gWorkerId);
    receiveWithTimeout(rSocket, 5000, response); // Wait for but ignore replay
}

ComponentSystem *pRootSystem=nullptr;
double simStartTime, simStopTime;
size_t nThreads = 1;

static int s_interrupted = 0;
#ifdef _WIN32
BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
{
    // what to do here?
    s_interrupted = 1;
    return TRUE;
}
#else
static void s_signal_handler(int signal_value)
{
    s_interrupted = 1;
}

static void s_catch_signals(void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}
#endif

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << PRINTWORKER << nowDateTime() << " Error: To few arguments!" << endl;
        return 1;
    }

    //cout << "argv: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
    gWorkerId = argv[1];
    string serverCtrlPort = argv[2];
    string workerCtrlPort = argv[3];

    // Read num threads argument
    if (argc == 5)
    {
        nThreads = size_t(atoi(argv[4]));
    }

    cout << PRINTWORKER << nowDateTime() << " Listening on port: " << workerCtrlPort << " Using: " << nThreads << " threads" << endl;
    cout << PRINTWORKER << nowDateTime() << " Server control port is: " << serverCtrlPort << endl;

    // Loading component libraries
    loadComponentLibraries("./componentLibraries");
    loadComponentLibraries("../componentLibraries/defaultLibrary");

    // Prepare our context and sockets
#ifdef _WIN32
    zmq::context_t context(1, 63);
#else
    zmq::context_t context(1);
#endif

    zmq::socket_t socket (context, ZMQ_REP);
    zmq::socket_t serverSocket( context, ZMQ_REQ);

    int linger_ms = 1000;
    socket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));
    serverSocket.setsockopt(ZMQ_LINGER, &linger_ms, sizeof(int));

    socket.bind( makeZMQAddress("*", workerCtrlPort).c_str() );
    serverSocket.connect( makeZMQAddress("localhost", serverCtrlPort).c_str() );

    const long client_timeout = 30000;
    const double dead_client_timout_min = 5;
    size_t nClientTimeouts = 0;

#ifdef _WIN32
    SetConsoleCtrlHandler( consoleCtrlHandler, TRUE );
#else
    s_catch_signals();
#endif
    bool keepRunning=true;
    while (keepRunning)
    {
        // Wait for next request from client
        zmq::message_t request;
        if(receiveWithTimeout(socket, client_timeout, request))
        {
            nClientTimeouts = 0;
            size_t offset=0;
            size_t msg_id = getMessageId(request, offset);
            cout << PRINTWORKER << nowDateTime() << " Received message with length: " << request.size() << " msg_id: " << msg_id << endl;
            if (msg_id == C_SetParam)
            {
                CM_SetParam_t msg = unpackMessage<CM_SetParam_t>(request, offset);
                cout << PRINTWORKER << nowDateTime() << " Client want to set parameter " << msg.name << " " << msg.value << endl;

                // Set parameter
                HString fullName = msg.name.c_str();
                bool rc = setParameter(pRootSystem, fullName, msg.value.c_str());
                // Send ack or nack
                if (rc)
                {
                    sendServerAck(socket);
                }
                else
                {
                    sendServerNAck(socket, "Failed to set parameter: "+msg.name);
                }
            }
            else if (msg_id == C_GetParam)
            {
                std::string msg = unpackMessage<std::string>(request, offset);
                cout << PRINTWORKER << nowDateTime() << " Client want to get parameter " << msg << endl;

                // Get parameter
                HString fullName = msg.c_str();
                string val = getParameter(pRootSystem, fullName);
                //! @todo what if root system name is first?

                // Send param value (as string) or nack
                if (val.empty())
                {
                    sendServerNAck(socket, "Could not get parameter");
                }
                else
                {
                    sendServerStringMessage(socket, S_GetParam_Reply, val);
                }
            }
            else if (msg_id == C_SendingHmf)
            {
                std::string hmf = unpackMessage<std::string>(request, offset);
                cout << PRINTWORKER << nowDateTime() << " Received hmf with size: " << hmf.size() << endl; //<< hmf << endl;

                // If a model is already loaded then delete it
                if (pRootSystem)
                {
                    delete pRootSystem;
                    pRootSystem=nullptr;
                }

                //! @todo loadHMFModel will hang (sometimes) if hmf empty
                if (!hmf.empty())
                {
                    pRootSystem = gHopsanCore.loadHMFModel(hmf.c_str(), simStartTime, simStopTime);
                }

                if (pRootSystem && (gHopsanCore.getNumErrorMessages() == 0) && (gHopsanCore.getNumFatalMessages() == 0) )
                {
                    cout << PRINTWORKER << nowDateTime() << " Model was loaded sucessfully" << endl;
                    sendServerAck(socket);
                }
                else
                {
                    cout << PRINTWORKER << nowDateTime() << " Error: Could not load the model" << endl;
                    sendServerNAck(socket, "Server could not load model");
                }
            }
            else if (msg_id == C_Simulate)
            {
                CM_Simulate_t msg = unpackMessage<CM_Simulate_t>(request, offset);
                // Start simulation
                SimulationHandler simulator;
                bool irc=false,src=false;
                TicToc timer;
                irc = simulator.initializeSystem(simStartTime, simStopTime, pRootSystem);
                timer.TocPrint(PRINTWORKER+nowDateTime()+" Initialize");
                if (irc)
                {
                    timer.Tic();
                    src = simulator.simulateSystem(simStartTime, simStopTime, 1, pRootSystem);
                    timer.TocPrint(PRINTWORKER+nowDateTime()+" Simulate");
                }
                timer.Tic();
                simulator.finalizeSystem(pRootSystem);
                timer.TocPrint(PRINTWORKER+nowDateTime()+" Finalize");

                if (irc && src)
                {
                    sendServerAck(socket);
                }
                else if (!irc)
                {
                    cout  << PRINTWORKER << nowDateTime() << " Model Init failed"  << endl;
                    sendServerNAck(socket, "Could not initialize system");
                }
                else
                {
                    cout  << PRINTWORKER << nowDateTime() << " Model simulation failed"  << endl;
                    sendServerNAck(socket, "Cold not simulate system");
                }
            }
            else if (msg_id == C_ReqResults)
            {
                string varName = unpackMessage<string>(request, offset);
                vector<ModelVariableInfo_t> vMVI;
                collectAllModelVariables(pRootSystem, vMVI, "");
                cout << PRINTWORKER << nowDateTime() << " Client requests variable: " << varName << " Sending: " << vMVI.size() << " variables!" << endl;

                //! @todo Check if simulation finished, ACK Nack
                vector<SM_Variable_Description_t> vars;
                for (size_t mvi=0; mvi<vMVI.size(); ++mvi )
                {
                    vars.push_back(SM_Variable_Description_t());
                    vars.back().name = vMVI[mvi].fullName.c_str();
                    vars.back().alias = "";
                    vars.back().unit = vMVI[mvi].unit.c_str();
                    vars.back().data.reserve(vMVI[mvi].dataLength);
                    // Copy if a data variable
                    if (vMVI[mvi].pData)
                    {
                        for (size_t t=0; t<vMVI[mvi].dataLength; ++t)
                        {
                            vars.back().data.push_back((*vMVI[mvi].pData)[t][vMVI[mvi].dataId]);
                        }
                    }
                    // Copy if a time data variable
                    else if (vMVI[mvi].pTimeData)
                    {
                        for (size_t t=0; t<vMVI[mvi].dataLength; ++t)
                        {
                            vars.back().data.push_back((*vMVI[mvi].pTimeData)[t]);
                        }
                    }
                }

                sendServerMessage<vector<SM_Variable_Description_t>>(socket,S_ReqResults_Reply,vars);
            }
            else if (msg_id == C_ReqMessages)
            {
                HopsanCoreMessageHandler *pHandler = gHopsanCore.getCoreMessageHandler();
                vector<SM_HopsanCoreMessage_t> messages;
                size_t nMessages = pHandler->getNumWaitingMessages();
                messages.resize(nMessages);
                cout << PRINTWORKER << nowDateTime() << " Client requests messages! " <<  "Sending: " << nMessages << " messages!" << endl;
                for (size_t i=0; i<nMessages; ++i)
                {
                    HString mess, tag, type;
                    pHandler->getMessage(mess, type, tag);
                    messages[i].message = mess.c_str();
                    messages[i].tag = tag.c_str();
                    messages[i].type = type[0];
                }

                sendServerMessage<vector<SM_HopsanCoreMessage_t>>(socket,S_ReqMessages_Reply,messages);
            }
            else if (msg_id == C_Bye)
            {
                cout << PRINTWORKER << nowDateTime() << " Client said godbye!" << endl;
                sendServerAck(socket);
                keepRunning = false;

                sendServerGoodby(serverSocket);
            }
            else
            {
                stringstream ss;
                ss << PRINTWORKER << nowDateTime() << " Error: Unknown message id: " << msg_id << endl;
                cout << ss.str() << endl;
                sendServerNAck(socket, ss.str());
            }
        }
        else
        {
            //Handle timout / exception
            nClientTimeouts++;
            if (double(nClientTimeouts)*double(client_timeout)/60000.0 >= dead_client_timout_min)
            {
                // Force quit after 5 minutes
                cout << PRINTWORKER << nowDateTime() << " Client has not sent any message for "<< dead_client_timout_min << " minutes. Terminating worker process!"  << endl;
                //! @todo should hanlde loong simulation time
                sendServerGoodby(serverSocket);
                keepRunning = false;
            }
        }

        //! @todo do we need to sleep here?
#ifndef _WIN32
        //sleep(1);
#else
        //Sleep (1);
#endif
        if (s_interrupted)
        {
            cout << PRINTWORKER << nowDateTime() << " Interrupt signal received, killing worker" << std::endl;
            keepRunning=false;
        }
    }

    // Delete the model if we have one
    if (pRootSystem)
    {
        delete pRootSystem;
        pRootSystem=nullptr;
    }

    cout << PRINTWORKER << nowDateTime() << " Closed!" << endl;
}
