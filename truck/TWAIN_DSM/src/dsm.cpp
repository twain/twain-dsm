/***************************************************************************
 * Copyright © 2006 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 ***************************************************************************/

/**
* @file dsm.cpp
* Data Source Manager.
* This software manages the interactions between the application 
* and the Source. 
* @author JFL Peripheral Solutions Inc.
* @date October 2005
*/

/*! \mainpage Data Source Manager
 *
 * The Source Manager provides the communication path between the 
 * application and the Source, supports the user’s selection of a  
 * Source, and loads the Source for access by the application.   
 * Communications from application to Source Manager arrive in the 
 * DSM_Entry( ) entry point.
 *
 *
 *
 *
 *
 *
 *
 *
 * Copyright © 2006 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 */


#if defined(_WIN32)
  #include "stdafx.h"
  #include <direct.h>
#else
 #include <dirent.h>
 #include <dlfcn.h>
 #include <unistd.h>
#endif // _WIN32

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "twain/twain.h"
#include "dsmdefs.h"
#include "dsm.h"

#include "resource.h" 

using namespace std;

/**
* Global pointer to logfile.  Logfile is used to give feedback.
* @see kLOG, kLOGERR, kLOGENV
*/
ofstream* gpLogFile = 0;

/**
* Global pointer to store PID.  Stores the current PID or 
* ThreadIDGetCurrentThreadId.  Used when writing to log file.
* @see kLOG, kLOGERR, kLOGENV
*/
#ifdef _WIN32
DWORD gOurPid;
extern HINSTANCE g_hinstDLL;
#else
pid_t gOurPid;
#endif // _WIN32

/**
* Define to write PID to LogFile.
* @see gpLogFile, gOurPid, kLOGERR, kLOGENV
*/
#define kLOG if(0 != gpLogFile && gpLogFile->is_open()) *gpLogFile << "DSM [" << gOurPid << "]: "

/**
* Define to write error to LogFile.
* @see gpLogFile, gOurPid, kLOG, kLOGENV
*/
#define kLOGERR if(0 != gpLogFile && gpLogFile->is_open()) *gpLogFile << "DSM [" << gOurPid << "]: Error - "

/**
* Enviroment varible of path to where to write the LogFile name.
* @see gpLogFile, gOurPid, kLOG, kLOGERR
*/
#define kLOGENV "TWAIN_LOG"

#ifdef _WIN32
#define kTWAIN_DS_DIR "c:\\windows\\twain_32"
// In non-windows, kTWAIN_DS_DIR is set on the compiler command line
#endif

/**
* Handles DAT_NULL calls from DS for Application.
* @param[in] _pOrigin Origin of message in this case a DS
* @param[in] _pDest destination of message in this case an App
* @param[in] _MSG message id: MSG_xxxx
* @return a valid TWRC_xxxx return code
* @todo I don't like how we only store the last message for the app. If
*       multiple DS's make a callback to a single app, we are going to lose MSG's.
*/
TW_INT16 DSM_Null(pTW_IDENTITY _pOrigin, pTW_IDENTITY _pDest, TW_UINT16 _MSG);

/**
* Scan for Data Sources.
* Recursively navigate the TWAIN datasource dir looking for data sources.
* Store all valid data sources in _pList upto a maximum of MAX_NUM_DS 
* data sources.
* @param[in] _szAbsPath starting directory to begin search.
* @param[out] _pAppIdentity the application requesting scan.
* @param[out] _pList list of valid Data Sources found in directory path.
* @return either EXIT_SUCCESS or EXIT_FAILURE.
*/
int scanDSDir(char* _szAbsPath, pTW_IDENTITY _pAppIdentity, pDS_LIST _pList);

/**
* Returns the current DSM status. Resets _gConditionCode to TWCC_SUCCESS.
* @param[in] _pAppIdentity Origin of message
* @param[in] _MSG message id: MSG_xxxx
* @param[out] _pStatus TW_STATUS structure
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_Status(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG, pTW_STATUS _pStatus);

/**
* Initializes or closes the DSM
* @param[in] _pAppIdentity Orgin of message
* @param[in] _MSG message id: MSG_xxxx
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_Parent(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG);

/**
* Source operations
* @param[in] _pAppIdentity Origin of message
* @param[in] _MSG message id: MSG_xxxx
* @param[in] _pSourceIdentity TW_IDENTITY structure
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_Identity(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG, pTW_IDENTITY _pSourceIdentity);

/**
* Register application's callback.
* @param[in] _pAppIdentity Origin of message
* @param[in] _MSG message id: MSG_xxxx valid = MSG_REGISTER_CALLBACK
* @param[in] _pData pointer to a callback struct
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_Callback(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG, pTW_CALLBACK _pData);

/**
* Initialize the DSM.  Clear globals and open data sources.
* @param[in] _pAppIdentity Origin of message
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_InitializeDSM(pTW_IDENTITY _pAppIdentity);


/**
* Opens the Data Source specified by pDSIdentity.  
* pDSIdentity must be valid, but if a null name and id
* is 0 then open default.
* @param[in] _pAppIdentity Origin of message
* @param[in] _pSourceIdentity TW_IDENTITY structure
* @return a valid TWRC_xxxx return code
* @todo check this TW_INT16 cast to ensure we are not losing data.
*/
TW_INT16 OpenDS(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);

/**
* Closes the Data Source specified by pDSIdentity.
* @param[in] _pAppIdentity Origin of message
* @param[in] _pSourceIdentity TW_IDENTITY structure
* @return a valid TWRC_xxxx return code
*/
TW_INT16 CloseDS(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);

/**
* Displays the source select dialog and sets the default source.
* @param[in] _pAppIdentity Origin of message
* @param[in,out] _pSourceIdentity TW_IDENTITY structure
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_SelectDS(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);

/**
* Calls the DS, _pSourceIdentity's, entry point with the operation.
* @param[in] _pAppIdentity Origin of message
* @param[in] _pSourceIdentity Data Source destination of message
* @param[in] _DG data group id: DG_xxxx
* @param[in] _DAT data argument type: DAT_xxxx
* @param[in] _MSG message id: MSG_xxxx
* @param[in,out] _pData pointer to data
* @return a valid TWRC_xxxx return code
*/
TW_INT16 EntryDS(pTW_IDENTITY _pAppIdentity,
                 pTW_IDENTITY _pSourceIdentity,
                 TW_UINT32 _DG,
                 TW_UINT16 _DAT,
                 TW_UINT16 _MSG,
                 TW_MEMREF _pData);

/**
* Loads a DS from disk and adds it to a global list of DS's.
* @param[in] _pAppIdentity Origin of message
* @param[in] _pPath The path to the library to open
* @param[in] _index the source array index
* @param[in] _pList a list of valid sources
* @return a valid TWRC_xxxx return code
*/
TW_INT16 LoadDS(pTW_IDENTITY _pAppIdentity,
                char* _pPath,
                TW_INT16 _index,
                pDS_LIST _pList);


/**
* UnLoads a DS from disk to remove them memory.
* @param[in] _pAppIdentity Origin of message
* @return a valid TWRC_xxxx return code
*/
TW_INT16 UnLoadAllDS(pTW_IDENTITY _pAppIdentity);

/**
* Goes through the applications supported data sources looking for one that has
* the exact same name as product name in the passed in identity. Will update the
* _pSourceIdentity structure to match the name.
* @param[in] _pAppIdentity Origin of message
* @param[in,out] _pSourceIdentity TW_IDENTITY structure
* @return a valid TWRC_xxxx return code
*/
TW_INT16 GetDSFromProductName(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);

/**
* Loads all the datasources found in the /usr/lib/twain directory
* @param[in] _pAppIdentity Origin of message
* @param[out] _pList the data source list to add the opened data sources too
* @return a valid TWRC_xxxx return code
*/
TW_INT16 LoadAllDataSources(pTW_IDENTITY _pAppIdentity, pDS_LIST _pList);

/**
* returns TRUE if the two identities support the same groups
* @param[in] _pAppIdentity The applications identity structure
* @param[in] _pSourceIdentity The sources identity structure
* @return TRUE if the two match, FALSE if they don't
*/
TW_BOOL SupportedMatch(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);

/**
* Copies the applications first available source into _pSourceIdentity.
* @param[in] _pAppIdentity The origin identity structure
* @param[out] _pSourceIdentity the identity structure to copy data into
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_GetFirst(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);

/**
* Copies the applications next available source into _pSourceIdentity. A call to
* DSM_GetFirst must have been made at least once before calling this function.
* @param[in] _pAppIdentity The origin identity structure
* @param[out] _pSourceIdentity the identity structure to copy data into
* @return a valid TWRC_xxxx return code
*/
TW_INT16 DSM_GetNext(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);


/**
* This routine will check if the current default source matches the
* applications supported groups.  If it does it will copy it into the default
* Source's identity (_pSourceIdentity), otherwise this routine will search for a source that
* does match the app's supported groups and copy it into _pSourceIdentity.
* @param[in] _pAppIdentity The application identity
* @param[in,out] _pSourceIdentity A pointer reference that will be set to point to the default identity.
* @return a valid TWRC_xxxx return code
*/
TW_INT16 GetMatchingDefault(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity);


/**
* Adds an application.
* Adds an application to the global list of applications.
* Adding the application will assign a unique ID to it, and also determine
* the compatible sources that it can use.
* @param[in,out] _pAppIdentity The application identity
* @return a valid TWRC_xxxx return code
*/
TW_INT16 AddApplication(pTW_IDENTITY _pAppIdentity);

/**
* Searches the list of registered applications for one that matches by name.
* @param[in] _pName the name of the application to find
* @return a pointer to the matching application or NULL if not found.
*/
pAPP_INFO findApplicationByName(pTW_STR32 _pName);


/**
* gets the current condition code for the application.  If the application
* cannot be determined, then the global condition code is returned.
* The condition code that is retrieved is then reset to TWCC_SUCCESS.
* @param[in] _pAppIdentity The application identity
* @return the condition code
*/
TW_UINT16 getConditionCode(pTW_IDENTITY _pAppIdentity);

/**
* sets the condition code for the application.  If the application
* cannot be determined, then the global condition code is set.
* @param[in] _pAppIdentity The application identity
* @param[in] _ConditionCode The condition code
* @return TRUE if the condition code could be set.
*/
TW_BOOL setConditionCode(pTW_IDENTITY _pAppIdentity, TW_UINT16 _ConditionCode);


/**
* prints to stdout information about the triplets.
* @param[in] _DG the Data Group
* @param[in] _DAT the Data Argument Type
* @param[in] _MSG the Message
* @param[in] _pData the Data
* @return return true if actualy printed triplet
*/
bool printTripletsInfo(const TW_UINT32    _DG,
                       const TW_UINT16    _DAT,
                       const TW_UINT16    _MSG,
                       const TW_MEMREF    _pData);

/**
* prints to stdout return code information.
* @param[in] rc the ReturnCode to print
*/
void printReturnCode( const TW_UINT16 rc );

/**
* Translates the _MSG passed in into a string and returns it
* @param[in] _MSG the TWAIN message to translate
* @return a string that represents the _MSG
*/
string StringFromMsg(const TW_UINT16 _MSG);

/**
* Translates the _DAT passed in into a string and returns it
* @param[in] _DAT the TWAIN data argument type to translate
* @return a string that represents the _DAT
*/
string StringFromDat(const TW_UINT16 _DAT);

/**
* Translates the _DG passed in into a string and returns it
* @param[in] _DG the TWAIN data group to translate
* @return a string that represents the _DG
*/
string StringFromDG(const TW_UINT32 _DG);

/**
* Translates the _Cap passed in into a string and returns it
* @param[in] _Cap the TWAIN Capability to translate
* @return a string that represents the _Cap
*/
string StringFromCap(const TW_UINT16 _Cap);

/**
* Translates the rc passed in into a string and returns it
* @param[in] rc the TWAIN Return Code to translate
* @return a string that represents the rc
*/
string StringFromRC(const TW_UINT16 rc);

/**
* Translates the cc passed in into a string and returns it
* @param[in] cc the TWAIN Condition Code to translate
* @return a string that represents the cc
*/
string StringFromCC(const TW_UINT16 cc);

/**
* This is the non-application specific condition code. It is set when something
* bad happens and the application making the call is uknown.
*/
TW_INT16 _gConditionCode = TWCC_SUCCESS;

/**
* A global status indicating frameworks status.
* True if the framework has been initialized.  
*/
TW_BOOL _gFrameworkInitialized = FALSE;

/**
* A global list of application using this DSM.
* Each application's info and status is stored here.
* With 32bit each application would have its own copy of the DSM data space
* but with this list it allows One application connect several times to 
* control several scanners at the same time as long as it uses a different 
* name with each connection.
*/
APP_LIST _gApplications;

/**
* The path to the default DS.  The Default DS is identified when the DSM is 
* opened.  A new Default is saved if SelectDlg is used.
*/
char _gDefaultDSPath[MAX_PATH];

/** 
* The name of the default DS.  The Default DS is identified when the DSM is 
* opened.  A new Default is saved is SelectDlg is used.
*/
TW_STR32 _gDefaultDSName;

#ifndef _WIN32
//////////////////////////////////////////////////////////////////////////////
TW_HANDLE DSM_Alloc(size_t _size)
{
  return malloc(_size);
}

//////////////////////////////////////////////////////////////////////////////
void DSM_Free(TW_HANDLE _pPtr)
{
  free(_pPtr);
}

//////////////////////////////////////////////////////////////////////////////
TW_MEMREF DSM_LockMemory(TW_HANDLE _pMemory)
{
  return (TW_MEMREF)_pMemory;
}

//////////////////////////////////////////////////////////////////////////////
void DSM_UnlockMemory(TW_MEMREF _pMemory)
{
  return;
}
#endif // _WIN32

//////////////////////////////////////////////////////////////////////////////
/**
* Data Source Manager Entry Point.
* The only entry point into the Data Source Manager.
* Defined in twain.h
*
* @param[in] _pOrigin Identifies the source module of the message. This could
*           identify an Application, a Source, or the Source Manager.
*
* @param[in] _pDest Identifies the destination module for the message.
*           This could identify an application or a data source.
*           If this is NULL, the message goes to the Source Manager.
*
* @param[in] _DG The Data Group. 
*           Example: DG_IMAGE.
*
* @param[in] _DAT The Data Attribute Type.
*           Example: DAT_IMAGEMEMXFER.
*    
* @param[in] _MSG The message.  Messages are interpreted by the destination module
*           with respect to the Data Group and the Data Attribute Type.  
*           Example: MSG_GET.
*
* @param[in,out] _pData A pointer to the data structure or variable identified 
*           by the Data Attribute Type.
*           Example: (TW_MEMREF)&ImageMemXfer
*                   where ImageMemXfer is a TW_IMAGEMEMXFER structure.
*                    
* @return a valid TWRC_xxxx return code.
*          Example: TWRC_SUCCESS.
*/
#ifdef _WIN32
TW_UINT16 FAR PASCAL
#else
FAR PASCAL TW_UINT16 
#endif
DSM_Entry(pTW_IDENTITY _pOrigin,
          pTW_IDENTITY _pDest,
          TW_UINT32    _DG,
          TW_UINT16    _DAT,
          TW_UINT16    _MSG,
          TW_MEMREF    _pData)
{
  TW_UINT16 rcDSM = TWRC_SUCCESS;

  // If the _pOrigin is a null pointer, there is nothing we can do, so return
  // an error.
  if(0 == _pOrigin)
  {
    setConditionCode(_pOrigin, TWCC_BUMMER);
    return TWRC_FAILURE;
  }

  if(!_gFrameworkInitialized)
  {
    rcDSM = DSM_InitializeDSM(_pOrigin);

    if(TWRC_SUCCESS != rcDSM)
    {
      setConditionCode(_pOrigin, TWCC_BUMMER);
      return rcDSM;
    }
  }

  // Print the triplets to stdout for information purposes
  bool bPrinted = printTripletsInfo(_DG, _DAT, _MSG, _pData);

  // first, see if this message is a DAT_NULL msg. This has to be watched for
  // because its a special message.
  if(DAT_NULL == _DAT)
  {
    rcDSM = DSM_Null(_pOrigin, _pDest, _MSG);
    if( bPrinted )
    {
      printReturnCode( rcDSM );
    }
    return rcDSM;
  }

  // second, sniff for the application forwarding an event to the DS. It may be
  // possible that the app has a message waiting for it because it didn't
  // register a callback.
  if( DAT_EVENT == _DAT && MSG_PROCESSEVENT == _MSG)
  {
    if(_pOrigin->Id > 0 && _pOrigin->Id <= MAX_NUM_APPS)
    {
      pAPP_INFO pAppInfo = &(_gApplications.AppInfo[kIN(_pOrigin->Id)]);

      if(TRUE == pAppInfo->Callback.bCallbackPending)
      {
        ((pTW_EVENT)(_pData))->TWMessage = pAppInfo->Callback.callback.Message;

        pAppInfo->Callback.bCallbackPending = FALSE;
        pAppInfo->Callback.callback.Message = NULL;
        return TWRC_DSEVENT;
      }
    }
    else
    {
      // Getting here means the app has not been registered yet.
      // What is a non-registered app doing calling MSG_PROCESSEVENT?
      setConditionCode(_pOrigin, TWCC_BADPROTOCOL);
      if( bPrinted )
      {
        printReturnCode( TWRC_FAILURE );
      }

      return TWRC_FAILURE;
    }
  }

  // Is this msg for us?
  if(0 == _pDest)
  {
    switch (_DAT)
    {
      case DAT_PARENT:
        rcDSM = DSM_Parent(_pOrigin, _MSG);
        break;

      case DAT_IDENTITY:
        rcDSM = DSM_Identity(_pOrigin, _MSG, (pTW_IDENTITY)_pData);
        break;

      case DAT_STATUS:
        rcDSM = DSM_Status(_pOrigin, _MSG, (pTW_STATUS)_pData);
        break;

      case DAT_CALLBACK:
        rcDSM = DSM_Callback(_pOrigin, _MSG, (pTW_CALLBACK)_pData);
        break;

      default:
        setConditionCode(_pOrigin, TWCC_BADPROTOCOL);
        rcDSM = TWRC_FAILURE;
        break;
    }
  }
  else // Not for us, send to appropriate source
  {
    // check if the application is open or not.  If it isn't, we have a bad sequence
    if( _pOrigin->Id > 0 && _pOrigin->Id <= MAX_NUM_APPS &&
        dsmState_Open == _gApplications.AppInfo[kIN(_pOrigin->Id)].CurrentState )
    {
      rcDSM = EntryDS(_pOrigin, _pDest, _DG, _DAT, _MSG, _pData);
    }
    else
    {
      setConditionCode(_pOrigin, TWCC_SEQERROR);
      rcDSM = TWRC_FAILURE;
    }
  }

  if( bPrinted )
  {
    printReturnCode( rcDSM );
  }

  return rcDSM;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_Status(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG, pTW_STATUS _pStatus)
{
  TW_INT16 result = TWRC_SUCCESS;

  switch (_MSG)
  {
    case MSG_GET:
      {
        // If the application has not registered with the DSM yet, then act on the
        // global condition code instead of the application specific one.
        // This will only happen on very rare occasions where the condition code
        // could not be set because the calling application was ambiguous.
        _pStatus->ConditionCode = getConditionCode(_pAppIdentity);
        _pStatus->Reserved = 0;
      }
      break;

    default:
      result = TWRC_FAILURE;
      setConditionCode(_pAppIdentity, TWCC_BADPROTOCOL);
      break;
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_Parent(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG)
{
  TW_INT16 result = TWRC_SUCCESS;
  assert(_pAppIdentity);

  switch (_MSG)
  {
    case MSG_OPENDSM:
      // see if we have reached our limit of maximum applications
      if(MAX_NUM_APPS == _gApplications.NumApps)
      {
        setConditionCode(_pAppIdentity, TWCC_MAXCONNECTIONS);
        return TWRC_FAILURE;
      }

      if(_pAppIdentity->ProductName[0] == 0)
      {
        setConditionCode(_pAppIdentity, TWCC_BADVALUE);
        return TWRC_FAILURE;
      }

      // Only add apps that have not already been added...
      if(0 == findApplicationByName(_pAppIdentity->ProductName))
      {
        // new application, add it to the list and get assigned an ID
        result = AddApplication(_pAppIdentity);
      }

      if(result == TWRC_SUCCESS)
      {
        assert(_pAppIdentity->Id <= MAX_NUM_APPS);
        pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

        // move DSM to state 3 for this app
        pAppInfo->CurrentState = dsmState_Open;

        result = LoadAllDataSources(_pAppIdentity, pAppInfo->pDSList);
      }
      break;

    case MSG_CLOSEDSM:
      {
        assert(_pAppIdentity->Id > 0 && _pAppIdentity->Id <= MAX_NUM_APPS);
        pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];
        // To close the DSM we must be open ... 
        if(dsmState_Open != pAppInfo->CurrentState)
        {
          setConditionCode(_pAppIdentity, TWCC_SEQERROR);
          return(TWRC_FAILURE);
        }
        // ...but no DS can be open
        if( -1 != pAppInfo->OpenSource )
        {
          setConditionCode(_pAppIdentity, TWCC_SEQERROR);
          return(TWRC_FAILURE);
        }

        //unload all DS for this App
        UnLoadAllDS(_pAppIdentity);

        delete pAppInfo->pDSList;
        pAppInfo->pDSList = NULL;
        pAppInfo->pOrigin = NULL;

        _gApplications.NumApps--;

        // switch applications DSM state to state 2
        pAppInfo->CurrentState = dsmState_Loaded;

        // If this is the last application to close the DSM then we need to 
        // prepair to exit.
        bool bAllAppsClosed = true;
        for( int i=0; i < MAX_NUM_APPS;  i++)
        {
          if( _gApplications.AppInfo[i].pOrigin != NULL &&
              dsmState_Open <= _gApplications.AppInfo[i].CurrentState )
          {
            bAllAppsClosed = false;
            break;
          }
        }

        if( bAllAppsClosed )
        {
          //if all are closed then there should not be any left
          assert(_gApplications.NumApps == 0);

          _gFrameworkInitialized = FALSE;
          if(gpLogFile)
          {
            if(gpLogFile->is_open())
            {
              *gpLogFile << endl;
              gpLogFile->close();
            }
            delete gpLogFile;
            gpLogFile = 0;
          }
        }
      }
      break;

    default:
      result = TWRC_FAILURE;
      setConditionCode(_pAppIdentity, TWCC_BADPROTOCOL);
      break;
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_Identity(pTW_IDENTITY _pAppIdentity, TW_UINT16 _MSG, pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 result = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  if(dsmState_Open == pAppInfo->CurrentState)
  {
    switch (_MSG)
    {
      case MSG_OPENDS:
        result = OpenDS(_pAppIdentity, _pSourceIdentity);
        break;

      case MSG_CLOSEDS:
        result = CloseDS(_pAppIdentity, _pSourceIdentity);
        break;

      case MSG_USERSELECT:
        result = DSM_SelectDS(_pAppIdentity, _pSourceIdentity);
        break;

      case MSG_GETFIRST:
        result = DSM_GetFirst(_pAppIdentity, _pSourceIdentity);
        break;

      case MSG_GETNEXT:
        result = DSM_GetNext(_pAppIdentity, _pSourceIdentity);
        break;

      case MSG_GETDEFAULT:
        result = GetMatchingDefault(_pAppIdentity, _pSourceIdentity);
        break;

      default:
        result = TWRC_FAILURE;
        setConditionCode(_pAppIdentity, TWCC_BADPROTOCOL);
        break;
    }
  }
  else
  {
    result = TWRC_FAILURE;
    setConditionCode(_pAppIdentity, TWCC_SEQERROR);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_Callback(pTW_IDENTITY _pAppIdentity,
                      TW_UINT16    _MSG,
                      pTW_CALLBACK _pData)
{
  TW_INT16 result = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  switch(_MSG)
  {
    case MSG_REGISTER_CALLBACK:
      {
        memset(&(pAppInfo->Callback), 0, sizeof(CallBackInfo));
        pAppInfo->Callback.callback.CallBackProc = _pData->CallBackProc;
        pAppInfo->Callback.callback.RefCon = _pData->RefCon;
      }
      break;

    default:
      result = TWRC_FAILURE;
      setConditionCode(_pAppIdentity, TWCC_BADPROTOCOL);
      break;
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_InitializeDSM(pTW_IDENTITY _pAppIdentity)
{
  TW_INT16 ret = TWRC_SUCCESS;

  // store our pid for convenience.
#ifdef _WIN32
  gOurPid = GetCurrentThreadId();
#else
  gOurPid = getpid();
#endif // _WIN32

  // see if a logfile is to be used
  char* logpath = getenv(kLOGENV);
  if(0 != logpath && 0 == gpLogFile)
  {
    gpLogFile = new ofstream(logpath, ios::app);

    if(!gpLogFile->is_open())
    {
      cerr << "DSM: Error - logging has been disabled because logfile could not be opened: " << logpath << endl;
      delete gpLogFile;
      gpLogFile = 0;
    }
  }

  memset(&_gApplications, 0, sizeof(_gApplications));
  memset(_gDefaultDSPath, 0, sizeof(_gDefaultDSPath));
  memset(_gDefaultDSName, 0, sizeof(_gDefaultDSName));

  _gFrameworkInitialized = TRUE;

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 OpenDS(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 result = TWRC_FAILURE;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  // check that we are in the proper state
  if(dsmState_Open != pAppInfo->CurrentState)
  {
    setConditionCode(_pAppIdentity, TWCC_SEQERROR);
    return(TWRC_FAILURE);
  }

  // Do we need to find a source to open
  if(0 == _pSourceIdentity->Id)
  {
    // Does the app know the name of the source it wants to open
    if(0 != _pSourceIdentity->ProductName[0])
    {
      // The application is passing me a TW_IDENTITY structure that contains
      // the name of the source to select.
      result = GetDSFromProductName(_pAppIdentity, _pSourceIdentity);

      // was the id found or specified by the app?
      if(TWRC_SUCCESS != result)
      {
        setConditionCode(_pAppIdentity, TWCC_NODS);
        return(result);
      }
    }

    // Does the application want me to choose the default?
    // Or no Source located by name
    if(0 == _pSourceIdentity->ProductName[0]) 
    {
      // -if the name of the source is NULL, and the id is 0, the application is
      //  telling me to select the default source.
      result = GetMatchingDefault(_pAppIdentity, _pSourceIdentity);

      // was the id found or specified by the app?
      if(TWRC_SUCCESS != result)
      {
        return(result);
      }
    }
  }

  // Do a quick check that the id of the DS given is not out of range
  if(_pSourceIdentity->Id > pAppInfo->pDSList->NumFiles)
  {
    setConditionCode(_pAppIdentity, TWCC_NODS);
    return(TWRC_FAILURE);
  }

  // open the ds
  pDS_INFO pDSInfo = &pAppInfo->pDSList->DSInfo[kIN(_pSourceIdentity->Id)];
  if(0 != pDSInfo->DS_Entry)
  {
    result = pDSInfo->DS_Entry(_pAppIdentity,
             DG_CONTROL,
             DAT_IDENTITY,
             MSG_OPENDS,
             (TW_MEMREF) _pSourceIdentity);

    if(TWRC_SUCCESS == result)
    {
      // update the applications datastruct with the successfully opened source.
      pAppInfo->OpenSource = TW_INT16(kIN(_pSourceIdentity->Id));
      pDSInfo->bOpen = TRUE;
    }
    else
    {
      setConditionCode(_pAppIdentity, TWCC_OPERATIONERROR);
    }
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 CloseDS(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 result = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  // check that we are in the proper state
  if(dsmState_Open != pAppInfo->CurrentState)
  {
    setConditionCode(_pAppIdentity, TWCC_SEQERROR);
    return(TWRC_FAILURE);
  }

  // close the ds
  pDS_INFO pDSInfo = &pAppInfo->pDSList->DSInfo[kIN(_pSourceIdentity->Id)];
  if(0 != pDSInfo->DS_Entry)
  {
    result = pDSInfo->DS_Entry(_pAppIdentity,
             DG_CONTROL,
             DAT_IDENTITY,
             MSG_CLOSEDS,
             (TW_MEMREF) _pSourceIdentity);

    if(TWRC_SUCCESS == result)
    {
      pAppInfo->OpenSource = -1;
      pDSInfo->bOpen = FALSE;
    }
    else
    {
      setConditionCode(_pAppIdentity, TWCC_OPERATIONERROR);
    }
  }

  return result;
}


/*
* data to pass in and out of select Dialog funtion SelectDlgProc
*/
pTW_IDENTITY  g_pSelectDlgSourceID; /**< @param[in,out] Default DS is passed in and the selected DS is passed back. */
pDS_LIST      g_pSelectDlgDSList;   /**< @param[in]     The list of DS to display */

/**
* Select DS process for dialog to allow user to select default DS
* @param[in] hWnd 
* @param[in] Message
* @param[in] wParam
* @param[in] lParam
* @return TRUE if message is handled
*/
BOOL CALLBACK SelectDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch(Message)
  {
    case WM_INITDIALOG:
    {
      HWND hListBox = ::GetDlgItem(hWnd, ID_LST_SOURCES);	
      if ( hListBox ) 
      {
        assert(g_pSelectDlgDSList);
        int       nIndex;

        for(int x = 0; x < g_pSelectDlgDSList->NumFiles; ++x)
        {
          pTW_IDENTITY ptmpIdent = &g_pSelectDlgDSList->DSInfo[x].Identity;

          nIndex = (int)SendMessage( hListBox, LB_ADDSTRING, (WPARAM)NULL, (LPARAM)ptmpIdent->ProductName);
          if(LB_ERR == nIndex)
          {
            break;
          }
          nIndex = (int)SendMessage( hListBox, LB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)ptmpIdent->Id);
          if(LB_ERR == nIndex)
          {
            break;
          }
        }
        if(g_pSelectDlgDSList->NumFiles == 0)
        {
          HWND hOK= ::GetDlgItem(hWnd, IDOK);
          EnableWindow(hOK, FALSE);
        }
        else if(g_pSelectDlgSourceID)
        {
          nIndex = (int)SendMessage( hListBox, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)g_pSelectDlgSourceID->ProductName);
          if(LB_ERR == nIndex)
          {
            nIndex = 0;
          }
          nIndex = (int)SendMessage( hListBox, LB_SETCURSEL, (WPARAM)nIndex, (LPARAM)NULL);
        }
      }
      return TRUE;
    }
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
          {
            HWND hListBox = ::GetDlgItem(hWnd, ID_LST_SOURCES);	
            int  nIndex   = 0;
            if ( hListBox ) 
            {
              nIndex = (int)SendMessage( hListBox, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
              if(LB_ERR == nIndex)
              {
                // if there is no selection should not have OK available to press in the first place.
                return TRUE;
              }
              nIndex = (int)SendMessage( hListBox, LB_GETITEMDATA, (WPARAM)nIndex, (LPARAM)0);
              if(LB_ERR != nIndex)
              {
                assert(nIndex <= g_pSelectDlgDSList->NumFiles);
                g_pSelectDlgSourceID = &g_pSelectDlgDSList->DSInfo[kIN(nIndex)].Identity;
              }
            }
            EndDialog(hWnd, IDOK);
            return TRUE;
          }
        case IDCANCEL:
          EndDialog(hWnd, IDCANCEL);
          return TRUE;
      }
      break;
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_SelectDS(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16    result = TWRC_SUCCESS;
  TW_IDENTITY SourceIdentity;

  SourceIdentity = *_pSourceIdentity;
  if(SourceIdentity.Id != 0)
  {
    // Even Twacker fails to set Id to 0 if it wants to set a particular DS
    kLOGERR << "MSG_USERSELECT failed to set Identity Id to 0" << endl;
    SourceIdentity.Id = 0;
    //setConditionCode(_pAppIdentity, TWCC_BUMMER);
    //result = TWRC_FAILURE;
    //return result;
  }

  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO   pAppInfo        = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];
    
  // If passed in a DS name we want to select it
  if(SourceIdentity.ProductName[0] != 0)
  {
    result = GetDSFromProductName(_pAppIdentity, &SourceIdentity);
    // If no match continue anyway.
  }

  // If not passed a DS or the name was not currently found
  // then selete the default
  if(SourceIdentity.Id == 0)
  {
    result = GetMatchingDefault(_pAppIdentity, &SourceIdentity);
    // If no match continue anyway.
  }

	// create	the	dialog window	
  g_pSelectDlgSourceID  = &SourceIdentity;
  g_pSelectDlgDSList    = pAppInfo->pDSList;
  int ret = DialogBox(g_hinstDLL, (LPCTSTR)IDD_DLG_SOURCE, (HWND)NULL, SelectDlgProc);
  if(ret == IDOK)
  {
    assert(g_pSelectDlgSourceID);
    *_pSourceIdentity   = *g_pSelectDlgSourceID;

    // save default source to Registry  
#ifdef _WIN32
  HKEY hKey;
  long status = ERROR_SUCCESS;
  char *szPath = pAppInfo->pDSList->DSInfo[kIN(g_pSelectDlgSourceID->Id)].szPath;

  // Open the key, creating it if it doesn't exist.
  if( RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Twain",
           NULL, NULL, NULL, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS )
  {
    status = RegSetValueEx( hKey, "Default Source", 0, REG_SZ, (LPBYTE)szPath, (DWORD)strlen((char*)szPath)+1 );
    if( status != ERROR_SUCCESS )
    {
      // Failed to save default DS to registry
      kLOGERR << "Failed to save default DS to registry" << endl;
      // Nothing preventing us from using the default right now
      setConditionCode(_pAppIdentity, TWCC_BUMMER);
    }
  }
  // Close the key.
  RegCloseKey(hKey);
#else
  // TODO: other OS save the default Source from stored location
  assert(0);
#endif

  assert(_pSourceIdentity->Id <= pAppInfo->pDSList->NumFiles);
    //pAppInfo->DefaultSource = (TW_INT16)_pSourceIdentity->Id;
  }
  else if(ret == IDCANCEL)
  {
    result = TWRC_CANCEL;
  }
  else if(ret == -1)
  {
    DWORD dwError = GetLastError();
    MessageBox(NULL, "Dialog failed!", "Error", MB_OK | MB_ICONINFORMATION);
    setConditionCode(_pAppIdentity, TWCC_BUMMER);
    result = TWRC_FAILURE;
  }

  return result;
}


//////////////////////////////////////////////////////////////////////////////
TW_INT16 EntryDS(pTW_IDENTITY _pAppIdentity,
                 pTW_IDENTITY _pSourceIdentity,
                 TW_UINT32 _DG,
                 TW_UINT16 _DAT,
                 TW_UINT16 _MSG,
                 TW_MEMREF _pData)
{
  TW_UINT16 result = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];
  assert(_pSourceIdentity && _pSourceIdentity->Id <= MAX_NUM_DS);
  pDS_INFO  pDSInfo  = &pAppInfo->pDSList->DSInfo[kIN(_pSourceIdentity->Id)];

  if(0 != pDSInfo->DS_Entry)
  {
    result = (pDSInfo->DS_Entry)(_pAppIdentity, _DG, _DAT, _MSG, _pData);
  }
  else
  {
    setConditionCode(_pAppIdentity, TWCC_OPERATIONERROR);
    result = TWRC_FAILURE;
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 GetDSFromProductName(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 ret = TWRC_FAILURE;

  if(0 == _pAppIdentity || 0 == _pSourceIdentity)
  {
    return ret;
  }

  if(0 == _pSourceIdentity->ProductName[0])
  {
    return ret;
  }
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &(_gApplications.AppInfo[kIN(_pAppIdentity->Id)]);

  for(int x = 0; x < pAppInfo->pDSList->NumFiles; ++x)
  {
    if(0 == strncmp(_pSourceIdentity->ProductName, pAppInfo->pDSList->DSInfo[x].Identity.ProductName, sizeof(TW_STR32)))
    {
      // match found, set index
      *_pSourceIdentity = pAppInfo->pDSList->DSInfo[x].Identity;
      ret = TWRC_SUCCESS;
      break;
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 LoadAllDataSources(pTW_IDENTITY _pAppIdentity, pDS_LIST _pList)
{
  TW_INT16 ret = TWRC_SUCCESS;

  // recursively navigate the TWAIN datasource dir looking for data sources.
  if(scanDSDir(kTWAIN_DS_DIR, _pAppIdentity, _pList) == EXIT_FAILURE)
  {
    ret = TWRC_FAILURE;
  }
  else
  {
    // mark the list as initialized now
    _pList->Initialized = TRUE;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
int scanDSDir(char* _szAbsPath, pTW_IDENTITY _pAppIdentity, pDS_LIST _pList)
{
#ifdef _WIN32
  WIN32_FIND_DATA   FileData;             // Data structure describes the file found
  HANDLE            hSearch;              // Search handle returned by FindFirstFile
  char              szABSFilename[PATH_MAX];
  BOOL              bFinished = FALSE;
  char              szPrevWorkDir[PATH_MAX];

  // Start searching for .ds files in the root directory.
  strcpy(szABSFilename, _szAbsPath);
  strcat(szABSFilename, "\\*.ds");
  hSearch = FindFirstFile(szABSFilename, &FileData);
  if (hSearch == INVALID_HANDLE_VALUE)
  {
    return EXIT_FAILURE;
  }

  /* Save the current working directory: */
  _getcwd( szPrevWorkDir, PATH_MAX );
  _chdir( _szAbsPath );

  while (!bFinished)
  {
    if(SNPRINTF(szABSFilename, PATH_MAX, "%s\\%s", _szAbsPath, FileData.cFileName) > 0)
    {
      if(TWRC_SUCCESS == LoadDS(_pAppIdentity, szABSFilename, kOUT(_pList->NumFiles), _pList))
      {
        _pList->NumFiles++;
      }
    }
  
    if (!FindNextFile(hSearch, &FileData))
    {
      bFinished = TRUE;
    }
  }
  
  if (!FindClose (hSearch))
  {
    _chdir( szPrevWorkDir );
    return EXIT_FAILURE;
  }

  // Start searching sub directories.
  strcpy(szABSFilename, _szAbsPath);
  strcat(szABSFilename, "\\*.*");
  hSearch = FindFirstFile(szABSFilename, &FileData);
  bFinished = FALSE;
  if (hSearch == INVALID_HANDLE_VALUE)
  {
    _chdir( szPrevWorkDir );
    return EXIT_FAILURE;
  }
  while (!bFinished)
  {
    if((strcmp(".", FileData.cFileName) != 0) &&
       (strcmp("..", FileData.cFileName) != 0) && 
       FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if(SNPRINTF(szABSFilename, PATH_MAX, "%s\\%s", _szAbsPath, FileData.cFileName) > 0)
      {
        scanDSDir(szABSFilename, _pAppIdentity, _pList);
      }
    }
  
    if (!FindNextFile(hSearch, &FileData))
    {
      bFinished = TRUE;
    }
  }
  
  _chdir( szPrevWorkDir );

  if (!FindClose (hSearch))
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

#else
  char szABSFilename[PATH_MAX];

  DIR *pdir; 
  if ((pdir=opendir(_szAbsPath)) == 0)
  {
    perror("opendir");
    return EXIT_FAILURE;
  }

  struct dirent *pfile; 
  while(errno=0, ((pfile=readdir(pdir)) != 0))
  { 
    if((strcmp(".", pfile->d_name) == 0) ||
       (strcmp("..", pfile->d_name) == 0))
    {
      continue;
    }

    if(SNPRINTF(szABSFilename, PATH_MAX, "%s/%s", _szAbsPath, pfile->d_name) < 0)
    {
      continue;
    }

    struct stat st;
    if(lstat(szABSFilename, &st) < 0)
    {
      perror("lstat");
      continue;
    }

    if(S_ISDIR(st.st_mode))
    {
      scanDSDir(szABSFilename, _pAppIdentity, _pList);
    }
    else if(S_ISREG(st.st_mode) && (0 != strstr(pfile->d_name, ".ds")))
    {
      if(TWRC_SUCCESS == LoadDS(_pAppIdentity, szABSFilename, kOUT(_pList->NumFiles), _pList))
      {
        _pList->NumFiles++;
      }
    }
  } 
  
  if(0 != errno)
  {
    perror("readdir");
  }

  closedir(pdir); 
  return EXIT_SUCCESS;
#endif
  return 0;
}


//////////////////////////////////////////////////////////////////////////////
TW_INT16 LoadDS(pTW_IDENTITY _pAppIdentity,
                char* _pPath,
                TW_INT16 _index,
                pDS_LIST _pList)
{
  TW_INT16 result = TWRC_SUCCESS;

  if(0 != _pPath && _index < MAX_NUM_DS)
  {
    if((_pList->DSInfo[kIN(_index)].pHandle = LOADLIBRARY(_pPath)) == 0)
    {
#ifndef _WIN32
      kLOGERR << "dlopen: " << dlerror() << endl;
#else
      kLOGERR << "Could not load library: " << _pPath << endl;
#endif //_WIN32
      result = TWRC_FAILURE;
    }
    else
    {
      if((_pList->DSInfo[kIN(_index)].DS_Entry = (DSENTRYPROC)LOADFUNCTION(_pList->DSInfo[kIN(_index)].pHandle, "DS_Entry")) == 0)
      {
#ifdef _WIN32 // dlsym returning NULL is not an error on Unix
        kLOGERR << "Could not find DSM_Entry function in DS: " << _pPath << endl;
        UNLOADLIBRARY(_pList->DSInfo[kIN(_index)].pHandle);
        _pList->DSInfo[kIN(_index)].pHandle = NULL;
        return TWRC_FAILURE; 
#endif //_WIN32
      }
#ifndef _WIN32
      char *error;
      if ((error = dlerror()) != 0)
      {
        kLOGERR << "dlsym: " << error << endl;
        return TWRC_FAILURE;
      }
#endif //_WIN32

      if(0 != _pList->DSInfo[kIN(_index)].DS_Entry) // was dlsym successful?
      {
        kLOG << "Loaded library: " << _pPath << endl;
        _pList->DSInfo[kIN(_index)].Identity.Id = _index;
        // Get the source to fill in the identity structure
        // This operation should never fail on any DS
        result = _pList->DSInfo[kIN(_index)].DS_Entry(_pAppIdentity, DG_CONTROL, DAT_IDENTITY, MSG_GET, (TW_MEMREF) &(_pList->DSInfo[kIN(_index)].Identity) );

        // Check to see if it is a match to the the application
        if(TWRC_SUCCESS != result ||
           TRUE != SupportedMatch(_pAppIdentity, &(_pList->DSInfo[kIN(_index)].Identity)) )
        {
          UNLOADLIBRARY(_pList->DSInfo[kIN(_index)].pHandle);
          _pList->DSInfo[kIN(_index)].pHandle = NULL;
          _pList->DSInfo[kIN(_index)].DS_Entry = NULL;
          return TWRC_FAILURE;
        }
        else
        {
          _pList->DSInfo[kIN(_index)].bOpen = FALSE;
          // The DS should not modify the Id
          // eventhough the spec states that the id will not be assigned until DSM 
          // sends MSG_OPENDS to DS
          // assert( _pList->DSInfo[kIN(_index)].Identity.Id == _index );
          _pList->DSInfo[kIN(_index)].Identity.Id = _index;
          strncpy(_pList->DSInfo[kIN(_index)].szPath, _pPath, MAX_PATH);
        }
      }
      else // bad dlsym
      {
        result = TWRC_FAILURE;
      }
    }
  }
  else // bad path or too many DS's already open
  {
    result = TWRC_FAILURE;
    setConditionCode(_pAppIdentity, TWCC_OPERATIONERROR);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////
TW_BOOL SupportedMatch(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity)
{
  TW_BOOL bRet = FALSE;
  TW_UINT32 sourceSupports, appSupports;

  // Mask out DG_CONTROL -- must ignore it since everything supports it.
  appSupports    = _pAppIdentity->SupportedGroups    & 0x00fffffe;
  sourceSupports = _pSourceIdentity->SupportedGroups & 0x00fffffe;

  if((appSupports & sourceSupports) != 0)
  {
    bRet = TRUE;
  }

  return bRet;
}

TW_INT16 UnLoadAllDS(pTW_IDENTITY _pAppIdentity)
{
  TW_INT16 result = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO   pAppInfo        = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];
  pDS_INFO    pDSInfo         = NULL;

  for( int i =0; i < pAppInfo->pDSList->NumFiles; i++)
  {
    pDSInfo = &pAppInfo->pDSList->DSInfo[i];
    if(0 != pDSInfo->DS_Entry)
    {
      if( TRUE == pDSInfo->bOpen )
      {
        assert( pDSInfo->bOpen );
        // No DS should be open because no application has a DS selected
        // but will close any that are open in case anything has got out of sync.
        // If DS is open, then close the DS
        result = pDSInfo->DS_Entry(_pAppIdentity,
                DG_CONTROL,
                DAT_IDENTITY,
                MSG_CLOSEDS,
                (TW_MEMREF) &pDSInfo->Identity);
        pDSInfo->bOpen = FALSE;
      }
      if(NULL != pDSInfo->pHandle)
      {
        UNLOADLIBRARY(pDSInfo->pHandle);
      }
      pDSInfo->pHandle = NULL;
      pDSInfo->DS_Entry = NULL;
    }
  }

  return(TWRC_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_GetFirst(pTW_IDENTITY _pAppIdentity,
                      pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 ret = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  // reset which source the app is pointing to
  pAppInfo->OpenSource = 0;

  // get the apps first supported source
  if(pAppInfo->pDSList->NumFiles > 0)
  {
    *_pSourceIdentity = pAppInfo->pDSList->DSInfo[0].Identity; // copy the source info
  }
  else // app doesn't have any supported sources
  {
    setConditionCode(_pAppIdentity, TWCC_NODS);
    ret = TWRC_FAILURE;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_GetNext(pTW_IDENTITY _pAppIdentity,
                     pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 ret = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  TW_INT16 srcId = pAppInfo->OpenSource;

  // Application must call MSG_GETFIRST before making this call
  if(-1 == srcId)
  {
    return TWRC_ENDOFLIST;
  }

  // have we reached the end of the supported sources?
  if((srcId + 1) < pAppInfo->pDSList->NumFiles)
  {
    srcId++;
    pAppInfo->OpenSource = srcId;
    *_pSourceIdentity = pAppInfo->pDSList->DSInfo[srcId].Identity; // copy the source info
  }
  else
  {
    return TWRC_ENDOFLIST;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 GetMatchingDefault(pTW_IDENTITY _pAppIdentity, pTW_IDENTITY _pSourceIdentity)
{
  TW_INT16 ret = TWRC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  bool bMatchFnd = false;

  // is there something to match to?
  if(pAppInfo->pDSList->NumFiles <= 0)
  {
    setConditionCode(_pAppIdentity, TWCC_NODS);
    return TWRC_FAILURE;
  }

  if(0 != _pSourceIdentity->Id)
  {
    setConditionCode(_pAppIdentity, TWCC_OPERATIONERROR);
    return TWRC_FAILURE;
  }

  // In Windows the default Data Source is stored in the registry
  // as the path to that DS.  We will need to compair this to the other DS as a match.
  // read default source from Registry
  memset(_gDefaultDSPath, 0, sizeof(_gDefaultDSPath));
  memset(_gDefaultDSName, 0, sizeof(_gDefaultDSName));

#ifdef _WIN32
  HKEY hKey;
  if( RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Twain",
      0, KEY_READ, &hKey) == ERROR_SUCCESS )
  {
    // Look for the subkey "Default Source".
    DWORD DWtype = REG_SZ;
    DWORD DWsize = sizeof(_gDefaultDSPath);
    BOOL bRunAtStartup = ( RegQueryValueEx( hKey, "Default Source", NULL, &DWtype, (LPBYTE)_gDefaultDSPath, &DWsize) == ERROR_SUCCESS );

    // Close the registry key handle.
    RegCloseKey(hKey);
  }
#else
  // TODO other OS read the default Source from stored location
  assert(0);
#endif


  // If current default source is not a match find a new default source
  // that will match this app
  for(int x = 0; x < pAppInfo->pDSList->NumFiles; ++x)
  {
    pTW_IDENTITY ptmpIdent = &(pAppInfo->pDSList->DSInfo[x].Identity);

    //if(TRUE == SupportedMatch(_pAppIdentity, ptmpIdent))
    {
      // Mark the first match to use as default 
      if(!bMatchFnd)
      {
        *_pSourceIdentity = *ptmpIdent;
        bMatchFnd = true;
      }
      // If the system default is a match we will use it and stop looking.
      if(0 == strnicmp(_gDefaultDSPath, pAppInfo->pDSList->DSInfo[x].szPath, sizeof(pAppInfo->pDSList->DSInfo[x].szPath)))
      {
        *_pSourceIdentity = *ptmpIdent;
        bMatchFnd = true;
        break;
      }
    }
  }

  if(!bMatchFnd)
  {
    setConditionCode(_pAppIdentity, TWCC_NODS);
    return TWRC_FAILURE;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////////////
TW_INT16 AddApplication(pTW_IDENTITY _pAppIdentity)
{
  TW_INT16 ret = TWRC_FAILURE;
  assert(_pAppIdentity);
  int i = 0;

  //Go through the list and find an empty location
  // Already tested that there is enough room to fit
  for (i=0; i<MAX_NUM_APPS; i++)
  {
    if(_gApplications.AppInfo[i].pOrigin == NULL)
    {
      // The application ID is always +1 greater then the array index it resides in.
      // The kOUT and kIN definitions help manage this.
      _gApplications.NumApps++;

      _pAppIdentity->Id         = kOUT(i);
      pAPP_INFO pAppInfo        = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

      pAppInfo->pOrigin         = _pAppIdentity;
      pAppInfo->OpenSource      = -1;
      pAppInfo->pDSList         = new DS_LIST;

      memset(pAppInfo->pDSList, 0, sizeof(DS_LIST));
      
      ret = TWRC_SUCCESS;
      return ret;
      break;
    }
  }

  // Something went wrong, the array is full
  assert(i<MAX_NUM_APPS);
  setConditionCode(_pAppIdentity, TWCC_MAXCONNECTIONS);
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
pAPP_INFO findApplicationByName(pTW_STR32 _pName)
{
  pAPP_INFO pAppInfo = NULL;

  for(TW_INT16 x = 0; x < MAX_NUM_APPS; x++)
  {
    if(_gApplications.AppInfo[x].pOrigin)
    {
      if(0 == strncmp(_gApplications.AppInfo[x].pOrigin->ProductName, _pName, sizeof(TW_STR32)))
      {
        // app found!
        pAppInfo = &(_gApplications.AppInfo[x]);
        break;
      }
    }
  }

  return pAppInfo;
}

//////////////////////////////////////////////////////////////////////////////
TW_UINT16 getConditionCode(pTW_IDENTITY _pAppIdentity)
{
  TW_UINT16 retCC = TWCC_SUCCESS;
  assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
  pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];

  // If the application has not registered with the DSM yet, then act on the
  // global condition code instead of the application specific one.
  // This will only happen on very rare occasions where the condition code
  // could not be set because the calling application was ambiguous.
  if(0 == _pAppIdentity->Id)
  {
    retCC = _gConditionCode;
    _gConditionCode = TWCC_SUCCESS; // reset
  }
  else
  {
    retCC = pAppInfo->ConditionCode;
    pAppInfo->ConditionCode = TWCC_SUCCESS; // reset
  }

  return retCC;
}

//////////////////////////////////////////////////////////////////////////////
TW_BOOL setConditionCode(pTW_IDENTITY _pAppIdentity, TW_UINT16 _ConditionCode)
{
  if(0 == _pAppIdentity)
  {
    _gConditionCode = _ConditionCode;
  }
  else
  {
    // If the application has not registered with the DSM yet, then act on the
    // global condition code instead of the application specific one.
    // This will only happen on very rare occasions where the condition code
    // could not be set because the calling application was ambiguous.
    if(0 == _pAppIdentity->Id)
    {
      _gConditionCode = _ConditionCode;
    }
    else
    {
      assert(_pAppIdentity && _pAppIdentity->Id <= MAX_NUM_APPS);
      pAPP_INFO pAppInfo = &_gApplications.AppInfo[kIN(_pAppIdentity->Id)];
      pAppInfo->ConditionCode = _ConditionCode;
    }
  }

  if(_ConditionCode != TWCC_SUCCESS)
  {
    kLOG << " - Condition Code: " << StringFromCC(_ConditionCode).c_str();
  }

  return TRUE;
}

/** Limit to the number of times to report the same message in a row to the log file */
#define TRACECOUNTLIMIT 5
//////////////////////////////////////////////////////////////////////////////
bool printTripletsInfo(const TW_UINT32    _DG,
                       const TW_UINT16    _DAT,
                       const TW_UINT16    _MSG,
                       const TW_MEMREF    _pData)
{
  static TW_UINT32 LastDG = 0;
  static TW_UINT16 LastDat = 0;
  static TW_UINT16 LastMsg = 0;
  static int nCount = 0;
  bool bPrint = false;

  if(0 == gpLogFile || !gpLogFile->is_open()) 
  {
    return bPrint;
  }

  // too many of these messages to log
  if( DG_CONTROL == _DG && DAT_EVENT == _DAT )
  {
    return bPrint;
  }
    
  if(_DG == LastDG && _DAT == LastDat && _MSG == LastMsg)
  {
    if( !(DG_CONTROL == _DG && DAT_CAPABILITY == _DAT) )
    {
      nCount++;
    }
  }
  else
  {
    if(nCount > TRACECOUNTLIMIT)
    {
      TW_STR32 sCount;
      SNPRINTF(sCount, 32, "%d", nCount - TRACECOUNTLIMIT);

      kLOG << "(last message repeated " << sCount << " more times, but not printed.)" << endl;
    }
    nCount = 1;
  }

  if(nCount <= TRACECOUNTLIMIT)
  {
    if(DG_CONTROL == _DG && DAT_NULL == _DAT)
    {
      *gpLogFile << endl << "   ";
    }
    *gpLogFile << "DSM [" << gOurPid << "]: " << StringFromDG(_DG).c_str() << " / " << StringFromDat(_DAT).c_str() << " / " << StringFromMsg(_MSG).c_str() << " ";
    if(DG_CONTROL == _DG && DAT_CAPABILITY == _DAT && NULL != _pData)
    {
      pTW_CAPABILITY _pCap = (pTW_CAPABILITY)_pData;
      *gpLogFile << "/ " << StringFromCap(_pCap->Cap).c_str() << " ";
      //<< "/ " << StringFromContainer(_pCap->ConType, _pCap->ConType).c_str() << " ";
    }
    bPrint = true;
  }

  LastDG = _DG;
  LastDat = _DAT;
  LastMsg = _MSG;

  return bPrint;
}

void printReturnCode( const TW_UINT16 rc )
{
  if(0 == gpLogFile || !gpLogFile->is_open()) 
  {
    return;
  }

  *gpLogFile << "= " << StringFromRC(rc).c_str() << endl;
}


//////////////////////////////////////////////////////////////////////////////
TW_INT16 DSM_Null(pTW_IDENTITY _pOrigin,
                  pTW_IDENTITY _pDest,
                  TW_UINT16    _MSG)
{
  TW_INT16 twrc = TWRC_SUCCESS;

  if(0 == _pOrigin)
  {
    kLOG << "Invalid source pointer" << endl;
    return TWRC_FAILURE;
  }

  if(0 == _pDest)
  {
    kLOG << "Invalid destination pointer" << endl;
    return TWRC_FAILURE;
  }

  // Invoke the applications callback to send this message along.
  if( MSG_DEVICEEVENT != _MSG &&
      MSG_CLOSEDSREQ  != _MSG &&
      MSG_XFERREADY   != _MSG)
  {
    setConditionCode(_pOrigin, TWCC_BADPROTOCOL);
    return TWRC_FAILURE;
  }

  pAPP_INFO pAppInfo = &(_gApplications.AppInfo[kIN(_pDest->Id)]);
  pTW_CALLBACK pCallback = &(pAppInfo->Callback.callback);
  if( 0 != pCallback->CallBackProc)
  {
    ((DSMENTRYPROC)(pCallback->CallBackProc))(
        _pOrigin,            // pTW_IDENTITY pOrigin
        _pDest,              // pTW_IDENTITY pDest
        DG_CONTROL,          // TW_UINT32    DG
        DAT_CALLBACK,        // TW_UINT16    DAT
        _MSG,                // TW_UINT16    MSG
        0);                  // TW_MEMREF    pData
  }
  else
  {
    // Application has not registered a callback. As a result, the msg will
    // be sent to the app the next time it forwards an event.
    // @todo I don't like how we only store the last message for the app. If
    // multiple DS's make a callback to a single app, we are going to lose MSG's
    pAppInfo->Callback.bCallbackPending = TRUE;
    pAppInfo->Callback.callback.Message = _MSG;
  }
  return twrc;
}

string StringFromDG(const TW_UINT32 _DG)
{
  switch(_DG)
  {
    case DG_CONTROL:
      return "DG_CONTROL";
      break;

    case DG_IMAGE:
      return "DG_IMAGE";
      break;

    case DG_AUDIO:
      return "DG_AUDIO";
      break;
  }

  TW_STR32 hex;
  SNPRINTF(hex, 32, "0x%04lx", _DG);

  return hex;
}

string StringFromDat(const TW_UINT16 _DAT)
{
  switch(_DAT)
  {
    case DAT_NULL:
      return "DAT_NULL";
      break;

    case DAT_CUSTOMBASE:
      return "DAT_CUSTOMBASE";
      break;

    case DAT_CAPABILITY:
      return "DAT_CAPABILITY";
      break;

    case DAT_EVENT:
      return "DAT_EVENT";
      break;

    case DAT_IDENTITY:
      return "DAT_IDENTITY";
      break;

    case DAT_PARENT:
      return "DAT_PARENT";
      break;

    case DAT_PENDINGXFERS:
      return "DAT_PENDINGXFERS";
      break;

    case DAT_SETUPMEMXFER:
      return "DAT_SETUPMEMXFER";
      break;

    case DAT_SETUPFILEXFER:
      return "DAT_SETUPFILEXFER";
      break;

    case DAT_STATUS:
      return "DAT_STATUS";
      break;

    case DAT_USERINTERFACE:
      return "DAT_USERINTERFACE";
      break;

    case DAT_XFERGROUP:
      return "DAT_XFERGROUP";
      break;

    case DAT_TWUNKIDENTITY:
      return "DAT_TWUNKIDENTITY";
      break;

    case DAT_CUSTOMDSDATA:
      return "DAT_CUSTOMDSDATA";
      break;

    case DAT_DEVICEEVENT:
      return "DAT_DEVICEEVENT";
      break;

    case DAT_FILESYSTEM:
      return "DAT_FILESYSTEM";
      break;

    case DAT_PASSTHRU:
      return "DAT_PASSTHRU";
      break;

    case DAT_CALLBACK:
      return "DAT_CALLBACK";
      break;

    case DAT_IMAGEINFO:
      return "DAT_IMAGEINFO";
      break;

    case DAT_IMAGELAYOUT:
      return "DAT_IMAGELAYOUT";
      break;

    case DAT_IMAGEMEMXFER:
      return "DAT_IMAGEMEMXFER";
      break;

    case DAT_IMAGENATIVEXFER:
      return "DAT_IMAGENATIVEXFER";
      break;

    case DAT_IMAGEFILEXFER:
      return "DAT_IMAGEFILEXFER";
      break;

    case DAT_CIECOLOR:
      return "DAT_CIECOLOR";
      break;

    case DAT_GRAYRESPONSE:
      return "DAT_GRAYRESPONSE";
      break;

    case DAT_RGBRESPONSE:
      return "DAT_RGBRESPONSE";
      break;

    case DAT_JPEGCOMPRESSION:
      return "DAT_JPEGCOMPRESSION";
      break;

    case DAT_PALETTE8:
      return "DAT_PALETTE8";
      break;

    case DAT_EXTIMAGEINFO:
      return "DAT_EXTIMAGEINFO";
      break;

    case DAT_AUDIOFILEXFER:
      return "DAT_AUDIOFILEXFER";
      break;

    case DAT_AUDIOINFO:
      return "DAT_AUDIOINFO";
      break;

    case DAT_AUDIONATIVEXFER:
      return "DAT_AUDIONATIVEXFER";
      break;
  }

  TW_STR32 hex;
  SNPRINTF(hex, 32, "0x%04x", _DAT);

  return hex;
}

string StringFromMsg(const TW_UINT16 _MSG)
{
  switch(_MSG)
  {
    case MSG_NULL:
      return "MSG_NULL";
      break;
    case MSG_CUSTOMBASE:
      return "MSG_CUSTOMBASE";
      break;
    case MSG_GET:
      return "MSG_GET";
      break;
    case MSG_GETCURRENT:
      return "MSG_GETCURRENT";
      break;
    case MSG_GETDEFAULT:
      return "MSG_GETDEFAULT";
      break;
    case MSG_GETFIRST:
      return "MSG_GETFIRST";
      break;
    case MSG_GETNEXT:
      return "MSG_GETNEXT";
      break;
    case MSG_SET:
      return "MSG_SET";
      break;
    case MSG_RESET:
      return "MSG_RESET";
      break;
    case MSG_QUERYSUPPORT:
      return "MSG_QUERYSUPPORT";
      break;
    case MSG_XFERREADY:
      return "MSG_XFERREADY";
      break;
    case MSG_CLOSEDSREQ:
      return "MSG_CLOSEDSREQ";
      break;
    case MSG_CLOSEDSOK:
      return "MSG_CLOSEDSOK";
      break;
    case MSG_DEVICEEVENT:
      return "MSG_DEVICEEVENT";
      break;
    case MSG_CHECKSTATUS:
      return "MSG_CHECKSTATUS";
      break;
    case MSG_OPENDSM:
      return "MSG_OPENDSM";
      break;
    case MSG_CLOSEDSM:
      return "MSG_CLOSEDSM";
      break;
    case MSG_OPENDS:
      return "MSG_OPENDS";
      break;
    case MSG_CLOSEDS:
      return "MSG_CLOSEDS";
      break;
    case MSG_USERSELECT:
      return "MSG_USERSELECT";
      break;
    case MSG_DISABLEDS:
      return "MSG_DISABLEDS";
      break;
    case MSG_ENABLEDS:
      return "MSG_ENABLEDS";
      break;
    case MSG_ENABLEDSUIONLY:
      return "MSG_ENABLEDSUIONLY";
      break;
    case MSG_PROCESSEVENT:
      return "MSG_PROCESSEVENT";
      break;
    case MSG_ENDXFER:
      return "MSG_ENDXFER";
      break;
    case MSG_CHANGEDIRECTORY:
      return "MSG_CHANGEDIRECTORY";
      break;
    case MSG_CREATEDIRECTORY:
      return "MSG_CREATEDIRECTORY";
      break;
    case MSG_DELETE:
      return "MSG_DELETE";
      break;
    case MSG_FORMATMEDIA:
      return "MSG_FORMATMEDIA";
      break;
    case MSG_GETCLOSE:
      return "MSG_GETCLOSE";
      break;
    case MSG_GETFIRSTFILE:
      return "MSG_GETFIRSTFILE";
      break;
    case MSG_GETINFO:
      return "MSG_GETINFO";
      break;
    case MSG_GETNEXTFILE:
      return "MSG_GETNEXTFILE";
      break;
    case MSG_RENAME:
      return "MSG_RENAME";
      break;
    case MSG_PASSTHRU:
      return "MSG_PASSTHRU";
      break;
    case MSG_REGISTER_CALLBACK:
      return "MSG_REGISTER_CALLBACK";
      break;
  }

  TW_STR32 hex;
  SNPRINTF(hex, 32, "0x%04x", _MSG);

  return hex;
}


string StringFromCap(const TW_UINT16 _Cap)
{
  switch(_Cap)
  {
    case CAP_CUSTOMBASE:
      return "CAP_CUSTOMBASE";
      break;

    case CAP_XFERCOUNT:
      return "CAP_XFERCOUNT";
      break;

    case ICAP_COMPRESSION:
      return "ICAP_COMPRESSION";
      break;

    case ICAP_PIXELTYPE:
      return "ICAP_PIXELTYPE";
      break;

    case ICAP_UNITS:
      return "ICAP_UNITS";
      break;

    case ICAP_XFERMECH:
      return "ICAP_XFERMECH";
      break;

    case CAP_AUTHOR:
      return "CAP_AUTHOR";
      break;

    case CAP_CAPTION:
      return "CAP_CAPTION";
      break;

    case CAP_FEEDERENABLED:
      return "CAP_FEEDERENABLED";
      break;

    case CAP_FEEDERLOADED:
      return "CAP_FEEDERLOADED";
      break;

    case CAP_TIMEDATE:
      return "CAP_TIMEDATE";
      break;

    case CAP_SUPPORTEDCAPS:
      return "CAP_SUPPORTEDCAPS";
      break;

    case CAP_EXTENDEDCAPS:
      return "CAP_EXTENDEDCAPS";
      break;

    case CAP_AUTOFEED:
      return "CAP_AUTOFEED";
      break;

    case CAP_CLEARPAGE:
      return "CAP_CLEARPAGE";
      break;

    case CAP_FEEDPAGE:
      return "CAP_FEEDPAGE";
      break;

    case CAP_REWINDPAGE:
      return "CAP_REWINDPAGE";
      break;

    case CAP_INDICATORS:
      return "CAP_INDICATORS";
      break;

    case CAP_SUPPORTEDCAPSEXT:
      return "CAP_SUPPORTEDCAPSEXT";
      break;

    case CAP_PAPERDETECTABLE:
      return "CAP_PAPERDETECTABLE";
      break;

    case CAP_UICONTROLLABLE:
      return "CAP_UICONTROLLABLE";
      break;

    case CAP_DEVICEONLINE:
      return "CAP_DEVICEONLINE";
      break;

    case CAP_AUTOSCAN:
      return "CAP_AUTOSCAN";
      break;

    case CAP_THUMBNAILSENABLED:
      return "CAP_THUMBNAILSENABLED";
      break;

    case CAP_DUPLEX:
      return "CAP_DUPLEX";
      break;

    case CAP_DUPLEXENABLED:
      return "CAP_DUPLEXENABLED";
      break;

    case CAP_ENABLEDSUIONLY:
      return "CAP_ENABLEDSUIONLY";
      break;

    case CAP_CUSTOMDSDATA:
      return "CAP_CUSTOMDSDATA";
      break;

    case CAP_ENDORSER:
      return "CAP_ENDORSER";
      break;

    case CAP_JOBCONTROL:
      return "CAP_JOBCONTROL";
      break;

    case CAP_ALARMS:
      return "CAP_ALARMS";
      break;

    case CAP_ALARMVOLUME:
      return "CAP_ALARMVOLUME";
      break;

    case CAP_AUTOMATICCAPTURE:
      return "CAP_AUTOMATICCAPTURE";
      break;

    case CAP_TIMEBEFOREFIRSTCAPTURE:
      return "CAP_TIMEBEFOREFIRSTCAPTURE";
      break;

    case CAP_TIMEBETWEENCAPTURES:
      return "CAP_TIMEBETWEENCAPTURES";
      break;

    case CAP_CLEARBUFFERS:
      return "CAP_CLEARBUFFERS";
      break;

    case CAP_MAXBATCHBUFFERS:
      return "CAP_MAXBATCHBUFFERS";
      break;

    case CAP_DEVICETIMEDATE:
      return "CAP_DEVICETIMEDATE";
      break;

    case CAP_POWERSUPPLY:
      return "CAP_POWERSUPPLY";
      break;

    case CAP_CAMERAPREVIEWUI:
      return "CAP_CAMERAPREVIEWUI";
      break;

    case CAP_DEVICEEVENT:
      return "CAP_DEVICEEVENT";
      break;

    case CAP_SERIALNUMBER:
      return "CAP_SERIALNUMBER";
      break;

    case CAP_PRINTER:
      return "CAP_PRINTER";
      break;

    case CAP_PRINTERENABLED:
      return "CAP_PRINTERENABLED";
      break;

    case CAP_PRINTERINDEX:
      return "CAP_PRINTERINDEX";
      break;

    case CAP_PRINTERMODE:
      return "CAP_PRINTERMODE";
      break;

    case CAP_PRINTERSTRING:
      return "CAP_PRINTERSTRING";
      break;

    case CAP_PRINTERSUFFIX:
      return "CAP_PRINTERSUFFIX";
      break;

    case CAP_LANGUAGE:
      return "CAP_LANGUAGE";
      break;

    case CAP_FEEDERALIGNMENT:
      return "CAP_FEEDERALIGNMENT";
      break;

    case CAP_FEEDERORDER:
      return "CAP_FEEDERORDER";
      break;

    case CAP_REACQUIREALLOWED:
      return "CAP_REACQUIREALLOWED";
      break;

    case CAP_BATTERYMINUTES:
      return "CAP_BATTERYMINUTES";
      break;

    case CAP_BATTERYPERCENTAGE:
      return "CAP_BATTERYPERCENTAGE";
      break;

    case ICAP_AUTOBRIGHT:
      return "ICAP_AUTOBRIGHT";
      break;

    case ICAP_BRIGHTNESS:
      return "ICAP_BRIGHTNESS";
      break;

    case ICAP_CONTRAST:
      return "ICAP_CONTRAST";
      break;

    case ICAP_CUSTHALFTONE:
      return "ICAP_CUSTHALFTONE";
      break;

    case ICAP_EXPOSURETIME:
      return "ICAP_EXPOSURETIME";
      break;

    case ICAP_FILTER:
      return "ICAP_FILTER";
      break;

    case ICAP_FLASHUSED:
      return "ICAP_FLASHUSED";
      break;

    case ICAP_GAMMA:
      return "ICAP_GAMMA";
      break;

    case ICAP_HALFTONES:
      return "ICAP_HALFTONES";
      break;

    case ICAP_HIGHLIGHT:
      return "ICAP_HIGHLIGHT";
      break;

    case ICAP_IMAGEFILEFORMAT:
      return "ICAP_IMAGEFILEFORMAT";
      break;

    case ICAP_LAMPSTATE:
      return "ICAP_LAMPSTATE";
      break;

    case ICAP_LIGHTSOURCE:
      return "ICAP_LIGHTSOURCE";
      break;

    case ICAP_ORIENTATION:
      return "ICAP_ORIENTATION";
      break;

    case ICAP_PHYSICALWIDTH:
      return "ICAP_PHYSICALWIDTH";
      break;

    case ICAP_PHYSICALHEIGHT:
      return "ICAP_PHYSICALHEIGHT";
      break;

    case ICAP_SHADOW:
      return "ICAP_SHADOW";
      break;

    case ICAP_FRAMES:
      return "ICAP_FRAMES";
      break;

    case ICAP_XNATIVERESOLUTION:
      return "ICAP_XNATIVERESOLUTION";
      break;

    case ICAP_YNATIVERESOLUTION:
      return "ICAP_YNATIVERESOLUTION";
      break;

    case ICAP_XRESOLUTION:
      return "ICAP_XRESOLUTION";
      break;

    case ICAP_YRESOLUTION:
      return "ICAP_YRESOLUTION";
      break;

    case ICAP_MAXFRAMES:
      return "ICAP_MAXFRAMES";
      break;

    case ICAP_TILES:
      return "ICAP_TILES";
      break;

    case ICAP_BITORDER:
      return "ICAP_BITORDER";
      break;

    case ICAP_CCITTKFACTOR:
      return "ICAP_CCITTKFACTOR";
      break;

    case ICAP_LIGHTPATH:
      return "ICAP_LIGHTPATH";
      break;

    case ICAP_PIXELFLAVOR:
      return "ICAP_PIXELFLAVOR";
      break;

    case ICAP_PLANARCHUNKY:
      return "ICAP_PLANARCHUNKY";
      break;

    case ICAP_ROTATION:
      return "ICAP_ROTATION";
      break;

    case ICAP_SUPPORTEDSIZES:
      return "ICAP_SUPPORTEDSIZES";
      break;

    case ICAP_THRESHOLD:
      return "ICAP_THRESHOLD";
      break;

    case ICAP_XSCALING:
      return "ICAP_XSCALING";
      break;

    case ICAP_YSCALING:
      return "ICAP_YSCALING";
      break;

    case ICAP_BITORDERCODES:
      return "ICAP_BITORDERCODES";
      break;

    case ICAP_PIXELFLAVORCODES:
      return "ICAP_PIXELFLAVORCODES";
      break;

    case ICAP_JPEGPIXELTYPE:
      return "ICAP_JPEGPIXELTYPE";
      break;

    case ICAP_TIMEFILL:
      return "ICAP_TIMEFILL";
      break;

    case ICAP_BITDEPTH:
      return "ICAP_BITDEPTH";
      break;

    case ICAP_BITDEPTHREDUCTION:
      return "ICAP_BITDEPTHREDUCTION";
      break;

    case ICAP_UNDEFINEDIMAGESIZE:
      return "ICAP_UNDEFINEDIMAGESIZE";
      break;

    case ICAP_IMAGEDATASET:
      return "ICAP_IMAGEDATASET";
      break;

    case ICAP_EXTIMAGEINFO:
      return "ICAP_EXTIMAGEINFO";
      break;

    case ICAP_MINIMUMHEIGHT:
      return "ICAP_MINIMUMHEIGHT";
      break;

    case ICAP_MINIMUMWIDTH:
      return "ICAP_MINIMUMWIDTH";
      break;

    case ICAP_FLIPROTATION:
      return "ICAP_FLIPROTATION";
      break;

    case ICAP_BARCODEDETECTIONENABLED:
      return "ICAP_BARCODEDETECTIONENABLED";
      break;

    case ICAP_SUPPORTEDBARCODETYPES:
      return "ICAP_SUPPORTEDBARCODETYPES";
      break;

    case ICAP_BARCODEMAXSEARCHPRIORITIES:
      return "ICAP_BARCODEMAXSEARCHPRIORITIES";
      break;

    case ICAP_BARCODESEARCHPRIORITIES:
      return "ICAP_BARCODESEARCHPRIORITIES";
      break;

    case ICAP_BARCODESEARCHMODE:
      return "ICAP_BARCODESEARCHMODE";
      break;

    case ICAP_BARCODEMAXRETRIES:
      return "ICAP_BARCODEMAXRETRIES";
      break;

    case ICAP_BARCODETIMEOUT:
      return "ICAP_BARCODETIMEOUT";
      break;

    case ICAP_ZOOMFACTOR:
      return "ICAP_ZOOMFACTOR";
      break;

    case ICAP_PATCHCODEDETECTIONENABLED:
      return "ICAP_PATCHCODEDETECTIONENABLED";
      break;

    case ICAP_SUPPORTEDPATCHCODETYPES:
      return "ICAP_SUPPORTEDPATCHCODETYPES";
      break;

    case ICAP_PATCHCODEMAXSEARCHPRIORITIES:
      return "ICAP_PATCHCODEMAXSEARCHPRIORITIES";
      break;

    case ICAP_PATCHCODESEARCHPRIORITIES:
      return "ICAP_PATCHCODESEARCHPRIORITIES";
      break;

    case ICAP_PATCHCODESEARCHMODE:
      return "ICAP_PATCHCODESEARCHMODE";
      break;

    case ICAP_PATCHCODEMAXRETRIES:
      return "ICAP_PATCHCODEMAXRETRIES";
      break;

    case ICAP_PATCHCODETIMEOUT:
      return "ICAP_PATCHCODETIMEOUT";
      break;

    case ICAP_FLASHUSED2:
      return "ICAP_FLASHUSED2";
      break;

    case ICAP_IMAGEFILTER:
      return "ICAP_IMAGEFILTER";
      break;

    case ICAP_NOISEFILTER:
      return "ICAP_NOISEFILTER";
      break;

    case ICAP_OVERSCAN:
      return "ICAP_OVERSCAN";
      break;

    case ICAP_AUTOMATICBORDERDETECTION:
      return "ICAP_AUTOMATICBORDERDETECTION";
      break;

    case ICAP_AUTOMATICDESKEW:
      return "ICAP_AUTOMATICDESKEW";
      break;

    case ICAP_AUTOMATICROTATE:
      return "ICAP_AUTOMATICROTATE";
      break;

    case ICAP_JPEGQUALITY:
      return "ICAP_JPEGQUALITY";
      break;

    case ACAP_AUDIOFILEFORMAT:
      return "ACAP_AUDIOFILEFORMAT";
      break;

    case ACAP_XFERMECH:
      return "ACAP_XFERMECH";
      break;
  }

  TW_STR32 hex;
  SNPRINTF(hex, 32, "CAP 0x%04x", _Cap);

  return hex;
}

string StringFromRC(const TW_UINT16 rc)
{
  switch(rc)
  {
    case TWRC_SUCCESS:
      return "TWRC_SUCCESS";
      break;

    case TWRC_FAILURE:
      return "TWRC_FAILURE";
      break;

    case TWRC_CHECKSTATUS:
      return "TWRC_CHECKSTATUS";
      break;

    case TWRC_CANCEL:
      return "TWRC_CANCEL";
      break;

    case TWRC_DSEVENT:
      return "TWRC_DSEVENT";
      break;

    case TWRC_NOTDSEVENT:
      return "TWRC_NOTDSEVENT";
      break;

    case TWRC_XFERDONE:
      return "TWRC_XFERDONE";
      break;

    case TWRC_ENDOFLIST:
      return "TWRC_ENDOFLIST";
      break;

    case TWRC_INFONOTSUPPORTED:
      return "TWRC_INFONOTSUPPORTED";
      break;

    case TWRC_DATANOTAVAILABLE:
      return "TWRC_DATANOTAVAILABLE";
      break;
  }

  TW_STR32 hex;
  SNPRINTF(hex, 32, "TWRC 0x%04x", rc);

  return hex;
}

string StringFromCC(const TW_UINT16 cc)
{
  switch(cc)
  {
    case TWCC_SUCCESS:
      return "TWRC_SUCCESS";
      break;

    case TWCC_BUMMER:
      return "Failure due to unknown causes";
      break;

    case TWCC_LOWMEMORY:
      return "Not enough memory to perform operation";
      break;

    case TWCC_NODS:
      return "No Data Source";
      break;

    case TWCC_MAXCONNECTIONS:
      return "DS is connected to max possible applications";
      break;

    case TWCC_OPERATIONERROR:
      return "DS or DSM reported error, application shouldn't";
      break;

    case TWCC_BADCAP:
      return "Unknown capability";
      break;

    case TWCC_BADPROTOCOL:
      return "Unrecognized MSG DG DAT combination";
      break;

    case TWCC_BADVALUE:
      return "Data parameter out of range";
      break;

    case TWCC_SEQERROR:
      return "DG DAT MSG out of expected sequence";
      break;

    case TWCC_BADDEST:
      return "Unknown destination Application/Source in DSM_Entry";
      break;

    case TWCC_CAPUNSUPPORTED:
      return "Capability not supported by source";
      break;

    case TWCC_CAPBADOPERATION:
      return "Operation not supported by capability";
      break;

    case TWCC_CAPSEQERROR:
      return "Capability has dependancy on other capability";
      break;

    case TWCC_DENIED:
      return "File System operation is denied (file is protected)";
      break;

    case TWCC_FILEEXISTS:
      return "Operation failed because file already exists.";
      break;

    case TWCC_FILENOTFOUND:
      return "File not found";
      break;

    case TWCC_NOTEMPTY:
      return "Operation failed because directory is not empty";
      break;

    case TWCC_PAPERJAM:
      return "The feeder is jammed";
      break;

    case TWCC_PAPERDOUBLEFEED:
      return "The feeder detected multiple pages";
      break;

    case TWCC_FILEWRITEERROR:
      return "Error writing the file (meant for things like disk full conditions)";
      break;

    case TWCC_CHECKDEVICEONLINE:
      return "The device went offline prior to or during this operation";
      break;
  }

  TW_STR32 hex;
  SNPRINTF(hex, 32, "TWCC 0x%04x", cc);

  return hex;
}

