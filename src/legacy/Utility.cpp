//-----------------------------------------------------------------------------
// File:	Utility.cpp
//
// Desc:	small little helpers
//
// Last modification: November 16, 2001
//
// Credits: 
//
// Copyright (c) 2001 - 2003 wolf@direct3d.net All rights reserved.
//-----------------------------------------------------------------------------
#include "stdio.h"
#include <tchar.h>

#include "Utility.h"

// Parser shortcuts
inline bool bIsChar(const char &c)
 {return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));}

inline bool bIsNum(const char &c)
 {return (c >= '0' && c <= '9');}

inline bool bIsUnderScore(const char &c)
 {return (c=='_');}

inline bool bIsPoint(const char &c)
 {return (c=='.');}

inline bool bIsComment(char *line) 
 {return (line[0] == '/' && line[1] == '/');}

inline bool bIsWhite(char c)
 {return (c == ' ' || c == '\t' || c == '\r' || c == '\n' \
	   || c == '(' || c == ')' || c == '-' || c == ',' \
	   || c == ';' ||c == '\'' || c == '/');}



//-----------------------------------------------------------------
// ParseTextLine
//
// Desc: returns in strTok a word of strTextLine
//		 returns in strTextLine the next word in strTextLine

// reads the string: lower_mesh01,shaders/OnlyColor.vso,shaderlevel0 in the 
// following order:
//		 
//				in cTokenString:					pcTextLine:
// first call:	lower_mesh01						,shaders/OnlyColor.vso,shaderlevel0
// second call:	,									shaders/OnlyColor.vso,shaderlevel0
// third call:	shaders								/OnlyColor.vso,shaderlevel0
// fourth call:  /									OnlyColor.vso,shaderlevel0
// fifth call:	OnlyColor.vso						,shaderlevel0
// six call:		,									shaderlevel0
// seventh call: shaderlevel0						" "


// reads the animation.cfg in the following order:
//				cTokenString:				pcTextLine:
// first call:		sex							"     m"
// second call:		m							""
// third call:		""							""
// fourth call:		""							""
// fifth call: ...
// seventh call:	0							"691 0 33  // BOTH_DEATH1
// eight call:		691							"0 33  // BOTH_DEATH1
// nineth call:		0							"33  // BOTH_DEATH1
// tenth call:		33							" // BOTH_DEATH1
// eleventh call:	""							""
// and so on ...

// Reads only words consisting of chars, numbers, points and underscores
//-----------------------------------------------------------------
void ParseTextLine (char *strTextLine, char *strTok)
{
	int	iLength = 0;
	char *cTemp = strTextLine;

	memset(strTok, 0, 1024 * sizeof(char));
	
	// remove comments
	if (bIsComment(cTemp))
		while (bIsChar(*cTemp) || bIsNum(*cTemp) \
				|| bIsUnderScore(*cTemp) || bIsPoint(*cTemp) || bIsWhite(*cTemp))
			*cTemp++;

	// remove white noise
	else if (bIsWhite(*cTemp))
		*cTemp++;
	
	// parse a regular word
	else if (bIsChar(*cTemp) || bIsNum(*cTemp) || bIsUnderScore(*cTemp) || bIsPoint(*cTemp)) 
		while (bIsChar(*cTemp) || bIsNum(*cTemp) || bIsUnderScore(*cTemp) || bIsPoint(*cTemp))		
			  strTok[iLength++] = *cTemp++;

	strcpy (strTextLine, cTemp);
}

//-----------------------------------------------------------------
// CheckNumber
//
// Desc: check the existance of a number in a string
//-----------------------------------------------------------------
int CheckNumber(const char *str)
{
	while (*str)
	{
		if (*str < '0' || *str > '9') return 1;
		str++;
	}

	return 1;
}

//-----------------------------------------------------------------
// ParseNumber
//
// Desc: returns a number in a string or 0 if there is no number
//-----------------------------------------------------------------
int ParseNumber(const char *str)
{
	char strNumber[5];
	int iNumber = 512;

	if (CheckNumber(str))
	{
		while (iNumber--)
		{
			_itoa( iNumber, strNumber, 10);
			if(strstr(str, strNumber))
				break;
		}
	}
	else 
	 return 0;

	return iNumber;
}

//-----------------------------------------------------------------
// ParseNumber
//
// Desc: returns a number in a string or 0 if there is no number
//-----------------------------------------------------------------
int CheckFile(const char *filename)
{
	FILE *f;

	f = fopen(filename, "rb");

	if (f)
	{
		fclose(f);
		return 1;
	}
	else return 0;
}

