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
* @file log.cpp
* Log messages.
* Provide logging for the messages to and from the Data Source Manager. 
* @author TWAIN Working Group
* @date March 2007
*/

#include "dsm.h"



/**
* Enviroment varible of path to where to write the LogFile name.
* @see CTwnDsmLog
*/
#define kLOGENV "TWAINDSM_LOG"

/**
* Enviroment varible of the fopen logmode to use (if you need to
* grow the log). The default behavior is to wipe it clean each time
* we start up...
* @see CTwnDsmLog
*/
#define kLOGMODEENV "TWAINDSM_LOGMODE"

/**
* Maximum message length we can handle...
* @see CTwnDsmLog
*/
#define TWNDSM_MAX_MSG 1024



/**
* Our implementation class where we hide our attributes...
*/
class CTwnDsmLogImpl
{
  public:
    /// Make sure we're squeaky clean...
    CTwnDsmLogImpl()
    {
      memset(&pod,0,sizeof(pod));
    }

  public:
    // If you add a class in future, (and I can't imagine why you
    // would) declare it here and not in the pod, or the memset
    // we do in the constructor will ruin your day...

    /** 
    * We use a pod system because it help prevents us from
    * making dumb initialization mistakes...
    */
    struct _pod
    {
      FILE *m_plog;                  /**< where we'll dump information. */
      char *m_message;               /**< buffer for our messages. */
      char  m_logpath[FILENAME_MAX]; /**< where we put the file. */
      char  m_logmode[16];           /**< how we fopen the file. */
    } pod;    /**< Pieces of data for CTwnDsmAppsImpl*/
};



/**
* The constructor for our class.  This is where we see if we have a
* file in the TWAINDSM_LOG environment variable.  If so, then we'll
* log stuff.  If not, then we'll log nothing.  TWAINDSM_LOGMODE
* selects how we open the file.  The default value is "w", which
* means it's wiped out each time a new session is started.  Setting
* this environmental to "a" will cause the log information to be
* appended to an existing file (a new one will still be created if
* needed...
*/
CTwnDsmLog::CTwnDsmLog()
{
  // Init stuff...
  m_ptwndsmlogimpl = new CTwnDsmLogImpl;

  // see if a logfile is to be used
  SGETENV(m_ptwndsmlogimpl->pod.m_logpath,NCHARS(m_ptwndsmlogimpl->pod.m_logpath),kLOGENV);

  // If we have a path, then get our mode...
  if (m_ptwndsmlogimpl->pod.m_logpath[0])
  {
    SGETENV(m_ptwndsmlogimpl->pod.m_logmode,NCHARS(m_ptwndsmlogimpl->pod.m_logmode),kLOGMODEENV);
    if (!m_ptwndsmlogimpl->pod.m_logmode[0])
    {
      // The default is to wipe the log clean...
      SSTRCPY(m_ptwndsmlogimpl->pod.m_logmode,sizeof(m_ptwndsmlogimpl->pod.m_logmode),"w");
    }

    // Only bother to allocate a buffer if logging is on...
    m_ptwndsmlogimpl->pod.m_message = (char*)calloc(TWNDSM_MAX_MSG,1);
    if (!m_ptwndsmlogimpl->pod.m_message)
    {
      kPANIC("Unable to allocate a buffer for logging...");
    }
  }
}



/**
* The destructor for our class.  Make sure the log is closed,
* free the buffer and destroy our implementation class...
*/
CTwnDsmLog::~CTwnDsmLog()
{
  if (m_ptwndsmlogimpl)
  {
    if (m_ptwndsmlogimpl->pod.m_plog)
    {
      fclose(m_ptwndsmlogimpl->pod.m_plog);
    }
    if (m_ptwndsmlogimpl->pod.m_message)
    {
      free(m_ptwndsmlogimpl->pod.m_message);
    }
    delete m_ptwndsmlogimpl;
    m_ptwndsmlogimpl = 0;
  }
}



/**
* Logging function.
*
* We provide a timestamp from hours to milliseconds, which can be
* used to help with performance, and to detect large, unexpected
* idle times.  The filename and line number in the source code is
* provided.  GetLastError or errno may offer a hint about a problem
* with a system call, but be careful, since it's not cleared and so
* it may report a message that has nothing to do with the current
* calls, or anything going on in the DSM.  The id of the thread that
* called us is useful for finding problems with unsafe use, or use
* that crosses thread boundaries in a bad way (like on Windows, when
* one has to stay in the same thread as the HWND if the DAT_NULL
* messages are going to work)...
*/
void CTwnDsmLog::Log(const int         _doassert,
                     const char* const _file,
                     const int         _line,
                     const char* const _format,
                     ...)
{
  // We've nothing to do, so bail...
  if (0 == m_ptwndsmlogimpl->pod.m_logpath[0])
  {
    return;
  }

  // Okay, now use the stack...
  UINT  nError;
  UINT  nChars;
  char *message;
  const char *file;

  // Grab the system error, this can be really useful...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    nError = GetLastError();
  if (nError == 0)
  {
    // Yeah, yeah...this is dumb, but I like a clean prefast log...  :)
    nError = 0;
  }
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    nError = errno;
  #else
    #error Sorry, we do not recognize this system...
  #endif

  // If we have no log yet, try to get one...
  if (0 == m_ptwndsmlogimpl->pod.m_plog)
  {
    FOPEN(m_ptwndsmlogimpl->pod.m_plog,m_ptwndsmlogimpl->pod.m_logpath,m_ptwndsmlogimpl->pod.m_logmode);
    if (0 == m_ptwndsmlogimpl->pod.m_plog)
    {
      fprintf(stderr,"DSM: Error - logging has been disabled because logfile could not be opened: %s,%s\r\n",m_ptwndsmlogimpl->pod.m_logpath,m_ptwndsmlogimpl->pod.m_logmode);
      m_ptwndsmlogimpl->pod.m_logpath[0] = 0;
    }
    return;
  }

  // Trim the filename down to just the filename, no path...
  file = 0;
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    // Only look for this on Windows...
    file = strrchr(_file,'\\');
  #endif
  if (!file)
  {
    // If we didn't find a backslash, try a forward slash...
    file = strrchr(_file,'/');
  }
  if (file)
  {
    // skip the slash...
    file = &file[1];
  }
  else
  {
    // Couldn't find any slashes...
    file = (char*)_file;
  }
  
  // Build the message header...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    SYSTEMTIME st;
    GetLocalTime(&st);
    nChars = SNPRINTF(m_ptwndsmlogimpl->pod.m_message,
                      TWNDSM_MAX_MSG,
                      #if (TWNDSM_CMP_VERSION >= 1400)
                        TWNDSM_MAX_MSG,
                      #endif
                      "[%02d%02d%02d%03d %-8s %4d %5d %p] ",
                      st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,
                      file,_line,
                      nError,
                      (void*)(UINT_PTR)GETTHREADID());
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    timeval tv;
    tm tm;
    gettimeofday(&tv,NULL);
    tzset();
    localtime_r(&tv.tv_sec,&tm);
    nChars = SNPRINTF(m_ptwndsmlogimpl->pod.m_message,
                      TWNDSM_MAX_MSG,
                      "[%02d%02d%02d%03ld %-8s %4d %5d %p] ",
                      tm.tm_hour,tm.tm_min,tm.tm_sec,tv.tv_usec / 1000,
                      file,_line,
                      nError,
                      (void*)GETTHREADID());

  #else
    #error Sorry, we do not recognize this system...
  #endif

  // This is the room remaining in the buffer, with room for a null...
  nChars = (TWNDSM_MAX_MSG - nChars) - 1;
  message = &m_ptwndsmlogimpl->pod.m_message[strlen(m_ptwndsmlogimpl->pod.m_message)];

  // Finally, tack on the user portion of the message...
  va_list valist;
  va_start(valist,_format);
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP) && (TWNDSM_CMP_VERSION >= 1400)
    _vsnprintf_s(message,nChars,nChars,_format,valist);
  #elif (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    _vsnprintf(message,nChars,_format,valist);
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    vsnprintf(message,nChars,_format,valist);
  #else
    #error Sorry, we do not recognize this system...
  #endif
  va_end(valist);

  // Write the message...
  fprintf(m_ptwndsmlogimpl->pod.m_plog,"%s\r\n",m_ptwndsmlogimpl->pod.m_message);

  // Do the assert, if asked for...
  if (_doassert)
  {
    assert(0);
  }
}
