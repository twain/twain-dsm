/***************************************************************************
 * TWAIN Data Source Manager version 2.1
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
* These are all the globals we should ever have in this project...
*/
HINSTANCE   g_hinstance     = 0; /**< Windows Instance handle for the DSM DLL... */
CTwnDsm    *g_ptwndsm       = 0; /**< The main DSM object */
CTwnDsmLog *g_ptwndsmlog    = 0; /**< The logging object, only access through macros */



/**
* Localization:  we have the selection box on Windows that we have
* to deal with, so this table gives us all our strings in UTF-8 format.
* TWAIN defines a lot of languages, and it makes sense to reference
* all of them.  Refer to the DSM_translations.txt file if you want to
* see the mapping between the hex UTF-8 and the viewable characters.
* The UTF-8 values are in hex so that the encoding of this file doesn't
* change the resulting output.
*
* Many languages still need translations...
*/
/*
* we need this because xcode insists on complaining about content
* inside of visual studio's code block
*/
#if (TWNDSM_OS == TWNDSM_OS_MACOSX)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#endif
#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
typedef struct
{
  TW_INT16  Language;     /**< Language */
  BYTE      CharSet;      /**< Character Set*/
  LANGID    LangId;       /**< Language Id*/
  char      *Title;       /**< the Title string*/
  char      *Sources;     /**< the Sources string */
  char      *Select;      /**< the Select string */
  char      *Cancel;      /**< the Cancel string */
} TwLocalize;

/**
* Localized strings for the select dialog
*/
static TwLocalize s_twlocalize[] =
{
      {TWLG_AFRIKAANS,          ANSI_CHARSET,       MAKELANGID(LANG_AFRIKAANS,SUBLANG_NEUTRAL),                 "Select Source","Sources:","Select","\x4b\x61\x6e\x73\x65\x6c\x6c\x65\x65\x72"},
      {TWLG_ALBANIA,            EASTEUROPE_CHARSET, MAKELANGID(LANG_ALBANIAN,SUBLANG_NEUTRAL),                  "","","",""},
      {TWLG_ARABIC,             ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_NEUTRAL),                    "","","",""},
      {TWLG_ARABIC_ALGERIA,     ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_ALGERIA),             "","","",""},
      {TWLG_ARABIC_BAHRAIN,     ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_BAHRAIN),             "","","",""},
      {TWLG_ARABIC_EGYPT,       ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_EGYPT),               "","","",""},
      {TWLG_ARABIC_IRAQ,        ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_IRAQ),                "","","",""},
      {TWLG_ARABIC_JORDAN,      ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_JORDAN),              "","","",""},
      {TWLG_ARABIC_KUWAIT,      ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_KUWAIT),              "","","",""},
      {TWLG_ARABIC_LEBANON,     ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_LEBANON),             "","","",""},
      {TWLG_ARABIC_LIBYA,       ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_LIBYA),               "","","",""},
      {TWLG_ARABIC_MOROCCO,     ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_MOROCCO),             "","","",""},
      {TWLG_ARABIC_OMAN,        ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_OMAN),                "","","",""},
      {TWLG_ARABIC_QATAR,       ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_QATAR),               "","","",""},
      {TWLG_ARABIC_SAUDIARABIA, ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_SAUDI_ARABIA),        "","","",""},
      {TWLG_ARABIC_SYRIA,       ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_SYRIA),               "","","",""},
      {TWLG_ARABIC_TUNISIA,     ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_TUNISIA),             "","","",""},
      {TWLG_ARABIC_UAE,         ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_UAE),                 "","","",""},
      {TWLG_ARABIC_YEMEN,       ARABIC_CHARSET,     MAKELANGID(LANG_ARABIC,SUBLANG_ARABIC_YEMEN),               "","","",""},
      {TWLG_ASSAMESE,           ANSI_CHARSET,       MAKELANGID(LANG_ASSAMESE,SUBLANG_NEUTRAL),                  "","","",""},
      {TWLG_BASQUE,             ANSI_CHARSET,       MAKELANGID(LANG_BASQUE,SUBLANG_NEUTRAL),                    "Select Source","Sources:","Select","\x55\x74\x7a\x69"},
      {TWLG_BENGALI,            ANSI_CHARSET,       MAKELANGID(LANG_BENGALI,SUBLANG_NEUTRAL),                   "","","",""},
      {TWLG_BIHARI,             0,                  0,                                                          "","","",""},
      {TWLG_BODO,               0,                  0,                                                          "","","",""},
      {TWLG_BULGARIAN,          RUSSIAN_CHARSET,    MAKELANGID(LANG_BULGARIAN,SUBLANG_NEUTRAL),                 "","","",""},
      {TWLG_BYELORUSSIAN,       RUSSIAN_CHARSET,    MAKELANGID(LANG_BELARUSIAN,SUBLANG_NEUTRAL),                "","","",""},
      {TWLG_CATALAN,            ANSI_CHARSET,       MAKELANGID(LANG_CATALAN,SUBLANG_NEUTRAL),                   "Select Source","Sources:","Select","\x43\x61\x6e\x63\x65\x6c\xc2\xb7\x6c\x61"},
      {TWLG_CHINESE,            GB2312_CHARSET,     MAKELANGID(LANG_CHINESE,SUBLANG_NEUTRAL),                   "\xe9\x80\x89\xe6\x8b\xa9\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90","\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90\x3a","\xe9\x80\x89\xe6\x8b\xa9","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CHINESE_HONGKONG,   CHINESEBIG5_CHARSET,MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_HONGKONG),          "\xe9\x81\xb8\xe6\x93\x87\xe5\xbd\xb1\xe5\x83\x8f\xe4\xbe\x86\xe6\xba\x90","\xe5\xbd\xb1\xe5\x83\x8f\xe4\xbe\x86\xe6\xba\x90\x3a","\xe7\xa2\xba\xe5\xae\x9a","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CHINESE_PRC,        GB2312_CHARSET,     MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),        "\xe9\x80\x89\xe6\x8b\xa9\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90","\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90\x3a","\xe9\x80\x89\xe6\x8b\xa9","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CHINESE_SIMPLIFIED, GB2312_CHARSET,     MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),        "\xe9\x80\x89\xe6\x8b\xa9\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90","\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90\x3a","\xe9\x80\x89\xe6\x8b\xa9","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CHINESE_SINGAPORE,  GB2312_CHARSET,     MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED),        "\xe9\x80\x89\xe6\x8b\xa9\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90","\xe6\x95\xb0\xe6\x8d\xae\xe6\xba\x90\x3a","\xe9\x80\x89\xe6\x8b\xa9","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CHINESE_TAIWAN,     CHINESEBIG5_CHARSET,MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL),       "\xe9\x81\xb8\xe6\x93\x87\xe5\xbd\xb1\xe5\x83\x8f\xe4\xbe\x86\xe6\xba\x90","\xe5\xbd\xb1\xe5\x83\x8f\xe4\xbe\x86\xe6\xba\x90\x3a","\xe7\xa2\xba\xe5\xae\x9a","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CHINESE_TRADITIONAL,CHINESEBIG5_CHARSET,MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL),       "\xe9\x81\xb8\xe6\x93\x87\xe5\xbd\xb1\xe5\x83\x8f\xe4\xbe\x86\xe6\xba\x90","\xe5\xbd\xb1\xe5\x83\x8f\xe4\xbe\x86\xe6\xba\x90\x3a","\xe7\xa2\xba\xe5\xae\x9a","\xe5\x8f\x96\xe6\xb6\x88"},
      {TWLG_CROATIA,            EASTEUROPE_CHARSET, MAKELANGID(LANG_CROATIAN,SUBLANG_NEUTRAL),                  "Select Source","Sources:","Select","\x4f\x64\x75\x73\x74\x61\x6e\x69"},
      {TWLG_CZECH,              EASTEUROPE_CHARSET, MAKELANGID(LANG_CZECH,SUBLANG_DEFAULT),                     "Select Source","Sources:","Select","\x53\x74\x6f\x72\x6e\x6f"},
      {TWLG_DANISH,             ANSI_CHARSET,       MAKELANGID(LANG_DANISH,SUBLANG_NEUTRAL),                    "\x56\xC3\xA6\x6C\x67\x20\x45\x6E\x68\x65\x64","\x45\x6E\x68\x65\x64","\x56\xE6\x6C\x67","\x41\x6E\x6E\x75\x6C\x6C\x65\x72"},
      {TWLG_DOGRI,              0,                  0,                                                          "","","",""},
      {TWLG_DUTCH,              ANSI_CHARSET,       MAKELANGID(LANG_DUTCH,SUBLANG_DUTCH),                       "\x53\x65\x6c\x65\x63\x74\x65\x65\x72\x20\x62\x72\x6f\x6e","\x42\x72\x6f\x6e\x6e\x65\x6e\x3a","\x53\x65\x6c\x65\x63\x74\x65\x72\x65\x6e","\x41\x6e\x6e\x75\x6c\x65\x72\x65\x6e"},
      {TWLG_DUTCH_BELGIAN,      ANSI_CHARSET,       MAKELANGID(LANG_DUTCH,SUBLANG_DUTCH_BELGIAN),               "\x53\x65\x6c\x65\x63\x74\x65\x65\x72\x20\x62\x72\x6f\x6e","\x42\x72\x6f\x6e\x6e\x65\x6e\x3a","\x53\x65\x6c\x65\x63\x74\x65\x72\x65\x6e","\x41\x6e\x6e\x75\x6c\x65\x72\x65\x6e"},
      {TWLG_ENGLISH,            ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),                   "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_AUSTRALIAN, ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_AUS),               "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_CANADIAN,   ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_CAN),               "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_IRELAND,    ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_EIRE),              "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_NEWZEALAND, ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_NZ),                "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_SOUTHAFRICA,ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_SOUTH_AFRICA),      "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_UK,         ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK),                "Select Source","Sources:","Select","Cancel"},
      {TWLG_ENGLISH_USA,        ANSI_CHARSET,       MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),                "Select Source","Sources:","Select","Cancel"},
      {TWLG_ESTONIAN,           BALTIC_CHARSET,     MAKELANGID(LANG_ESTONIAN,SUBLANG_NEUTRAL),                  "Select Source","Sources:","Select","\x4b\x75\x73\x74\x75\x74\x61"},
      {TWLG_FAEROESE,           EASTEUROPE_CHARSET, MAKELANGID(LANG_FAEROESE,SUBLANG_NEUTRAL),                  "","","",""},
      {TWLG_FARSI,              ARABIC_CHARSET,     MAKELANGID(LANG_FARSI,SUBLANG_NEUTRAL),                     "","","",""},
      {TWLG_FINNISH,            ANSI_CHARSET,       MAKELANGID(LANG_FINNISH,SUBLANG_NEUTRAL),                   "Select Source","Sources:","Select","\x50\x65\x72\x75\x75\x74\x61"},
      {TWLG_FRENCH,             ANSI_CHARSET,       MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH),                     "\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72\x20\x73\x6f\x75\x72\x63\x65","\x53\x6f\x75\x72\x63\x65\x73\x3a","\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72","\x41\x6e\x6e\x75\x6c\x65\x72"},
      {TWLG_FRENCH_BELGIAN,     ANSI_CHARSET,       MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_BELGIAN),             "\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72\x20\x73\x6f\x75\x72\x63\x65","\x53\x6f\x75\x72\x63\x65\x73\x3a","\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72","\x41\x6e\x6e\x75\x6c\x65\x72"},
      {TWLG_FRENCH_CANADIAN,    ANSI_CHARSET,       MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_CANADIAN),            "\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72\x20\x73\x6f\x75\x72\x63\x65","\x53\x6f\x75\x72\x63\x65\x73\x3a","\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72","\x41\x6e\x6e\x75\x6c\x65\x72"},
      {TWLG_FRENCH_LUXEMBOURG,  ANSI_CHARSET,       MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_LUXEMBOURG),          "\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72\x20\x73\x6f\x75\x72\x63\x65","\x53\x6f\x75\x72\x63\x65\x73\x3a","\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72","\x41\x6e\x6e\x75\x6c\x65\x72"},
      {TWLG_FRENCH_SWISS,       ANSI_CHARSET,       MAKELANGID(LANG_FRENCH,SUBLANG_FRENCH_SWISS),               "\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72\x20\x73\x6f\x75\x72\x63\x65","\x53\x6f\x75\x72\x63\x65\x73\x3a","\x53\xc3\xa9\x6c\x65\x63\x74\x69\x6f\x6e\x6e\x65\x72","\x41\x6e\x6e\x75\x6c\x65\x72"},
      {TWLG_GERMAN,             ANSI_CHARSET,       MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN),                     "\x51\x75\x65\x6c\x6c\x65\x20\x77\xc3\xa4\x68\x6c\x65\x6e","\x51\x75\x65\x6c\x6c\x65\x6e\x3a","\x41\x75\x73\x77\xc3\xa4\x68\x6c\x65\x6e","\x41\x62\x62\x72\x65\x63\x68\x65\x6e"},
      {TWLG_GERMAN_AUSTRIAN,    ANSI_CHARSET,       MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_AUSTRIAN),            "\x51\x75\x65\x6c\x6c\x65\x20\x77\xc3\xa4\x68\x6c\x65\x6e","\x51\x75\x65\x6c\x6c\x65\x6e\x3a","\x41\x75\x73\x77\xc3\xa4\x68\x6c\x65\x6e","\x41\x62\x62\x72\x65\x63\x68\x65\x6e"},
      {TWLG_GERMAN_LIECHTENSTEIN,ANSI_CHARSET,      MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_LIECHTENSTEIN),       "\x51\x75\x65\x6c\x6c\x65\x20\x77\xc3\xa4\x68\x6c\x65\x6e","\x51\x75\x65\x6c\x6c\x65\x6e\x3a","\x41\x75\x73\x77\xc3\xa4\x68\x6c\x65\x6e","\x41\x62\x62\x72\x65\x63\x68\x65\x6e"},
      {TWLG_GERMAN_LUXEMBOURG,  ANSI_CHARSET,       MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_LUXEMBOURG),          "\x51\x75\x65\x6c\x6c\x65\x20\x77\xc3\xa4\x68\x6c\x65\x6e","\x51\x75\x65\x6c\x6c\x65\x6e\x3a","\x41\x75\x73\x77\xc3\xa4\x68\x6c\x65\x6e","\x41\x62\x62\x72\x65\x63\x68\x65\x6e"},
      {TWLG_GERMAN_SWISS,       ANSI_CHARSET,       MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_SWISS),               "\x51\x75\x65\x6c\x6c\x65\x20\x77\xc3\xa4\x68\x6c\x65\x6e","\x51\x75\x65\x6c\x6c\x65\x6e\x3a","\x41\x75\x73\x77\xc3\xa4\x68\x6c\x65\x6e","\x41\x62\x62\x72\x65\x63\x68\x65\x6e"},
      {TWLG_GREEK,              GREEK_CHARSET,      MAKELANGID(LANG_GREEK,SUBLANG_DEFAULT),                     "","","",""},
      {TWLG_GUJARATI,           GREEK_CHARSET,      MAKELANGID(LANG_GUJARATI,SUBLANG_DEFAULT),                  "","","",""},
      {TWLG_HARYANVI,           0,                  0,                                                          "","","",""},
      {TWLG_HEBREW,             HEBREW_CHARSET,     MAKELANGID(LANG_HEBREW,SUBLANG_NEUTRAL),                    "","","",""},
      {TWLG_HINDI,              HEBREW_CHARSET,     MAKELANGID(LANG_HINDI,SUBLANG_NEUTRAL),                     "","","",""},
      {TWLG_HUNGARIAN,          EASTEUROPE_CHARSET, MAKELANGID(LANG_HUNGARIAN,SUBLANG_NEUTRAL),                 "Select Source","Sources:","Select","\x4d\xc3\xa9\x67\x73\x65"},
      {TWLG_ICELANDIC,          ANSI_CHARSET,       MAKELANGID(LANG_ICELANDIC,SUBLANG_NEUTRAL),                 "","","",""},
      {TWLG_INDONESIAN,         ANSI_CHARSET,       MAKELANGID(LANG_INDONESIAN,SUBLANG_NEUTRAL),                "\x50\x69\x6c\x69\x74\x68\x20\x53\x75\x6d\x62\x65\x72","\x53\x75\x6d\x62\x65\x72\x3a","\x50\x69\x6c\x69\x74\x68","\x42\x61\x74\x61\x6c"},
      {TWLG_ITALIAN,            ANSI_CHARSET,       MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN),                   "\x53\x65\x6c\x65\x7a\x69\x6f\x6e\x61\x20\x6f\x72\x69\x67\x69\x6e\x65","\x4f\x72\x69\x67\x69\x6e\x69\x3a","\x53\x65\x6c\x65\x7a\x69\x6f\x6e\x61","\x41\x6e\x6e\x75\x6c\x6c\x61"},
      {TWLG_ITALIAN_SWISS,      ANSI_CHARSET,       MAKELANGID(LANG_ITALIAN,SUBLANG_ITALIAN_SWISS),             "\x53\x65\x6c\x65\x7a\x69\x6f\x6e\x61\x20\x6f\x72\x69\x67\x69\x6e\x65","\x4f\x72\x69\x67\x69\x6e\x69\x3a","\x53\x65\x6c\x65\x7a\x69\x6f\x6e\x61","\x41\x6e\x6e\x75\x6c\x6c\x61"},
      {TWLG_JAPANESE,           SHIFTJIS_CHARSET,   MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT),                  "\xe5\x8e\x9f\xe7\xa8\xbf\xe3\x81\xae\xe9\x81\xb8\xe6\x8a\x9e","\xe5\x8e\x9f\xe7\xa8\xbf\x3a","\xe9\x81\xb8\xe6\x8a\x9e","\xe3\x82\xad\xe3\x83\xa3\xe3\x83\xb3\xe3\x82\xbb\xe3\x83\xab"},
      {TWLG_KANNADA,            ANSI_CHARSET,       MAKELANGID(LANG_KANNADA,SUBLANG_NEUTRAL),                   "","","",""},
      {TWLG_KASHMIRI,           ANSI_CHARSET,       MAKELANGID(LANG_KASHMIRI,SUBLANG_NEUTRAL),                  "","","",""},
      {TWLG_KOREAN,             HANGUL_CHARSET,     MAKELANGID(LANG_KOREAN,SUBLANG_KOREAN),                     "\xec\x9e\xa5\xec\xb9\x98\x20\xec\x84\xa0\xed\x83\x9d","\xec\x9e\xa5\xec\xb9\x98","\xec\x84\xa0\xed\x83\x9d","\xec\xb7\xa8\xec\x86\x8c"},
      {TWLG_KOREAN_JOHAB,       JOHAB_CHARSET,      MAKELANGID(LANG_KOREAN,SUBLANG_KOREAN),                     "\xec\x9e\xa5\xec\xb9\x98\x20\xec\x84\xa0\xed\x83\x9d","\xec\x9e\xa5\xec\xb9\x98","\xec\x84\xa0\xed\x83\x9d","\xec\xb7\xa8\xec\x86\x8c"},
      {TWLG_LATVIAN,            BALTIC_CHARSET,     MAKELANGID(LANG_LATVIAN,SUBLANG_NEUTRAL),                   "","","",""},
      {TWLG_LITHUANIAN,         BALTIC_CHARSET,     MAKELANGID(LANG_LITHUANIAN,SUBLANG_NEUTRAL),                "","","",""},
      {TWLG_MALAYALAM,          BALTIC_CHARSET,     MAKELANGID(LANG_MALAYALAM,SUBLANG_NEUTRAL),                 "","","",""},
      {TWLG_MARATHI,            ANSI_CHARSET,       MAKELANGID(LANG_MARATHI,SUBLANG_NEUTRAL),                   "","","",""},
      {TWLG_MARWARI,            0,                  0,                                                          "","","",""},
      {TWLG_MEGHALAYAN,         0,                  0,                                                          "","","",""},
      {TWLG_MIZO,               0,                  0,                                                          "","","",""},
      {TWLG_NAGA,               0,                  0,                                                          "","","",""},
      {TWLG_NORWEGIAN,          ANSI_CHARSET,       MAKELANGID(LANG_NORWEGIAN,SUBLANG_NEUTRAL),                 "","","",""},
      {TWLG_NORWEGIAN_BOKMAL,   ANSI_CHARSET,       MAKELANGID(LANG_NORWEGIAN,SUBLANG_NORWEGIAN_BOKMAL),        "","","",""},
      {TWLG_NORWEGIAN_NYNORSK,  ANSI_CHARSET,       MAKELANGID(LANG_NORWEGIAN,SUBLANG_NORWEGIAN_NYNORSK),       "","","",""},
      {TWLG_ORISSI,             0,                  0,                                                          "","","",""},
      {TWLG_POLISH,             EASTEUROPE_CHARSET, MAKELANGID(LANG_POLISH,SUBLANG_NEUTRAL),                    "Select Source","Sources:","Select","\x41\x6e\x75\x6c\x75\x6a"},
      {TWLG_PORTUGUESE,         EASTEUROPE_CHARSET, MAKELANGID(LANG_PORTUGUESE,SUBLANG_PORTUGUESE),             "\x53\x65\x6c\x65\x63\x69\x6f\x6e\x61\x72\x20\x4f\x72\x69\x67\x65\x6d","\x4f\x72\x69\x67\x65\x6e\x73\x3a","\x53\x65\x6c\x65\x63\x69\x6f\x6e\x61\x72","\x43\x61\x6e\x63\x65\x6c\x61\x72"},
      {TWLG_PORTUGUESE_BRAZIL,  ANSI_CHARSET,       MAKELANGID(LANG_PORTUGUESE,SUBLANG_PORTUGUESE_BRAZILIAN),   "\x53\x65\x6c\x65\x63\x69\x6f\x6e\x61\x72\x20\x4f\x72\x69\x67\x65\x6d","\x4f\x72\x69\x67\x65\x6e\x73\x3a","\x53\x65\x6c\x65\x63\x69\x6f\x6e\x61\x72","\x43\x61\x6e\x63\x65\x6c\x61\x72"},
      {TWLG_PUNJABI,            ANSI_CHARSET,       MAKELANGID(LANG_PUNJABI,SUBLANG_NEUTRAL),                   "","","",""},
      {TWLG_PUSHTU,             0,                  0,                                                          "","","",""},
      {TWLG_ROMANIAN,           EASTEUROPE_CHARSET, MAKELANGID(LANG_ROMANIAN,SUBLANG_NEUTRAL),                  "","","",""},
      {TWLG_RUSSIAN,            RUSSIAN_CHARSET,    MAKELANGID(LANG_RUSSIAN,SUBLANG_DEFAULT),                   "\xd0\x92\xd1\x8b\xd0\xb1\xd1\x80\xd0\xb0\xd1\x82\xd1\x8c\x20\xd0\xb8\xd1\x81\xd1\x82\xd0\xbe\xd1\x87\xd0\xbd\xd0\xb8\xd0\xba","\xd0\x98\xd1\x81\xd1\x82\xd0\xbe\xd1\x87\xd0\xbd\xd0\xb8\xd0\xba\xd0\xb8\x3a","\xd0\x92\xd1\x8b\xd0\xb1\xd1\x80\xd0\xb0\xd1\x82\xd1\x8c","\xd0\x9e\xd1\x82\xd0\xbc\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x82\xd1\x8c"},
      {TWLG_SERBIAN_CYRILLIC,   ANSI_CHARSET,       MAKELANGID(LANG_SERBIAN,SUBLANG_SERBIAN_CYRILLIC),          "","","",""},
      {TWLG_SERBIAN_LATIN,      EASTEUROPE_CHARSET, MAKELANGID(LANG_SERBIAN,SUBLANG_SERBIAN_LATIN),             "","","",""},
      {TWLG_SIKKIMI,            0,                  0,                                                          "","","",""},
      {TWLG_SLOVAK,             EASTEUROPE_CHARSET, MAKELANGID(LANG_SLOVAK,SUBLANG_NEUTRAL),                    "","","",""},
      {TWLG_SLOVENIAN,          EASTEUROPE_CHARSET, MAKELANGID(LANG_SLOVENIAN,SUBLANG_NEUTRAL),                 "Select Source","Sources:","Select","\x50\x72\x65\x6b\x69\x6e\x69"},
      {TWLG_SPANISH,            ANSI_CHARSET,       MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH),                   "\x53\x65\x6c\x65\x63\x63\x69\xc3\xb3\x6e\x20\x64\x65\x20\x66\x75\x65\x6e\x74\x65","\x46\x75\x65\x6e\x74\x65\x73\x3a","\x53\x65\x6c\x65\x63\x63\x69\x6f\x6e\x61\x72","\x43\x61\x6e\x63\x65\x6c\x61\x72"},
      {TWLG_SPANISH_MEXICAN,    ANSI_CHARSET,       MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH_MEXICAN),           "\x53\x65\x6c\x65\x63\x63\x69\xc3\xb3\x6e\x20\x64\x65\x20\x66\x75\x65\x6e\x74\x65","\x46\x75\x65\x6e\x74\x65\x73\x3a","\x53\x65\x6c\x65\x63\x63\x69\x6f\x6e\x61\x72","\x43\x61\x6e\x63\x65\x6c\x61\x72"},
      {TWLG_SPANISH_MODERN,     ANSI_CHARSET,       MAKELANGID(LANG_SPANISH,SUBLANG_SPANISH_MODERN),            "\x53\x65\x6c\x65\x63\x63\x69\xc3\xb3\x6e\x20\x64\x65\x20\x66\x75\x65\x6e\x74\x65","\x46\x75\x65\x6e\x74\x65\x73\x3a","\x53\x65\x6c\x65\x63\x63\x69\x6f\x6e\x61\x72","\x43\x61\x6e\x63\x65\x6c\x61\x72"},
      {TWLG_SWEDISH,            ANSI_CHARSET,       MAKELANGID(LANG_SWEDISH,SUBLANG_SWEDISH),                   "Select Source","Sources:","Select","\x41\x76\x62\x72\x79\x74"},
      {TWLG_SWEDISH_FINLAND,    ANSI_CHARSET,       MAKELANGID(LANG_SWEDISH,SUBLANG_SWEDISH_FINLAND),           "Select Source","Sources:","Select","\x41\x76\x62\x72\x79\x74"},
      {TWLG_TAMIL,              ANSI_CHARSET,       MAKELANGID(LANG_TAMIL,SUBLANG_NEUTRAL),                     "","","",""},
      {TWLG_TELUGU,             ANSI_CHARSET,       MAKELANGID(LANG_TELUGU,SUBLANG_NEUTRAL),                    "","","",""},
      {TWLG_THAI,               THAI_CHARSET,       MAKELANGID(LANG_THAI,SUBLANG_NEUTRAL),                      "","","",""},
      {TWLG_TRIPURI,            0,                  0,                                                          "","","",""},
      {TWLG_TURKISH,            TURKISH_CHARSET,    MAKELANGID(LANG_TURKISH,SUBLANG_DEFAULT),                   "\x4b\x61\x79\x6e\x61\x6b\x20\x73\x65\xc3\xa7\x69\x6e\x69\x7a","\x4b\x61\x79\x6e\x61\x6b","\x53\x65\xc3\xa7\x69\x6e\x69\x7a","\xc4\xb0\x70\x74\x61\x6c"},
      {TWLG_UKRANIAN,           RUSSIAN_CHARSET,    MAKELANGID(LANG_UKRAINIAN,SUBLANG_NEUTRAL),                 "","","",""},
      {TWLG_URDU,               ANSI_CHARSET,       MAKELANGID(LANG_URDU,SUBLANG_NEUTRAL),                      "","","",""},
      {TWLG_VIETNAMESE,         VIETNAMESE_CHARSET, MAKELANGID(LANG_VIETNAMESE,SUBLANG_NEUTRAL),                "","","",""},
      {-1, 0, 0, 0, 0, 0, 0} // must be last...
};
#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    // We don't have anything for here...
#else
    #error Sorry, we do not recognize this system...
#endif
#if (TWNDSM_OS == TWNDSM_OS_MACOSX)
#pragma clang diagnostic pop
#endif

#if TWNDSM_OS_64BIT
  #define TWNDSM_DS_REG_LOC "Software\\Microsoft\\Windows NT\\CurrentVersion\\TWAIN64"
#else
  #define TWNDSM_DS_REG_LOC "Software\\Microsoft\\Windows NT\\CurrentVersion\\TWAIN"
#endif


/**
* @defgroup MemFunctions declarations for our memory management functions...
* @{
*/

#if 0
/**
* Memory Allocate
* @param[in] _size the size of the memory to allocate
* @return the handle to the memory allocated
*/
static TW_HANDLE PASCAL DSM_MemAllocate(TW_UINT32 _size);

/**
* Memory Free
* @param[in] _handle the handle to the memory to free
*/
static void PASCAL DSM_MemFree(TW_HANDLE _handle);

/**
* Memory Lock
* @param[in] _handle the handle to the memory to lock
* @return locked pointer to the memory in the handle
*/
static TW_MEMREF PASCAL DSM_MemLock(TW_HANDLE _handle);

/**
* Memory Unlock
* @param[in] _handle the handle to the memory to unlock
*/
static void PASCAL DSM_MemUnlock(TW_HANDLE _handle);
// @}
#endif

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
      if (   ( (_MSG == MSG_GET) || (_MSG == MSG_CHECKSTATUS) )
          && (_DAT == DAT_STATUS)
          && (_DG  == DG_CONTROL)
          && (0 != _pData)
          )
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
  // and don't have any other applications with it open,
  // then destroy our object.  We don't want to have any resources
  // lingering around after we destroy our CTwnDsm object!!!
  if (   (TWRC_SUCCESS == rcDSM)
      && (_MSG == MSG_CLOSEDSM)
      && (_DAT == DAT_PARENT)
      && (_DG  == DG_CONTROL)
      && g_ptwndsm->DSMGetState() != dsmState_Open )
  {
      delete g_ptwndsm;
      g_ptwndsm = 0;
  }

  // All done...
  return rcDSM;
}



/*
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
  kLOG((kLOGINFO,"version: %s",TWNDSM_VERSION_STR));

  // Get our application object...
  pod.m_ptwndsmapps = new CTwnDsmApps();
  if (!pod.m_ptwndsmapps)
  {
      kPANIC("Failed to new CTwnDsmApps!!!");
  }
}



/*
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



/*
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
  TW_CALLBACK2 *ptwcallback2;
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
  bPrinted = printTripletsInfo(_pOrigin,_pDest,_DG,_DAT,_MSG,_pData);

  // Sniff for the application forwarding an event to the
  // DS. It may be possible that the app has a message waiting for
  // it because it didn't register a callback.
  if (   (DAT_EVENT == _DAT)
      && (MSG_PROCESSEVENT == _MSG))
  {
    // Check that the AppID and DSID are valid...
    if (!pod.m_ptwndsmapps->AppValidateIds(pAppId,pDSId))
    {
      kLOG((kLOGINFO,"Bad TW_IDENTITY"));
      pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADPROTOCOL);
      rcDSM = TWRC_FAILURE;
    }
    else if (pod.m_ptwndsmapps->DsCallbackIsWaiting(pAppId,(TWID_T)pDSId->Id))
    {
      ptwcallback2 = pod.m_ptwndsmapps->DsCallback2Get(pAppId,(TWID_T)pDSId->Id);
      ((TW_EVENT*)(_pData))->TWMessage = ptwcallback2->Message;
      if( g_ptwndsmlog )
      {
        char szMsg[64];
        StringFromMsg(szMsg,NCHARS(szMsg),ptwcallback2->Message);
        kLOG((kLOGINFO,"%.32s retrieving DAT_EVENT / %s\n", pAppId->ProductName, szMsg));
      }
      ptwcallback2->Message = 0;
      pod.m_ptwndsmapps->DsCallbackSetWaiting(pAppId,(TWID_T)pDSId->Id,FALSE);
      rcDSM = TWRC_DSEVENT;
    }
    // No callback, so fall on through...
  }

  // Is this msg for us?
  if( TWRC_SUCCESS == rcDSM )
  {
  switch (_DAT)
    {
      case DAT_IDENTITY:
        // If the pDSId is 0 then the message is intended for us.  We're
        // going to force the matter if _MSG is MSG_CLOSEDS, otherwise
        // we send the MSG_CLOSEDS to the driver, but never process it
        // ourselves, which seems like a terrible idea...
        if ((pDSId == 0) || (_MSG == MSG_CLOSEDS))
        {
          rcDSM = DSM_Identity(pAppId,_MSG,(TW_IDENTITY*)_pData);
          break;
        }
        // else we fall thru to send the message onto the DS

      default:
        // check if the application is open or not.  If it isn't, we have a bad sequence
        if (dsmState_Open == pod.m_ptwndsmapps->AppGetState(pAppId))
        {
            // Check that the AppID and DSID are valid...
            if (!pod.m_ptwndsmapps->AppValidateIds(pAppId,pDSId))
            {
              kLOG((kLOGINFO,"Bad TW_IDENTITY"));
              pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADPROTOCOL);
              rcDSM = TWRC_FAILURE;
            }

            // Issue the command...
            else if (0 != pod.m_ptwndsmapps->DsGetEntryProc(pAppId,(TWID_T)pDSId->Id))
            {
              // Don't send a new message if the DS is still processing a previous message
              // or if the application has not returned back from recieving callback.
              // Place a Try | Catch around the function so we can maintain correct state 
              // in the case of an exception
			  //
			  // We are only enforcing this new behavior for TWAIN 2.2 applications and
			  // and higher.  Older apps can still use the 'wrong' behavior.  We need this
			  // to preserve backwards compability, and to give ourselves a chance to
			  // inform developers of the new requirement...
			  //
              if(((((pAppId->ProtocolMajor*10) + (pAppId->ProtocolMinor)) <= 201) || 
                     !pod.m_ptwndsmapps->DsIsProcessingMessage(pAppId,(TWID_T)pDSId->Id)     ) && 
                 ((((pAppId->ProtocolMajor*10) + (pAppId->ProtocolMinor)) <= 202) ||
                     !pod.m_ptwndsmapps->DsIsAppProcessingCallback(pAppId,(TWID_T)pDSId->Id) )    ) 
              {
                pod.m_ptwndsmapps->DsSetProcessingMessage(pAppId,(TWID_T)pDSId->Id,TRUE);
                try
                {
                  // Create a local copy of the AppIdentity
                  TW_IDENTITY AppId = *pod.m_ptwndsmapps->AppGetIdentity(pAppId);

                  rcDSM = (pod.m_ptwndsmapps->DsGetEntryProc(&AppId,(TWID_T)pDSId->Id))(
                                          &AppId,
                                          _DG,
                                          _DAT,
                                          _MSG,
                                          _pData);
                }
                catch(...)
                {
                  rcDSM = TWRC_FAILURE;
                  pod.m_ptwndsmapps->AppSetConditionCode(pAppId,TWCC_BUMMER);
                  kLOG((kLOGERR,"Exception caught while DS was processing message.  Returning Failure."));
                }
                pod.m_ptwndsmapps->DsSetProcessingMessage(pAppId,(TWID_T)pDSId->Id,FALSE);
              }
              else
              {
                if( _DAT == DAT_EVENT && _MSG == MSG_PROCESSEVENT)
                {
                  kLOG((kLOGINFO,"Nested DAT_EVENT / MSG_PROCESSEVENT Ignored"));
                  rcDSM = TWRC_NOTDSEVENT;
                  ((TW_EVENT*)(_pData))->TWMessage = MSG_NULL;
                }
                else
                {
                  kLOG((kLOGERR,"Nested calls back to the DS.  Returning Failure."));
                  pod.m_ptwndsmapps->AppSetConditionCode(pAppId,TWCC_SEQERROR);
                  rcDSM = TWRC_FAILURE;
                }
              }
            }

            // For some reason we have no pointer to the dsentry function...
            else
            {
              kLOG((kLOGERR,"Unable to find driver, check your AppId and DsId values..."));
              pod.m_ptwndsmapps->AppSetConditionCode(pAppId,TWCC_OPERATIONERROR);
              kLOG((kLOGERR,"DS_Entry is null...%ld",(TWID_T)pAppId->Id));
              rcDSM = TWRC_FAILURE;
            }
        }
        else
        {
            kLOG((kLOGINFO,"DS is not open"));
            pod.m_ptwndsmapps->AppSetConditionCode(pAppId,TWCC_SEQERROR);
            rcDSM = TWRC_FAILURE;
        }
        break;

      case DAT_PARENT:
        rcDSM = DSM_Parent(pAppId,_MSG,_pData);
        break;

      case DAT_TWUNKIDENTITY:
        rcDSM = DSM_TwunkIdentity(pAppId,_MSG,(TW_TWUNKIDENTITY*)_pData);
        break;

      case DAT_ENTRYPOINT:
        rcDSM = DSM_Entrypoint(pAppId,_MSG,(TW_ENTRYPOINT*)_pData);
        break;

      case DAT_STATUS:
        if( _MSG == MSG_CHECKSTATUS )
        {
          _MSG = MSG_GET;
          kLOG((kLOGINFO, "MSG_CHECKSTATUS is Depreciated using MSG_GET"));
        }

        // If we get a DSId then it is intended to be passed along to the driver.
        // If the DSId is null then the request is handled by the DSM
        // If we're talking to a driver (state 4 or higher), then we
        // will pass the DAT_STATUS request down to it...
        if (  0 != pDSId
          &&  (dsmState_Open == pod.m_ptwndsmapps->AppGetState(pAppId))
          &&  pod.m_ptwndsmapps->AppValidateIds(pAppId,pDSId)
          &&  (0 != pod.m_ptwndsmapps->DsGetEntryProc(pAppId,(TWID_T)pDSId->Id)))
        {
          // Create a local copy of the AppIdentity
          TW_IDENTITY AppId = *pod.m_ptwndsmapps->AppGetIdentity(pAppId);

          rcDSM = (pod.m_ptwndsmapps->DsGetEntryProc(&AppId,(TWID_T)pDSId->Id))(
                                  &AppId,
                                  _DG,
                                  _DAT,
                                  _MSG,
                                  _pData);
        }
        // Otherwise, handle it ourself...
        else
        {
          rcDSM = DSM_Status(pAppId,_MSG,(TW_STATUS*)_pData);
        }
        break;

      case DAT_CALLBACK:
        // DAT_CALLBACK can be either from an Application registering its Callback, 
        // or from a DS Invoking a request to send a message to the Application
        rcDSM = DSM_Callback(_pOrigin,_pDest,_MSG,(TW_CALLBACK*)_pData);
        break;

      case DAT_CALLBACK2:
        // DAT_CALLBACK2 can be either from an Application registering its Callback, 
        // or from a DS Invoking a request to send a message to the Application
        rcDSM = DSM_Callback2(_pOrigin,_pDest,_MSG,(TW_CALLBACK2*)_pData);
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
    printResults(_DG,_DAT,_MSG,_pData,rcDSM);
  }

  return rcDSM;
}

/*
* Return the state of the DSM by checking the state of all applications
*/
DSM_State CTwnDsm::DSMGetState()
{
  DSM_State CurrentState = pod.m_ptwndsmapps->AppGetState();
  return CurrentState;
}

/*
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



/*
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



/*
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
  if (0 == _pAppId || (TWID_T)_pAppId->Id >= pod.m_ptwndsmapps->AppGetNumApp())
  {
      kLOG((kLOGERR,"_pAppId is null"));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
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

      case MSG_SET:
        result = DSM_SetDefaultDS(_pAppId,_pDsId);
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

/*
* Handle DAT_TWUNKIDENTITY.  This is here for backwards compatibility. 
* DAT_TWUNKIDENTITY is undocumented.  It was used by the Twunking
* layer.  Some old applications use it to get the path to the DS.
* We need to continue to support it.
*/
TW_INT16 CTwnDsm::DSM_TwunkIdentity(TW_IDENTITY  *_pAppId,
                                    TW_UINT16     _MSG,
                                TW_TWUNKIDENTITY *_pTwunkId)
{
  TW_INT16  result = TWRC_SUCCESS;

  // Validate...
  if (0 == _pAppId || (TWID_T)_pAppId->Id >= pod.m_ptwndsmapps->AppGetNumApp())
  {
    kLOG((kLOGERR,"_pAppId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (dsmState_Open != pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
    return TWRC_FAILURE;
  }
  else if (MSG_GET != _MSG)
  {
    kLOG((kLOGERR,"protocol error"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
    return TWRC_FAILURE;
  }
  else if (0 == _pTwunkId)
  {
    kLOG((kLOGERR,"_pTwunkId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  // Check that the DSID is valid...
  else if (!pod.m_ptwndsmapps->AppValidateIds(_pAppId,&_pTwunkId->identity))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(0,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  SSTRCPY((char*)_pTwunkId->dsPath, sizeof(_pTwunkId->dsPath),(char*)pod.m_ptwndsmapps->DsGetPath(_pAppId,(TWID_T)_pTwunkId->identity.Id));

  return result;
}



/*
* Handle DAT_ENTRYPOINT.  This handles an application asking
* for entry point information.  Drivers have this information
* pushed to them in a different part of the code (just before
* DG_CONTORL/DAT_IDENTITY/MSG_OPENDS is received)
*/
TW_INT16 CTwnDsm::DSM_Entrypoint(TW_IDENTITY    *_pAppId,
                                 TW_UINT16      _MSG,
                                 TW_ENTRYPOINT  *_pEntrypoint)
{
  // Validate...
  if (0 == _pAppId)
  {
    kLOG((kLOGERR,"_pAppId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (MSG_GET != _MSG)
  {
    kLOG((kLOGERR,"protocol error"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
    return TWRC_FAILURE;
  }
  else if (0 == _pEntrypoint)
  {
    kLOG((kLOGERR,"_pEntrypoint is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (0 == _pEntrypoint->Size)
  {
    kLOG((kLOGERR,"_pEntrypoint is zero, it needs to be set to the size of TW_ENTRYPOINT..."));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (!(_pAppId->SupportedGroups & DF_APP2))
  {
    kLOG((kLOGERR,"_pAppId->SupportedGroups must include the DF_APP2 flag to make this call..."));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADPROTOCOL);
    return TWRC_FAILURE;
  }

  // This is the TWAIN 2.0 minimum size.  If we add more values
  // in future we should create a new structure (ex: TW_ENTRYPOINT2)
  // and then add an if-statement for it...
  if (_pEntrypoint->Size < sizeof(TW_ENTRYPOINT))
  {
    kLOG((kLOGERR,"_pEntrypoint->Size minimum is %ld, we got %ld...",sizeof(TW_ENTRYPOINT),_pEntrypoint->Size));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  // TWAIN 2.0
  // Stock the structure with interesting stuff...
  else if (_pEntrypoint->Size == sizeof(TW_ENTRYPOINT))
  {
    _pEntrypoint->DSM_Entry        = ::DSM_Entry;
    _pEntrypoint->DSM_MemAllocate  = DSM_MemAllocate;
    _pEntrypoint->DSM_MemFree      = DSM_MemFree;
    _pEntrypoint->DSM_MemLock      = DSM_MemLock;
    _pEntrypoint->DSM_MemUnlock    = DSM_MemUnlock;
  }

  // Uh-oh...
  else
  {
    kLOG((kLOGERR,"_pEntrypoint->Size cannot be larger than %ld, we got %ld...",sizeof(TW_ENTRYPOINT),_pEntrypoint->Size));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  // All done...
  return TWRC_SUCCESS;
}



/*
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
  TW_CALLBACK2 *ptwcallback2;

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

        ptwcallback2 = pod.m_ptwndsmapps->DsCallback2Get(_pOrigin,(TWID_T)_pDest->Id);
        ptwcallback2->CallBackProc = ((TW_CALLBACK*)_pData)->CallBackProc;
        ptwcallback2->RefCon = (TWID_T)((TW_CALLBACK*)_pData)->RefCon;
        ptwcallback2->Message = ((TW_CALLBACK*)_pData)->Message;
        pod.m_ptwndsmapps->DsCallbackSetWaiting(_pOrigin,(TWID_T)_pDest->Id,FALSE);
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



/*
* We've received a callback.  MSG_REGISTER_CALLBACK are from
* the Application and MSG_INVOKE_CALLBACK (Mac OSx only) 
* are from the DS.
* If Callbacks have not been registered by the App then when the 
* DS Invokes a callback, make a note of it so the next time the 
* application hits us with a Windows message for us to process, 
* we can send it the callback message...
*/
TW_INT16 CTwnDsm::DSM_Callback2(TW_IDENTITY *_pOrigin,
                               TW_IDENTITY *_pDest,
                               TW_UINT16    _MSG,
                               TW_CALLBACK2 *_pData)
{
  TW_INT16      result;
  TW_CALLBACK2 *ptwcallback2;

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

        ptwcallback2 = pod.m_ptwndsmapps->DsCallback2Get(_pOrigin,(TWID_T)_pDest->Id);
        memcpy(ptwcallback2,_pData,sizeof(*ptwcallback2));
        pod.m_ptwndsmapps->DsCallbackSetWaiting(_pOrigin,(TWID_T)_pDest->Id,FALSE);
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

        ptwcallback2 = (TW_CALLBACK2*)_pData;
        result = DSM_Null(_pDest,_pOrigin,ptwcallback2->Message);
      }
      break;

    default:
      result = TWRC_FAILURE;
      pod.m_ptwndsmapps->AppSetConditionCode(_pOrigin,TWCC_BADPROTOCOL);
      break;
  }

  return result;
}



/*
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
  TW_INT16      result;
  TW_ENTRYPOINT twentrypoint;
 
  // Validate...
  if (0 == _pAppId)
  {
      kLOG((kLOGERR,"_pAppId is null"));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
      return TWRC_FAILURE;
  }
  else if (   ((TWID_T)_pAppId->Id < 1)
           || ((TWID_T)_pAppId->Id >= pod.m_ptwndsmapps->AppGetNumApp()))
  {
      kLOG((kLOGERR,"id is out of range...%d",(int)(TWID_T)_pAppId->Id));
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_MAXCONNECTIONS);
      return TWRC_FAILURE;
  }

  // Init stuff...
  result = TWRC_SUCCESS;

  // check that we are in the proper state
  if (dsmState_Open != pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    kLOG((kLOGERR,"DSM must be open before opening DS"));
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
  if (0 == (TWID_T)_pDsId->Id)
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
        return result;
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
        return result;
      }
    }
  }

  // Load the driver...
  result = pod.m_ptwndsmapps->LoadDS(_pAppId,(TWID_T)_pDsId->Id);
  if (result != TWRC_SUCCESS)
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_NODS);
    return TWRC_FAILURE;
  }

  // open the ds
  if (0 != pod.m_ptwndsmapps->DsGetEntryProc(_pAppId,(TWID_T)_pDsId->Id))
  {
    // If the DS reports support for DF_DS2, then send it our DAT_ENTRYPOINT
    // information.  Failure to handle this is treated like a failure to open...
    result = TWRC_SUCCESS;

    // Create a local copy of the AppIdentity
    TW_IDENTITY AppId = *pod.m_ptwndsmapps->AppGetIdentity(_pAppId);

    if (_pDsId->SupportedGroups & DF_DS2)
    {
      memset(&twentrypoint,0,sizeof(twentrypoint));
      twentrypoint.Size = sizeof(TW_ENTRYPOINT);
      twentrypoint.DSM_Entry        = ::DSM_Entry;
      twentrypoint.DSM_MemAllocate  = DSM_MemAllocate;
      twentrypoint.DSM_MemFree      = DSM_MemFree;
      twentrypoint.DSM_MemLock      = DSM_MemLock;
      twentrypoint.DSM_MemUnlock    = DSM_MemUnlock;
      result = pod.m_ptwndsmapps->DsGetEntryProc(&AppId,(TWID_T)_pDsId->Id)(
                                &AppId,
                                DG_CONTROL,
                                DAT_ENTRYPOINT,
                                MSG_SET,
                                (TW_MEMREF)&twentrypoint);
    }

    // We have a problem...
    if (TWRC_SUCCESS != result)
    {
      kLOG((kLOGERR,"DAT_ENTRYPOINT failed..."));
      pod.m_ptwndsmapps->AppSetConditionCode(&AppId,TWCC_OPERATIONERROR);
    }

    // Okay, we're good.  Either we're a 1.x driver, or we were able to
    // push down our entrypoint info, so open the ds...
    else
    {
      result = pod.m_ptwndsmapps->DsGetEntryProc(&AppId,(TWID_T)_pDsId->Id)(
                                &AppId,
                                DG_CONTROL,
                                DAT_IDENTITY,
                                MSG_OPENDS,
                                (TW_MEMREF)_pDsId);

      // Oh well...
      if (TWRC_SUCCESS != result)
      {
        kLOG((kLOGINFO,"MSG_OPENDS failed..."));
        TW_UINT16  rcDSMStatus;
		TW_STATUS  twstatus = { 0, { 0 } };
        // If the call to MSG_OPENDS fails, then we need to get the DAT_STATUS and squirrel
        // it away, because we're going to close this data source soon...
        rcDSMStatus = (pod.m_ptwndsmapps->DsGetEntryProc(&AppId,(TWID_T)_pDsId->Id))(
					              &AppId,
					              DG_CONTROL,
					              DAT_STATUS,
					              MSG_GET,
					              (TW_MEMREF)&twstatus);
        if (rcDSMStatus == TWRC_SUCCESS)
        {
          pod.m_ptwndsmapps->AppSetConditionCode(&AppId,twstatus.ConditionCode);
        }
        else
        {
          pod.m_ptwndsmapps->AppSetConditionCode(&AppId,TWCC_NODS);
        }
      }
    }
  }

  // Remember that we opened this DS...
  if (TWRC_SUCCESS == result)
  {
    // Starting with TWAIN 2.1 the application will use DAT_IDENTITY / MSG_SET
    // to set the default DS
    if( ( _pAppId->ProtocolMajor == 2
       && _pAppId->ProtocolMinor == 0 )
     || _pAppId->ProtocolMajor < 2 ) 
    {
      #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
        // skip...
      #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
		int iResult;
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
            iResult = fwrite
			(
				pod.m_ptwndsmapps->DsGetPath(_pAppId,(TWID_T)_pDsId->Id),
                1,
                strlen(pod.m_ptwndsmapps->DsGetPath(_pAppId,(TWID_T)_pDsId->Id)),
                pfile
			);
			if (iResult < (int)strlen(pod.m_ptwndsmapps->DsGetPath(_pAppId,(TWID_T)_pDsId->Id)))
			{
				kLOG((kLOGERR,"fwrite defaultds failed..."));
			}
            fclose(pfile);
          }
        }
      #else
        #error Sorry, we do not recognize this system...
      #endif
    }
  }

  // If we had an error, make sure we unload the ds...
  else
  {
    pod.m_ptwndsmapps->UnloadDS(_pAppId,(TWID_T)_pDsId->Id);
  }

  // All done...
  return result;
}



/*
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
  else if (   ((TWID_T)_pAppId->Id < 1)
           || ((TWID_T)_pAppId->Id >= pod.m_ptwndsmapps->AppGetNumApp()))
  {
    kLOG((kLOGERR,"id out of range...%d",(int)(TWID_T)_pAppId->Id));
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
  if (0 != pod.m_ptwndsmapps->DsGetEntryProc(_pAppId,(TWID_T)_pDsId->Id))
  {
    // Create a local copy of the AppIdentity
    TW_IDENTITY AppId = *pod.m_ptwndsmapps->AppGetIdentity(_pAppId);

    result = (pod.m_ptwndsmapps->DsGetEntryProc(&AppId,(TWID_T)_pDsId->Id))(
                             &AppId,
                             DG_CONTROL,
                             DAT_IDENTITY,
                             MSG_CLOSEDS,
                             (TW_MEMREF)_pDsId);

    if (TWRC_SUCCESS != result)
    {
      pod.m_ptwndsmapps->AppSetConditionCode(&AppId,TWCC_OPERATIONERROR);
      return result;
    }

    // Cleanup...
    pod.m_ptwndsmapps->UnloadDS(&AppId,(TWID_T)_pDsId->Id);
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
* @param[in] _hmodule handle to the application that loaded the DSM
* @param[in] _dwReasonCalled why this function is being called
* @return TRUE
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
      if( g_ptwndsm )
      {
        if( g_ptwndsm->DSMGetState() == dsmState_Open )
        {
          // This should never happen!
          // The Application should always close any open DS, then Close the DSM.
          kLOG((kLOGERR,"The DSM was left in an open state when it was unloaded!"));
        }
        delete g_ptwndsm;
        g_ptwndsm = 0;
      }
      break;
  }
  return(TRUE);
}
#elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    // Nothing for us to do...
#else
    #error Sorry, we do not recognize this system...
#endif



#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
/**
* We support a selection dialog on Windows.  I wish we didn't, it's
* more trouble than it's worth, but it's part of that legacy thing.
* This function is properly constructed for use with DialogBox...
* @param[in] _hWnd Window handle of the dialog
* @param[in] _Message message
* @param[in] _wParam wparam
* @param[in] _lParam lparam
* @return FALSE if we processed the message
*/
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
  #error Sorry, we do not recognize this system...
#endif



#if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
/**
* We support a selection dialog on Windows.  This function is
* part of our CTwnDsm class, so we don't have to have a lot
* of pointers, and we can keep things private, unlike what we
* would have to do if we put this code into the function we
* actually pass to DialogBox...
* @param[in] hwnd Window handle of the dialog
* @param[in] lParam lparam
* @return TRUE
*/
BOOL CALLBACK EnumChildProc
(
  HWND hwnd,
  LPARAM lParam
)
{
  ::SendMessage(hwnd,WM_SETFONT,(WPARAM)lParam,(LPARAM)TRUE);
  return TRUE;
}



BOOL CTwnDsm::SelectDlgProc(HWND hWnd,
                            UINT Message,
                            WPARAM wParam,
                            LPARAM /*lParam - unused*/)
{
  TW_IDENTITY  *pAppId;
  int           nIndex;
  int           nSelect;
  TW_UINT32     x;
  char         *szProductName;
  HWND          hListBox;
  RECT          rectParent;
  RECT          rectSelect;
  HWND          hParent;
  LANGID        LangId;
  wchar_t       uzUnicode[128];
  HFONT         hfont;
  LOGFONT       lf;
  POINT         point;
  int           nWidth;
  int           nHeight;

  // Init stuff...
  nSelect = 0;
  pAppId = pod.m_pSelectDlgAppId;

  // Process the message...
  switch (Message)
  {
    case WM_INITDIALOG:
      
      // If the caller wants us to figure out the language, do this...
      if (pAppId->Version.Language == (TW_UINT16)TWLG_USERLOCALE)
      {
      // Try to find the user's default language...
      LangId = ::GetUserDefaultLangID();
      for (nIndex = 0;
           s_twlocalize[nIndex].Language >= 0;
           nIndex++)
      {
        if (    (s_twlocalize[nIndex].LangId == LangId)
            &&  s_twlocalize[nIndex].LangId
            &&  s_twlocalize[nIndex].Title
            &&  s_twlocalize[nIndex].Title[0]
            &&  s_twlocalize[nIndex].Sources
            &&  s_twlocalize[nIndex].Sources[0]
            &&  s_twlocalize[nIndex].Select
            &&  s_twlocalize[nIndex].Select[0]
            &&  s_twlocalize[nIndex].Cancel
            &&  s_twlocalize[nIndex].Cancel[0])
        {
            break;
        }
      }

      // If that doesn't work, try for the primary language...
      if (s_twlocalize[nIndex].Language < 0)
      {
        LangId &= 0xFF;
        for (nIndex = 0;
             s_twlocalize[nIndex].Language >= 0;
             nIndex++)
        {
          if (  ((s_twlocalize[nIndex].LangId & 0xFF) == LangId)
            &&  s_twlocalize[nIndex].LangId
            &&  s_twlocalize[nIndex].Title
            &&  s_twlocalize[nIndex].Title[0]
            &&  s_twlocalize[nIndex].Sources
            &&  s_twlocalize[nIndex].Sources[0]
            &&  s_twlocalize[nIndex].Select
            &&  s_twlocalize[nIndex].Select[0]
            &&  s_twlocalize[nIndex].Cancel
            &&  s_twlocalize[nIndex].Cancel[0])
          {
            break;
          }
        }
      }
    }

    // Otherwise, use whatever the caller gave us...
    else
    {
      for (nIndex = 0;
           s_twlocalize[nIndex].Language >= 0;
           nIndex++)
      {
        if (    (s_twlocalize[nIndex].Language == pAppId->Version.Language)
            &&  s_twlocalize[nIndex].LangId
            &&  s_twlocalize[nIndex].Title
            &&  s_twlocalize[nIndex].Title[0]
            &&  s_twlocalize[nIndex].Sources
            &&  s_twlocalize[nIndex].Sources[0]
            &&  s_twlocalize[nIndex].Select
            &&  s_twlocalize[nIndex].Select[0]
            &&  s_twlocalize[nIndex].Cancel
            &&  s_twlocalize[nIndex].Cancel[0])
        {
          break;
        }
      }
    }

    // If we didn't find our language, go for English...
    if (s_twlocalize[nIndex].Language < 0)
    {
      for (nIndex = 0;
           s_twlocalize[nIndex].Language >= 0;
           nIndex++)
      {
        if (    (s_twlocalize[nIndex].Language == TWLG_ENGLISH)
            &&  s_twlocalize[nIndex].LangId
            &&  s_twlocalize[nIndex].Title
            &&  s_twlocalize[nIndex].Title[0]
            &&  s_twlocalize[nIndex].Sources
            &&  s_twlocalize[nIndex].Sources[0]
            &&  s_twlocalize[nIndex].Select
            &&  s_twlocalize[nIndex].Select[0]
            &&  s_twlocalize[nIndex].Cancel
            &&  s_twlocalize[nIndex].Cancel[0])
        {
          break;
        }
      }
    }

    // If we found something, then use it...
    if (s_twlocalize[nIndex].Language >= 0)
    {
      // Set our font...
      memset(&lf,0,sizeof(lf));
      lf.lfHeight = 16;
      lf.lfCharSet = s_twlocalize[nIndex].CharSet;
      hfont = CreateFontIndirect(&lf);
      EnumChildWindows(hWnd,EnumChildProc,(LPARAM)hfont);

      MultiByteToWideChar(CP_UTF8,0,s_twlocalize[nIndex].Title,-1,uzUnicode,sizeof(uzUnicode)/sizeof(*uzUnicode));
      SetWindowTextW(hWnd,uzUnicode);

      MultiByteToWideChar(CP_UTF8,0,s_twlocalize[nIndex].Sources,-1,uzUnicode,sizeof(uzUnicode)/sizeof(*uzUnicode));
      SetWindowTextW(::GetDlgItem(hWnd,IDC_STATIC),uzUnicode);

      MultiByteToWideChar(CP_UTF8,0,s_twlocalize[nIndex].Select,-1,uzUnicode,sizeof(uzUnicode)/sizeof(*uzUnicode));
      SetWindowTextW(::GetDlgItem(hWnd,IDOK),uzUnicode);

      MultiByteToWideChar(CP_UTF8,0,s_twlocalize[nIndex].Cancel,-1,uzUnicode,sizeof(uzUnicode)/sizeof(*uzUnicode));
      SetWindowTextW(::GetDlgItem(hWnd,IDCANCEL),uzUnicode);
    }

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

    // Center our dialog on the window reported to us in MSG_OPENDS...
    hParent = (HWND)pod.m_ptwndsmapps->AppHwnd(pAppId);
    if (hParent)
    {
      GetClientRect(hParent,&rectParent);
      GetWindowRect(hWnd,&rectSelect);
          nWidth  = (rectSelect.right - rectSelect.left);
          nHeight = (rectSelect.bottom - rectSelect.top);
      point.x = (rectParent.right - rectParent.left) / 2;
      point.y = (rectParent.bottom - rectParent.top) / 2;
      ClientToScreen(hParent,&point);
      point.x -= nWidth / 2;
      point.y -= nHeight / 2;
      // keep the dialog visible on the screen
      if(point.x < 0)
      {
        point.x = 0;
      }
      if(point.y < 0)
      {
        point.y = 0;
      }
      MoveWindow(hWnd,point.x,point.y,nWidth,nHeight,FALSE);
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
            hListBox = ::GetDlgItem(hWnd, ID_LST_SOURCES); 
            nIndex   = 0;
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
  #error Sorry, we do not recognize this system...
#endif



/*
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
    kLOG((kLOGERR,"_pAppId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  if (   ((TWID_T)_pAppId->Id < 1)
      || ((TWID_T)_pAppId->Id >= pod.m_ptwndsmapps->AppGetNumApp()))
  {
    kLOG((kLOGERR,"_pAppId.Id is out of range"));
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

  /** @todo scanDSDir needs to be done with each MSG_USERSELECT  
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

      // a.walling - Get the HWND of the parent window, if any, and use it as the dialog's parent
      HWND hParent = (HWND)pod.m_ptwndsmapps->AppHwnd(_pAppId);

      // create the dialog window
      int ret = (int)::DialogBoxW(g_hinstance,
                                  (LPCWSTR)IDD_DLG_SOURCE,
                                  (HWND)hParent,
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
                             TWNDSM_DS_REG_LOC,
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
    #error Sorry, we do not recognize this system...
  #endif
}


/*
* Set the datasource as the default
*/
TW_INT16 CTwnDsm::DSM_SetDefaultDS(TW_IDENTITY *_pAppId,
                                   TW_IDENTITY *_pDsId)
{
  TW_INT16 result = TWRC_SUCCESS;

  // Validate app ...
  if (0 == _pAppId)
  {
    kLOG((kLOGERR,"_pAppId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if ( (TWID_T)_pAppId->Id < 1
         || (TWID_T)_pAppId->Id >= pod.m_ptwndsmapps->AppGetNumApp() )
  {
    kLOG((kLOGERR,"_pAppId.Id is out of range...%d",(int)(TWID_T)_pAppId->Id));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }
  else if (dsmState_Open != pod.m_ptwndsmapps->AppGetState(_pAppId))
  {
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_SEQERROR);
    return TWRC_FAILURE;
  }

  // Validate DS
  if(0 == _pDsId)
  {
    kLOG((kLOGERR,"_pDsId is null"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADDEST);
    return TWRC_FAILURE;
  }
  else if ((TWID_T)_pDsId->Id < 1
        || (TWID_T)_pDsId->Id >= MAX_NUM_DS)
  {
    kLOG((kLOGERR,"Id is out of range 0 - 49..."));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  char     *szPath = pod.m_ptwndsmapps->DsGetPath(_pAppId,(TWID_T)_pDsId->Id); // Get the path we're using...

  if(!szPath)
  {
    kLOG((kLOGERR,"DS is not valid"));
    pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BADVALUE);
    return TWRC_FAILURE;
  }

  // Windows... save default source to Registry  
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    HKEY      hKey;
    long      status = ERROR_SUCCESS;

    // Open the key, creating it if it doesn't exist.
    if (RegCreateKeyEx(HKEY_CURRENT_USER, 
                       TWNDSM_DS_REG_LOC,
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
        result = TWRC_FAILURE;
      }

      // Close the key.
      RegCloseKey(hKey);
    }

  // Linux looks in the user's directory...
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    int iResult;
    FILE *pfile;
    char *szHome;
    char szFile[FILENAME_MAX];
    szHome = getenv("HOME");
    if (szHome)
    {
      SSTRCPY(szFile,sizeof(szFile),szHome);
      SSTRCAT(szFile,sizeof(szFile),"/.twndsmrc/defaultds");
      FOPEN(pfile,szFile,"w");
      if (pfile)
      {
        iResult = fwrite(szPath, 1, strlen(szPath), pfile);
		if (iResult < (int)strlen(pod.m_ptwndsmapps->DsGetPath(_pAppId,(TWID_T)_pDsId->Id)))
		{
			kLOG((kLOGERR,"fwrite defaultds failed..."));
		}
        fclose(pfile);
      }
    }

  // eek...
  #else
    #error Sorry, we do not recognize this system...
    result = TWRC_FAILURE
  #endif

  return result;
}


/*
* Invoke the user selection dialog box.  We only support this for
* Windows, for Linux it's a bad protocol, since there is no way
* to query the user (nicely) across all consoles and graphical
* interfaces for all distributions...
*/
TW_INT16 CTwnDsm::GetDSFromProductName(TW_IDENTITY *_pAppId,
                                       TW_IDENTITY *_pDsId)
{
  TWID_T ii;

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
    if (0 == strncmp((char*)_pDsId->ProductName,
                     (char*)pod.m_ptwndsmapps->DsGetIdentity(_pAppId,ii)->ProductName,
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



/*
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

  /** @todo scanDSDir needs to be done with each MSG_GETFIRST  
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

  // All done...
  return TWRC_SUCCESS;
}



/*
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

  // Prep for a call to GetNext...
  pod.m_nextDsId += 1;

  // We're out of items...
  if (pod.m_nextDsId > pod.m_ptwndsmapps->AppGetNumDs(_pAppId))
  {
    pod.m_nextDsId = 0;
    return TWRC_ENDOFLIST;
  }

  // Return info on the this driver...
  *_pDsId = *pod.m_ptwndsmapps->DsGetIdentity(_pAppId,pod.m_nextDsId);

  // All done...
  return TWRC_SUCCESS;
}



/*
* Get the identity of the default source...
*/
TW_INT16 CTwnDsm::GetMatchingDefault(TW_IDENTITY *_pAppId,
                                     TW_IDENTITY *_pDsId)
{
  bool      bMatchFnd = false;
  bool      bDefaultFound = false;
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
  if (0 != (TWID_T)_pDsId->Id)
  {
    kLOG((kLOGINFO,"Please make sure your TW_IDENTITY.Id for your driver (the destination) is zeroed out before making this call..."));
    //pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_OPERATIONERROR);
    //return TWRC_FAILURE;
  }

  // In Windows the default Data Source is stored in the registry
  // as the path to that DS.  We will need to compare this to the other DS as a match.
  // read default source from Registry
  memset(pod.m_DefaultDSPath,0,sizeof(pod.m_DefaultDSPath));

  // Windows uses the registry...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     TWNDSM_DS_REG_LOC,
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS )
    {
      // Look for the subkey "Default Source".
      DWORD DWtype = REG_SZ;
      DWORD DWsize = sizeof(pod.m_DefaultDSPath);
      bDefaultFound = ( RegQueryValueEx(hKey,"Default Source",NULL,&DWtype,(LPBYTE)pod.m_DefaultDSPath,&DWsize) == ERROR_SUCCESS);

      // Close the registry key handle.
      RegCloseKey(hKey);
    }

  // Linux looks in the user's directory...
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    int iResult;
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
        iResult = fread(pod.m_DefaultDSPath,1,sizeof(pod.m_DefaultDSPath)-1,pfile);
		if (iResult <= 0)
		{
			kLOG((kLOGINFO,"The defaultds file is empty, this is okay..."));
			pod.m_DefaultDSPath[0] = 0;
		}
        bDefaultFound = true;
        fclose(pfile);
      }
    }

  // eek...
  #else
    #error Sorry, we do not recognize this system...
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

      //If no default was saved no need to go checking for a match
      if(!bDefaultFound)
      {
        break;
      }
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


/*
* Log the triplets that the application sends to us...
*/
bool CTwnDsm::printTripletsInfo(const TW_IDENTITY *_pOrigin,
                                const TW_IDENTITY *_pDest,
                                const TW_UINT32 _DG,
                                const TW_UINT16 _DAT,
                                const TW_UINT16 _MSG,
                                const TW_MEMREF _pData)
{
  char szDg[64];
  char szDat[64];
  char szMsg[64];
  char szData[128];
  TW_CAPABILITY *_pCap;

  // Don't spend time processing the triplet if we are not logging.
  if( !g_ptwndsmlog )
  {
    return false;
  }

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

  memset(szData, 0, sizeof(szData));

  // If we have data do some extra work to see what it might be
  if(NULL != _pData)
  {
    // If we're a capability, try to tell them what cap it is...
    if (   (DG_CONTROL == _DG)
        && (DAT_CAPABILITY == _DAT) )
    {
      _pCap = (TW_CAPABILITY*)_pData;
      StringFromCap(szData,NCHARS(szData),_pCap->Cap);

      // If sending a container, try to tell them what...
      if( MSG_SET == _MSG )
      {
        char szType[32];
        StringFromConType(szType,NCHARS(szType),_pCap->ConType);
        SSTRCAT(szData,NCHARS(szData),szType);
      }
    }
  }

  // Print out the orgin and Destination
  kLOG((kLOGINFO,"%.32s -> %.32s",_pOrigin? (char*)_pOrigin->ProductName:"DSM", _pDest? (char*)_pDest->ProductName:"DSM"));
  
  // Print them
  if(strlen(szData))
  {
    kLOG((kLOGINFO,"%s/%s/%s/%s",szDg,szDat,szMsg,szData));
  }
  else
  {
    kLOG((kLOGINFO,"%s/%s/%s",szDg,szDat,szMsg));
  }
  g_ptwndsmlog->Indent(1);

  // All done...
  return true;
}

/*
* Log the results after processing the Triplet
*/
void CTwnDsm::printResults(const TW_UINT32 _DG,
                           const TW_UINT16 _DAT,
                           const TW_UINT16 _MSG,
                           const TW_MEMREF _pData,
                           const TW_UINT16 _RC)
{
  char szRc[64];

  StringFromRC(szRc,NCHARS(szRc),_RC);

  // If we have data do some extra work to see what it might be
  if( NULL != _pData && TWRC_FAILURE != _RC)
  {
    if( DG_CONTROL == _DG
     && DAT_CAPABILITY == _DAT 
     && ( MSG_GET == _MSG || MSG_GETCURRENT == _MSG || MSG_GETDEFAULT == _MSG || MSG_RESET == _MSG ) )
    {
        TW_CAPABILITY *_pCap = (TW_CAPABILITY*)_pData;
        char szType[32];
        StringFromConType(szType, NCHARS(szType), _pCap->ConType);
        SSTRCAT(szRc, NCHARS(szRc), szType);
    }
    else if( DG_CONTROL == _DG
     && DAT_PENDINGXFERS == _DAT )
    {
        TW_PENDINGXFERS *pXfer = (TW_PENDINGXFERS*)_pData;
        char szValue[32];
        SSNPRINTF(szValue, NCHARS(szValue), NCHARS(szValue), " Count = %d", pXfer->Count==0xFFFF?-1:pXfer->Count);
        SSTRCAT(szRc, NCHARS(szRc), szValue);
    }
    else if( DG_CONTROL == _DG
     && DAT_STATUS == _DAT )
    {
        TW_STATUS *pStatus = (TW_STATUS*)_pData;
        char szStatus[32];
        StringFromConditionCode(szStatus, NCHARS(szStatus), pStatus->ConditionCode);
        SSTRCAT(szRc, NCHARS(szRc), szStatus);
    }
  }

  // ... and add a blank line
  SSTRCAT(szRc,NCHARS(szRc),"\n");

  g_ptwndsmlog->Indent(-1);
  kLOG((kLOGINFO,szRc));
}

/*
* DAT_NULL is used by a driver to send certain messages back to the
* application, like MSG_XFERREADY...
*/
TW_INT16 CTwnDsm::DSM_Null(TW_IDENTITY *_pAppId,
                           TW_IDENTITY *_pDsId,
                           TW_UINT16    _MSG)
{
  TW_CALLBACK2 *ptwcallback2 = 0;
  TW_INT16      result = TWRC_SUCCESS;
  TW_MEMREF     MemRef = 0; 
  bool          bPrinted = false;

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
  ptwcallback2 = pod.m_ptwndsmapps->DsCallback2Get(_pAppId,(TWID_T)_pDsId->Id);

  // We have something to call...
  if (   (0 != ptwcallback2)
    && (ptwcallback2->CallBackProc))
  {
    // RefCon is returned back to the calling application in pData
    // Unfortunately RefCon is defined as TW_INT32
    // Application writers that want to store a pointer in RefCon
    // on 64bit will need to store an index to local storage.
    MemRef = (TW_MEMREF)ptwcallback2->RefCon;

    // Set flag to prevent Application from sending a new message 
    // before returning back from recieving this callback.
    pod.m_ptwndsmapps->DsSetAppProcessingCallback(_pAppId,(TWID_T)_pDsId->Id,TRUE);

    // We should have a try/catch around this...
    // Send a message from DS to the Application.
    // Rare case where the origin is the DS and dest is the App
    try
    {
      // Create a local copy of the AppIdentity
      TW_IDENTITY AppId = *pod.m_ptwndsmapps->AppGetIdentity(_pAppId);

      // Print the triplets to stdout for information purposes
      bPrinted = printTripletsInfo(NULL,&AppId,DG_CONTROL,DAT_NULL,_MSG,MemRef);

	  // Send a pointer to the data...
      result = ((DSMENTRYPROC)(ptwcallback2->CallBackProc))(
          pod.m_ptwndsmapps->DsGetIdentity(&AppId,(TWID_T)_pDsId->Id),
          &AppId,
          DG_CONTROL,
          DAT_NULL,
          _MSG,
          MemRef);
    }
    catch(...)
    {
      pod.m_ptwndsmapps->AppSetConditionCode(_pAppId,TWCC_BUMMER);
      kLOG((kLOGERR,"Exception caught while App was processing message.  Returning Failure."));
      result = TWRC_FAILURE;
    }

    pod.m_ptwndsmapps->DsSetAppProcessingCallback(_pAppId,(TWID_T)_pDsId->Id,FALSE);
  }

  // Application has not registered a callback. As a result, the msg will
  // be sent to the app the next time it forwards an event.
  // Each App's DS has a callback structure.   This way multiple DS's can make
  // a callback to a single app, and we are not going to lose MSG's
  else
  {
	  if (ptwcallback2)
	  {
		  if (ptwcallback2->Message != 0)
		  {
				char szMsg[64];
				StringFromMsg(szMsg, NCHARS(szMsg), ptwcallback2->Message);
				kLOG((kLOGERR, "%.32s NEVER retrieved DAT_EVENT / %s\n", _pAppId->ProductName, szMsg));
		  }
		  ptwcallback2->Message = _MSG;
	  }
	  pod.m_ptwndsmapps->DsCallbackSetWaiting(_pAppId,(TWID_T)_pDsId->Id,TRUE);
      pod.m_ptwndsmapps->AppWakeup(_pAppId);
  }

  // Log how it went...
  if (bPrinted)
  {
    printResults(DG_CONTROL,DAT_NULL,_MSG,MemRef,result);
  }

  return result;
}



/*
* Convert a DG_ data group numerical value to a string...
*/
void CTwnDsm::StringFromDg(char      *_szDg,
                     const int        _nChars,
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



/*
* Convert a DAT_ data argument type numerical value to a string...
*/
void CTwnDsm::StringFromDat(char     *_szDat,
                      const int       _nChars,
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

    case DAT_STATUSUTF8:
      SSTRCPY(_szDat,_nChars,"DAT_STATUSUTF8");
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

    case DAT_ICCPROFILE:
      SSTRCPY(_szDat,_nChars,"DAT_ICCPROFILE");
      break;

    case DAT_IMAGEMEMFILEXFER:
      SSTRCPY(_szDat,_nChars,"DAT_IMAGEMEMFILEXFER");
      break;

    case DAT_ENTRYPOINT:
      SSTRCPY(_szDat,_nChars,"DAT_ENTRYPOINT");
      break;
  }
}



/*
* Convert a MSG_ message numerical value to a string...
*/
void CTwnDsm::StringFromMsg(char     *_szMsg,
                      const int       _nChars,
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

    case MSG_GETHELP:
      SSTRCPY(_szMsg,_nChars,"MSG_GETHELP");
      break;

    case MSG_GETLABEL:
      SSTRCPY(_szMsg,_nChars,"MSG_GETLABEL");
      break;

    case MSG_GETLABELENUM:
      SSTRCPY(_szMsg,_nChars,"MSG_GETLABELENUM");
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

    case MSG_RESETALL:
      SSTRCPY(_szMsg,_nChars,"MSG_RESETALL");
      break;
  }
}



/*
* Convert a CAP_ or ICAP_ capability numerical value to a string...
*/
void CTwnDsm::StringFromCap(char     *_szCap,
                      const int       _nChars,
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

    case CAP_CAMERASIDE:
      SSTRCPY(_szCap,_nChars,"CAP_CAMERASIDE");
      break;

    case CAP_SEGMENTED:
      SSTRCPY(_szCap,_nChars,"CAP_SEGMENTED");
      break;

    case CAP_CAMERAENABLED:
      SSTRCPY(_szCap,_nChars,"CAP_CAMERAENABLED");
      break;

    case CAP_CAMERAORDER:
      SSTRCPY(_szCap,_nChars,"CAP_CAMERAORDER");
      break;

    case CAP_MICRENABLED:
      SSTRCPY(_szCap,_nChars,"CAP_MICRENABLED");
      break;

    case CAP_FEEDERPREP:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDERPREP");
      break;

    case CAP_FEEDERPOCKET:
      SSTRCPY(_szCap,_nChars,"CAP_FEEDERPOCKET");
      break;

    case CAP_AUTOMATICSENSEMEDIUM:
      SSTRCPY(_szCap,_nChars,"CAP_AUTOMATICSENSEMEDIUM");
      break;

    case CAP_CUSTOMINTERFACEGUID:
      SSTRCPY(_szCap,_nChars,"CAP_CUSTOMINTERFACEGUID");
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

    case ICAP_AUTODISCARDBLANKPAGES:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTODISCARDBLANKPAGES");
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

    case ICAP_FEEDERTYPE:
      SSTRCPY(_szCap,_nChars,"ICAP_FEEDERTYPE");
      break;

    case ICAP_ICCPROFILE:
      SSTRCPY(_szCap,_nChars,"ICAP_ICCPROFILE");
      break;

    case ICAP_AUTOSIZE:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOSIZE");
      break;

    case ICAP_AUTOMATICCROPUSESFRAME:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICCROPUSESFRAME");
      break;

    case ICAP_AUTOMATICLENGTHDETECTION:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICLENGTHDETECTION");
      break;

    case ICAP_AUTOMATICCOLORENABLED:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICCOLORENABLED");
      break;

    case ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE:
      SSTRCPY(_szCap,_nChars,"ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE");
      break;

    case ICAP_COLORMANAGEMENTENABLED:
      SSTRCPY(_szCap,_nChars,"ICAP_COLORMANAGEMENTENABLED");
      break;

    case ICAP_IMAGEMERGE:
      SSTRCPY(_szCap,_nChars,"ICAP_IMAGEMERGE");
      break;

    case ICAP_IMAGEMERGEHEIGHTTHRESHOLD:
      SSTRCPY(_szCap,_nChars,"ICAP_IMAGEMERGEHEIGHTTHRESHOLD");
      break;

    case ICAP_SUPPORTEDEXTIMAGEINFO:
      SSTRCPY(_szCap,_nChars,"ICAP_SUPPORTEDEXTIMAGEINFO");
      break;

    case ACAP_AUDIOFILEFORMAT:
      SSTRCPY(_szCap,_nChars,"ACAP_AUDIOFILEFORMAT");
      break;

    case ACAP_XFERMECH:
      SSTRCPY(_szCap,_nChars,"ACAP_XFERMECH");
      break;
  }
}



/*
* Convert a TWRC_ return code numerical value to a string...
*/
void CTwnDsm::StringFromRC(char     *_szRc,
                     const int       _nChars,
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

/*
* Convert a Container type to a string...
*/
void CTwnDsm::StringFromConType(char     *_szData,
                          const int       _nChars,
                          const TW_UINT16 _ConType)
{
  switch (_ConType)
  {
    default:    
      SSNPRINTF(_szData,_nChars,_nChars," TWON_0x%04x",_ConType);
      break;

    case TWON_DONTCARE16:
      SSTRCPY(_szData,_nChars," TWON_DONTCARE16");
      break;

    case TWON_ARRAY:
      SSTRCPY(_szData,_nChars," TWON_ARRAY");
      break;

    case TWON_ENUMERATION:
      SSTRCPY(_szData,_nChars," TWON_ENUMERATION");
      break;

    case TWON_ONEVALUE:
      SSTRCPY(_szData,_nChars," TWON_ONEVALUE ");
      break;

    case TWON_RANGE:
      SSTRCPY(_szData,_nChars," TWON_RANGE");
      break;
  }
}

/*
* Convert a Condition Code to a string...
*/
void CTwnDsm::StringFromConditionCode(char     *_szData,
                                const int       _nChars,
                                const TW_UINT16 _ConCode)
{
  switch (_ConCode)
  {
    default:    
      SSNPRINTF(_szData,_nChars,_nChars," TWCC_0x%04x",_ConCode);
      break;

    case TWCC_SUCCESS:
      SSTRCPY(_szData,_nChars," TWCC_SUCCESS");
      break;

    case TWCC_BUMMER:
      SSTRCPY(_szData,_nChars," TWCC_BUMMER");
      break;

    case TWCC_LOWMEMORY:
      SSTRCPY(_szData,_nChars," TWCC_LOWMEMORY");
      break;

    case TWCC_NODS:
      SSTRCPY(_szData,_nChars," TWCC_NODS");
      break;

    case TWCC_MAXCONNECTIONS:
      SSTRCPY(_szData,_nChars," TWCC_MAXCONNECTIONS");
      break;

    case TWCC_OPERATIONERROR:
      SSTRCPY(_szData,_nChars," TWCC_OPERATIONERROR");
      break;

    case TWCC_BADCAP:
      SSTRCPY(_szData,_nChars," TWCC_BADCAP");
      break;

    case TWCC_BADPROTOCOL:
      SSTRCPY(_szData,_nChars," TWCC_BADPROTOCOL");
      break;

    case TWCC_BADVALUE:
      SSTRCPY(_szData,_nChars," TWCC_BADVALUE");
      break;

    case TWCC_SEQERROR:
      SSTRCPY(_szData,_nChars," TWCC_SEQERROR");
      break;

    case TWCC_BADDEST:
      SSTRCPY(_szData,_nChars," TWCC_BADDEST");
      break;

    case TWCC_CAPUNSUPPORTED:
      SSTRCPY(_szData,_nChars," TWCC_CAPUNSUPPORTED");
      break;

    case TWCC_CAPBADOPERATION:
      SSTRCPY(_szData,_nChars," TWCC_CAPBADOPERATION");
      break;

    case TWCC_CAPSEQERROR:
      SSTRCPY(_szData,_nChars," TWCC_CAPSEQERROR");
      break;

    case TWCC_DENIED:
      SSTRCPY(_szData,_nChars," TWCC_DENIED");
      break;

    case TWCC_FILEEXISTS:
      SSTRCPY(_szData,_nChars," TWCC_FILEEXISTS");
      break;

    case TWCC_FILENOTFOUND:
      SSTRCPY(_szData,_nChars," TWCC_FILENOTFOUND");
      break;

    case TWCC_NOTEMPTY:
      SSTRCPY(_szData,_nChars," TWCC_NOTEMPTY");
      break;

    case TWCC_PAPERJAM:
      SSTRCPY(_szData,_nChars," TWCC_PAPERJAM");
      break;

    case TWCC_PAPERDOUBLEFEED:
      SSTRCPY(_szData,_nChars," TWCC_PAPERDOUBLEFEED");
      break;

    case TWCC_FILEWRITEERROR:
      SSTRCPY(_szData,_nChars," TWCC_FILEWRITEERROR");
      break;

    case TWCC_CHECKDEVICEONLINE:
      SSTRCPY(_szData,_nChars," TWCC_CHECKDEVICEONLINE");
      break;

    case TWCC_INTERLOCK:
      SSTRCPY(_szData,_nChars," TWCC_INTERLOCK");
      break;

    case TWCC_DAMAGEDCORNER:
      SSTRCPY(_szData,_nChars," TWCC_DAMAGEDCORNER");
      break;

    case TWCC_FOCUSERROR:
      SSTRCPY(_szData,_nChars," TWCC_FOCUSERROR");
      break;

    case TWCC_DOCTOOLIGHT:
      SSTRCPY(_szData,_nChars," TWCC_DOCTOOLIGHT");
      break;

    case TWCC_DOCTOODARK:
      SSTRCPY(_szData,_nChars," TWCC_DOCTOODARK");
      break;

    case TWCC_NOMEDIA:
      SSTRCPY(_szData,_nChars," TWCC_NOMEDIA");
      break;
    }
}


/*
* Allocate memory on behalf of the caller (could be the application
* or the driver)...
*/
TW_HANDLE PASCAL DSM_MemAllocate (TW_UINT32 _bytes)
{
  TW_HANDLE handle;

  // Validate...
  if (0 == _bytes)
  {
    kLOG((kLOGERR,"_bytes is zero..."));
    return (TW_HANDLE)NULL;
  }

  // Windows...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    handle = (TW_HANDLE)::GlobalAlloc(GPTR,_bytes);
  if (0 == handle)
  {
      kLOG((kLOGERR,"DSM_MemAllocate failed to allocate %ld bytes...",_bytes));
      return (TW_HANDLE)NULL;
  }
  return handle;
  
  // MacOS
  #elif (TWNDSM_OS == TWNDSM_OS_MACOSX)
    handle = (TW_HANDLE)NewHandleClear(_bytes);
  if (0 == handle)
  {
      kLOG((kLOGERR,"DSM_MemAllocate failed to allocate %ld bytes...",_bytes));
      return (TW_HANDLE)NULL;
  }
  return handle;

  // Linux
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    handle = (TW_HANDLE)calloc(_bytes,1);
  if (0 == handle)
  {
      kLOG((kLOGERR,"DSM_MemAllocate failed to allocate %ld bytes...",_bytes));
      return (TW_HANDLE)NULL;
  }
  return handle;

  // Oops...
  #else
    #error Sorry, we do not recognize this system...
  #endif
}



/*
* Free memory on behalf of the caller (could be the application
* or the driver)...
*/
void PASCAL DSM_MemFree (TW_HANDLE _handle)
{
  // Validate...
  if (0 == _handle)
  {
    kLOG((kLOGERR,"ignoring attempt to free null handle..."));
    return;
  }

  // Windows...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    ::GlobalFree(_handle);
    
  // MacOS
  #elif (TWNDSM_OS == TWNDSM_OS_MACOSX)
    DisposeHandle((Handle)_handle);

  // Linux...
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    free(_handle);

  // Oops...
  #else
    #error Sorry, we do not recognize this system...
  #endif
}



/*
* Lock memory on behalf of the caller...
*/
TW_MEMREF PASCAL DSM_MemLock (TW_HANDLE _handle)
{
  // Validate...
  if (0 == _handle)
  {
    kLOG((kLOGERR,"attempting to lock null handle..."));
    return (TW_MEMREF)NULL;
  }

  // Windows...technically we shouldn't have to do the
  // lock, since we allocated with GPTR, but I'm nervous
  // that we might get a GHND sent to us.  And since
  // this is a no-op for a GPTR, what they hey...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    return (TW_MEMREF)::GlobalLock(_handle);

  // MacOS
  #elif (TWNDSM_OS == TWNDSM_OS_MACOSX)
    return _handle ? *_handle : 0;

  // Linux...
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    return (TW_MEMREF)_handle;

  // Oops...
  #else
    #error Sorry, we do not recognize this system...
  #endif
}



/*
* Unlock memory on behalf of the caller, if needed.  This function
* is a placeholder at the moment.  It'll get more interesting if it
* has to do locking on Mac OS/X...
*/
void PASCAL DSM_MemUnlock (TW_HANDLE _handle)
{
  // Validate...
  if (0 == _handle)
  {
    kLOG((kLOGERR,"attempting to unlock null handle..."));
    return;
  }

  // Windows...
  #if (TWNDSM_CMP == TWNDSM_CMP_VISUALCPP)
    ::GlobalUnlock(_handle);

  // Linux...
  #elif (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)

  // Oops...
  #else
    #error Sorry, we do not recognize this system...
  #endif
}

/*
* This function wraps the function loading calls. Linux has a 
* special way to check dlsym failures.
*/
void* DSM_LoadFunction(void* _pHandle, const char* _pszSymbol)
{
  void* pRet = 0;
#if (TWNDSM_OS == TWNDSM_OS_MACOSX)
  pRet = CFBundleGetFunctionPointerForName((CFBundleRef)_pHandle,
					   CFStringCreateWithCStringNoCopy(0, _pszSymbol, kCFStringEncodingUTF8, 0));
#else

  #if (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
    dlerror();    /* Clear any existing error */
  #endif

  // Try to get the entry point...
  pRet = LOADFUNCTION(_pHandle, _pszSymbol);

#if (TWNDSM_CMP == TWNDSM_CMP_GNUGPP)
  char* psz_error = 0;

  if((psz_error = dlerror()) != NULL)
  {
    kLOG((kLOGERR,"dlsym error: %s",psz_error));
    pRet = 0;
  }
#endif
#endif
  return pRet;
}
