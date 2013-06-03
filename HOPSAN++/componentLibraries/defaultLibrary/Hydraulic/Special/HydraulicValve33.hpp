#ifndef HYDRAULICVALVE33_HPP_INCLUDED
#define HYDRAULICVALVE33_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicValve33.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Tue 4 Jun 2013 00:07:02
//! @brief A load sensing hydraulic directional valve
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/HydraulicComponents.nb*/

using namespace hopsan;

class HydraulicValve33 : public ComponentQ
{
private:
     double rho;
     double Cq;
     double Sd;
     double Frap;
     double Frat;
     double Xap0;
     double Xat0;
     double Xvmax;
     double plam;
     Port *mpPp;
     Port *mpPt;
     Port *mpPa;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[6];
     int mNstep;
     //Port Pp variable
     double pp;
     double qp;
     double Tp;
     double dEp;
     double cp;
     double Zcp;
     //Port Pt variable
     double pt;
     double qt;
     double Tt;
     double dEt;
     double ct;
     double Zct;
     //Port Pa variable
     double pa;
     double qa;
     double Ta;
     double dEa;
     double ca;
     double Zca;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double xv;
     //outputVariables
     //LocalExpressions variables
     double Ks;
     double Kspa;
     double Ksta;
     //Port Pp pointer
     double *mpND_pp;
     double *mpND_qp;
     double *mpND_Tp;
     double *mpND_dEp;
     double *mpND_cp;
     double *mpND_Zcp;
     //Port Pt pointer
     double *mpND_pt;
     double *mpND_qt;
     double *mpND_Tt;
     double *mpND_dEt;
     double *mpND_ct;
     double *mpND_Zct;
     //Port Pa pointer
     double *mpND_pa;
     double *mpND_qa;
     double *mpND_Ta;
     double *mpND_dEa;
     double *mpND_ca;
     double *mpND_Zca;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpxv;
     //inputParameters pointers
     double *mprho;
     double *mpCq;
     double *mpSd;
     double *mpFrap;
     double *mpFrat;
     double *mpXap0;
     double *mpXat0;
     double *mpXvmax;
     double *mpplam;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new HydraulicValve33();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(6,6);
        systemEquations.create(6);
        delayedPart.create(7,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPp=addPowerPort("Pp","NodeHydraulic");
        mpPt=addPowerPort("Pt","NodeHydraulic");
        mpPa=addPowerPort("Pa","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("xv","Spool position","m",0.,&mpxv);

        //Add inputParammeters to the component
            addInputVariable("rho", "&mpoil density", "kg/m3", 860.,&mprho);
            addInputVariable("Cq", "&mpFlow coefficient.", "", 0.67,&mpCq);
            addInputVariable("Sd", "&mpspool diameter", "m", 0.01,&mpSd);
            addInputVariable("Frap", "&mpSpool cricle fraction(P-A)", "", \
1.,&mpFrap);
            addInputVariable("Frat", "&mpSpool cricle fraction(A-T)", "", \
1.,&mpFrat);
            addInputVariable("Xap0", "&mpUnderlap", "m", 0.,&mpXap0);
            addInputVariable("Xat0", "&mpUnderlap", "m", 0.,&mpXat0);
            addInputVariable("Xvmax", "&mpMax opening", "m", 0.01,&mpXvmax);
            addInputVariable("plam", "&mpTurbulence onset pressure", "Pa", \
10000.,&mpplam);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,6);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pp
        mpND_pp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Pressure);
        mpND_qp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Flow);
        mpND_Tp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Temperature);
        mpND_dEp=getSafeNodeDataPtr(mpPp, NodeHydraulic::HeatFlow);
        mpND_cp=getSafeNodeDataPtr(mpPp, NodeHydraulic::WaveVariable);
        mpND_Zcp=getSafeNodeDataPtr(mpPp, NodeHydraulic::CharImpedance);
        //Port Pt
        mpND_pt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Pressure);
        mpND_qt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Flow);
        mpND_Tt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Temperature);
        mpND_dEt=getSafeNodeDataPtr(mpPt, NodeHydraulic::HeatFlow);
        mpND_ct=getSafeNodeDataPtr(mpPt, NodeHydraulic::WaveVariable);
        mpND_Zct=getSafeNodeDataPtr(mpPt, NodeHydraulic::CharImpedance);
        //Port Pa
        mpND_pa=getSafeNodeDataPtr(mpPa, NodeHydraulic::Pressure);
        mpND_qa=getSafeNodeDataPtr(mpPa, NodeHydraulic::Flow);
        mpND_Ta=getSafeNodeDataPtr(mpPa, NodeHydraulic::Temperature);
        mpND_dEa=getSafeNodeDataPtr(mpPa, NodeHydraulic::HeatFlow);
        mpND_ca=getSafeNodeDataPtr(mpPa, NodeHydraulic::WaveVariable);
        mpND_Zca=getSafeNodeDataPtr(mpPa, NodeHydraulic::CharImpedance);

        //Read variables from nodes
        //Port Pp
        pp = (*mpND_pp);
        qp = (*mpND_qp);
        Tp = (*mpND_Tp);
        dEp = (*mpND_dEp);
        cp = (*mpND_cp);
        Zcp = (*mpND_Zcp);
        //Port Pt
        pt = (*mpND_pt);
        qt = (*mpND_qt);
        Tt = (*mpND_Tt);
        dEt = (*mpND_dEt);
        ct = (*mpND_ct);
        Zct = (*mpND_Zct);
        //Port Pa
        pa = (*mpND_pa);
        qa = (*mpND_qa);
        Ta = (*mpND_Ta);
        dEa = (*mpND_dEa);
        ca = (*mpND_ca);
        Zca = (*mpND_Zca);

        //Read inputVariables from nodes
        xv = (*mpxv);

        //Read inputParameters from nodes
        rho = (*mprho);
        Cq = (*mpCq);
        Sd = (*mpSd);
        Frap = (*mpFrap);
        Frat = (*mpFrat);
        Xap0 = (*mpXap0);
        Xat0 = (*mpXat0);
        Xvmax = (*mpXvmax);
        plam = (*mpplam);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Kspa = 3.14159*Frap*Ks*Sd*limit(Xap0 + xv,0.,Xap0 + Xvmax);
        Ksta = 3.14159*Frat*Ks*Sd*limit(Xap0 - xv,0.,Xat0 + Xvmax);

        //Initialize delays

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(6);
        Vec stateVark(6);
        Vec deltaStateVar(6);

        //Read variables from nodes
        //Port Pp
        Tp = (*mpND_Tp);
        cp = (*mpND_cp);
        Zcp = (*mpND_Zcp);
        //Port Pt
        Tt = (*mpND_Tt);
        ct = (*mpND_ct);
        Zct = (*mpND_Zct);
        //Port Pa
        Ta = (*mpND_Ta);
        ca = (*mpND_ca);
        Zca = (*mpND_Zca);

        //Read inputVariables from nodes
        xv = (*mpxv);

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Kspa = 3.14159*Frap*Ks*Sd*limit(Xap0 + xv,0.,Xap0 + Xvmax);
        Ksta = 3.14159*Frat*Ks*Sd*limit(Xap0 - xv,0.,Xat0 + Xvmax);

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = qp;
        stateVark[1] = qt;
        stateVark[2] = qa;
        stateVark[3] = pp;
        stateVark[4] = pt;
        stateVark[5] = pa;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Valve33
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =qp + Kspa*signedSquareL(-pa + pp,plam);
          systemEquations[1] =qt + Ksta*signedSquareL(-pa + pt,plam);
          systemEquations[2] =qa - Kspa*signedSquareL(-pa + pp,plam) - \
Ksta*signedSquareL(-pa + pt,plam);
          systemEquations[3] =pp - lowLimit(cp + qp*Zcp*onPositive(pp),0);
          systemEquations[4] =pt - lowLimit(ct + qt*Zct*onPositive(pt),0);
          systemEquations[5] =pa - lowLimit(ca + qa*Zca*onPositive(pa),0);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = Kspa*dxSignedSquareL(-pa + pp,plam);
          jacobianMatrix[0][4] = 0;
          jacobianMatrix[0][5] = -(Kspa*dxSignedSquareL(-pa + pp,plam));
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = Ksta*dxSignedSquareL(-pa + pt,plam);
          jacobianMatrix[1][5] = -(Ksta*dxSignedSquareL(-pa + pt,plam));
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = -(Kspa*dxSignedSquareL(-pa + pp,plam));
          jacobianMatrix[2][4] = -(Ksta*dxSignedSquareL(-pa + pt,plam));
          jacobianMatrix[2][5] = Kspa*dxSignedSquareL(-pa + pp,plam) + \
Ksta*dxSignedSquareL(-pa + pt,plam);
          jacobianMatrix[3][0] = -(Zcp*dxLowLimit(cp + \
qp*Zcp*onPositive(pp),0)*onPositive(pp));
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[3][5] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = -(Zct*dxLowLimit(ct + \
qt*Zct*onPositive(pt),0)*onPositive(pt));
          jacobianMatrix[4][2] = 0;
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[5][0] = 0;
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = -(Zca*dxLowLimit(ca + \
qa*Zca*onPositive(pa),0)*onPositive(pa));
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          qp=stateVark[0];
          qt=stateVark[1];
          qa=stateVark[2];
          pp=stateVark[3];
          pt=stateVark[4];
          pa=stateVark[5];
        }

        //Calculate the delayed parts

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];

        //Write new values to nodes
        //Port Pp
        (*mpND_pp)=pp;
        (*mpND_qp)=qp;
        (*mpND_dEp)=dEp;
        //Port Pt
        (*mpND_pt)=pt;
        (*mpND_qt)=qt;
        (*mpND_dEt)=dEt;
        //Port Pa
        (*mpND_pa)=pa;
        (*mpND_qa)=qa;
        (*mpND_dEa)=dEa;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICVALVE33_HPP_INCLUDED
