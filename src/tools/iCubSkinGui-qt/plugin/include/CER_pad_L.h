// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2019 Istituto Italiano di Tecnologia (IIT)
 *
 * This software may be modified and distributed under the terms of the
 * GPL-2+ license. See the accompanying LICENSE file for details.
 */

/*!
 *  \brief     In here there are the class definitions for proximal and distal paddle of CER(mk2)
 *  \author    Valentina Gaggero
 *  \date      July 2019
 *  \copyright GPL-2+ license
 */

#include "include/TouchSensor.h"

#include <iostream>

#ifndef __CER_pad_L_H__
#define __CER_pad_L_H__

class CER_paddleFlexProximal_L : public TouchSensor
{
public:

    CER_paddleFlexProximal_L(double cx,double cy,double th,double gain=1.0,int layoutNum=0,int lrMirror=0)
    {
        const double DEG2RAD=M_PI/180.0;

        const double CST=cos(DEG2RAD*th);
        const double SNT=sin(DEG2RAD*th);

        dGain=gain;
        ilrMirror=lrMirror;
        ilayoutNum=layoutNum;
        nVerts=4;
        nTaxels=48;
        m_RadiusOrig=1.8;

        for (int i = 0; i<nTaxels; ++i) dX[0] = dY[0] = 0.0;

        int n = 0;

        dX[n]=  0.0;   dY[n++]=  0.0;  //    0
        dX[n]=  5.0;   dY[n++]=  0.0;  //    1
        dX[n]= 10.0;   dY[n++]=  0.0;  //    2
        dX[n]= 20.0;   dY[n++]= -5.0;  //    3
        dX[n]= 15.0;   dY[n++]= -5.0;  //    4
        dX[n]= 10.0;   dY[n++]= -5.0;  //    5
        dX[n]= 20.0;   dY[n++]=-10.0;  //    6
        dX[n]= 15.0;   dY[n++]=-10.0;  //    7
        dX[n]=  5.0;   dY[n++]= -5.0;  //    8
        dX[n]= 10.0;   dY[n++]=-10.0;  //    9
        dX[n]=  5.0;   dY[n++]=-10.0;  //    10
        dX[n]=  0.0;   dY[n++]=-10.0;  //    11
        dX[n]=  5.0;   dY[n++]= 10.0;  //    12
        dX[n]=  0.0;   dY[n++]= 10.0;  //    13
        dX[n]= 10.0;   dY[n++]= 10.0;  //    14
        dX[n]= 15.0;   dY[n++]= 10.0;  //    15
        dX[n]= 20.0;   dY[n++]= 10.0;  //    16
        dX[n]= 20.0;   dY[n++]=  5.0;  //    17
        dX[n]= 15.0;   dY[n++]=  5.0;  //    18

        dX[n]= 10.0;   dY[n++]=  5.0;  //    19
        dX[n]=  5.0;   dY[n++]=  5.0;  //    20
        dX[n]= 15.0;   dY[n++]=  0.0;  //    21
        dX[n]= 20.0;   dY[n++]=  0.0;  //    22
        connected[n++]=false;          //    23
        dX[n]=  5.0;  dY[n++]=  25.0;  //    24

        dX[n]=  0.0;  dY[n++]=  25.0;  //    25
        dX[n]= 10.0;  dY[n++]=  25.0;  //    26
        dX[n]= 10.0;  dY[n++]=  20.0;  //    27
        dX[n]= 15.0;  dY[n++]=  20.0;  //    28
        dX[n]= 20.0;  dY[n++]=  20.0;  //    29
        dX[n]= 20.0;  dY[n++]=  15.0;  //    30
        dX[n]= 15.0;  dY[n++]=  15.0;  //    31
        dX[n]=  5.0;  dY[n++]=  20.0;  //    32
        dX[n]= 10.0;  dY[n++]=  15.0;  //    33
        dX[n]=  5.0;  dY[n++]=  15.0;  //    34
        dX[n]=  0.0;  dY[n++]=  15.0;  //    35
        dX[n]=  5.0;  dY[n++]=  35.0;  //    36

        dX[n]=  0.0;  dY[n++]=  35.0;  //    37
        dX[n]= 10.0;  dY[n++]=  35.0;  //    38
        dX[n]= 15.0;  dY[n++]=  35.0;  //    39
        dX[n]= 20.0;  dY[n++]=  35.0;  //    40
        dX[n]= 10.0;  dY[n++]=  30.0;  //    41
        dX[n]= 20.0;  dY[n++]=  30.0;  //    42
        dX[n]= 15.0;  dY[n++]=  30.0;  //    43
        dX[n]= 20.0;  dY[n++]=  25.0;  //    44
        dX[n]=  5.0;  dY[n++]=  30.0;  //    45
        dX[n]= 15.0;  dY[n++]=  25.0;  //    46
        connected[n++] = false;        //    47


        for (int i=0; i<nTaxels; ++i)
        {
            double x=dX[i]-0.0;
            double y=dY[i]-0.0;

            if (lrMirror) x=-x;

            dX[i]=cx+CST*x-SNT*y;
            dY[i]=cy+SNT*x+CST*y;
        }

        dXv[0]=-2.5;  dYv[0]= 37.5;
        dXv[1]= -2.5;  dYv[1]= -12.5;
        dXv[2]= 22.5;  dYv[2]= -12.5;
        dXv[3]=22.5;  dYv[3]= 37.5;





        for (int i=0; i<nVerts; ++i)
        {
            double x=dXv[i];
            double y=dYv[i];
            if (lrMirror) x=-x;
            dXv[i]=cx+CST*x-SNT*y;
            dYv[i]=cy+SNT*x+CST*y;
        }

        // in static definition
        //dXmin=dYmin= HUGE;
        //dXmax=dYmax=-HUGE;

        for (int i=0; i<nVerts; ++i)
        {
            if (dXv[i]<dXmin) dXmin=dXv[i];
            if (dXv[i]>dXmax) dXmax=dXv[i];
            if (dYv[i]<dYmin) dYmin=dYv[i];
            if (dYv[i]>dYmax) dYmax=dYv[i];
        }

        dXc=cx;
        dYc=cy;
    }
};


class CER_paddleFlexDistal_L : public TouchSensor
{
public:

    CER_paddleFlexDistal_L(double cx,double cy,double th,double gain=1.0,int layoutNum=0,int lrMirror=0)
    {
        const double DEG2RAD=M_PI/180.0;

        const double CST=cos(DEG2RAD*th);
        const double SNT=sin(DEG2RAD*th);

        dGain=gain;
        ilrMirror=lrMirror;
        ilayoutNum=layoutNum;
        nVerts=8;
        nTaxels=48;
        m_RadiusOrig=1.8;

        for (int i = 0; i<nTaxels; ++i) dX[0] = dY[0] = 0.0;

        int n = 0;

        dX[n]=  0.0;  dY[n++]= 0.0;  //    0
        dX[n]=  5.0;  dY[n++]= 0.0;  //    1
        dX[n]= 10.0;  dY[n++]= 0.0;  //    2
        dX[n]= 15.0;  dY[n++]= 0.0;  //    3
        dX[n]= 20.0;  dY[n++]= 0.0;  //    4
        dX[n]= 25.0;  dY[n++]= 0.0;  //    5
        dX[n]=0.0;  dY[n++]=-5.0;  //    6
        dX[n]= 5.0;  dY[n++]=-5.0;  //    7
        dX[n]=10.0;  dY[n++]=-5.0;  //    8
        dX[n]=15.0;  dY[n++]=-5.0;  //    9
        dX[n]=20.0;  dY[n++]=-5.0;  //    10
        dX[n]=25.0;  dY[n++]=-5.0;  //    11
        dX[n]=  5.0;  dY[n++]=10.0;  //    12
        dX[n]= 10.0;  dY[n++]=10.0;  //    13
        dX[n]=15.0;  dY[n++]=10.0;  //    14
        dX[n]=20.0;  dY[n++]=10.0;  //    15
        dX[n]=5.0;  dY[n++]=5.0;  //    16
        dX[n]= 0.0;  dY[n++]=5.0;  //    17
        dX[n]=25.0;  dY[n++]=10.0;  //    18
        dX[n]=10.0;  dY[n++]=5.0;  //    19
        dX[n]=15.0;  dY[n++]=5.0;  //    20
        dX[n]=20.0;  dY[n++]=5.0;  //    21
        dX[n]=25.0;  dY[n++]=5.0;  //    22

        connected[n++] = false;      //    23

        dX[n]=  5.0;  dY[n++]=  20.0;  //    24
        dX[n]= 10.0;  dY[n++]=  20.0;  //    25
        dX[n]= 15.0;  dY[n++]=  20.0;  //    26
        dX[n]= 20.0;  dY[n++]=  20.0;  //    27

        dX[n]= 5.0;  dY[n++]=  15.0;  //    28
        dX[n]= 0.0;  dY[n++]=  15.0;  //    29
        dX[n]= 25.0;  dY[n++]=  20.0;  //    30
        dX[n]= 10.0;  dY[n++]=  15.0;  //    31
        dX[n]=  15.0;  dY[n++]=  15.0;  //    32
        dX[n]= 20.0;  dY[n++]=  15.0;  //    33
        dX[n]=  0.0;  dY[n++]=  10.0;  //    34
        dX[n]= 25.0;  dY[n++]=  15.0;  //    35
        dX[n]=  25.0;  dY[n++]=  25.0;  //    36

        dX[n]=  10.0;  dY[n++]=  30.0;  //    37
        dX[n]= 5.0;  dY[n++]=  25.0;  //    38
        dX[n]= 20.0;  dY[n++]=  25.0;  //    39
        dX[n]= 25.0;  dY[n++]=  30.0;  //    40
        dX[n]= 15.0;  dY[n++]=  30.0;  //    41
        dX[n]= 10.0;  dY[n++]=  25.0;  //    42
        dX[n]= 15.0;  dY[n++]=  35.0;  //    43
        dX[n]= 20.0;  dY[n++]=  30.0;  //    44
        dX[n]= 20.0;  dY[n++]=  35.0;  //    45
        dX[n]= 15.0;  dY[n++]=  25.0;  //    46
        connected[n++] = false;        //    47



       for (int i=0; i<nTaxels; ++i)
       {
           double x=dX[i]-0.0;
           double y=dY[i]-0.0;

           if (lrMirror) x=-x;

           dX[i]=cx+CST*x-SNT*y;
           dY[i]=cy+SNT*x+CST*y;
       }

       //dXv[0]=-2.5;  dYv[0]= 37.5;
       dXv[0]=-2.5;  dYv[0]= 17.5;
       dXv[1]= -2.5; dYv[1]= -7.5;
       dXv[2]= 27.5; dYv[2]= -7.5;
       dXv[3]=27.5;  dYv[3]= 32.5;
       dXv[4]=22.5;  dYv[4]= 37.5;
       dXv[5]=12.5;  dYv[5]= 37.5;
       dXv[6]=7.5;   dYv[6]= 32.5;
       dXv[7]=0.0;   dYv[7]=  22.5;





       for (int i=0; i<nVerts; ++i)
       {
           double x=dXv[i];
           double y=dYv[i];
           if (lrMirror) x=-x;
           dXv[i]=cx+CST*x-SNT*y;
           dYv[i]=cy+SNT*x+CST*y;
       }

       // in static definition
       //dXmin=dYmin= HUGE;
       //dXmax=dYmax=-HUGE;


        for (int i=0; i<nVerts; ++i)
        {
            if (dXv[i]<dXmin) dXmin=dXv[i];
            if (dXv[i]>dXmax) dXmax=dXv[i];
            if (dYv[i]<dYmin) dYmin=dYv[i];
            if (dYv[i]>dYmax) dYmax=dYv[i];
        }

        dXc=cx;
        dYc=cy;
    }


};

#endif
