/*
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/



#ifndef _LCFPSYDUMMY1_H_
#define _LCFPSYDUMMY1_H_


#include <lbs/epos_cpositioner.h>


class CLeavingPsy: public CPositioner
{
public:
        static CLeavingPsy* NewL(TAny* aConstructionParameters);
        CLeavingPsy::~CLeavingPsy();

private:
        void        ConstructL(TAny* aConstructionParameters);

public: // from CPositioner

        void        NotifyPositionUpdate(TPositionInfoBase& aPosInfo,
                                                          TRequestStatus& aStatus);
        void        CancelNotifyPositionUpdate();

private: // member variables

        TRequestStatus* iStatusPtr;

};




#endif

// End of file
