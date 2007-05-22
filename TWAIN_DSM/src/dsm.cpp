/***************************************************************************
 * Copyright © 2007 TWAIN Working Group:  Adobe Systems Incorporated,
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
* @author TWAIN Working Group
* @date March 2007
*/

/*! \mainpage Data Source Manager
 *
 * The Source Manager provides the communication path between the 
 * Application and the Source, supports the user’s selection of a  
 * Source, and loads the Source for access by the Application.   
 * Communications from Application to Source Manager or the Source
 * to Source Manager (via DAT_NULL) arrive in exclusively through
 * the DSM_Entry() entry point.
 *
 *
 *
 *
 *
 *
 *
 *
 * Copyright © 2007 TWAIN Working Group:  Adobe Systems Incorporated,
 * AnyDoc Software Inc., Eastman Kodak Company, 
 * Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., 
 * Ricoh Corporation, and Xerox Corporation.
 * All rights reserved.
 */

#include "dsm.h"



/*
** These are all the globals we should ever have in this project...
*/
HINSTANCE   g_hinstance     = 0; /**< Windows Instance handle for the DSM DLL... */
CTwnDsm    *g_ptwndsm       = 0; /**< The main DSM object */
CTwnDsmLog *g_ptwndsmlog    = 0; /**< The logging object, only access through macros */



/**
* Data Source Manager Entry Point.
*
* The only entry point into the Data Source Manager.  This is the only
* function not a part of CTwnDsm.  It's responsible for managing the
* class.  We keep things organized this way to make sure that we don't
* allocate any resources until MSG_OPENDSM, and that we release all
* resources as a part of MSG_CLOSEDSM.  If this is done correctly then
* we won't mind if the library isn't freed (which has been seen to
* happen sometimes with COM)...
*
* Defined in twain.h
*
* @param[in] _pOrigin Identifies the source module of the message. This could
*            identify an Application, a Source, or the Source Manager.
*
* @param[in] _pDest Identifies the destination module for the message.
*            This could identify an application or a data source.
*            If this is NULL, the message goes to the Source Manager.
*
* @param[in] _DG The Data Group. 
*            Example: DG_IMAGE.
*
* @param[in] _DAT The Data Attribute Type.
*            Example: DAT_IMAGEMEMXFER.
*    
* @param[in] _MSG The message.  Messages are interpreted by the destination module
*            with respect to the Data Group and the Data Attribute Type.  
*            Example: MSG_GET.
*
* @param[in,out] _pData A pointer to the data structure or variable identified 
*            by the Data Attribute Type.
*            Example: (TW_MEMREF)&ImageMemXfer
*                   where ImageMemXfer is a TW_IMAGEMEMXFER structure.
*                    
* @return a valid TWRC_xxxx return code.
*            Example: TWRC_SUCCESS.
*/
DSMENTRY DSM_Entry(TW_IDENTITY  *_pOrigin,
                   TW_IDENTITY  *_pDest,
                   TW_UINT32     _DG,
                   TW_UINT16     _DAT,
                   TW_UINT16     _MSG,
                   TW_MEMREF     _pData)
{
  TW_UINT16 rcDSM;

  // Validate...
  if (0 == _pOrigin)
  {
    return TWRC_FAILURE;
  }

  // If we're processing DG_CONTROL/DAT_PARENT/MSG_OPENDSM then see
  // if we need to create our CTwnDsm object.  We don't want to
  // allocate any resources prior to new CTwnDsm!!!
  if (   (_MSG == MSG_OPENDSM)
      && (_DAT == DAT_PARENT)
      && (_DG  == DG_CONTROL)
      && (0 == g_ptwndsm))
  {
      g_ptwndsm = new CTwnDsm;
      if (0 == g_ptwndsm)
      {
          kPANIC("Failed to new CTwnDsm!!!");
          return TWRC_FAILURE;
      }
  }

  // If we have no CTwnDsm object, then we're in trouble, but
  // try to handle DAT_STATUS in case it gets called before
  // MSG_OPENDSM or after MSG_CLOSEDSM...
  if (0 == g_ptwndsm)
  {
      if (   (_MSG == MSG_GET)
          && (_DAT == DAT_STATUS)
          && (_DG  == DG_CONTROL)
          && (0 != _pData))
      {
        ((TW_STATUS*)_pData)->ConditionCode = TWCC_BUMMER;
        return (TWRC_SUCCESS);
      }
      else
      {
        //kLOG((kLOGERR,"DAT_STATUS called before MSG_OPENDSM or after MSG_CLOSEDSM..."));
        return (TWRC_FAILURE);
      }
  }

  // Transfer control over to our dsm object, otherwise we'll
  // be doing g_ptwndsm all over the place...
  rcDSM = g_ptwndsm->DSM_Entry(_pOrigin,_pDest,_DG,_DAT,_MSG,_pData);

  // If we successfully processed DG_CONTROL/DAT_PARENT/MSG_CLOSEDSM,
  // then destroy our object.  We don't want to have any resources
  // lingering around after we destroy our CTwnDsm object!!!
  if (   (TWRC_SUCCESS == rcDSM)
      && (_MSG == MSG_CLOSEDSM)
      && (_DAT == DAT_PARENT)
      && (_DG  == DG_CONTROL))
  {
      delete g_ptwndsm;
      g_ptwndsm = 0;
  }

  // All done...
  return rcDSM;
}



/**
* Our constructor...
* Clean out the pod and set stuff.  Get logging set up so we
* can have a clue what's going on...
*/
CTwnDsm::CTwnDsm()
{
  // Zero out the pod...
  memset(&pod,0,sizeof(pod));

  // Get our logging object...
  g_ptwndsmlog = new CTwnDsmLog;
  if (!g_ptwndsmlog)
  {
      kPANIC("Failed to new CTwnDsmLog!!!");
  }

  // If logging is on, then this is a good chance to dump information
  // about ourselves...
  kLOG((kLOGINFO,"************************************************"));
  kLOG((kLOGINFO,"%s",TWNDSM_ORGANIZATION));
  kLOG((kLOGINFO,"%s",TWNDSM_DESCRIPTION));
  kLOG((kLOGINFO,"version: %s.%s",TWNDSM_VERSIONMAJOR,TWNDSM_VERSIONMINOR));

  // Get our application object...
  pod.m_ptwndsmapps = new CTwnDsmApps();
  if (!pod.m_ptwndsmapps)
  {
      kPANIC("Failed to new CTwnDsmApps!!!");
  }
}



/**
* Our destructor...
* Free any resources we might have...
*/
CTwnDsm::~CTwnDsm()
{
  if (pod.m_ptwndsmapps)
  {
    delete pod.m_ptwndsmapps;
  }
  if (g_ptwndsmlog)
  {
    delete g_ptwndsmlog;
  }
  memset(&pod,0,sizeof(pod));
}



/**
* This is where we finish up the DSM_Entry duties inside of the
* context of the class...
*/
TW_UINT16 CTwnDsm::DSM_Entry(TW_IDENTITY  *_pOrigin,
                             TW_IDENTITY  *_pDest,
                             TW_UINT32     _DG,
                             TW_UINT16     _DAT,
                             TW_UINT16     _MSG,
                             TW_MEMREF     _pData)
{
  TW_UINT16     rcDSM   = TWRC_SUCCESS;
  bool          bPrinted;
  TW_CALLBACK  *ptwcallback;
  TW_IDENTITY  *pAppId  = _pOrigin;
  TW_IDENTITY  *pDSId   = _pDest;

  // Do a test to see if pOrigin is a DS instead of App, if so then switch pAppId and pDSId
  // MSG_INVOKE_CALLBACK was only used on the Mac and is now deprecated (ver 2.1)
  // it is here for backwords capabiltiy
  if ( (_DAT == DAT_NULL /*&& _DG == DG_CONTROL */)
    || (_DAT == DAT_CALLBACK && _MSG == MSG_INVOKE_CALLBACK /*&& _DG == DG_CONTROL */) )
  {
    pAppId  = _pDest;
    pDSId   = _pOrigin;
  }

  // Print the triplets to stdout for information purposes
  bPrinted = printTripletsInfo(_DG,_DAT,_MSG,_pData);

  // Sniff for the application forwarding an event to the
  // DS. It may be possible that the app has a message waiting for
  // it because it didn't register a callback.
  if (   (DAT_EVENT == _DAT)
      && (MSG_PROCESSEVENT == _MSG))
  {
    // Check that the AppID and DSID are valid...
    if (!pod.m_ptwndsmapps->AppValidateIds(pAppId,pDSId))
    {
      pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADPROTOCOL);
      rcDSM = TWRC_FAILURE;
    }
    else if (pod.m_ptwndsmapps->DsCallbackIsWaiting(pAppId,pDSId->Id))
    {
      ptwcallback = pod.m_ptwndsmapps->DsCallbackGet(pAppId,pDSId->Id);
      ((TW_EVENT*)(_pData))->TWMessage = ptwcallback->Message;
      ptwcallback->Message = 0;
      pod.m_ptwndsmapps->DsCallbackSetWaiting(pAppId,pDSId->Id,FALSE);
      rcDSM = TWRC_DSEVENT;
    }
    // No callback, so fall on through...
  }

  // Is this msg for us?
  if( TWRC_SUCCESS == rcDSM )
  {
  switch (_DAT)
    {
      default:
        // check if the application is open or not.  If it isn't, we have a bad sequence
        if (dsmState_Open == pod.m_ptwndsmapps->AppGetState(pAppId))
        {
            // Check that the AppID and DSID are valid...
            if (!pod.m_ptwndsmapps->AppValidateIds(pAppId,pDSId))
            {
              pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADPROTOCOL);
              rcDSM = TWRC_FAILURE;
            }

            // Issue the command...
            else if (0 != pod.m_ptwndsmapps->DsGetEntryProc(pAppId,pDSId->Id))
            {
              rcDSM = (pod.m_ptwndsmapps->DsGetEntryProc(pAppId,pDSId->Id))(
                                      pod.m_ptwndsmapps->AppGetIdentity(pAppId),
                                      _DG,
                                      _DAT,
                                      _MSG,
                                      _pData);
            }

            // For some reason we have no pointer to the dsentry function...
            else
            {
              pod.m_ptwndsmapps->AppSetConditionCode(pAppId,TWCC_OPERATIONERROR);
              kLOG((kLOGERR,"DS_Entry is null...%ld",pAppId->Id));
              rcDSM = TWRC_FAILURE;
            }
        }
        else
        {
            pod.m_ptwndsmapps->AppSetConditionCode(pAppId,TWCC_SEQERROR);
            rcDSM = TWRC_FAILURE;
        }
        break;

      case DAT_PARENT:
        rcDSM = DSM_Parent(pAppId,_MSG,_pData);
        break;

      case DAT_IDENTITY:
        rcDSM = DSM_Identity(pAppId,_MSG,(TW_IDENTITY*)_pData);
        break;

      case DAT_STATUS:
        rcDSM = DSM_Status(pAppId,_MSG,(TW_STATUS*)_pData);
        break;

      case DAT_CALLBACK:
          // DAT_CALLBACK can be either from an Application registering its Callback, 
          // or from a DS Invoking a request to send a message to the Application
        rcDSM = DSM_Callback(_pOrigin,_pDest,_MSG,(TW_CALLBACK*)_pData);
        break;

      case DAT_NULL:
        // Note how the origin and destination are switched for this
        // call (and only this call).  Because, of course, this
        // message is being send from the driver to the application...
        rcDSM = DSM_Null(pAppId,pDSId,_MSG);
        break;
    }
  }

  // Log how it went...
  if (bPrinted)
  {
    char szRc[64];
    StringFromRC(szRc,NCHARS(szRc),rcDSM);
    kLOG((kLOGINFO,szRc));
  }

  return rcDSM;
}



/**
* Handle DAT_STATUS.  Just a few things of note, we handle some
* DAT_STATUS stuff in DSM_Entry.  And per the spec we have to
* clear the condition code when we are done.  I've also put in
* MSG_CHECKSTATUS, because I can't imagine why we have the silly
* thing if it isn't for this function...
*/
TW_INT16 CTwnDsm::DSM_Status(TW_IDENTITY  *_pAppId,
                             TW_UINT16     _MSG,
                             TW_STATUS    *_pStatus)
{
  TW_INT16 result = TWRC_SUCCESS;

  switch (_MSG)
  {
    case MSG_GET:
    case MSG_CHECKSTATUS:
       _pStatus->ConditionCode = pod.m_ptwndsmapps->AppGetConditionCode(_pAppId);
       _pStatus->Reserved = 0;
      break;

    default:
      result = TWRC_FAILURE;
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
      break;
  }

  return result;
}



/**
* Handle DAT_PARENT.  This is where the DSM is expected to
* do most of its contribution, which is finding drivers for
* the application...
*/
TW_INT16 CTwnDsm::DSM_Parent(TW_IDENTITY  *_pAppId,
                             TW_UINT16     _MSG,
                             TW_MEMREF     _MemRef)
{
  TW_UINT16 result;

  // Validate...
  if (0 == _pAppId)
  {
      kLOG((kLOGERR,"_pAppId is null"));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
      return TWRC_FAILURE;
  }

  // Init stuff...
  result = TWRC_SUCCESS;

  // Process the message...
  switch (_MSG)
  {
    case MSG_OPENDSM:
      // Try to add the proposed item...
      result = pod.m_ptwndsmapps->AddApp(_pAppId,_MemRef);
      break;

    case MSG_CLOSEDSM:
      // Try to remove the proposed item...
      result = pod.m_ptwndsmapps->RemoveApp(_pAppId);
      break;

    default:
      result = TWRC_FAILURE;
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
      break;
  }

  return result;
}



/**
* Handle DAT_IDENTITY.  This is where the DSM is expected to
* do most of its contribution, which is finding drivers for
* the application...
*/
TW_INT16 CTwnDsm::DSM_Identity(TW_IDENTITY  *_pAppId,
                               TW_UINT16     _MSG,
                               TW_IDENTITY  *_pDsId)
{
  TW_INT16  result;

  // Validate...
  if (0 == _pAppId)
  {
      kLOG((kLOGERR,"_pAppId is null"));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
      return TWRC_FAILURE;
  }
  else if (_pAppId->Id >= MAX_NUM_APPS)
  {
      kLOG((kLOGERR,"too many apps"));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_MAXCONNECTIONS);
      return TWRC_FAILURE;
  }

  // Init stuff...
  result = TWRC_SUCCESS;

  // Pick the message...
  if (dsmState_Open == pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    switch (_MSG)
    {
      case MSG_OPENDS:
        result = OpenDS(_pAppId,_pDsId);
        break;

      case MSG_CLOSEDS:
        result = CloseDS(_pAppId,_pDsId);
        break;

      case MSG_USERSELECT:
        result = DSM_SelectDS(_pAppId,_pDsId);
        break;

      case MSG_GETFIRST:
        result = DSM_GetFirst(_pAppId,_pDsId);
        break;

      case MSG_GETNEXT:
        result = DSM_GetNext(_pAppId,_pDsId);
        break;

      case MSG_GETDEFAULT:
        result = GetMatchingDefault(_pAppId,_pDsId);
        break;

      default:
        result = TWRC_FAILURE;
        pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
        break;
    }
  }
  else
  {
    result = TWRC_FAILURE;
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
  }

  return result;
}



/**
* We've received a callback.  MSG_REGISTER_CALLBACK are from
* the Application and MSG_INVOKE_CALLBACK (Mac OSx only) 
* are from the DS.
* If Callbacks have not been registered by the App then when the 
* DS Invokes a callback, make a note of it so the next time the 
* application hits us with a Windows message for us to process, 
* we can send it the callback message...
*/
TW_INT16 CTwnDsm::DSM_Callback(TW_IDENTITY *_pOrigin,
                               TW_IDENTITY *_pDest,
                               TW_UINT16    _MSG,
                               TW_CALLBACK *_pData)
{
  TW_INT16      result;
  TW_CALLBACK  *ptwcallback;

  // Init stuff...
  result = TWRC_SUCCESS;

  // Take action on the message...
  switch (_MSG)
  {
    case MSG_REGISTER_CALLBACK:
      {
        // Origin is an App
        // Check that the ids are valid...
        if (!pod.m_ptwndsmapps->AppValidateIds(_pOrigin,_pDest))
        {
          pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADPROTOCOL);
          return TWRC_FAILURE;
        }
        if(0 == _pData)
        {
          kLOG((kLOGERR,"Invalid data"));
          pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADVALUE);
          return TWRC_FAILURE;
        }

        ptwcallback = pod.m_ptwndsmapps->DsCallbackGet(_pOrigin,_pDest->Id);
        memcpy(ptwcallback,_pData,sizeof(*ptwcallback));
        pod.m_ptwndsmapps->DsCallbackSetWaiting(_pOrigin,_pDest->Id,FALSE);
      }
      break;

    case MSG_INVOKE_CALLBACK:
      {
        // For backwards capability only.  MSG_INVOKE_CALLBACK is deprecated - use DAT_NULL
        // Origin is a DS
        // Check that the ids are valid...
        kLOG((kLOGINFO,"MSG_INVOKE_CALLBACK is deprecated - use DAT_NULL"));
        if (!pod.m_ptwndsmapps->AppValidateIds(_pDest,_pOrigin))
        {
          pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADPROTOCOL);
          return TWRC_FAILURE;
        }
        if(0 == _pData)
        {
          kLOG((kLOGERR,"Invalid data"));
          pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADVALUE);
          return TWRC_FAILURE;
        }

        ptwcallback = (TW_CALLBACK*)_pData;
        result = DSM_Null(_pDest,_pOrigin,ptwcallback->Message);
      }
      break;

    default:
      result = TWRC_FAILURE;
      pod.m_ptwndsmapps->AppSetConditionCode(_pOrigin,TWCC_BADPROTOCOL);
      break;
  }

  return result;
}



/**
* Open the specified driver.  We're using the application identity
* and the driver identity we picked up during MSG_OPENDSM.  The
* application is just telling us which driver to load.  As part of
* a successful open we'll remember this driver's full path and file
* name as the new default driver.  On Windows this information goes
* into the registry.  On Linux we put it under the user's home
* directory...
*/
TW_INT16 CTwnDsm::OpenDS(TW_IDENTITY *_pAppId,
                         TW_IDENTITY *_pDsId)
{
  TW_INT16  result;
 
  // Validate...
  if (0 == _pAppId)
  {
      kLOG((kLOGERR,"_pAppId is null"));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
      return TWRC_FAILURE;
  }
  else if (   (_pAppId->Id < 1)
           || (_pAppId->Id >= MAX_NUM_APPS))
  {
      kLOG((kLOGERR,"id is out of range...%d",_pAppId->Id));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_MAXCONNECTIONS);
      return TWRC_FAILURE;
  }

  // Init stuff...
  result = TWRC_SUCCESS;

  // check that we are in the proper state
  if (dsmState_Open != pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
    return(TWRC_FAILURE);
  }

  // check for valid data
  if(0 == _pDsId)
  {
    kLOG((kLOGERR,"_pDsId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }

  // Do we need to find a source to open
  if (0 == _pDsId->Id)
  {
    // Does the app know the name of the source it wants to open
    if (0 != _pDsId->ProductName[0])
    {
      // The application is passing me a TW_IDENTITY structure that contains
      // the name of the source to select.
      result = GetDSFromProductName(_pAppId,_pDsId);

      // was the id found or specified by the app?
      if (TWRC_SUCCESS != result)
      {
        pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_NODS);
        return(result);
      }
    }

    // Does the application want me to choose the default?
    // Or no Source located by name
    if (0 == _pDsId->ProductName[0]) 
    {
      // -if the name of the source is NULL, and the id is 0, the application is
      //  telling me to select the default source.
      result = GetMatchingDefault(_pAppId,_pDsId);

      // was the id found or specified by the app?
      if (TWRC_SUCCESS != result)
      {
        return(result);
      }
    }
  }

  // Load the driver...
  result = pod.m_ptwndsmapps->LoadDS(_pAppId,_pDsId->Id);
  if (result != TWRC_SUCCESS)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_NODS);
    return(TWRC_FAILURE);
  }

  // open the ds
  if (0 != pod.m_ptwndsmapps->DsGetEntryProc(_pAppId,_pDsId->Id))
  {
    result = pod.m_ptwndsmapps->DsGetEntryProc(_pAppId,_pDsId->Id)(
                            pod.m_ptwndsmapps->AppGetIdentity(_pAppId),
                            DG_CONTROL,
                            DAT_IDENTITY,
                            MSG_OPENDS,
                            (TW_MEMREF)_pDsId);

    if (TWRC_SUCCESS != result)
    {
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    }
  }

  // Remember that we opened this DS...
  if (TWRC_SUCCESS == result)
  {
    #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
      // skip...
    #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
      FILE *pfile;
      char *szHome;
      char szFile[FILENAME_MAX];
      szHome = getenv("HOME");
      if (szHome)
      {
        SSTRCPY(szFile,sizeof(szFile),szHome);
        SSTRCAT(szFile,sizeof(szFile),"/.twndsmrc");
        mkdir(szFile,0660);
        SSTRCAT(szFile,sizeof(szFile),"/defaultds");
        FOPEN(pfile,szFile,"w");
        if (pfile)
        {
          fwrite(pod.m_ptwndsmapps->DsGetPath(_pAppId,_pDsId->Id),
                 1,
                 strlen(pod.m_ptwndsmapps->DsGetPath(_pAppId,_pDsId->Id)),
                 pfile);
          fclose(pfile);
        }
      }
    #else
      #error Sorry, we don't recognize this system...
    #endif
  }


  return result;
}



/**
* Close the specified driver...
*/
TW_INT16 CTwnDsm::CloseDS(TW_IDENTITY *_pAppId,
                          TW_IDENTITY *_pDsId)
{
  TW_INT16  result;

  // Validate...
  if (0 == _pAppId)
  {
    kLOG((kLOGERR,"_pAppId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (   (_pAppId->Id < 1)
           || (_pAppId->Id >= MAX_NUM_APPS))
  {
    kLOG((kLOGERR,"id out of range...%d",_pAppId->Id));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  // Init stuff...
  result = TWRC_SUCCESS;

  // check that we are in the proper state
  if (dsmState_Open != pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
    return TWRC_FAILURE;
  }

  // Check for valid DS
  if(0 == _pDsId)
  {
    kLOG((kLOGERR,"_pDsId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }

  // close the ds
  if (0 != pod.m_ptwndsmapps->DsGetEntryProc(_pAppId,_pDsId->Id))
  {
    result = (pod.m_ptwndsmapps->DsGetEntryProc(_pAppId,_pDsId->Id))(
                             pod.m_ptwndsmapps->AppGetIdentity(_pAppId),
                             DG_CONTROL,
                             DAT_IDENTITY,
                             MSG_CLOSEDS,
                             (TW_MEMREF)_pDsId);

    if (TWRC_SUCCESS != result)
    {
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
      return result;
    }

    // Cleanup...
    pod.m_ptwndsmapps->UnloadDS(_pAppId,_pDsId->Id);
  }

  // All done...
  return result;
}



#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
/**
* DllMain is only needed for Windows, and it's only needed to collect
* our instance handle, which is also our module handle.  Don't ever
* put anything else in here, not even logging messages.  It just isn't
* safe...
*/
BOOL WINAPI DllMain(HINSTANCE _hmodule,
                    DWORD     _dwReasonCalled,
                    LPVOID)
{
  switch (_dwReasonCalled)
  {
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_ATTACH:
      g_hinstance = _hmodule;
      break;
    case DLL_PROCESS_DETACH:
      break;
  }
  return(TRUE);
}
#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    // Nothing for us to do...
#else
    #error Sorry, we don't recognize this system...
#endif



/**
* We support a selection dialog on Windows.  I wish we didn't, it's
* more trouble than it's worth, but it's part of that legacy thing.
* This function is properly constructed for use with DialogBox...
*/
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
BOOL CALLBACK SelectDlgProc(HWND   _hWnd,
                            UINT   _Message,
                            WPARAM _wParam,
                            LPARAM _lParam)
{
  if (g_ptwndsm)
  {
    return g_ptwndsm->SelectDlgProc(_hWnd,_Message,_wParam,_lParam);
  }
  else
  {
    return TRUE;
  }
}
#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
  // We don't have one of these...
#else
  #error Sorry, we don't recognize this system...
#endif



/**
* We support a selection dialog on Windows.  This function is
* part of our CTwnDsm class, so we don't have to have a lot
* of pointers, and we can keep things private, unlike what we
* would have to do if we put this code into the function we
* actually pass to DialogBox...
*/
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
BOOL CTwnDsm::SelectDlgProc(HWND hWnd, UINT Message,
                            WPARAM wParam,
                            LPARAM /*lParam - unused*/)
{
  TW_IDENTITY  *pAppId;
  int           nIndex;
  int           nSelect;
  TW_UINT32     x;
  char         *szProductName;
  HWND          hListBox;

  // Init stuff...
  nSelect = 0;
  pAppId = pod.m_pSelectDlgAppId;

  // Process the message...
  switch (Message)
  {
    case WM_INITDIALOG:
      hListBox = ::GetDlgItem(hWnd,ID_LST_SOURCES);
      if (hListBox) 
      {
        SendMessage(hListBox,LB_RESETCONTENT,(WPARAM)NULL,(LPARAM)NULL);

        for (x = 1; x < MAX_NUM_DS; ++x)
        {
          // We expect the list to be contiguous...
          szProductName = pod.m_ptwndsmapps->DsGetIdentity(pAppId,x)->ProductName;
          if (!szProductName[0])
          {
              break;
          }
          // Display the name...
          nIndex = (int)SendMessage(hListBox,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)szProductName);
          if (LB_ERR == nIndex)
          {
            break;
          }
          // Associate the id with the name...
          nIndex = (int)SendMessage(hListBox,
                                    LB_SETITEMDATA,
                                    (WPARAM)nIndex,
                                    (LPARAM)pod.m_ptwndsmapps->DsGetIdentity(pAppId,x)->Id);
          if (LB_ERR == nIndex)
          {
            break;
          }
          // Remember this item if it's the default...
          if (!strcmp(pod.m_ptwndsmapps->DsGetPath(pAppId,x),pod.m_DefaultDSPath))
          {
            nSelect = x;
          }
        }
        // If we have no drivers, then disable the OK button...
        if (pod.m_ptwndsmapps->AppGetNumDs(pAppId) < 1)
        {
          HWND hOK= ::GetDlgItem(hWnd,IDOK);
          EnableWindow(hOK, FALSE);
        }
        // Otherwise select the defaulted item...
        else
        {
          nIndex = (int)SendMessage(hListBox,
                                    LB_FINDSTRINGEXACT,
                                    (WPARAM)-1,
                                    (LPARAM)pod.m_ptwndsmapps->DsGetIdentity(pAppId,nSelect)->ProductName);
          if (LB_ERR == nIndex)
          {
            nIndex = 0;
          }
          SendMessage(hListBox,LB_SETCURSEL,(WPARAM)nIndex,(LPARAM)NULL);
        }
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_LST_SOURCES: 
          if (HIWORD(wParam) != LBN_DBLCLK) 
            break; 
            // drop through...

        case IDOK:
          {
            HWND hListBox = ::GetDlgItem(hWnd, ID_LST_SOURCES); 
            int  nIndex   = 0;
            if ( hListBox ) 
            {
              nIndex = (int)SendMessage(hListBox,LB_GETCURSEL,(WPARAM)0,(LPARAM)0);
              if (LB_ERR == nIndex)
              {
                // if there is no selection should not have OK available
                // to press in the first place.
                return TRUE;
              }
              nIndex = (int)SendMessage(hListBox,LB_GETITEMDATA,(WPARAM)nIndex,(LPARAM)0);
              if (LB_ERR != nIndex)
              {
                pod.m_pSelectDlgDsId = pod.m_ptwndsmapps->DsGetIdentity(pAppId,nIndex);
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
#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
  // We don't have anything to do on Linux...
#else
  #error Sorry, we don't recognize this system...
#endif



/**
* Invoke the user selection dialog box.  We only support this for
* Windows, for Linux it's a bad protocol, since there is no way
* to query the user (nicely) across all consoles and graphical
* interfaces for all distributions...
*/
TW_INT16 CTwnDsm::DSM_SelectDS(TW_IDENTITY *_pAppId,
                               TW_IDENTITY *_pDsId)
{
  // Validate...
  if (0 == _pAppId)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  if (   (_pAppId->Id < 1)
      || (_pAppId->Id >= MAX_NUM_APPS))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (0 == _pDsId)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }
  else if (dsmState_Open != pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
    return TWRC_FAILURE;
  }

  /** @TODO scanDSDir needs to be done with each MSG_USERSELECT  
    currently we are only scanDSDir when an App opens the DSM **/

  // Make sure the id is 0 before we go into this...
  _pDsId->Id = 0;

  // Windows...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)

      HKEY      hKey;
      long      status;
      char     *szPath;
      TW_INT16  result;

      // Set the context...
      result = TWRC_SUCCESS;
      pod.m_pSelectDlgAppId = _pAppId;
        
      // If passed in a DS name we want to select it
      if (_pDsId->ProductName[0] != 0)
      {
        result = GetDSFromProductName(_pAppId,_pDsId);
        // If no match continue anyway.
      }

      // If not passed a DS or the name was not currently found
      // then selete the default
      _pDsId->Id = 0;
      result = GetMatchingDefault(_pAppId,_pDsId);
      pod.m_pSelectDlgDsId = _pDsId;

      // create the dialog window
      int ret = (int)::DialogBox(g_hinstance,
                                 (LPCTSTR)IDD_DLG_SOURCE,
                                 (HWND)NULL,
                                 (DLGPROC)::SelectDlgProc);

      // User picked something...
      if (ret == IDOK)
      {
        // Validate the result...
        if (!pod.m_pSelectDlgDsId)
        {
          kLOG((kLOGERR,"We came out of the Select Dialog with a null..."));
          pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
          return TWRC_FAILURE;
        }

        // Copy the data over...
        *_pDsId = *pod.m_pSelectDlgDsId;

        // save default source to Registry  
        // sanity check...
        if (   (pod.m_pSelectDlgDsId->Id < 1)
            || (pod.m_pSelectDlgDsId->Id >= MAX_NUM_DS))
        {
          // Failed to save default DS to registry
          kLOG((kLOGERR,"Id is out of range 0 - 49..."));
          // Nothing preventing us from using the default right now
          pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BUMMER);
        }

        else
        {
          // Get the path we're using...
          status = ERROR_SUCCESS;
          szPath = pod.m_ptwndsmapps->DsGetPath(pod.m_pSelectDlgAppId,pod.m_pSelectDlgDsId->Id);

          // Open the key, creating it if it doesn't exist.
          if (RegCreateKeyEx(HKEY_CURRENT_USER,
                             "Software\\Microsoft\\Windows NT\\CurrentVersion\\Twain",
                             NULL,
                             NULL,
                             NULL,
                             KEY_READ | KEY_WRITE, NULL,
                             &hKey,
                             NULL) == ERROR_SUCCESS)
          {
            status = RegSetValueEx(hKey,"Default Source",0,REG_SZ,(LPBYTE)szPath,(DWORD)strlen((char*)szPath)+1);
            if (status != ERROR_SUCCESS)
            {
              // Failed to save default DS to registry
              kLOG((kLOGERR,"Failed to save default DS to registry"));
              // Nothing preventing us from using the default right now
              pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BUMMER);
            }
          }
          // Close the key.
          RegCloseKey(hKey);
        }
      }

      // We're cancelling...
      else if (ret == IDCANCEL)
      {
        result = TWRC_CANCEL;
      }

      // Something back happened...
      else if (ret == -1)
      {
        ::MessageBox(NULL,"Dialog failed!","Error",MB_OK|MB_ICONINFORMATION);
        pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BUMMER);
        result = TWRC_FAILURE;
      }

      return result;

  // We don't support the user selection box on linux...
  #elif  (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)

    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
    return TWRC_FAILURE;

  // Ruh-roh, Reorge...
  #else
    #error Sorry, we don't recognize this system...
  #endif
}



/**
* Invoke the user selection dialog box.  We only support this for
* Windows, for Linux it's a bad protocol, since there is no way
* to query the user (nicely) across all consoles and graphical
* interfaces for all distributions...
*/
TW_INT16 CTwnDsm::GetDSFromProductName(TW_IDENTITY *_pAppId,
                                       TW_IDENTITY *_pDsId)
{
  TW_UINT32 ii;

  // Validate...
  if (   !pod.m_ptwndsmapps->AppValidateId(_pAppId)
      || (0 == _pDsId))
  {
    kLOG((kLOGERR,"bad _pAppId or _pDsId..."));
    return TWRC_FAILURE;
  }
  else if (0 == _pDsId->ProductName[0])
  {
    return TWRC_FAILURE;
  }

  // Search for a match on the ProductName...
  for (ii = 1; ii < MAX_NUM_DS; ++ii)
  {
    // Note that TW_STR32 type is NUL-filled, not NUL-terminated...
    if (0 == strncmp(_pDsId->ProductName,
                     pod.m_ptwndsmapps->DsGetIdentity(_pAppId,ii)->ProductName,
                     sizeof(TW_STR32)))
    {
      // match found, set the index
      *_pDsId = *pod.m_ptwndsmapps->DsGetIdentity(_pAppId,ii);
      return TWRC_SUCCESS;
    }
  }

  // Uh-oh...
  return TWRC_FAILURE;
}



/**
* Get the identity for the first driver we found, or TWRC_ENDOFLIST
* if we don't have any...
*/
TW_INT16 CTwnDsm::DSM_GetFirst(TW_IDENTITY *_pAppId,
                               TW_IDENTITY *_pDsId)
{
  // Validate...
  if (   !pod.m_ptwndsmapps->AppValidateId(_pAppId)
      || (0 == _pDsId))
  {
    kLOG((kLOGERR,"bad _pAppId or _pDsId..."));
    return TWRC_FAILURE;
  }

  /** @TODO scanDSDir needs to be done with each MSG_GETFIRST  
      currently we are only scanDSDir when an App opens the DSM **/

  // There are no supported drivers...
  if (pod.m_ptwndsmapps->AppGetNumDs(_pAppId) < 1)
  {
    // Make sure we fail of GetNext is called...
    pod.m_nextDsId = pod.m_ptwndsmapps->AppGetNumDs(_pAppId) + 1;
    return TWRC_ENDOFLIST;
  }

  // Check for valid DS
  if(0 == _pDsId)
  {
    kLOG((kLOGERR,"_pDsId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }


  // Return info on the first driver we found...
  pod.m_nextDsId = 1;
  *_pDsId = *pod.m_ptwndsmapps->DsGetIdentity(_pAppId,pod.m_nextDsId);

  // Prep for a call to GetNext...
  pod.m_nextDsId += 1;

  // All done...
  return TWRC_SUCCESS;
}



/**
* Get the identity for the next driver we found, or TWRC_ENDOFLIST
* if we've run out...
*/
TW_INT16 CTwnDsm::DSM_GetNext(TW_IDENTITY *_pAppId,
                              TW_IDENTITY *_pDsId)
{
  // Validate...
  if (   !pod.m_ptwndsmapps->AppValidateId(_pAppId)
      || (0 == _pDsId))
  {
    kLOG((kLOGERR,"bad _pAppId or _pDsId..."));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }

  // Applications must call MSG_GETFIRST before making this call...
  if (pod.m_nextDsId == 0)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
    return TWRC_FAILURE;
  }
  // We're out of items...
  else if (pod.m_nextDsId > pod.m_ptwndsmapps->AppGetNumDs(_pAppId))
  {
    pod.m_nextDsId = 0;
    return TWRC_ENDOFLIST;
  }

  // Return info on the this driver...
  *_pDsId = *pod.m_ptwndsmapps->DsGetIdentity(_pAppId,pod.m_nextDsId);

  // Prep for a call to GetNext...
  pod.m_nextDsId += 1;

  // All done...
  return TWRC_SUCCESS;
}



/**
* Get the identity for the next driver we found, or TWRC_ENDOFLIST
* if we've run out...
*/
TW_INT16 CTwnDsm::GetMatchingDefault(TW_IDENTITY *_pAppId,
                                     TW_IDENTITY *_pDsId)
{
  bool      bMatchFnd = false;
  TW_UINT32 ii;

  // Validate...
  if (   !pod.m_ptwndsmapps->AppValidateId(_pAppId)
      || (0 == _pDsId))
  {
    kLOG((kLOGERR,"bad _pAppId or _pDsId..."));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }

  // is there something to match to?
  if (pod.m_ptwndsmapps->AppGetNumDs(_pAppId) < 1)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_NODS);
    return TWRC_FAILURE;
  }

  // Something very bad may be happening, so don't let the
  // application get away with this...
  if (0 != _pDsId->Id)
  {
    kLOG((kLOGERR,"Please make sure your TW_IDENTITY.Id for your driver (the destination) is zeroed out before making this call..."));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    return TWRC_FAILURE;
  }

  // In Windows the default Data Source is stored in the registry
  // as the path to that DS.  We will need to compare this to the other DS as a match.
  // read default source from Registry
  memset(pod.m_DefaultDSPath,0,sizeof(pod.m_DefaultDSPath));

  // Windows uses the registry...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion\\Twain",
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS )
    {
      // Look for the subkey "Default Source".
      DWORD DWtype = REG_SZ;
      DWORD DWsize = sizeof(pod.m_DefaultDSPath);
      BOOL bRunAtStartup = ( RegQueryValueEx(hKey,"Default Source",NULL,&DWtype,(LPBYTE)pod.m_DefaultDSPath,&DWsize) == ERROR_SUCCESS);

      /// @TODO Check the result...
      // There wasn't any use of bRunAtStartup before I added this, and
      // this is obviously pretty lame, but until I find out how the value
      // was to be used I don't want to get rid of it...
      if (bRunAtStartup)
      {
        bRunAtStartup = bRunAtStartup;
      }

      // Close the registry key handle.
      RegCloseKey(hKey);
    }

  // Linux looks in the user's directory...
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    FILE *pfile;
    char *szHome;
    char szFile[FILENAME_MAX];
    memset(pod.m_DefaultDSPath,0,sizeof(pod.m_DefaultDSPath));
    szHome = getenv("HOME");
    if (szHome)
    {
      SSTRCPY(szFile,sizeof(szFile),szHome);
      SSTRCAT(szFile,sizeof(szFile),"/.twndsmrc/defaultds");
      FOPEN(pfile,szFile,"r");
      if (pfile)
      {
        fread(pod.m_DefaultDSPath,1,sizeof(pod.m_DefaultDSPath)-1,pfile);
        fclose(pfile);
      }
    }

  // eek...
  #else
    #error Sorry, we don't recognize this system...
  #endif


  // If current default source is not a match find a new default source
  // that will match this app
  for (ii = 1; ii < MAX_NUM_DS; ++ii)
  {
    // Mark the first match to use as default, if we don't
    // find a match, this will be the one we go with...
    if (!bMatchFnd)
    {
      *_pDsId = *pod.m_ptwndsmapps->DsGetIdentity(_pAppId,ii);
      bMatchFnd = true;
    }

    // If the system default is a match we will use it and stop looking.
    if (0 == STRNICMP(pod.m_DefaultDSPath,
                      pod.m_ptwndsmapps->DsGetPath(_pAppId,ii),
                      sizeof(pod.m_DefaultDSPath)))
    {
      *_pDsId = *pod.m_ptwndsmapps->DsGetIdentity(_pAppId,ii);
      bMatchFnd = true;
      break;
    }
  }

  if (!bMatchFnd)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_NODS);
    return TWRC_FAILURE;
  }

  return TWRC_SUCCESS;
}



/**
* Log the triplets that the application sends to us...
*/
bool CTwnDsm::printTripletsInfo(const TW_UINT32 _DG,
                                const TW_UINT16 _DAT,
                                const TW_UINT16 _MSG,
                                const TW_MEMREF _pData)
{
  char szDg[64];
  char szDat[64];
  char szMsg[64];
  char szCap[128];
  TW_CAPABILITY *_pCap;

  // too many of these messages to log...
  if (    (DG_CONTROL == _DG)
      &&  (DAT_EVENT == _DAT))
  {
    return false;
  }

  // Convert them...
  StringFromDg(szDg,NCHARS(szDg),_DG);
  StringFromDat(szDat,NCHARS(szDat),_DAT);
  StringFromMsg(szMsg,NCHARS(szMsg),_MSG);

  // Print them, (and give us a blank line)...
  kLOG((kLOGINFO,""));
  kLOG((kLOGINFO,"%s/%s/%s",szDg,szDat,szMsg));

  // If we're a capability, do some extra work and
  // try to tell them what cap it is...
  if (   (DG_CONTROL == _DG)
      && (DAT_CAPABILITY == _DAT)
      && (NULL != _pData))
  {
    _pCap = (TW_CAPABILITY*)_pData;
    StringFromCap(szCap,NCHARS(szCap),_pCap->Cap);
    kLOG((kLOGINFO,szCap));
  }

  // All done...
  return true;
}



/**
* DAT_NULL is used by a driver to send certain messages back to the
* application, like MSG_XFERREADY...
*/
TW_INT16 CTwnDsm::DSM_Null(TW_IDENTITY *_pAppId,
                           TW_IDENTITY *_pDsId,
                           TW_UINT16    _MSG)
{
  TW_CALLBACK *ptwcallback;

  // Validate...
  if ( !pod.m_ptwndsmapps->AppValidateIds(_pAppId,_pDsId) )
  {
    kLOG((kLOGERR,"bad _pAppId or _pDsId..."));
    return TWRC_FAILURE;
  }

  // Invoke the application's callback to send this message along.
  if (   (MSG_DEVICEEVENT != _MSG)
      && (MSG_CLOSEDSOK   != _MSG)
      && (MSG_CLOSEDSREQ  != _MSG)
      && (MSG_XFERREADY   != _MSG))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
    return TWRC_FAILURE;
  }

  // Get the current callback...
  ptwcallback = pod.m_ptwndsmapps->DsCallbackGet(_pAppId,_pDsId->Id);

  // We have something to call...
  if (0 != ptwcallback)
  {
    // We should have a try/catch around this...
    ((DSMENTRYPROC)(ptwcallback->CallBackProc))(
        pod.m_ptwndsmapps->AppGetIdentity(_pAppId),
        pod.m_ptwndsmapps->DsGetIdentity(_pAppId,_pDsId->Id),
        DG_CONTROL,
        DAT_NULL,
        _MSG,
        0);
  }

  // Application has not registered a callback. As a result, the msg will
  // be sent to the app the next time it forwards an event.
  // Each App's DS has a callback structure.   This way multiple DS's can make
  // a callback to a single app, and we are not going to lose MSG's
  else
  {
    ptwcallback->Message = _MSG;
    pod.m_ptwndsmapps->DsCallbackSetWaiting(_pAppId,_pDsId->Id,TRUE);
    pod.m_ptwndsmapps->AppWakeup(_pAppId);
  }

  return TWRC_SUCCESS;
}



/**
* Convert a DG_ data group numerical value to a string...
*/
void CTwnDsm::StringFromDg(char      *_szDg,
                           int        _nChars,
                     const TW_UINT32  _DG)
{
  switch(_DG)
  {
    default:
      SSNPRINTF(_szDg,_nChars,_nChars,"DG_0x%04lx",_DG);
      break;

    case DG_CONTROL:
      SSTRCPY(_szDg,_nChars,"DG_CONTROL");
      break;

    case DG_IMAGE:
      SSTRCPY(_szDg,_nChars,"DG_IMAGE");
      break;

    case DG_AUDIO:
      SSTRCPY(_szDg,_nChars,"DG_AUDIO");
      break;
  }
}



/**
* Convert a DAT_ data argument type numerical value to a string...
*/
void CTwnDsm::StringFromDat(char     *_szDat,
                            int       _nChars,
                      const TW_UINT16 _DAT)
{
  switch(_DAT)
  {
    default:
      SSNPRINTF(_szDat,_nChars,_nChars,"DAT_0x%04x",_DAT);

    case DAT_NULL:
      SSTRCPY(_szDat,_nChars,"DAT_NULL");
      break;

    case DAT_CUSTOMBASE:
      SSTRCPY(_szDat,_nChars,"DAT_CUSTOMBASE");
      break;

    case DAT_CAPABILITY:
      SSTRCPY(_szDat,_nChars,"DAT_CAPABILITY");
      break;

    case DAT_EVENT:
      SSTRCPY(_szDat,_nChars,"DAT_EVENT");
      break;

    case DAT_IDENTITY:
      SSTRCPY(_szDat,_nChars,"DAT_IDENTITY");
      break;

    case DAT_PARENT:
      SSTRCPY(_szDat,_nChars,"DAT_PARENT");
      break;

    case DAT_PENDINGXFERS:
      SSTRCPY(_szDat,_nChars,"DAT_PENDINGXFERS");
      break;

    case DAT_SETUPMEMXFER:
      SSTRCPY(_szDat,_nChars,"DAT_SETUPMEMXFER");
      break;

    case DAT_SETUPFILEXFER:
      SSTRCPY(_szDat,_nChars,"DAT_SETUPFILEXFER");
      break;

    case DAT_STATUS:
      SSTRCPY(_szDat,_nChars,"DAT_STATUS");
      break;

    case DAT_USERINTERFACE:
      SSTRCPY(_szDat,_nChars,"DAT_USERINTERFACE");
      break;

    case DAT_XFERGROUP:
      SSTRCPY(_szDat,_nChars,"DAT_XFERGROUP");
      break;

    case DAT_TWUNKIDENTITY:
      SSTRCPY(_szDat,_nChars,"DAT_TWUNKIDENTITY");
      break;

    case DAT_CUSTOMDSDATA:
      SSTRCPY(_szDat,_nChars,"DAT_CUSTOMDSDATA");
      break;

    case DAT_DEVICEEVENT:
      SSTRCPY(_szDat,_nChars,"DAT_DEVICEEVENT");
      break;

    case DAT_FILESYSTEM:
      SSTRCPY(_szDat,_nChars,"DAT_FILESYSTEM");
      break;

    case DAT_PASSTHRU:
      SSTRCPY(_szDat,_nChars,"DAT_PASSTHRU");
      break;

    case DAT_CALLBACK:
      SSTRCPY(_szDat,_nChars,"DAT_CALLBACK");
      break;

    case DAT_IMAGEINFO:
      SSTRCPY(_szDat,_nChars,"DAT_IMAGEINFO");
      break;

    case DAT_IMAGELAYOUT:
      SSTRCPY(_szDat,_nChars,"DAT_IMAGELAYOUT");
      break;

    case DAT_IMAGEMEMXFER:
      SSTRCPY(_szDat,_nChars,"DAT_IMAGEMEMXFER");
      break;

    case DAT_IMAGENATIVEXFER:
      SSTRCPY(_szDat,_nChars,"DAT_IMAGENATIVEXFER");
      break;

    case DAT_IMAGEFILEXFER:
      SSTRCPY(_szDat,_nChars,"DAT_IMAGEFILEXFER");
      break;

    case DAT_CIECOLOR:
      SSTRCPY(_szDat,_nChars,"DAT_CIECOLOR");
      break;

    case DAT_GRAYRESPONSE:
      SSTRCPY(_szDat,_nChars,"DAT_GRAYRESPONSE");
      break;

    case DAT_RGBRESPONSE:
      SSTRCPY(_szDat,_nChars,"DAT_RGBRESPONSE");
      break;

    case DAT_JPEGCOMPRESSION:
      SSTRCPY(_szDat,_nChars,"DAT_JPEGCOMPRESSION");
      break;

    case DAT_PALETTE8:
      SSTRCPY(_szDat,_nChars,"DAT_PALETTE8");
      break;

    case DAT_EXTIMAGEINFO:
      SSTRCPY(_szDat,_nChars,"DAT_EXTIMAGEINFO");
      break;

    case DAT_AUDIOFILEXFER:
      SSTRCPY(_szDat,_nChars,"DAT_AUDIOFILEXFER");
      break;

    case DAT_AUDIOINFO:
      SSTRCPY(_szDat,_nChars,"DAT_AUDIOINFO");
      break;

    case DAT_AUDIONATIVEXFER:
      SSTRCPY(_szDat,_nChars,"DAT_AUDIONATIVEXFER");
      break;
  }
}



/**
* Convert a MSG_ message numerical value to a string...
*/
void CTwnDsm::StringFromMsg(char     *_szMsg,
                            int       _nChars,
                      const TW_UINT16 _MSG)
{
  switch (_MSG)
  {
    default:  
      SSNPRINTF(_szMsg,_nChars,_nChars,"MSG_0x%04x",_MSG);
      break;

    case MSG_NULL:
      SSTRCPY(_szMsg,_nChars,"MSG_NULL");
      break;

    case MSG_CUSTOMBASE:
      SSTRCPY(_szMsg,_nChars,"MSG_CUSTOMBASE");
      break;

    case MSG_GET:
      SSTRCPY(_szMsg,_nChars,"MSG_GET");
      break;

    case MSG_GETCURRENT:
      SSTRCPY(_szMsg,_nChars,"MSG_GETCURRENT");
      break;

    case MSG_GETDEFAULT:
      SSTRCPY(_szMsg,_nChars,"MSG_GETDEFAULT");
      break;

    case MSG_GETFIRST:
      SSTRCPY(_szMsg,_nChars,"MSG_GETFIRST");
      break;

    case MSG_GETNEXT:
      SSTRCPY(_szMsg,_nChars,"MSG_GETNEXT");
      break;

    case MSG_SET:
      SSTRCPY(_szMsg,_nChars,"MSG_SET");
      break;

    case MSG_RESET:
      SSTRCPY(_szMsg,_nChars,"MSG_RESET");
      break;

    case MSG_QUERYSUPPORT:
      SSTRCPY(_szMsg,_nChars,"MSG_QUERYSUPPORT");
      break;

    case MSG_XFERREADY:
      SSTRCPY(_szMsg,_nChars,"MSG_XFERREADY");
      break;

    case MSG_CLOSEDSREQ:
      SSTRCPY(_szMsg,_nChars,"MSG_CLOSEDSREQ");
      break;

    case MSG_CLOSEDSOK:
      SSTRCPY(_szMsg,_nChars,"MSG_CLOSEDSOK");
      break;

    case MSG_DEVICEEVENT:
      SSTRCPY(_szMsg,_nChars,"MSG_DEVICEEVENT");
      break;

    case MSG_CHECKSTATUS:
      SSTRCPY(_szMsg,_nChars,"MSG_CHECKSTATUS");
      break;

    case MSG_OPENDSM:
      SSTRCPY(_szMsg,_nChars,"MSG_OPENDSM");
      break;

    case MSG_CLOSEDSM:
      SSTRCPY(_szMsg,_nChars,"MSG_CLOSEDSM");
      break;

    case MSG_OPENDS:
      SSTRCPY(_szMsg,_nChars,"MSG_OPENDS");
      break;

    case MSG_CLOSEDS:
      SSTRCPY(_szMsg,_nChars,"MSG_CLOSEDS");
      break;

    case MSG_USERSELECT:
      SSTRCPY(_szMsg,_nChars,"MSG_USERSELECT");
      break;

    case MSG_DISABLEDS:
      SSTRCPY(_szMsg,_nChars,"MSG_DISABLEDS");
      break;

    case MSG_ENABLEDS:
      SSTRCPY(_szMsg,_nChars,"MSG_ENABLEDS");
      break;

    case MSG_ENABLEDSUIONLY:
      SSTRCPY(_szMsg,_nChars,"MSG_ENABLEDSUIONLY");
      break;

    case MSG_PROCESSEVENT:
      SSTRCPY(_szMsg,_nChars,"MSG_PROCESSEVENT");
      break;

    case MSG_ENDXFER:
      SSTRCPY(_szMsg,_nChars,"MSG_ENDXFER");
      break;

    case MSG_CHANGEDIRECTORY:
      SSTRCPY(_szMsg,_nChars,"MSG_CHANGEDIRECTORY");
      break;

    case MSG_CREATEDIRECTORY:
      SSTRCPY(_szMsg,_nChars,"MSG_CREATEDIRECTORY");
      break;

    case MSG_DELETE:
      SSTRCPY(_szMsg,_nChars,"MSG_DELETE");
      break;

    case MSG_FORMATMEDIA:
      SSTRCPY(_szMsg,_nChars,"MSG_FORMATMEDIA");
      break;

    case MSG_GETCLOSE:
      SSTRCPY(_szMsg,_nChars,"MSG_GETCLOSE");
      break;

    case MSG_GETFIRSTFILE:
      SSTRCPY(_szMsg,_nChars,"MSG_GETFIRSTFILE");
      break;

    case MSG_GETINFO:
      SSTRCPY(_szMsg,_nChars,"MSG_GETINFO");
      break;

    case MSG_GETNEXTFILE:
      SSTRCPY(_szMsg,_nChars,"MSG_GETNEXTFILE");
      break;

    case MSG_RENAME:
      SSTRCPY(_szMsg,_nChars,"MSG_RENAME");
      break;

    case MSG_PASSTHRU:
      SSTRCPY(_szMsg,_nChars,"MSG_PASSTHRU");
      break;

    case MSG_REGISTER_CALLBACK:
      SSTRCPY(_szMsg,_nChars,"MSG_REGISTER_CALLBACK");
      break;
  }
}



/**
* Convert a CAP_ or ICAP_ capability numerical value to a string...
*/
void CTwnDsm::StringFromCap(char     *_szCap,
                            int       _nChars,
                      const TW_UINT16 _Cap)
{
  switch (_Cap)
  {
    default:
      SSNPRINTF(_szCap,_nChars,_nChars,"CAP_0x%04x",_Cap);
      break;

    case CAP_CUSTOMBASE:
      SSTRCPY(_szCap,_nChars,"CAP_CUSTOMBASE");
      break;

    case CAP_XFERCOUNT:
      SSTRCPY(_szCap,_nChars,"CAP_XFERCOUNT");
      break;

    case ICAP_COMPRESSION:
      SSTRCPY(_szCap,_nChars,"ICAP_COMPRESSION");
      break;

    case ICAP_PIXELTYPE:
      SSTRCPY(_szCap,_nChars,"ICAP_PIXELTYPE");
      break;

    case ICAP_UNITS:
      SSTRCPY(_szCap,_nChars,"ICAP_UNITS");
      break;

    case ICAP_XFERMECH:
      SSTRCPY(_szCap,_nChars,"ICAP_XFERMECH");
      break;

    case CAP_AUTHOR:
      SSTRCPY(_szCap,_nChars,"CAP_AUTHOR");
      break;

    case CAP_CAPTION:
      SSTRCPY(_szCap,_nChars,"CAP_CAPTION");
      break;

    case CAP_FEEDERENABLED:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDERENABLED");
      break;

    case CAP_FEEDERLOADED:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDERLOADED");
      break;

    case CAP_TIMEDATE:
      SSTRCPY(_szCap,_nChars,"CAP_TIMEDATE");
      break;

    case CAP_SUPPORTEDCAPS:
      SSTRCPY(_szCap,_nChars,"CAP_SUPPORTEDCAPS");
      break;

    case CAP_EXTENDEDCAPS:
      SSTRCPY(_szCap,_nChars,"CAP_EXTENDEDCAPS");
      break;

    case CAP_AUTOFEED:
      SSTRCPY(_szCap,_nChars,"CAP_AUTOFEED");
      break;

    case CAP_CLEARPAGE:
      SSTRCPY(_szCap,_nChars,"CAP_CLEARPAGE");
      break;

    case CAP_FEEDPAGE:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDPAGE");
      break;

    case CAP_REWINDPAGE:
      SSTRCPY(_szCap,_nChars,"CAP_REWINDPAGE");
      break;

    case CAP_INDICATORS:
      SSTRCPY(_szCap,_nChars,"CAP_INDICATORS");
      break;

    case CAP_SUPPORTEDCAPSEXT:
      SSTRCPY(_szCap,_nChars,"CAP_SUPPORTEDCAPSEXT");
      break;

    case CAP_PAPERDETECTABLE:
      SSTRCPY(_szCap,_nChars,"CAP_PAPERDETECTABLE");
      break;

    case CAP_UICONTROLLABLE:
      SSTRCPY(_szCap,_nChars,"CAP_UICONTROLLABLE");
      break;

    case CAP_DEVICEONLINE:
      SSTRCPY(_szCap,_nChars,"CAP_DEVICEONLINE");
      break;

    case CAP_AUTOSCAN:
      SSTRCPY(_szCap,_nChars,"CAP_AUTOSCAN");
      break;

    case CAP_THUMBNAILSENABLED:
      SSTRCPY(_szCap,_nChars,"CAP_THUMBNAILSENABLED");
      break;

    case CAP_DUPLEX:
      SSTRCPY(_szCap,_nChars,"CAP_DUPLEX");
      break;

    case CAP_DUPLEXENABLED:
      SSTRCPY(_szCap,_nChars,"CAP_DUPLEXENABLED");
      break;

    case CAP_ENABLEDSUIONLY:
      SSTRCPY(_szCap,_nChars,"CAP_ENABLEDSUIONLY");
      break;

    case CAP_CUSTOMDSDATA:
      SSTRCPY(_szCap,_nChars,"CAP_CUSTOMDSDATA");
      break;

    case CAP_ENDORSER:
      SSTRCPY(_szCap,_nChars,"CAP_ENDORSER");
      break;

    case CAP_JOBCONTROL:
      SSTRCPY(_szCap,_nChars,"CAP_JOBCONTROL");
      break;

    case CAP_ALARMS:
      SSTRCPY(_szCap,_nChars,"CAP_ALARMS");
      break;

    case CAP_ALARMVOLUME:
      SSTRCPY(_szCap,_nChars,"CAP_ALARMVOLUME");
      break;

    case CAP_AUTOMATICCAPTURE:
      SSTRCPY(_szCap,_nChars,"CAP_AUTOMATICCAPTURE");
      break;

    case CAP_TIMEBEFOREFIRSTCAPTURE:
      SSTRCPY(_szCap,_nChars,"CAP_TIMEBEFOREFIRSTCAPTURE");
      break;

    case CAP_TIMEBETWEENCAPTURES:
      SSTRCPY(_szCap,_nChars,"CAP_TIMEBETWEENCAPTURES");
      break;

    case CAP_CLEARBUFFERS:
      SSTRCPY(_szCap,_nChars,"CAP_CLEARBUFFERS");
      break;

    case CAP_MAXBATCHBUFFERS:
      SSTRCPY(_szCap,_nChars,"CAP_MAXBATCHBUFFERS");
      break;

    case CAP_DEVICETIMEDATE:
      SSTRCPY(_szCap,_nChars,"CAP_DEVICETIMEDATE");
      break;

    case CAP_POWERSUPPLY:
      SSTRCPY(_szCap,_nChars,"CAP_POWERSUPPLY");
      break;

    case CAP_CAMERAPREVIEWUI:
      SSTRCPY(_szCap,_nChars,"CAP_CAMERAPREVIEWUI");
      break;

    case CAP_DEVICEEVENT:
      SSTRCPY(_szCap,_nChars,"CAP_DEVICEEVENT");
      break;

    case CAP_SERIALNUMBER:
      SSTRCPY(_szCap,_nChars,"CAP_SERIALNUMBER");
      break;

    case CAP_PRINTER:
      SSTRCPY(_szCap,_nChars,"CAP_PRINTER");
      break;

    case CAP_PRINTERENABLED:
      SSTRCPY(_szCap,_nChars,"CAP_PRINTERENABLED");
      break;

    case CAP_PRINTERINDEX:
      SSTRCPY(_szCap,_nChars,"CAP_PRINTERINDEX");
      break;

    case CAP_PRINTERMODE:
      SSTRCPY(_szCap,_nChars,"CAP_PRINTERMODE");
      break;

    case CAP_PRINTERSTRING:
      SSTRCPY(_szCap,_nChars,"CAP_PRINTERSTRING");
      break;

    case CAP_PRINTERSUFFIX:
      SSTRCPY(_szCap,_nChars,"CAP_PRINTERSUFFIX");
      break;

    case CAP_LANGUAGE:
      SSTRCPY(_szCap,_nChars,"CAP_LANGUAGE");
      break;

    case CAP_FEEDERALIGNMENT:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDERALIGNMENT");
      break;

    case CAP_FEEDERORDER:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDERORDER");
      break;

    case CAP_REACQUIREALLOWED:
      SSTRCPY(_szCap,_nChars,"CAP_REACQUIREALLOWED");
      break;

    case CAP_BATTERYMINUTES:
      SSTRCPY(_szCap,_nChars,"CAP_BATTERYMINUTES");
      break;

    case CAP_BATTERYPERCENTAGE:
      SSTRCPY(_szCap,_nChars,"CAP_BATTERYPERCENTAGE");
      break;

    case ICAP_AUTOBRIGHT:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOBRIGHT");
      break;

    case ICAP_BRIGHTNESS:
      SSTRCPY(_szCap,_nChars,"ICAP_BRIGHTNESS");
      break;

    case ICAP_CONTRAST:
      SSTRCPY(_szCap,_nChars,"ICAP_CONTRAST");
      break;

    case ICAP_CUSTHALFTONE:
      SSTRCPY(_szCap,_nChars,"ICAP_CUSTHALFTONE");
      break;

    case ICAP_EXPOSURETIME:
      SSTRCPY(_szCap,_nChars,"ICAP_EXPOSURETIME");
      break;

    case ICAP_FILTER:
      SSTRCPY(_szCap,_nChars,"ICAP_FILTER");
      break;

    case ICAP_FLASHUSED:
      SSTRCPY(_szCap,_nChars,"ICAP_FLASHUSED");
      break;

    case ICAP_GAMMA:
      SSTRCPY(_szCap,_nChars,"ICAP_GAMMA");
      break;

    case ICAP_HALFTONES:
      SSTRCPY(_szCap,_nChars,"ICAP_HALFTONES");
      break;

    case ICAP_HIGHLIGHT:
      SSTRCPY(_szCap,_nChars,"ICAP_HIGHLIGHT");
      break;

    case ICAP_IMAGEFILEFORMAT:
      SSTRCPY(_szCap,_nChars,"ICAP_IMAGEFILEFORMAT");
      break;

    case ICAP_LAMPSTATE:
      SSTRCPY(_szCap,_nChars,"ICAP_LAMPSTATE");
      break;

    case ICAP_LIGHTSOURCE:
      SSTRCPY(_szCap,_nChars,"ICAP_LIGHTSOURCE");
      break;

    case ICAP_ORIENTATION:
      SSTRCPY(_szCap,_nChars,"ICAP_ORIENTATION");
      break;

    case ICAP_PHYSICALWIDTH:
      SSTRCPY(_szCap,_nChars,"ICAP_PHYSICALWIDTH");
      break;

    case ICAP_PHYSICALHEIGHT:
      SSTRCPY(_szCap,_nChars,"ICAP_PHYSICALHEIGHT");
      break;

    case ICAP_SHADOW:
      SSTRCPY(_szCap,_nChars,"ICAP_SHADOW");
      break;

    case ICAP_FRAMES:
      SSTRCPY(_szCap,_nChars,"ICAP_FRAMES");
      break;

    case ICAP_XNATIVERESOLUTION:
      SSTRCPY(_szCap,_nChars,"ICAP_XNATIVERESOLUTION");
      break;

    case ICAP_YNATIVERESOLUTION:
      SSTRCPY(_szCap,_nChars,"ICAP_YNATIVERESOLUTION");
      break;

    case ICAP_XRESOLUTION:
      SSTRCPY(_szCap,_nChars,"ICAP_XRESOLUTION");
      break;

    case ICAP_YRESOLUTION:
      SSTRCPY(_szCap,_nChars,"ICAP_YRESOLUTION");
      break;

    case ICAP_MAXFRAMES:
      SSTRCPY(_szCap,_nChars,"ICAP_MAXFRAMES");
      break;

    case ICAP_TILES:
      SSTRCPY(_szCap,_nChars,"ICAP_TILES");
      break;

    case ICAP_BITORDER:
      SSTRCPY(_szCap,_nChars,"ICAP_BITORDER");
      break;

    case ICAP_CCITTKFACTOR:
      SSTRCPY(_szCap,_nChars,"ICAP_CCITTKFACTOR");
      break;

    case ICAP_LIGHTPATH:
      SSTRCPY(_szCap,_nChars,"ICAP_LIGHTPATH");
      break;

    case ICAP_PIXELFLAVOR:
      SSTRCPY(_szCap,_nChars,"ICAP_PIXELFLAVOR");
      break;

    case ICAP_PLANARCHUNKY:
      SSTRCPY(_szCap,_nChars,"ICAP_PLANARCHUNKY");
      break;

    case ICAP_ROTATION:
      SSTRCPY(_szCap,_nChars,"ICAP_ROTATION");
      break;

    case ICAP_SUPPORTEDSIZES:
      SSTRCPY(_szCap,_nChars,"ICAP_SUPPORTEDSIZES");
      break;

    case ICAP_THRESHOLD:
      SSTRCPY(_szCap,_nChars,"ICAP_THRESHOLD");
      break;

    case ICAP_XSCALING:
      SSTRCPY(_szCap,_nChars,"ICAP_XSCALING");
      break;

    case ICAP_YSCALING:
      SSTRCPY(_szCap,_nChars,"ICAP_YSCALING");
      break;

    case ICAP_BITORDERCODES:
      SSTRCPY(_szCap,_nChars,"ICAP_BITORDERCODES");
      break;

    case ICAP_PIXELFLAVORCODES:
      SSTRCPY(_szCap,_nChars,"ICAP_PIXELFLAVORCODES");
      break;

    case ICAP_JPEGPIXELTYPE:
      SSTRCPY(_szCap,_nChars,"ICAP_JPEGPIXELTYPE");
      break;

    case ICAP_TIMEFILL:
      SSTRCPY(_szCap,_nChars,"ICAP_TIMEFILL");
      break;

    case ICAP_BITDEPTH:
      SSTRCPY(_szCap,_nChars,"ICAP_BITDEPTH");
      break;

    case ICAP_BITDEPTHREDUCTION:
      SSTRCPY(_szCap,_nChars,"ICAP_BITDEPTHREDUCTION");
      break;

    case ICAP_UNDEFINEDIMAGESIZE:
      SSTRCPY(_szCap,_nChars,"ICAP_UNDEFINEDIMAGESIZE");
      break;

    case ICAP_IMAGEDATASET:
      SSTRCPY(_szCap,_nChars,"ICAP_IMAGEDATASET");
      break;

    case ICAP_EXTIMAGEINFO:
      SSTRCPY(_szCap,_nChars,"ICAP_EXTIMAGEINFO");
      break;

    case ICAP_MINIMUMHEIGHT:
      SSTRCPY(_szCap,_nChars,"ICAP_MINIMUMHEIGHT");
      break;

    case ICAP_MINIMUMWIDTH:
      SSTRCPY(_szCap,_nChars,"ICAP_MINIMUMWIDTH");
      break;

    case ICAP_FLIPROTATION:
      SSTRCPY(_szCap,_nChars,"ICAP_FLIPROTATION");
      break;

    case ICAP_BARCODEDETECTIONENABLED:
      SSTRCPY(_szCap,_nChars,"ICAP_BARCODEDETECTIONENABLED");
      break;

    case ICAP_SUPPORTEDBARCODETYPES:
      SSTRCPY(_szCap,_nChars,"ICAP_SUPPORTEDBARCODETYPES");
      break;

    case ICAP_BARCODEMAXSEARCHPRIORITIES:
      SSTRCPY(_szCap,_nChars,"ICAP_BARCODEMAXSEARCHPRIORITIES");
      break;

    case ICAP_BARCODESEARCHPRIORITIES:
      SSTRCPY(_szCap,_nChars,"ICAP_BARCODESEARCHPRIORITIES");
      break;

    case ICAP_BARCODESEARCHMODE:
      SSTRCPY(_szCap,_nChars,"ICAP_BARCODESEARCHMODE");
      break;

    case ICAP_BARCODEMAXRETRIES:
      SSTRCPY(_szCap,_nChars,"ICAP_BARCODEMAXRETRIES");
      break;

    case ICAP_BARCODETIMEOUT:
      SSTRCPY(_szCap,_nChars,"ICAP_BARCODETIMEOUT");
      break;

    case ICAP_ZOOMFACTOR:
      SSTRCPY(_szCap,_nChars,"ICAP_ZOOMFACTOR");
      break;

    case ICAP_PATCHCODEDETECTIONENABLED:
      SSTRCPY(_szCap,_nChars,"ICAP_PATCHCODEDETECTIONENABLED");
      break;

    case ICAP_SUPPORTEDPATCHCODETYPES:
      SSTRCPY(_szCap,_nChars,"ICAP_SUPPORTEDPATCHCODETYPES");
      break;

    case ICAP_PATCHCODEMAXSEARCHPRIORITIES:
      SSTRCPY(_szCap,_nChars,"ICAP_PATCHCODEMAXSEARCHPRIORITIES");
      break;

    case ICAP_PATCHCODESEARCHPRIORITIES:
      SSTRCPY(_szCap,_nChars,"ICAP_PATCHCODESEARCHPRIORITIES");
      break;

    case ICAP_PATCHCODESEARCHMODE:
      SSTRCPY(_szCap,_nChars,"ICAP_PATCHCODESEARCHMODE");
      break;

    case ICAP_PATCHCODEMAXRETRIES:
      SSTRCPY(_szCap,_nChars,"ICAP_PATCHCODEMAXRETRIES");
      break;

    case ICAP_PATCHCODETIMEOUT:
      SSTRCPY(_szCap,_nChars,"ICAP_PATCHCODETIMEOUT");
      break;

    case ICAP_FLASHUSED2:
      SSTRCPY(_szCap,_nChars,"ICAP_FLASHUSED2");
      break;

    case ICAP_IMAGEFILTER:
      SSTRCPY(_szCap,_nChars,"ICAP_IMAGEFILTER");
      break;

    case ICAP_NOISEFILTER:
      SSTRCPY(_szCap,_nChars,"ICAP_NOISEFILTER");
      break;

    case ICAP_OVERSCAN:
      SSTRCPY(_szCap,_nChars,"ICAP_OVERSCAN");
      break;

    case ICAP_AUTOMATICBORDERDETECTION:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICBORDERDETECTION");
      break;

    case ICAP_AUTOMATICDESKEW:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICDESKEW");
      break;

    case ICAP_AUTOMATICROTATE:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICROTATE");
      break;

    case ICAP_JPEGQUALITY:
      SSTRCPY(_szCap,_nChars,"ICAP_JPEGQUALITY");
      break;

    case ACAP_AUDIOFILEFORMAT:
      SSTRCPY(_szCap,_nChars,"ACAP_AUDIOFILEFORMAT");
      break;

    case ACAP_XFERMECH:
      SSTRCPY(_szCap,_nChars,"ACAP_XFERMECH");
      break;
  }
}



/**
* Convert a TWRC_ return code numerical value to a string...
*/
void CTwnDsm::StringFromRC(char     *_szRc,
                           int       _nChars,
                     const TW_UINT16 _rc)
{
  switch (_rc)
  {
    default:    
      SSNPRINTF(_szRc,_nChars,_nChars,"TWRC_0x%04x",_rc);
      break;

    case TWRC_SUCCESS:
      SSTRCPY(_szRc,_nChars,"TWRC_SUCCESS");
      break;

    case TWRC_FAILURE:
      SSTRCPY(_szRc,_nChars,"TWRC_FAILURE");
      break;

    case TWRC_CHECKSTATUS:
      SSTRCPY(_szRc,_nChars,"TWRC_CHECKSTATUS");
      break;

    case TWRC_CANCEL:
      SSTRCPY(_szRc,_nChars,"TWRC_CANCEL");
      break;

    case TWRC_DSEVENT:
      SSTRCPY(_szRc,_nChars,"TWRC_DSEVENT");
      break;

    case TWRC_NOTDSEVENT:
      SSTRCPY(_szRc,_nChars,"TWRC_NOTDSEVENT");
      break;

    case TWRC_XFERDONE:
      SSTRCPY(_szRc,_nChars,"TWRC_XFERDONE");
      break;

    case TWRC_ENDOFLIST:
      SSTRCPY(_szRc,_nChars,"TWRC_ENDOFLIST");
      break;

    case TWRC_INFONOTSUPPORTED:
      SSTRCPY(_szRc,_nChars,"TWRC_INFONOTSUPPORTED");
      break;

    case TWRC_DATANOTAVAILABLE:
      SSTRCPY(_szRc,_nChars,"TWRC_DATANOTAVAILABLE");
      break;
  }
}
