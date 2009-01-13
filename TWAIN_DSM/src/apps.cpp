/***************************************************************************
 * TWAIN Data Source Manager version 2.0 
 * Manages image acquisition data sources used by a machine. 
 * Copyright © 2007 TWAIN Working Group:  
 * Adobe Systems Incorporated,AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Contact the TWAIN Working Group by emailing the Technical Subcommittee at 
 * twainwg@twain.org or mailing us at 13090 Hwy 9, Suite 3, Boulder Creek, CA 95006.
 *
 ***************************************************************************/

/**
* @file apps.cpp
* Support the Application and Driver data for the Data Source Manager. 
* @author TWAIN Working Group
* @date March 2007
*/

#include "dsm.h"



/**
* Describes everything we need to know about the Data Source over
* the course of the session...
*/
typedef struct
{
  TW_IDENTITY   Identity;               /**< Identity info for data source */
  TW_HANDLE     pHandle;                /**< returned by LOADLIBRARY(...) */
  DSENTRYPROC   DS_Entry;               /**< function pointer to the DS_Entry function -- set by dlsym(...) */
  char          szPath[FILENAME_MAX];   /**< location of the DS */
  TW_CALLBACK   twcallback;             /**< callback structure */
  TW_BOOL       bCallbackPending;       /**< True if an application is old style and a callback was supposed to be made to it */
} DS_INFO;



/**
* Structure to hold a list of Data Sources.
*/
typedef struct
{
  TW_UINT16     NumFiles;            /**< Number of items in list */
  DS_INFO       DSInfo[MAX_NUM_DS];  /**< array of Data Sources */
} DS_LIST;



/**
* Structure to hold data about a connected application, we use
* DS_LIST so we don't have to allocate memory that we don't
* need, on the theory that few applications will load more than
* one driver at a time...
*/
typedef struct
{
  TW_IDENTITY  identity;         /**< the application's identity. */
  TW_INT16     ConditionCode;    /**< the apps condition code. */
  DSM_State    CurrentState;     /**< the current state of the DSM for this app. */
  DS_LIST     *pDSList;          /**< Each Application has a list of DS that it discovers each time the app opens the DSM. */
  HWND         hwnd;             /**< the window that will monitor for events on Windows */
} APP_INFO;



/**
* Impl Class to hold list of connected applications.
* In 32bit enviroments each application will connect to a seperate
* instance of DSM data but with this list it allows ONE application
* to connect several time, as long as it uses a different name with
* each connection.  I'm still not sure why you'd want to do that,
* but there it is.  This class is intended to hide the gory details
* of how we're storing the data, so an impl is used.
*/
class CTwnDsmAppsImpl
{
  public:

    /**
    * Our CTwnDsmAppsImpl constructor.
    */
    CTwnDsmAppsImpl()
    {
      memset(&pod,0,sizeof(pod));
    }

    /**
    * Scan for Data Sources.
    * Recursively navigate the TWAIN datasource dir looking for data sources.
    * Store all valid data sources in _pList upto a maximum of MAX_NUM_DS 
    * data sources.
    * @param[in] _szAbsPath starting directory to begin search.
    * @param[out] _pAppId the application requesting scan.
    * @return either EXIT_SUCCESS or EXIT_FAILURE.
    */
    int scanDSDir(char        *_szAbsPath,
                  TW_IDENTITY *_pAppId);

    /**
    * Translates the cc passed in into a string and returns it
    * @param[in] cc the TWAIN Condition Code to translate
    * @return a string that represents the cc
    */
    const char *StringFromCC(const TW_UINT16 cc);

    /**
    * Loads a DS from disk and adds it to a global list of DS's.
    * @param[in] _pAppId Origin of message
    * @param[in] _pPath The path to the library to open
    * @param[in] _DsId the source array index
    * @param[in] _boolKeepOpen if set to true keeps DS open after successful load
    * @return a valid TWRC_xxxx return code
    */
    TW_INT16 LoadDS(TW_IDENTITY *_pAppId,
                    char        *_pPath,
                    TW_UINT32    _DsId,
                    bool         _boolKeepOpen);

    /**
    * Set the condition code.
    * @param[in] _pAppId Origin of message
    * @param[in] _ConditionCode new code to remember
    */
    void AppSetConditionCode(TW_IDENTITY *_pAppId,
                             TW_UINT16    _ConditionCode);

  public:
    // If you add a class in future, declare it here and not in
    // the pod, or the memset in the constructor will ruin your
    // day...

    /** 
    * We use a pod (Pieces of Data) system because it help prevents us from
    * making dumb initialization mistakes.
    */
    struct _pod
    {
      APP_INFO    m_AppInfo[MAX_NUM_APPS];  /**< list of applications. */
      TW_UINT16   m_conditioncode;          /**< we use this if we have no apps. */
    } pod; /**< Pieces of data for CTwnDsmAppsImpl*/
};



/**
* The constructor, where we create our implementation object...
*/
CTwnDsmApps::CTwnDsmApps()
{
  m_ptwndsmappsimpl = new CTwnDsmAppsImpl;
  if (!m_ptwndsmappsimpl)
  {
    kLOG((kLOGERR,"new of CTwnDsmAppsImpl failed..."));
  }
}



/**
* The destructor, where we destroy our implementation object...
*/
CTwnDsmApps::~CTwnDsmApps()
{
  if (m_ptwndsmappsimpl)
  {
    delete m_ptwndsmappsimpl;
    m_ptwndsmappsimpl = 0;
  }
}



/**
* Add an application.
* Attempt to add an application to our list.  Truth be told we only expect
* an application to do this once, but for legacy's sake we support the
* ability to do it more than once, but the Application has to provide a
* unique ProductName in its Identity.  If an Application really has to
* support multiple drivers, then it's recommended that it do this through
* seperate processes, since their is no guarantee that any two drivers will
* operator correctly in the same process...
*/
TW_UINT16 CTwnDsmApps::AddApp(TW_IDENTITY *_pAppId,
                              TW_MEMREF    _MemRef)
{
  int ii;
  char szDsm[FILENAME_MAX];

  // Validate...
  if (_pAppId->ProductName[0] == 0)
  {
    kLOG((kLOGERR,"AppId.ProductName is empty"));
    AppSetConditionCode(0,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  kLOG((kLOGINFO,"Application: \"%0.32s\"", _pAppId->Manufacturer));
  kLOG((kLOGINFO,"             \"%0.32s\"", _pAppId->ProductFamily));
  kLOG((kLOGINFO,"             \"%0.32s\" version: %u.%u", _pAppId->ProductName, _pAppId->Version.MajorNum, _pAppId->Version.MinorNum));
  kLOG((kLOGINFO,"             TWAIN %u.%u", _pAppId->ProtocolMajor, _pAppId->ProtocolMinor));

  // Check to see if this app has already been opened, and
  // if so, treat it as a sequence error, because this app
  // is already open...
  for (ii = 1; ii < MAX_NUM_APPS; ii++)
  {
    if (!strncmp(m_ptwndsmappsimpl->pod.m_AppInfo[ii].identity.ProductName,_pAppId->ProductName,sizeof(TW_STR32)))
    {
      kLOG((kLOGERR,"A successful MSG_OPENDSM was already done for %s...",_pAppId->ProductName));
      AppSetConditionCode(0,TWCC_SEQERROR);
      return TWRC_FAILURE;
    }
  }

  //Go through the list and find an empty location
  // Already tested that there is enough room to fit
  for (ii=1; ii < MAX_NUM_APPS; ii++)
  {
    if (!m_ptwndsmappsimpl->pod.m_AppInfo[ii].identity.Id)
    {
      // The application ID is always +1 greater then the array index it resides in.
      // We just let the 0-index stay empty...
      _pAppId->Id = ii;
      _pAppId->SupportedGroups |= DF_DSM2;
      m_ptwndsmappsimpl->pod.m_AppInfo[ii].identity = *_pAppId;
      m_ptwndsmappsimpl->pod.m_AppInfo[ii].hwnd     = (HWND)(_MemRef?*(HWND*)_MemRef:0);
      m_ptwndsmappsimpl->pod.m_AppInfo[ii].pDSList  = (DS_LIST*)calloc(sizeof(DS_LIST)+1,1);
      if (!m_ptwndsmappsimpl->pod.m_AppInfo[ii].pDSList)
      {
        kLOG((kLOGERR,"calloc failed for %s...",_pAppId->ProductName));
        AppSetConditionCode(0,TWCC_LOWMEMORY);
        return TWRC_FAILURE;
      }
      break;
    }
  }

  // We've run out of room...
  if (ii >= MAX_NUM_APPS)
  {
    kLOG((kLOGERR,"We've hit the maximum number of connections..."));
    AppSetConditionCode(0,TWCC_MAXCONNECTIONS);
    return TWRC_FAILURE;
  }

  // Work out the full path to our drivers (if needed)...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    (void)::GetWindowsDirectory(szDsm,sizeof(szDsm));
    SSTRCAT(szDsm,sizeof(szDsm),"\\");
    SSTRCAT(szDsm,sizeof(szDsm),kTWAIN_DS_DIR);
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    SSTRCPY(szDsm,sizeof(szDsm),kTWAIN_DS_DIR);
  #else
    #error Sorry, we do not recognize this system...
  #endif

  // Recursively navigate the TWAIN datasource dir looking for data sources.
  // Ignor error continue with what we found even if it is nothing
  m_ptwndsmappsimpl->scanDSDir(szDsm,_pAppId);

  // Maybe one of many DS failed but we still found some.
  AppSetConditionCode(_pAppId, TWCC_SUCCESS);

  // Move DSM to state 3 for this app...
  m_ptwndsmappsimpl->pod.m_AppInfo[ii].CurrentState = dsmState_Open;

  // at this point we can safely add our flag to the caller's
  // application id, but don't bother to do it unless they put
  // their flag in there first...
  if (_pAppId->SupportedGroups & DF_APP2)
  {
    _pAppId->SupportedGroups |= DF_DSM2;
  }

  // All done...
  return TWRC_SUCCESS;
}



/**
* Remove an application.
* Attempt to add an application to our list.  Truth be told we only expect
* an application to do this once, but for legacy's sake we support the
* ability to do it more than once, but the Application has to provide a
* unique ProductName in its Identity.  If an Application really has to
* support multiple drivers, then it's recommended that it do this through
* seperate processes, since their is no guarantee that any two drivers will
* operator correctly in the same process...
*/
TW_UINT16 CTwnDsmApps::RemoveApp(TW_IDENTITY *_pAppId)
{
  int nIndex;
  DS_INFO *pDSInfo;
  TW_PENDINGXFERS twpendingxfers;
  TW_USERINTERFACE twuserinterface;

  // Validate...
  if (   (_pAppId->Id < 1)
      || (_pAppId->Id >= MAX_NUM_APPS))
  {
    kLOG((kLOGERR,"_id is out of range...%d",_pAppId->Id));
    AppSetConditionCode(0,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  // To close the DSM we must be open.  I don't really like this
  // piece of code.  To my way of thinking a close should always
  // succeed, even if there are mice in the DVD drive and the
  // monitor is on fire.  The notion of a failure message during
  // a close is annoying.  But there might be a good reason for
  // this I'm not aware of...
  if (dsmState_Open != m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].CurrentState)
  {
    AppSetConditionCode(0,TWCC_SEQERROR);
    return TWRC_FAILURE;
  }

  // Get rid of our list of drivers...
  if (m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList)
  {
    // Check all the driver slots, if we find an open slot, shotgun it
    // with the sequence of close out commands and shut it down.  This
    // really isn't something we should have to do, but it makes us
    // more robust...
    for (nIndex = 1;
         nIndex < m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles;
         nIndex++)
    {
      pDSInfo = &m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[nIndex];
      if (pDSInfo->DS_Entry)
      {
        kLOG((kLOGERR,"MSG_CLOSEDSM called with drivers still open."));
        kLOG((kLOGINFO,"The application should not be doing this."));
        kLOG((kLOGINFO,"The DSM is going to try to gracefully shutdown the drivers..."));

        memset(&twpendingxfers,0,sizeof(twpendingxfers));
        memset(&twuserinterface,0,sizeof(twuserinterface));

        pDSInfo->DS_Entry(_pAppId,DG_IMAGE,DAT_PENDINGXFERS,MSG_ENDXFER,(TW_MEMREF)&twpendingxfers);
        pDSInfo->DS_Entry(_pAppId,DG_IMAGE,DAT_PENDINGXFERS,MSG_RESET,(TW_MEMREF)&twpendingxfers);
        pDSInfo->DS_Entry(_pAppId,DG_CONTROL,DAT_USERINTERFACE,MSG_DISABLEDS,(TW_MEMREF)&twuserinterface);
        pDSInfo->DS_Entry(_pAppId,DG_CONTROL,DAT_IDENTITY,MSG_DISABLEDS,(TW_MEMREF)&pDSInfo->Identity);
        UnloadDS(_pAppId,nIndex);
      }
    }

    // Okay, we can blow away the memory now...
    free(m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList);
    m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList = NULL;
  }

  // Scrub this application's structure...
  memset(&m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].identity,
         0,
         sizeof(m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].identity));

  // Switch application's DSM state to state 2...
  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].CurrentState = dsmState_Loaded;

  // All done...
  return TWRC_SUCCESS;
}



/**
* Validate an application's identity.
* Make sure we're dealing with good data...
*/
TW_BOOL CTwnDsmApps::AppValidateId(TW_IDENTITY *_pAppId)
{
  if (!_pAppId)
  {
    kLOG((kLOGERR,"_pAppId is null..."));
    return false;
  }
  else if (_pAppId->Id >= MAX_NUM_APPS)
  {
    kLOG((kLOGERR,"invalid App ID...%d",_pAppId->Id));
    return false;
  }
  return true;
}


/**
* Validate an application's identity and its DS identity.
* Make sure we're dealing with good data...
*/
TW_BOOL CTwnDsmApps::AppValidateIds(TW_IDENTITY *_pAppId, TW_IDENTITY *_pDSId)
{
  if(!AppValidateId(_pAppId))
  {
    return false;
  }
  else
  {
    if (!_pDSId)
    {
      kLOG((kLOGERR,"_pDSId is null..."));
      return false;
    }
    else if (_pDSId->Id >= MAX_NUM_DS)
    {
      kLOG((kLOGERR,"invalid DS ID...%d",_pDSId->Id));
      return false;
    }
    else if (!m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList)
    {
      kLOG((kLOGERR,"List of DS for app is invalid"));
      return false;
    }
    else if (_pDSId->Id > m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles)
    {
      kLOG((kLOGERR,"The DS ID for app is not valid"));
      return false;
    }
  }
  return true;
}


/**
* Validate an application's identity.
* Make sure we're dealing with good data...
*/
TW_IDENTITY *CTwnDsmApps::AppGetIdentity(TW_IDENTITY *_pAppId)
{
  if (AppValidateId(_pAppId))
  {
    return &m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].identity;
  }
  kLOG((kLOGERR,"bad _pAppId..."));
  return NULL;
}



/**
* Return the condition code and clear it internally.
* Every open application maintains its own conditioncode, which
* is going to be used in state 3.  In state 4 and higher the
* condition code is coming from the driver.  In state 2 there is
* no condition code, so we have to use a value that is global to
* this instance of the DSM.  The code is cleared internally,
* per the specification...
*/
TW_UINT16 CTwnDsmApps::AppGetConditionCode(TW_IDENTITY *_pAppId)
{
  TW_UINT16 conditioncode;

  // Return the application specific value...
  if (AppValidateId(_pAppId))
  {
    conditioncode = m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].ConditionCode;
    m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].ConditionCode = TWCC_SUCCESS;
    m_ptwndsmappsimpl->pod.m_conditioncode = TWCC_SUCCESS;
    return conditioncode;
  }
  // Uh-oh, we have no app, so return the global value...
  else
  {
    conditioncode = m_ptwndsmappsimpl->pod.m_conditioncode;
    m_ptwndsmappsimpl->pod.m_conditioncode = TWCC_SUCCESS;
    return conditioncode;
  }
}



/*
* Set the condition code.
* The same rules apply here as they do for AppGetConditionCode.
* This is the interface function...
*/
void CTwnDsmApps::AppSetConditionCode(TW_IDENTITY *_pAppId,
                                      TW_UINT16    _ConditionCode)
{
  return m_ptwndsmappsimpl->AppSetConditionCode(_pAppId,_ConditionCode);
}



/**
* Set the condition code.
* The same rules apply here as they do for AppGetConditionCode.
* This is the implemenation function.
*/
void CTwnDsmAppsImpl::AppSetConditionCode(TW_IDENTITY *_pAppId,
                                          TW_UINT16    _ConditionCode)
{
  // We have no application identity to work with...
  if (   (0 == _pAppId)
      || (0 == _pAppId->Id)
      || (0 == pod.m_AppInfo[_pAppId->Id].identity.Id))
  {
    pod.m_conditioncode = _ConditionCode;
  }

  // This is where we normally expect to be...
  else
  {
    pod.m_AppInfo[_pAppId->Id].ConditionCode = _ConditionCode;
  }

  // Make a note of this in the log...
  if (_ConditionCode != TWCC_SUCCESS)
  {
    kLOG((kLOGINFO,"Condition Code: %s",StringFromCC(_ConditionCode)));
  }

  // All done...
  return;
}


DSM_State CTwnDsmApps::AppGetState()
{
  // Initialize to PreSession and update it if we find an application that is further along.
  DSM_State CurrentState = dsmState_PreSession;

  for (int AppID = 1; AppID<MAX_NUM_APPS; AppID++)
  {
    if(m_ptwndsmappsimpl->pod.m_AppInfo[AppID].CurrentState > CurrentState)
    {
      CurrentState = m_ptwndsmappsimpl->pod.m_AppInfo[AppID].CurrentState;
    }
  }
  return CurrentState;
}

/**
* Get our current state.
* There are really only two states that the DSM can occupy, state
* 2 where it's loaded, but hasn't had a MSG_OPENDSM done for the
* specified application identity.  And state 3, where a MSG_OPENDSM
* has been successfully performed...
*/
DSM_State CTwnDsmApps::AppGetState(TW_IDENTITY *_pAppId)
{
  // Return the application specific state...
  if (AppValidateId(_pAppId))
  {
    return m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].CurrentState;
  }
  // Otherwise we must be in state 2.  Good luck ever getting state 1
  // out of the DSM...  :)
  else
  {
    return dsmState_Loaded;
  }
}



/**
* Get our hwnd.
* Windows needs this to help center the user select window...
*/
void *CTwnDsmApps::AppHwnd(TW_IDENTITY *_pAppId)
{
  // Return the hwnd...
  if (AppValidateId(_pAppId))
  {
    return m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].hwnd;
  }
  // Otherwise return a null...
  else
  {
    return 0;
  }
}



/**
* Get the number of drivers found.
* When LoadDS() is called during AddApp() we browse for drivers and
* keep the identity for each one we find.  This just tells us how
* many we found...
*/
TW_UINT32 CTwnDsmApps::AppGetNumDs(TW_IDENTITY *_pAppId)
{
  // Return the number of drivers we found...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList)
  {
    return m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles;
  }
  // Not a lot of choice, the value has to be zero...
  else
  {
    return 0;
  }
}



/**
* Get the identity for the specified driver.
* When LoadDS() is called during AddApp() we browse for drivers and
* keep the identity for each one we find.  This just tells us how
* many we found...
*/
TW_IDENTITY *CTwnDsmApps::DsGetIdentity(TW_IDENTITY *_pAppId,
                                        TW_UINT32    _DsId)
{
  // Return a pointer to the driver's identity...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    return &m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].Identity;
  }
  // Something is toasted, so return NULL...
  else
  {
    kLOG((kLOGERR,"Returning NULL from DsGetIdentity..."));
    return NULL;
  }
}



/**
* Get the DS_Entry function for the specified driver.
* Every driver has to have one of these...
*/
DSENTRYPROC CTwnDsmApps::DsGetEntryProc(TW_IDENTITY *_pAppId,
                                        TW_UINT32    _DsId)
{
  // Return a pointer to the driver's identity...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    return m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].DS_Entry;
  }
  // Something is toasted, so return NULL...
  else
  {
    kLOG((kLOGERR,"Returning NULL from DsGetEntryProc..."));
    return NULL;
  }
}



/**
* Get the full path and filename for the specified driver.
* We use this to uniquely identify each driver, since the ProductName
* is not guaranteed to be unique, though it should be...
*/
char *CTwnDsmApps::DsGetPath(TW_IDENTITY *_pAppId,
                             TW_UINT32    _DsId)
{
  // Return a pointer to the driver's file path and name...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    return m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].szPath;
  }
  // Something is toasted, so return NULL...
  else
  {
    kLOG((kLOGERR,"Returning NULL from DsGetPath..."));
    return NULL;
  }
}



/**
* Get a point to the TW_CALLBACK for the specified driver.
* This is optional for drivers on Windows.  On Linux it's the only
* way to use DAT_NULL to let an Application know about messages
* going from the Driver to the Application
*/
TW_CALLBACK *CTwnDsmApps::DsCallbackGet(TW_IDENTITY *_pAppId,
                                        TW_UINT32    _DsId)
{
  // Return a pointer to the driver's TW_CALLBACK...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    return &m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].twcallback;
  }
  // Something is toasted, so return NULL...
  else
  {
    kLOG((kLOGERR,"Returning NULL from DsCallbackGet..."));
    return NULL;
  }
}



/**
* Check if a callback is waiting.
* This allows the DSM to help a driver get its message to an
* application...
*/
TW_BOOL CTwnDsmApps::DsCallbackIsWaiting(TW_IDENTITY *_pAppId,
                                         TW_UINT32    _DsId)
{
  // Check the waiting flag...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    return m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].bCallbackPending;
  }
  // Something is toasted, so return FALSE...
  else
  {
    kLOG((kLOGERR,"Returning FALSE from DsCallbackIsWaiting..."));
    return FALSE;
  }
}



/**
* Set the callback flag.
* This is how we know when we have something for the Application...
*/
void CTwnDsmApps::DsCallbackSetWaiting(TW_IDENTITY *_pAppId,
                                       TW_UINT32    _DsId,
                                       TW_BOOL      _Waiting)
{
  // Set the waiting flag...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].bCallbackPending = _Waiting;
  }
  // Something is toasted, so whine about it...
  else
  {
    kLOG((kLOGERR,"Unable to properly handle DsCallbackSetWaiting..."));
  }
}



/**
* Turn a TWCC_ condition code into a string...
*/
const char *CTwnDsmAppsImpl::StringFromCC(const TW_UINT16 cc)
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

  static TW_STR32 hex;
  SSNPRINTF(hex, NCHARS(hex), 32, "TWCC 0x%04x", cc);

  return hex;
}



/**
* Find all of the drivers.
* We recursively descend into the driver directory, looking for
* files with a .ds extension, opening them and getting their
* TW_IDENTITY.  Which is why it's critical that drivers do as
* little as possible during this operation.  It's easy to know
* when it's happening, because the application's TW_IDENTITY is
* empty, which is the hint that the DSM is browsing...
*/
int CTwnDsmAppsImpl::scanDSDir(char        *_szAbsPath,
                               TW_IDENTITY *_pAppId)
{
  // Validate...
  if (   !_szAbsPath
      || !_pAppId)
  {
    return EXIT_FAILURE;
  }

  //
  // Take care of VC++...
  //
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    WIN32_FIND_DATA   FileData;             // Data structure describes the file found
    HANDLE            hSearch;              // Search handle returned by FindFirstFile
    char              szABSFilename[FILENAME_MAX];
    BOOL              bFinished = FALSE;
    char              szPrevWorkDir[FILENAME_MAX];

    // Start searching for .ds files in the root directory.
    SSTRCPY(szABSFilename, NCHARS(szABSFilename), _szAbsPath);
    SSTRCAT(szABSFilename, NCHARS(szABSFilename), "\\*.ds");
    hSearch = FindFirstFile(szABSFilename,&FileData);

    // If we find something, squirrel it away and anything else we find...
    if (hSearch != INVALID_HANDLE_VALUE)
    {
      /* Save the current working directory: */
      char *szResult = _getcwd( szPrevWorkDir, sizeof(szPrevWorkDir) );
      if (szResult == (char*)NULL)
      {
        return EXIT_FAILURE;
      }
      int iResult = _chdir(_szAbsPath);
      if (iResult != 0)
      {
        return EXIT_FAILURE;
      }
    
      while (!bFinished)
      {
        if (SSNPRINTF(szABSFilename, NCHARS(szABSFilename), FILENAME_MAX, "%s\\%s", _szAbsPath, FileData.cFileName) > 0)
        {
          if (TWRC_SUCCESS == LoadDS(_pAppId,
                                     szABSFilename,
                                     pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles+1,
                                     false))
          {
            pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles++;
          }
        }
      
        if (!FindNextFile(hSearch, &FileData))
        {
          bFinished = TRUE;
        }
      }
    
      if (!FindClose (hSearch))
      {
        (void)_chdir( szPrevWorkDir );
        return EXIT_FAILURE;
      }
    }

    // Start searching sub directories.
    SSTRCPY(szABSFilename, NCHARS(szABSFilename), _szAbsPath);
    SSTRCAT(szABSFilename, NCHARS(szABSFilename), "\\*.*");
    hSearch = FindFirstFile(szABSFilename, &FileData);
    bFinished = FALSE;
    if (hSearch == INVALID_HANDLE_VALUE)
    {
      (void)_chdir( szPrevWorkDir );
      return EXIT_FAILURE;
    }
    while (!bFinished)
    {
      if (   (strcmp(".", FileData.cFileName) != 0)
          && (strcmp("..", FileData.cFileName) != 0)
          && (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
      {
        if (SSNPRINTF(szABSFilename, NCHARS(szABSFilename), FILENAME_MAX, "%s\\%s", _szAbsPath, FileData.cFileName) > 0)
        {
          scanDSDir(szABSFilename,_pAppId);
        }
      }
  
      if (!FindNextFile(hSearch, &FileData))
      {
        bFinished = TRUE;
      }
    }
  
    (void)_chdir( szPrevWorkDir );

    if (!FindClose (hSearch))
    {
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    
    
  //
  // Take care of g++...
  //
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    char szABSFilename[FILENAME_MAX];

    DIR *pdir;
    if ((pdir=opendir(_szAbsPath)) == 0)
    {
      perror("opendir");
      return EXIT_FAILURE;
    }

    struct dirent *pfile; 
    while(errno=0, ((pfile=readdir(pdir)) != 0))
    { 
      if ( (strcmp(".", pfile->d_name) == 0)
       || (strcmp("..", pfile->d_name) == 0) )
      {
        continue;
      }

      if (SNPRINTF(szABSFilename,FILENAME_MAX,"%s/%s",_szAbsPath,pfile->d_name) < 0)
      {
        continue;
      }

      struct stat st;
      if (lstat(szABSFilename, &st) < 0)
      {
        perror("lstat");
        continue;
      }

      if (S_ISDIR(st.st_mode))
      {
        scanDSDir(szABSFilename,_pAppId);
      }
      else if (S_ISREG(st.st_mode) && (0 != strstr(pfile->d_name, ".ds")))
      {
        if (TWRC_SUCCESS == LoadDS(_pAppId,
                                   szABSFilename,
                                   pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles+1,
                                   false))
        {
          pod.m_AppInfo[_pAppId->Id].pDSList->NumFiles++;
        }
      }
    } 
  
    if (0 != errno)
    {
      perror("readdir");
    }

    closedir(pdir); 
    return EXIT_SUCCESS;



  //
  // meh!
  //
  #else
    #error Sorry, we do not recognize this system...
  #endif
}



/**
* Load a specific driver.
* This is the interface function that's called by CTwnDsm
*/
TW_INT16 CTwnDsmApps::LoadDS(TW_IDENTITY  *_pAppId,
                             TW_UINT32     _DsId)
{
  // Load the specified driver...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS))
  {
    #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
      // Make the DS directory the current directoy while we load the DS so that any DLLs that
      // are loaded with the DS can be found.
      char      szPrevWorkDir[FILENAME_MAX];
      char      szWorkDir[FILENAME_MAX];

      SSTRCPY(szWorkDir, NCHARS(szWorkDir), m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].szPath);
      // strip filename from path
      size_t x = strlen(szWorkDir);
      while(x > 0)
      {
        if(PATH_SEPERATOR == szWorkDir[x-1])
        {
          szWorkDir[x-1] = 0;
          break;
        }
        --x;
      }
      /* Save the current working directory: */
      memset( szPrevWorkDir, 0, NCHARS(szPrevWorkDir) );
      _getcwd( szPrevWorkDir, NCHARS(szPrevWorkDir) );
      (void)_chdir( szWorkDir );
    #endif

    TW_INT16 result = m_ptwndsmappsimpl->LoadDS(_pAppId,
                                     m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].szPath,
                                     _DsId,
                                     true);
    #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
      if(0!=strlen(szPrevWorkDir))
      {
        (void)_chdir( szPrevWorkDir );
      }
    #endif
    return result;
  }
  // Something is toasted, so return LoadDS...
  else
  {
    kLOG((kLOGERR,"Returning TWRC_FAILURE from LoadDS..."));
    return TWRC_FAILURE;
  }
}



/**
* Load a driver.
* This is the implementation function.  We use this both to browse
* for drivers during MSG_GETFIRST/MSG_GETNEXT and to load a specific
* driver during MSG_OPENDS.  Which is why we need the path and the
* keep open flag...
*/
TW_INT16 CTwnDsmAppsImpl::LoadDS(TW_IDENTITY *_pAppId,
                                 char        *_pPath,
                                 TW_UINT32    _DsId,
                                 bool         _boolKeepOpen)
{
  TW_INT16  result = TWRC_SUCCESS;
  DS_INFO  *pDSInfo;
  bool hook;

  // Validate...
  if (   (0 == _pPath)
      || (_DsId >= MAX_NUM_DS))
  {
    // bad path or too many DS's already open
    AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    return TWRC_FAILURE;
  }

  // Initialize stuff...
  pDSInfo = &pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId];

  // Only log DS details when processing a MSG_OPENDS message
  if(_boolKeepOpen)
  {
    kLOG((kLOGINFO,"Datasource: \"%0.32s\"", pDSInfo->Identity.Manufacturer));
    kLOG((kLOGINFO,"            \"%0.32s\"", pDSInfo->Identity.ProductFamily));
    kLOG((kLOGINFO,"            \"%0.32s\" version: %u.%u", pDSInfo->Identity.ProductName, pDSInfo->Identity.Version.MajorNum, pDSInfo->Identity.Version.MinorNum));
    kLOG((kLOGINFO,"            TWAIN %u.%u", pDSInfo->Identity.ProtocolMajor, pDSInfo->Identity.ProtocolMinor));
  }

  // Only hook this driver if we've been asked to keep the driver
  // open (meaning we're processing a MSG_OPENDS) and if we see
  // that the driver is 1.x...(by checking the absence of DF_DS2)
  hook = _boolKeepOpen && !(pDSInfo->Identity.SupportedGroups & DF_DS2);

  // Try to load the driver...  We load the driver again if we are keeping
  // it open.  This LoadLibrary is always closed so we dont hook this time.
  pDSInfo->pHandle = LOADLIBRARY(_pPath,false);
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    if (0 == pDSInfo->pHandle)
    {
      kLOG((kLOGERR,"Could not load library: %s",_pPath));
      AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
      return TWRC_FAILURE;
    }
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    if (0 == pDSInfo->pHandle)
    {
      // This is a bit skanky, and not the sort of thing I really want
      // a user to have to see, but more info is better than less, so
      // hopefully someone will be able to sort out what the cryptic
      // message means and we can FAQ it...
      fprintf(stderr,">>> error loading <%s>\r\n",_pPath);
      fprintf(stderr,">>> %s\r\n",dlerror());
      fprintf(stderr,">>> please contact your scanner or driver vendor for more\r\n");
      fprintf(stderr,">>> help, if that doesn't help then check out the FAQ at\r\n");
      fprintf(stderr,">>> http://www.twain.org\r\n");
      kLOG((kLOGERR,"Could not load library: %s",_pPath));
      kLOG((kLOGERR,dlerror()));
      AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
      return TWRC_FAILURE;
    }
  #else
    #error Sorry, we do not recognize this system...
  #endif

  // Try to get the entry point...
  pDSInfo->DS_Entry = (DSENTRYPROC)DSM_LoadFunction(pDSInfo->pHandle,"DS_Entry");

  if (pDSInfo->DS_Entry == 0)
  {
    #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
      // The WIATwain.ds does not have an entry point 
      if(0 == strstr(_pPath, "wiatwain.ds"))
      {
        kLOG((kLOGERR,"Could not find DS_Entry function in DS: %s",_pPath));
      }
      else
      {
        kLOG((kLOGINFO,"Could not find DS_Entry function in DS: %s",_pPath));
      }
    #else
      kLOG((kLOGERR,"Could not find DS_Entry function in DS: %s",_pPath));
    #endif

    UNLOADLIBRARY(pDSInfo->pHandle,false);
    pDSInfo->pHandle = NULL;
    AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    return TWRC_FAILURE;
  }

  // Report success and squirrel away the index...
  kLOG((kLOGINFO,"Loaded library: %s",_pPath));
  pDSInfo->Identity.Id = _DsId;

  // Get the source to fill in the identity structure
  // This operation should never fail on any DS
  // We need the NULL to be backwards compatible with the
  // older DSM.  This is the only way a driver can tell if
  // it's being talked to directly by the DSM instead of
  // by the application (with the DSM as a passthru)...
  result = pDSInfo->DS_Entry(NULL,DG_CONTROL,DAT_IDENTITY,MSG_GET,(TW_MEMREF)&pDSInfo->Identity);
  if (result != TWRC_SUCCESS)
  {
    UNLOADLIBRARY(pDSInfo->pHandle,false);
    pDSInfo->pHandle = NULL;
    pDSInfo->DS_Entry = NULL;
    AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    return TWRC_FAILURE;
  }

  // Compare the supported groups.  Note that the & is correct
  // because we are comparing bits...
  // we do not want to compare DG_CONTROL because is it supported by all
  if ( !(  (_pAppId->SupportedGroups & DG_MASK & ~DG_CONTROL)              // app supports
         & (pDSInfo->Identity.SupportedGroups & DG_MASK & ~DG_CONTROL) ) ) // source supports
  {
    UNLOADLIBRARY(pDSInfo->pHandle,false);
    pDSInfo->pHandle = NULL;
    pDSInfo->DS_Entry = NULL;
    AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    return TWRC_FAILURE;
  }

  // The DS should not modify the Id even though the spec states
  // that the id will not be assigned until DSM sends MSG_OPENDS to DS
  pDSInfo->Identity.Id = _DsId;
  SSTRNCPY(pDSInfo->szPath, NCHARS(pDSInfo->szPath),_pPath,FILENAME_MAX);

  // We clear the library to avoid cluttering up the virtual address space, and
  // to prevent scary weirdness that can result from multiple drivers being
  // loaded (if the application wants to load multiple drivers, that's its risk).
  UNLOADLIBRARY(pDSInfo->pHandle,false);
  pDSInfo->pHandle = NULL;
  pDSInfo->DS_Entry = NULL;

  // At this point you're probably scratching your head.  Here's the deal.
  // When the DSM issues DG_CONTROL/DAT_IDENTITY/MSG_GET without an
  // AppIdentity structure it alerts the driver that it's being called by
  // the DSM and not by the application, most likely to bring up the user
  // selection dialog.  A driver should use this information to create --
  // and more importantly -- to destroy its internal data structures,
  // because it will get no other chance to clean itself up.
  //
  // It's worth interjecting at this point that Microsoft warns against
  // any but the most minimal activity in DllMain, so relying on doing
  // the create/destroy in there is very risky.  The same goes for the
  // __attribute(constructor)/__attribute(destructor) with GNU.
  //
  // The problem is that the DSM issues DG_CONTROL/DAT_IDENTITY/MSG_GET
  // just prior to DG_CONTROL/DAT_IDENTITY/MSG_OPEN.  If a driver is keyed
  // to the AppIdentity being NULL, it'll incorrectly clean itself up.
  //
  // This means we need to unload and reload the library, to give the
  // driver a consistent look.
  if (_boolKeepOpen == true)
  {
    pDSInfo->pHandle = LOADLIBRARY(_pPath,hook);
    #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    if (0 == pDSInfo->pHandle)
    {
      kLOG((kLOGERR,"Could not load library: %s",_pPath));
      AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
      return TWRC_FAILURE;
    }
    #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    if (0 == pDSInfo->pHandle)
    {
      // This is a bit skanky, and not the sort of thing I really want
      // a user to have to see, but more info is better than less, so
      // hopefully someone will be able to sort out what the cryptic
      // message means and we can FAQ it...
      fprintf(stderr,">>> error loading <%s>\r\n",_pPath);
      fprintf(stderr,">>> %s\r\n",dlerror());
      fprintf(stderr,">>> please contact your scanner or driver vendor for more\r\n");
      fprintf(stderr,">>> help, if that doesn't help then check out the FAQ at\r\n");
      fprintf(stderr,">>> http://www.twain.org\r\n");
      kLOG((kLOGERR,"Could not load library: %s",_pPath));
      kLOG((kLOGERR,dlerror()));
      AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
      return TWRC_FAILURE;
    }
    #else
    #error Sorry, we do not recognize this system...
    #endif

    // Try to get the entry point...
    pDSInfo->DS_Entry = (DSENTRYPROC)DSM_LoadFunction(pDSInfo->pHandle,"DS_Entry");
    if (pDSInfo->DS_Entry == 0)
    {
      kLOG((kLOGERR,"Could not find DSM_Entry function in DS: %s",_pPath));
      UNLOADLIBRARY(pDSInfo->pHandle,hook);
      pDSInfo->pHandle = NULL;
      AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
      return TWRC_FAILURE; 
    }
  }

  // All done...
  return result;
}

/**
* Unload a specific driver.
* I don't care if the called sends this function a bouquet of pink
* bunnies, I think that close type functions shouldn't fail (unless,
* of course they're doing something vital, in which case that
* vital activity shouldn't be in the close function in the first
* place -- so there)...
*/
void CTwnDsmApps::UnloadDS(TW_IDENTITY  *_pAppId,
                           TW_UINT32     _DsId)
{
  // Unload the specified driver...
  if (    AppValidateId(_pAppId)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList
      &&  (_DsId < MAX_NUM_DS)
      &&  m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].pHandle)
  {
    // Unload the library...
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    BOOL retval = 
#else
    int retval = 
#endif
    UNLOADLIBRARY(m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].pHandle,true);

    #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
      if(0 == retval)
      {
        kLOG((kLOGERR,"failed to unload datasource"));
      }
    #else
      if(0 != retval)
      {
        kLOG((kLOGERR,"dlclose: %s",dlerror()));
      }
    #endif

    m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].DS_Entry = 0;
    m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].pDSList->DSInfo[_DsId].pHandle = 0;
  }
}



/**
* Wakeup an application.
* We need this in Windows when we send DAT_NULL to an application, otherwise
* it'll sit there like a lump on a bog until an event comes along to wake it
* up so it can process the message.
*/
void CTwnDsmApps::AppWakeup(TW_IDENTITY *_pAppId)
{
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
  BOOL boolResult;
    if (   AppValidateId(_pAppId)
        && m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].hwnd)
    {
      // Force the parent to process a message.  WM_NULL is
      // safe, because it's a no-op...
      boolResult = ::PostMessage(m_ptwndsmappsimpl->pod.m_AppInfo[_pAppId->Id].hwnd,WM_NULL,(WPARAM)0,(LPARAM)0);
      if (!boolResult)
      {
        kLOG((kLOGERR,"PostMessage failed..."));
      }
    }
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    kLOG((kLOGERR,"We shouldn't be here in AppWakeup..."));
    // We don't support this path on this platform, use
    // callbacks instead...
    // Make the compiler happy...
    _pAppId = _pAppId;
  #else
    #error Sorry, we do not recognize this system...
  #endif
}
