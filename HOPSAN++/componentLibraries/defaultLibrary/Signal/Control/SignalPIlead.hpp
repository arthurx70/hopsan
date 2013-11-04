#ifndef SIGNALPILEAD_HPP_INCLUDED
#define SIGNALPILEAD_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file C:\HopsanTrunk\HOPSAN++\componentLibraries\defaultLibrary\Signal\Control\Sign\alPIlead.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 23 Oct 2013 11:27:17
//! @brief PI-controler with lead filter in feadback path
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/SignalControlComponents.nb*/

using namespace hopsan;

class SignalPIlead : public ComponentSignal
{
private:
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[4];
     int mNstep;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double yref;
     double kx;
     double y;
     double wa;
     double da;
     double umin;
     double umax;
     //outputVariables
     double u;
     double err;
     double Ierr;
     double uI;
     //LocalExpressions variables
     double k1;
     double w11;
     double w21;
     double w22;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpyref;
     double *mpkx;
     double *mpy;
     double *mpwa;
     double *mpda;
     double *mpumin;
     double *mpumax;
     //outputVariables pointers
     double *mpu;
     double *mperr;
     double *mpIerr;
     double *mpuI;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     Delay mDelayedPart40;
     Delay mDelayedPart41;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalPIlead();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("yref","Reference value","",0.,&mpyref);
            addInputVariable("kx","Break frequency","rad/s",1.,&mpkx);
            addInputVariable("y","Actual value","",0.,&mpy);
            addInputVariable("wa","Break frequency","rad/s",1.,&mpwa);
            addInputVariable("da","relative damping","",1.,&mpda);
            addInputVariable("umin","Minium output signal","",-1.,&mpumin);
            addInputVariable("umax","Maximum output signal","",1.,&mpumax);

        //Add outputVariables to the component
            addOutputVariable("u","control signal","",0.,&mpu);
            addOutputVariable("err","adjusted error signal","",0.,&mperr);
            addOutputVariable("Ierr","limited adjusted error \
signal","",0.,&mpIerr);
            addOutputVariable("uI","control signal from \
integral","",0.,&mpuI);

//==This code has been autogenerated using Compgen==
        //Add constants/parameters
        mpSolver = new EquationSystemSolver(this,4);
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        yref = (*mpyref);
        kx = (*mpkx);
        y = (*mpy);
        wa = (*mpwa);
        da = (*mpda);
        umin = (*mpumin);
        umax = (*mpumax);

        //Read outputVariables from nodes
        u = (*mpu);
        err = (*mperr);
        Ierr = (*mpIerr);
        uI = (*mpuI);

//==This code has been autogenerated using Compgen==

        //LocalExpressions
        k1 = wa/kx;
        w11 = wa;
        w21 = wa/(2.*da);
        w22 = wa;

        //Initialize delays
        delayParts1[1] = (-2*err*w21 + err*mTimestep*w21*w22 - 2*w22*y + \
mTimestep*w21*w22*y + 2*w21*yref - mTimestep*w21*w22*yref)/(2*w21 + \
mTimestep*w21*w22);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts4[1] = (-2*uI - Ierr*k1*mTimestep*w11)/2.;
        mDelayedPart41.initialize(mNstep,delayParts4[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(4);
        Vec stateVark(4);
        Vec deltaStateVar(4);

        //Read variables from nodes

        //Read inputVariables from nodes
        yref = (*mpyref);
        kx = (*mpkx);
        y = (*mpy);
        wa = (*mpwa);
        da = (*mpda);
        umin = (*mpumin);
        umax = (*mpumax);

        //LocalExpressions
        k1 = wa/kx;
        w11 = wa;
        w21 = wa/(2.*da);
        w22 = wa;

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = err;
        stateVark[1] = u;
        stateVark[2] = Ierr;
        stateVark[3] = uI;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //PIlead
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =err + (2*w22*y + mTimestep*w21*w22*y - \
2*w21*yref - mTimestep*w21*w22*yref)/(2*w21 + mTimestep*w21*w22) + \
delayedPart[1][1];
          systemEquations[1] =u - limit(err*k1 + uI,umin,umax);
          systemEquations[2] =Ierr - err*dxLimit(limit(err*k1 + \
uI,umin,umax),umin,umax);
          systemEquations[3] =uI - (Ierr*k1*mTimestep*w11)/2. + \
delayedPart[4][1];

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[1][0] = -(k1*dxLimit(err*k1 + uI,umin,umax));
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = -dxLimit(err*k1 + uI,umin,umax);
          jacobianMatrix[2][0] = -dxLimit(limit(err*k1 + \
uI,umin,umax),umin,umax);
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = -(k1*mTimestep*w11)/2.;
          jacobianMatrix[3][3] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          err=stateVark[0];
          u=stateVark[1];
          Ierr=stateVark[2];
          uI=stateVark[3];
        }

        //Calculate the delayed parts
        delayParts1[1] = (-2*err*w21 + err*mTimestep*w21*w22 - 2*w22*y + \
mTimestep*w21*w22*y + 2*w21*yref - mTimestep*w21*w22*yref)/(2*w21 + \
mTimestep*w21*w22);
        delayParts4[1] = (-2*uI - Ierr*k1*mTimestep*w11)/2.;

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //outputVariables
        (*mpu)=u;
        (*mperr)=err;
        (*mpIerr)=Ierr;
        (*mpuI)=uI;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart41.update(delayParts4[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // SIGNALPILEAD_HPP_INCLUDED
