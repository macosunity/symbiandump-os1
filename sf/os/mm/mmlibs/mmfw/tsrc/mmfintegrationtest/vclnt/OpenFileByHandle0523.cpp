// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "TestRecord.h"
#include "OpenFileByHandle0523.h"

#include <caf/caf.h>

/**
 * Constructor
 */
CTestMmfVclntOpenFile0523::CTestMmfVclntOpenFile0523(const TDesC& aTestName,const TDesC& aSectName,const TDesC& aKeyName,const TBool aRec)
	:iRec (aRec)
	{
	// store the name of this test case
	// this is the name that is used by the script file
	// Each test step initialises it's own name
	iTestStepName = aTestName;
	iSectName = aSectName;
	iKeyName = aKeyName;
	}

CTestMmfVclntOpenFile0523* CTestMmfVclntOpenFile0523::NewL(const TDesC& aTestName, const TDesC& aSectName,const TDesC& aKeyName,const TBool aRec)
	{
	CTestMmfVclntOpenFile0523* self = new (ELeave) CTestMmfVclntOpenFile0523(aTestName,aSectName,aKeyName,aRec);
	return self;
	}

CTestMmfVclntOpenFile0523* CTestMmfVclntOpenFile0523::NewLC(const TDesC& aTestName, const TDesC& aSectName,const TDesC& aKeyName,const TBool aRec)
	{
	CTestMmfVclntOpenFile0523* self = CTestMmfVclntOpenFile0523::NewL(aTestName,aSectName,aKeyName,aRec);
	CleanupStack::PushL(self);
	return self;
	}

void CTestMmfVclntOpenFile0523::MvruoOpenComplete(TInt aError)
	{
	iError = aError;
	INFO_PRINTF1(_L("Open complete callback"));
	CActiveScheduler::Stop();
	}

void CTestMmfVclntOpenFile0523::MvruoRecordComplete(TInt aError)
	{
	iError = aError;
	INFO_PRINTF1(_L("Record complete callback"));
	CActiveScheduler::Stop();
	}

void CTestMmfVclntOpenFile0523::MvruoPrepareComplete(TInt aError)
	{
	iError = aError;
	INFO_PRINTF1(_L("Prepare complete callback"));
	CActiveScheduler::Stop();
	}
	
void CTestMmfVclntOpenFile0523::MvruoEvent(const TMMFEvent& /*aEvent*/)
	{
	}

/**
 * Load and initialise an audio file.
 */
TVerdict CTestMmfVclntOpenFile0523::DoTestStepL()
	{
	return( PerformTestStepL() );
	}

TVerdict CTestMmfVclntOpenFile0523::PerformTestStepL()
	{
	TVerdict ret = EFail;

	INFO_PRINTF1(_L("Test : Video Recorder - OpenFileL(RFile&)"));

	RFs		fs;
	RFile	file;

	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);
	User::LeaveIfError(fs.ShareProtected());

	iError = KErrTimedOut;

	TPtrC filename;
	if(!GetStringFromConfig(iSectName,iKeyName,filename))
		{
		return EInconclusive;
		}

	User::LeaveIfError(file.Replace(fs,filename,EFileWrite));
	CleanupClosePushL(file);

	CVideoRecorderUtility* rec = CVideoRecorderUtility::NewL(*this);
	CleanupStack::PushL(rec);

	const TUid KVidTstControllerUid = {KMmfVideoTestControllerUid};
	TRAP(iError,rec->OpenFileL(file,
							   NULL,
							   KVidTstControllerUid,
							   KUidMdaBmpClipFormat,
							   KNullDesC8,
							   KMMFFourCCCodeNULL));

	if(iError == KErrNone)
		{
		INFO_PRINTF1(_L("CVideoRecorderUtility: Open file"));
		// Wait for initialisation callback
		CActiveScheduler::Start();
		}

	if(iError == KErrNotSupported)
		{
		ret = EPass;
		}

	// Check for errors.
	if ((iError == KErrNone) && (rec != NULL))
		{
		rec->Prepare();
		CActiveScheduler::Start();
		if(iError != KErrNone)
			{
			INFO_PRINTF2(_L("Prepare callback : error %d"), iError);
			return EInconclusive;
			}

		if(iRec)
			{
			iError = KErrTimedOut;
			rec->Record();
			INFO_PRINTF1(_L("CVideoRecorderUtility: Record"));
			// Wait for init callback
			CActiveScheduler::Start();
			if(iError == KErrNone)
				{
				ret = EPass;
				}
			}
		else
			{
			ret = EPass;
			}
		}
	
	rec->Close();

	INFO_PRINTF1(_L("CVideoRecorderUtility: Destroy"));
	User::After(KOneSecond); // wait for deletion to shut down devsound
	if(iError != KErrNone)
		{
		ERR_PRINTF2( _L("CVideoRecorderUtility failed with error %d"),iError );
		}

	CleanupStack::PopAndDestroy(3,&fs);
	
	return	ret;
	}
