/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
//desc:test LCleanup class used in leaving template function for LS12
//option:
//date:2008-8-12 15:58:1
//author:pingorliu
//type: CT

template<class T>
void funcL()
{

	LCleanedupHandle<T> baz;//check:-improper
	LString8 s(KMaxString);//check:-improper
	LData buf(KMaxBuf)   ;//check:-improper

        /*this is a function call */ LData buf(KMaxBuf) /*hello*/    ;//check:-improper
	


}
