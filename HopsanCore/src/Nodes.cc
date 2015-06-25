/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   Nodes.cc
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains the register_nodes function that registers all built in nodes
//!
//$Id$
#include "Nodes.h"
#include "Port.h"

//! @defgroup Nodes Nodes

//! @defgroup NodeHydraulic NodeHydraulic
//! @ingroup Nodes

//! @defgroup NodePneumatic NodePneumatic
//! @ingroup Nodes

//! @defgroup NodeMechanic NodeMechanic
//! @ingroup Nodes

//! @defgroup NodeMechanicRotational NodeMechanicRotational
//! @ingroup Nodes

//! @defgroup NodeSignal NodeSignal
//! @ingroup Nodes

//! @defgroup NodeElectric NodeElectric
//! @ingroup Nodes

//! @defgroup NodeMechanic2D NodeMechanic2D
//! @ingroup Nodes
//!
//! @defgroup NodeEmpty NodeEmpty
//! @ingroup Nodes

using namespace hopsan;

//!
//! @brief Registers the creator function of all built in nodes
//! @param [in] pNodeFactory A pointer the the node factory in which to register the nodes
//!
void hopsan::register_default_nodes(NodeFactory* pNodeFactory)
{
    pNodeFactory->registerCreatorFunction("NodeSignal", NodeSignal::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeHydraulic", NodeHydraulic::CreatorFunction);
    //pNodeFactory->registerCreatorFunction("NodeHydraulicTemperature", NodeHydraulicTemperature::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodePneumatic", NodePneumatic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic", NodeMechanic::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanicRotational", NodeMechanicRotational::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeElectric", NodeElectric::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeMechanic2D", NodeMechanic2D::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeModelica", NodeModelica::CreatorFunction);
    pNodeFactory->registerCreatorFunction("NodeEmpty", NodeEmpty::CreatorFunction);
}

HydraulicNodeDataPointerStructT hopsan::getHydraulicNodeDataPointerStruct(const Port *pPort)
{
    HydraulicNodeDataPointerStructT data;
    if (pPort->getNodeType() == "NodeHydraulic")
    {
        data.pQ = pPort->getNodeDataPtr(NodeHydraulic::Flow);
        data.pP = pPort->getNodeDataPtr(NodeHydraulic::Pressure);
//        data.pT = pPort->getNodeDataPtr(NodeHydraulic::Temperature);
//        data.pQdot = pPort->getNodeDataPtr(NodeHydraulic::HeatFlow);
        data.pZc = pPort->getNodeDataPtr(NodeHydraulic::CharImpedance);
        data.pC = pPort->getNodeDataPtr(NodeHydraulic::WaveVariable);
    }
    return data;
}

HydraulicNodeDataValueStructT hopsan::getHydraulicNodeDataValueStruct(const Port *pPort)
{
    HydraulicNodeDataValueStructT data;
    const Node *pNode = pPort->getNodePtr();
    data.q = pNode->getDataValue(NodeHydraulic::Flow);
    data.p = pNode->getDataValue(NodeHydraulic::Pressure);
//    data.T = pNode->getDataValue(NodeHydraulic::Temperature);
//    data.Qdot = pNode->getDataValue(NodeHydraulic::HeatFlow);
    data.Zc = pNode->getDataValue(NodeHydraulic::CharImpedance);
    data.c = pNode->getDataValue(NodeHydraulic::WaveVariable);
    return data;
}

HydraulicNodeDataValueStructT hopsan::getHydraulicNodeDataValueStruct(const HydraulicNodeDataPointerStructT &rPtrStruct)
{
    HydraulicNodeDataValueStructT data;
    data.q = *rPtrStruct.pQ;
    data.p = *rPtrStruct.pP;
//    data.T = *rPtrStruct.pT;
//    data.Qdot = *rPtrStruct.pQdot;
    data.Zc = *rPtrStruct.pZc;
    data.c = *rPtrStruct.pC;
    return data;
}
