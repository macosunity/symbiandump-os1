// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// This is the class implementation for the Module Information Tests
// EPOC includes.sue
// 
//

// LBS includes. 
#include <lbssatellite.h>
#include <lbs/lbsnetprotocolbase.h>
#include <lbs/lbsassistancedatabuilderset.h>
#include <e32math.h>
#include <lbs/lbsloccommon.h>
#include <lbs/lbsx3p.h>
#include <lbs/lbsnetprotocolbase.h>
#include <lbs/lbslocdatasourceclasstypes.h>
#include <lbs/lbslocdatasourcegpsbase.h>
#include <e32property.h>
#include "lbsnetinternalapi.h"
#include "LbsInternalInterface.h"
#include "LbsExtendModuleInfo.h"
#include "lbsdevloggermacros.h"

// LBS test includes.
#include "ctlbsx3pgpsoptions.h"
#include <lbs/test/tlbsutils.h>
#include "argutils.h"

#include <lbs/test/activeyield.h>

const TInt KTimeOut = 30*1000*1000;
const TInt KAdviceSystemStatusTimeout = 40*1000*1000;
const TInt KTransmitTimeOut = 50*1000*1000;
const TInt KSmallTimeOut = 3*1000*1000;

/**
Static Constructor
*/
CT_LbsX3PGpsOptions* CT_LbsX3PGpsOptions::New(CT_LbsHybridX3PServer& aParent)
	{
	// Note the lack of ELeave.
	// This means that having insufficient memory will return NULL;
	CT_LbsX3PGpsOptions* testStep = new CT_LbsX3PGpsOptions(aParent);
	if (testStep)
		{
		TInt err = KErrNone;

		TRAP(err, testStep->ConstructL());
		if (err)
			{
			delete testStep;
			testStep = NULL;
			}
		}
	return testStep;
	}


/**
 * Constructor
 */
CT_LbsX3PGpsOptions::CT_LbsX3PGpsOptions(CT_LbsHybridX3PServer& aParent) 
	: CT_LbsHybridX3PStep(aParent), iProxy(NULL), 
		iNetworkMethod(ENetworkMethodInvalid),
		iPlannedPositionOriginator(EPositionOriginatorUnkown),
		iNetRequestRejected(EFalse),
		iAdminGpsMode(CLbsAdmin::EGpsModeUnknown)
	{
	SetTestStepName(KLbsX3PGpsOptions);
	}


void CT_LbsX3PGpsOptions::ConstructL()
	{
	// Create the base class objects.
	CT_LbsHybridX3PStep::ConstructL();
	User::LeaveIfError(iServer.Connect());
	User::LeaveIfError(iTransmitter.Open(iServer));	
	}

/**
 * Destructor
 */
CT_LbsX3PGpsOptions::~CT_LbsX3PGpsOptions()
	{
	iTransmitter.Close();
	iServer.Close();	
	}

TVerdict CT_LbsX3PGpsOptions::doTestStepPreambleL()
	{
	INFO_PRINTF1(_L("&gt;&gt;CT_LbsX3PGpsOptions::doTestStepPreambleL()"));
	CT_LbsHybridX3PStep::doTestStepPreambleL();
	iNetworkExpectsMeasurements = EFalse;
	iNetworkExpectsPositions = EFalse;
	
	// Get the GPS mode set in the Admin
	CLbsAdmin* admin = CLbsAdmin::NewL();
	CleanupStack::PushL(admin);

	TESTL(KErrNone == admin->Get(KLbsSettingRoamingGpsMode, iAdminGpsMode));
	CleanupStack::PopAndDestroy(admin);
	
	// Set the network step
	iNetworkMethod = ENetworkMethodInvalid;
	
	SetTestStepResult(EPass);
	T_LbsUtils utils;

	// Get the Network Method
	_LIT(KNetworkMethod, "NetworkMethod");
	TInt networkMethodInt;
	if(GetIntFromConfig(ConfigSection(), KNetworkMethod, networkMethodInt))
		{
		iNetworkMethod = static_cast<TChosenNetworkMethod>(networkMethodInt);
		TEST(iNetworkMethod != ENetworkMethodInvalid);
		}
	else
		{
		INFO_PRINTF1(_L("CT_LbsX3PGpsOptions::doTestStepPreambleL() - FAILED: no network method configured"));
		TEST(EFalse);
		}

	// Is network method supported by module?
	TPositionModuleInfoExtended::TDeviceGpsModeCapabilities deviceCapabilities;
	TInt err = LbsModuleInfo::GetDeviceCapabilities(KLbsGpsLocManagerUid, deviceCapabilities);	
	if((!(deviceCapabilities & TPositionModuleInfoExtended::EDeviceGpsModeTerminalAssisted)) && (!(deviceCapabilities & TPositionModuleInfoExtended::EDeviceGpsModeSimultaneousTATB)))
		{
		if(iNetworkMethod == ETerminalAssistedNetworkMethod)
			{
			iNetRequestRejected = ETrue;
			}
		}
	if((!(deviceCapabilities & TPositionModuleInfoExtended::EDeviceGpsModeTerminalBased)) && (!(deviceCapabilities & TPositionModuleInfoExtended::EDeviceGpsModeSimultaneousTATB)))
		{
		if(iNetworkMethod == ETerminalBasedNetworkMethod)
			{
			iNetRequestRejected = ETrue;
			}
		}	
	
	// Get the position originator
	_LIT(KPositionOriginator, "PositionOriginator");
	TInt positionOriginatorInt;
	if(GetIntFromConfig(ConfigSection(), KPositionOriginator, positionOriginatorInt))
		{
		iPlannedPositionOriginator = static_cast<TPlannedPositionOriginator>(positionOriginatorInt);
		TEST(iPlannedPositionOriginator != EPositionOriginatorUnkown);
		}
	else
		{
		INFO_PRINTF1(_L("CT_LbsX3PGpsOptions::doTestStepPreambleL() - FAILED: no position originator configured"));
		TEST(EFalse);
		}

	iProxy = CNetProtocolProxy::NewL();

	return TestStepResult();
	}

TVerdict CT_LbsX3PGpsOptions::doTestStepPostambleL()
	{
	INFO_PRINTF1(_L("&gt;&gt;CT_LbsX3PGpsOptions::doTestStepPostambleL()"));
	iNetworkExpectsMeasurements = EFalse;
	iNetworkExpectsPositions = EFalse;
	iNetworkMethod = ENetworkMethodInvalid;
	iPlannedPositionOriginator = EPositionOriginatorUnkown;
	delete iProxy;
	iProxy = NULL;
	CT_LbsHybridX3PStep::doTestStepPostambleL();
	return TestStepResult();
	}

TVerdict CT_LbsX3PGpsOptions::doTestStepL()
	{
	INFO_PRINTF1(_L("CT_LbsX3PGpsOptions::doTestStepL()"));	
	// Stop the test if the preamble failed
	TESTL(TestStepResult() == EPass);
	
	// >> AdviceSystemStatus(ESystemStatusNone)
	TESTL(iProxy->WaitForResponse(KAdviceSystemStatusTimeout) == ENetMsgGetCurrentCapabilitiesResponse);
	INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> AdviceSystemStatus(ESystemStatusNone)"));	
	CLbsNetworkProtocolBase::TLbsSystemStatus getStatus;
	TInt cleanupCnt;
	cleanupCnt = iProxy->GetArgsLC(ENetMsgGetCurrentCapabilitiesResponse, &getStatus);
	TESTL(getStatus == CLbsNetworkProtocolBase::ESystemStatusNone);
	CleanupStack::PopAndDestroy(cleanupCnt);

//Initiate X3P start
	// >> TransmitPosition() from App
	_LIT(KThirdParty,"+4407825255981"); 
	const TInt KPriority=6;
	TLbsTransmitPositionOptions options(static_cast<TTimeIntervalMicroSeconds>(KTransmitTimeOut));
	iTransmitter.SetTransmitOptions(options);
	INFO_PRINTF1(_L("				(App) TransmitPosition()"));
	CTransmitServerWatcher *pWatch = CTransmitServerWatcher::NewLC(iTransmitter, this);	
	pWatch->IssueTransmitPosition(KThirdParty, KPriority);
	
	// RequestTransmitLocation() at PM
	TESTL(iProxy->WaitForResponse(KTimeOut) == ENetMsgRequestTransmitLocation);
	INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RequestTransmitLocation()"));	
	TBufC16<14> thirdParty(KThirdParty);
	TPtr16 ptr = thirdParty.Des(); 
	HBufC16* getThirdParty = NULL;
	TLbsNetSessionId* getSessionId = NULL;
	TInt getPriority(0);
	cleanupCnt = iProxy->GetArgsLC(ENetMsgRequestTransmitLocation, &getSessionId, &getThirdParty, &getPriority); 
	TESTL(ptr.Compare(*getThirdParty)==KErrNone);	
	TESTL(getPriority == KPriority);
	iSessionId = *getSessionId; //session ID is initialised by LBS
	CleanupStack::PopAndDestroy(cleanupCnt);

	// << ProcessStatusUpdate(EServiceTransmitThirdParty)
	INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessStatusUpdate(EServiceTransmitThirdParty)"));
	MLbsNetworkProtocolObserver::TLbsNetProtocolService service = MLbsNetworkProtocolObserver::EServiceTransmitThirdParty;		
	iProxy->CallL(ENetMsgProcessStatusUpdate, &service);
//End Initiate

//Reference Position Notification Start	
	// << ProcessLocationUpdate(refpos)
	TPositionInfo refPosInfo;	
	INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessLocationUpdate(Ref Pos)"));
	refPosInfo = ArgUtils::ReferencePositionInfo();								
	iProxy->CallL(ENetMsgProcessLocationUpdate, &iSessionId, &refPosInfo);
//Reference Position Notification End

//Assistance Data Notification Start
	// ProcessAssistanceData()
	TInt getReason = KErrNone;
	TLbsAsistanceDataGroup dataRequestMask = EAssistanceDataReferenceTime;
	RLbsAssistanceDataBuilderSet assistanceData;
	ArgUtils::PopulateLC(assistanceData);
	INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessAssistanceData(EAssistanceDataReferenceTime)"));
	iProxy->CallL(ENetMsgProcessAssistanceData, &dataRequestMask, &assistanceData, &getReason);
	CleanupStack::PopAndDestroy(&assistanceData);
// Assistance Data Notification End
	
// Network Location Request Start
	// ProcessLocationRequest()
	const TBool emergency(EFalse);
	TLbsNetPosRequestQuality quality = ArgUtils::QualityAlpha2(); 
	TLbsNetPosRequestMethod method   = RequestNetworkMethod();
	INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessLocationRequest(QualityAlpha2)"));
	iProxy->CallL(ENetMsgProcessLocationRequest, &iSessionId, &emergency, &service, &quality, &method);
// Network Location Request Stop

	//Start the timer
	TTime timerStart;
	timerStart.HomeTime();
	
	TLbsAsistanceDataGroup getDataGroup;
	
	// Module should ask for assistance data if mode supported
	if(!iNetRequestRejected)
		{
		// RequestAssistanceData(0)
		TESTL(iProxy->WaitForResponse(KSmallTimeOut) == ENetMsgRequestAssistanceData); 
		INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RequestAssistanceData(0)"));		
		cleanupCnt = iProxy->GetArgsLC(ENetMsgRequestAssistanceData, &getDataGroup);
		TESTL(getDataGroup == EAssistanceDataNone);
		CleanupStack::PopAndDestroy(cleanupCnt);		
		}

	// need to yield to scheduler here to allow gps mode notifications and ref pos notification to happen
	CheckForObserverEventTestsL(KTimeOut, *this);
	INFO_PRINTF1(_L("				Got Ref Pos"));
	
	DecideWhatNetworkReceives();
	//Find the time elapsed from timer
	TTimeIntervalMicroSeconds microseconds;
 	TTime timerStop;
 	timerStop.HomeTime();
 	microseconds = timerStop.MicroSecondsFrom(timerStart); 
	TInt64 timeElapsed = microseconds.Int64();
						
/*** NRH's Alpha2 timer expires; should get a response now ***/
	//Test that we do not get response before alpha2 has expired
	TInt delta = 1000*1000; // 1s
	getSessionId = NULL;
	getReason = KErrNone;
	TPositionSatelliteInfo* getPositionInfo = NULL;
	TPositionGpsMeasurementInfo* getMeasurementInfo = NULL;	
	if(iNetRequestRejected)	// network has asked for an unsupported gps mode
		{
		// we may get an error response _before_ alpha2 timer expires
		TESTL(iProxy->WaitForResponse(KAlpha2Timeout-timeElapsed+delta) == ENetMsgRespondLocationRequest); 	
		INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RespondLocationRequest"));
		cleanupCnt = iProxy->GetArgsLC(ENetMsgRespondLocationRequest, &getSessionId, &getReason, &getPositionInfo);
		TEST(getReason==KErrNotSupported);
		INFO_PRINTF2(_L("					RespondLocationRequest reason = %x"), getReason);		
		CleanupStack::PopAndDestroy(cleanupCnt);
		}
	else
		{
		// The measurements:
		if(iNetworkExpectsMeasurements)	// If network expecting measurements, they will come first 
			{
			getSessionId = NULL;
			getReason = KErrNone;
			getMeasurementInfo = NULL;			
			// Should not get a response before Alpha2 timer expires:
			TESTL(iProxy->WaitForResponse(KAlpha2Timeout-timeElapsed-delta) == ENetMsgTimeoutExpired); 
			INFO_PRINTF1(_L("				No RespondLocationRequest before Alpha2 timer expired"));
			TESTL(iProxy->WaitForResponse(2*delta) == ENetMsgRespondLocationRequest);		
			INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RespondLocationRequest(meas)"));
			cleanupCnt = iProxy->GetArgsLC(ENetMsgRespondLocationRequest, &getSessionId, &getReason, &getMeasurementInfo); 
			TESTL(getSessionId->SessionNum() == iSessionId.SessionNum());
			if(getReason != KErrNone)
				{
				INFO_PRINTF2(_L("getReason = 0x%x"), getReason);
				}			
			TESTL(getReason==KErrNone);
			TESTL(getMeasurementInfo->PositionClassType() == EPositionGpsMeasurementInfoClass);
			CleanupStack::PopAndDestroy(cleanupCnt);
			
			// Make another request from the network
			INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessLocationRequest(Quality)"));	
			quality = ArgUtils::Quality(); 
			iProxy->CallL(ENetMsgProcessLocationRequest, &iSessionId, &emergency, &service, &quality, &method);

			// module will request assistance data again (mask 0)
			// >> RequestAssistanceData(0)
			TESTL(iProxy->WaitForResponse(KSmallTimeOut) == ENetMsgRequestAssistanceData); 
			INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RequestAssistanceData(0)"));
			cleanupCnt = iProxy->GetArgsLC(ENetMsgRequestAssistanceData, &getDataGroup);
			TESTL(getDataGroup == EAssistanceDataNone);		
			CleanupStack::PopAndDestroy(cleanupCnt);	
			
			// >> RespondLocationRequest() 
			TESTL(iProxy->WaitForResponse(KTTimeout + delta) == ENetMsgRespondLocationRequest);
			INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RespondLocationRequest(meas)"));		
			getSessionId = NULL;
			getReason = KErrNone;
			getMeasurementInfo = NULL;				
			cleanupCnt = iProxy->GetArgsLC(ENetMsgRespondLocationRequest, &getSessionId, &getReason, &getMeasurementInfo);			

			// Check it is measurement
			TESTL(getMeasurementInfo->PositionClassType() == EPositionGpsMeasurementInfoClass);
			TESTL(getSessionId->SessionNum() == iSessionId.SessionNum());
			TESTL(getReason == KErrNone);
			CleanupStack::PopAndDestroy(cleanupCnt);			
			
			if(iPlannedPositionOriginator == EPositionOriginatorModule)
				{ // if hybrid need to do one more request (which will result in module returning gps position)
				// << ProcessLocationRequest(SessionId, HybridMode, t)
				INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessLocationRequest(SessionId, HybridMode, t)"));
				iProxy->CallL(ENetMsgProcessLocationRequest, &iSessionId, &emergency, &service, &quality, &method);

				// >> RequestAssistanceData(0)
				INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RequestAssistanceData(0)"));
				TESTL(iProxy->WaitForResponse(KTimeOut) == ENetMsgRequestAssistanceData); 
				cleanupCnt = iProxy->GetArgsLC(ENetMsgRequestAssistanceData, &getDataGroup);
				TESTL(getDataGroup == EAssistanceDataNone);
				CleanupStack::PopAndDestroy(cleanupCnt);					
				}			
			}
		
		// The final position:
		if(iPlannedPositionOriginator == EPositionOriginatorModule)
			{
			// The module should return the position
			ASSERT(iNetworkExpectsPositions);
			INFO_PRINTF1(_L("				Network expecting position"));
			// >> RespondLocationRequest() - first measurement, second position.
			TESTL(iProxy->WaitForResponse(KTTimeout + delta) == ENetMsgRespondLocationRequest);
			INFO_PRINTF1(_L(">>>>>>>>>>>>>>>>>>>>>>>>>>> RespondLocationRequest(gpspos)"));			
			getPositionInfo = NULL;
			getSessionId = NULL;
			getReason = KErrNone;			
			cleanupCnt = iProxy->GetArgsLC(ENetMsgRespondLocationRequest, &getSessionId, &getReason, &getPositionInfo);
			// check it is a position
			TESTL(getPositionInfo->PositionClassType() == (EPositionInfoClass|EPositionCourseInfoClass|EPositionSatelliteInfoClass|EPositionExtendedSatelliteInfoClass));
			TESTL(getSessionId->SessionNum() == iSessionId.SessionNum());
			TESTL(getReason == KErrNone);
			// Test position is the same as in the ini file data fed to the GPS module
			// $update,1,0,49.2,3.5,50,20,20*
			TPosition gpsPos;
			getPositionInfo->GetPosition(gpsPos);
			TESTL(gpsPos.Latitude()==49.2 && gpsPos.Longitude()==3.5 && gpsPos.Altitude()==50 && gpsPos.HorizontalAccuracy()==20 && gpsPos.VerticalAccuracy()==20); 			
			CleanupStack::PopAndDestroy(cleanupCnt);
			// << ProcessLocationUpdate(SessionId, FinalNetworkPosition)
			// Return modules' position as FinalNetworkPosition
			TPositionInfo gpsPosInfo;
			gpsPosInfo.SetPosition(gpsPos);
			gpsPosInfo.SetUpdateType(EPositionUpdateGeneral);
			gpsPosInfo.SetPositionMode(TPositionModuleInfo::ETechnologyTerminal | TPositionModuleInfo::ETechnologyAssisted);
			gpsPosInfo.SetPositionModeReason(EPositionModeReasonNone);			
			INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessLocationUpdate(SessionId, FinalNetworkPosition)"));
			iProxy->CallL(ENetMsgProcessLocationUpdate, &iSessionId, &gpsPosInfo);			
			}
		else if(iPlannedPositionOriginator == EPositionOriginatorNetwork)
			{
			// The network should return the position
			TPositionInfo positionInfo = ArgUtils::AccurateNetworkPositionInfo();
			// << ProcessLocationUpdate(SessionId, FinalNetworkPosition)
			// Return FinalNetworkPosition
			INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessLocationUpdate(SessionId, FinalNetworkPosition)"));
			iProxy->CallL(ENetMsgProcessLocationUpdate, &iSessionId, &positionInfo);			
			}
		}
		
// Session Complete Start
	TInt reason;
	if(iNetRequestRejected)
		{
		reason = KErrNotSupported;
		}
	else
		{
		reason = KErrNone;
		}
	INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessSessionComplete()"));		
	iProxy->CallL(ENetMsgProcessSessionComplete, &iSessionId, &reason);
	MLbsNetworkProtocolObserver::TLbsNetProtocolServiceMask activeServiceMask = MLbsNetworkProtocolObserver::EServiceNone;
	INFO_PRINTF1(_L("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ProcessStatusUpdate(EServiceNone)"));		
	iProxy->CallL(ENetMsgProcessStatusUpdate, &activeServiceMask);
// Session Complete Stop
	
	// need to yield to scheduler here to allow gps mode notifications and transmit pos notification to happen
	CheckForObserverEventTestsL(KTimeOut, *this);
	INFO_PRINTF1(_L("				Got Transmitted Pos"));	
	
	// Done. Now cleanup...	
	CleanupStack::PopAndDestroy(pWatch);
	
	return TestStepResult();
	}


TLbsNetPosRequestMethod CT_LbsX3PGpsOptions::RequestNetworkMethod()
/**
 This method will chose the appropiate method requested by the network, depending 
 on the test settings
 
 @return The network method
 */
	{
	
	TLbsNetPosRequestMethod method;
	
	switch(iNetworkMethod)
		{
		case ETerminalBasedNetworkMethod:
			{
			iNetworkExpectsPositions = ETrue;
			method = ArgUtils::RequestTerminalBasedMethod();
			break;
			}
		case ETerminalBasedTerminalAssistedNetworkMethod:
			{
			iNetworkExpectsMeasurements = ETrue;
			iNetworkExpectsPositions = ETrue;
			method = ArgUtils::RequestHybridMethod();
			break;
			}
		case ETerminalAssistedNetworkMethod:
			{
			iNetworkExpectsMeasurements = ETrue;
			method = ArgUtils::RequestTAPMethod();
			break;
			}
		case ETerminalAssistedTerminalBasedNetworkMethod:
			{
			iNetworkExpectsMeasurements = ETrue;
			iNetworkExpectsPositions = ETrue;
			method = ArgUtils::RequestTerminalAssistedAndTerminalBasedMethod();
			break;
			}
		case ENetworkMethodNotSet:
			{
			//we should set mode according to default admin 
			TEST(iAdminGpsMode != CLbsAdmin::EGpsModeUnknown);
			switch(iAdminGpsMode)
			{
			case CLbsAdmin::EGpsPreferTerminalBased:
				{
				iNetworkExpectsPositions = ETrue;
				break;
				}
			case CLbsAdmin::EGpsPreferTerminalAssisted:
			case CLbsAdmin::EGpsAlwaysTerminalAssisted:
				{
				iNetworkExpectsMeasurements = ETrue;
				break;
				}
			case CLbsAdmin::EGpsAutonomous:	// test framework doesn't currently support this
			default:
				{
				User::Invariant();
				}
			}	
			method = ArgUtils::RequestUnspecifiedMethod();
			break;
			}
		default:
			{
			User::Invariant();
			}
		}
	return method;
	}


void CT_LbsX3PGpsOptions::DecideWhatNetworkReceives()
/**
 This method checks what the module was finally set to and decides if the network 
 should expect measuments, or positions
 */
	{
	TLbsGpsOptionsArray options;
	TPckg<TLbsGpsOptionsArray> pckgOptions(options);
	TEST(KErrNone == RProperty::Get(KUidSystemCategory, ELbsTestAGpsModuleModeChanges, pckgOptions));

	if(options.ClassType() & ELbsGpsOptionsArrayClass)
		{ 
		if(options.NumOptionItems() > 1)
			{
			// don't change anything (from what was set in RequestNetworkMethod()), the module is running in hybrid
			return;
			}
		}
	switch(options.GpsMode())
		{
		case CLbsAdmin::EGpsAutonomous:
			{
			iNetworkExpectsPositions = EFalse;
			iNetworkExpectsMeasurements = EFalse;
			break;
			}
		case CLbsAdmin::EGpsPreferTerminalBased:
		case CLbsAdmin::EGpsAlwaysTerminalBased:
			{
			iNetworkExpectsMeasurements = EFalse;
			break;
			}
		case CLbsAdmin::EGpsPreferTerminalAssisted:
		case CLbsAdmin::EGpsAlwaysTerminalAssisted:
			{
			iNetworkExpectsPositions = EFalse;
			break;
			}
		default:
			{
			// change nothing
			}
		return;
		}
	}

void CT_LbsX3PGpsOptions::OnTransmitRefPosition(TInt32 aErr, const TPositionInfoBase& /* aPosInfo */)
	{
	TESTL(aErr==KErrNone);	
	ReturnToTestStep();
	}
	
void CT_LbsX3PGpsOptions::OnTransmitPosition(TInt32 aErr, const TPositionInfoBase& /* aPosInfo */)
	{
	if(iNetRequestRejected)
		{
		TESTL(aErr==KErrNotSupported);
		}
	else
		{
		// TO DO - verify technology type is as expected?
		TESTL(aErr==KErrNone);
		}	
	
	ReturnToTestStep();
	}
