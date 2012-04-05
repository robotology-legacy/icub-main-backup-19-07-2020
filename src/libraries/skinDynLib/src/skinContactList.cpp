/*
 * Copyright (C) 2010-2011 RobotCub Consortium
 * Author: Andrea Del Prete
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include "iCub/skinDynLib/skinContactList.h"
#include <iCub/ctrl/math.h>

using namespace std;
using namespace yarp::os;
using namespace iCub::skinDynLib;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   CONSTRUCTORS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
skinContactList::skinContactList()
:vector<skinContact>(){}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
skinContactList::skinContactList(const size_type &n, const skinContact& value)
:vector<skinContact>(n, value){}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
skinContactList skinContactList::filterBodyPart(const BodyPart &bp)
{
    skinContactList res;
    for(iterator it=begin(); it!=end(); it++)
        if(it->getBodyPart() == bp)
            res.push_back(*it);
    return res;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
map<BodyPart, skinContactList> skinContactList::splitPerBodyPart()
{
    map<BodyPart, skinContactList> res;
    for(iterator it=begin(); it!=end(); it++)
        res[it->getBodyPart()].push_back(*it);
    return res;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   SERIALIZATION methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool skinContactList::read(ConnectionReader& connection)
{
    int listLength = connection.expectInt();
    if(listLength<0)
        return false;
    if(listLength!=this->size())
        this->resize(listLength);

    for(iterator it=begin(); it!=end(); it++)
        it->read(connection);

    return !connection.isError();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool skinContactList::write(ConnectionWriter& connection)
{
    connection.appendInt(this->size());
    for(iterator it=begin(); it!=end(); it++)
        if(!it->write(connection))
            return false;

    return !connection.isError();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
dynContactList skinContactList::toDynContactList() const
{
    dynContactList res(this->size());
    const_iterator itSkin = begin();
    for(dynContactList::iterator itDyn=res.begin(); itDyn!=res.end(); itDyn++)
    {
        *itDyn = (dynContact)(*itSkin);
        itSkin++;
    }
    return res;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//bool skinContactList::fromDynContactList(dynContactList &l)
//{
//    resize(l.size());
//    for(unsigned int i=0; i<size(); i++)
//        operator[](i) = *(dynamic_cast<skinContact*> (&l[i]));
//    return true;
//}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
string skinContactList::toString() const
{
    stringstream ss;
    for(const_iterator it=begin();it!=end();it++)
        ss<<"- "<<it->toString()<<";\n";
    return ss.str();
}


