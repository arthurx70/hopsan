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
//! @file   MechanicRackAndPinion.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-06-19
//!
//! @brief Contains a mechanic rack and pinion component
//!
//$Id: MechanicRackAndPinion.hpp 2974 2011-05-30 11:12:44Z petno25 $

#ifndef MECHANICBALLSCREW_HPP_INCLUDED
#define MECHANICBALLSCREW_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //Verified with AMESim 2011-03-21
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicBallScrew : public ComponentQ
    {

    private:
        double *mpL, *mpNy, *mpJ, *mpB;
        double k;
        double num[3];
        double den[3];
        SecondOrderTransferFunction mFilter;
        Integrator mInt;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_me1, *mpND_c1, *mpND_Zx1,
               *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicBallScrew();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            addInputVariable("L", "Screw Lead", "m", 0.001, &mpL);
            addInputVariable("ny", "Screw Efficiency", "-", 0.9, &mpNy);

            addInputVariable("J", "Moment of Inertia", "kgm^2", 1.0, &mpJ);
            addInputVariable("B", "Viscous Friction", "Nms/rad", 10, &mpB);

            addConstant("k", "Spring Coefficient", "Nm/rad", 0.0, k);
        }


        void initialize()
        {
            double L, ny, gearRatio, J, B;
            L = (*mpL);
            ny = (*mpNy);
            J = (*mpJ);
            B = (*mpB);

            gearRatio = L/(2.0*pi*ny);

            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            num[0] = 0.0;
            num[1] = 1.0;
            num[2] = 0.0;
            den[0] = k;
            den[1] = B;
            den[2] = J;
            mFilter.initialize(mTimestep, num, den, 0, 0);      //Must initialize with zero, otherwise filter may give static offset with zero input
            mInt.initialize(mTimestep, 0, 0);                   //Must initialize with zero, otherwise filter may give static offset with zero input

            (*mpND_me1) = J*gearRatio;
        }


        void simulateOneTimestep()
        {
            double f1, x1, v1, c1, Zx1;
            double t2, a2, w2, c2, Zx2;
            double L, ny, gearRatio, J, B;
            L = (*mpL);
            ny = (*mpNy);
            J = (*mpJ);
            B = (*mpB);

            gearRatio = L/(2.0*pi*ny);

            //Get variable values from nodes
            c1  = (*mpND_c1)*gearRatio;
            Zx1 = (*mpND_Zx1)*pow(gearRatio, 2.0);
            c2  = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            den[1] = B+Zx1+Zx2;

            mFilter.setDen(den);
            w2 = mFilter.update(c1-c2);
            a2 = mInt.update(w2);
            t2 = c2 + Zx2*w2;

            v1 = -w2*gearRatio;
            x1 = -a2*gearRatio;
            f1 = (c1 + Zx1*v1)/gearRatio;


            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_me1) = J*gearRatio;
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
        }
    };
}

#endif // MECHANICBALLSCREW_HPP_INCLUDED

