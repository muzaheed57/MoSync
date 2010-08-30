/*
* Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
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


#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "astring.h"
#include "fileline.h"
#include "numval.h"
#include "tokens.h"

#ifdef __LINUX__
#include <linux/limits.h>
#define _MAX_PATH PATH_MAX
#endif //__LINUX__

#ifdef __VC32__
#pragma warning( push, 1 )	// MS STL libraries do not compile cleanly, temporarily set warning level to 1
#pragma warning( disable : 4710 )	// function not inlined.
#endif
FileLineManager::FileLineManager():
	iOffsetLineNumber(0) {}
FileLineManager::~FileLineManager() {}
void FileLineManager::SetCurrentFile(const String& a)
	{
	iCurrentFileName = &*(iAllFileNames.insert(a).first);
	}
#ifdef __VC32__
#pragma warning( pop )
#endif

void FileLineManager::SetBase(const String& aFileName,int aLineNumber)
	{
	SetCurrentFile(aFileName);
	iBaseFileName = iCurrentFileName;
	iOffsetLineNumber=aLineNumber;
	}

void FileLineManager::SetPath(const String& aDriveAndDirectory)
	{
	iBasePath=aDriveAndDirectory;
	}

String FileLineManager::ExtractFileName( const String & Text)
	{

    // The string that is passed to this procedure is expected to contain
    // a file specification followed by a " followed by other stuff that
    // is generated by the preprocessor.  We can discard the latter stuff.
    // This will leave either an empty string (implying the base file name)
    // or a unix-format relative file specification.

    // This string class has very little functionality, so we end up
    // doing traditional C-style stuff on a char buffer to achieve
    // what we need  {sigh}...

    char buffer[_MAX_PATH +1];
    // (older version of this code used heap allocation but I prefer
    //  this way which is safer and does less heap-churning)

    
    String result;      // This is what we will pass back

	// Copy the text to our working buffer

    int n = Text.Length();
    if ( n >= _MAX_PATH ) n = _MAX_PATH; // Unlikely, but you never know
    strncpy(buffer, Text.GetAssertedNonEmptyBuffer(), n);
    buffer[n] = '\0';                  // add zero terminator

    // truncate to the expected double quote character

    char * pquote = strchr(buffer, '"');
    if ( pquote != NULL ) * pquote = '\0';

    n = strlen(buffer);


    // If we now have an empty string then replace it with the
    // base filename string that should already be defined.

    if ( n == 0 )
        {
        n = iBaseFileName->Length();
        if ( n > _MAX_PATH ) n = _MAX_PATH;
        if (n>0) strncpy(buffer, iBaseFileName->GetAssertedNonEmptyBuffer(), n);
        buffer[n] = '\0';
        }

#ifndef __LINUX__
    // Replace all the unix-like forward slashes with DOS-like backslashes

    while ( n > 0 )
        {
        n -=1;
        if ( buffer[n] == '/' ) buffer[n] = '\\';
        }
#endif //__LINUX__
    
    result = buffer;
    return result;
	}

void FileLineManager::SetInclude(const String& aNameText,int aLineNumber)
	{
	SetCurrentFile(ExtractFileName(aNameText));
	iOffsetLineNumber = aLineNumber;	
	}

void FileLineManager::PostInclude( char* aNameText, char * aRealLineNumber, int aLineNumber)
	{	// Returning to a file (after having included another file).
	SetCurrentFile(ExtractFileName(aNameText));
	int val = atoi(aRealLineNumber);
	iOffsetLineNumber=aLineNumber - val + 1;
	}

int	FileLineManager::GetErrorLine(int aCurrentLineNumber) const
	{
	return aCurrentLineNumber-iOffsetLineNumber;
	}

const String* FileLineManager::GetCurrentFile() const
	{ 
	return iCurrentFileName;
	}

