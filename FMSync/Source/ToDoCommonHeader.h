/*
	File:		ToDoCommonHeader.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Mon, Nov 17, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00---	file created

	To Do:
*/

#pragma once

#ifndef __ToDoCommonHeader__
#define __ToDoCommonHeader__

#define BitAtPosition(pos)						((ULong)1 << (pos))
#define GetBitMacro(bitfield, index)		((bitfield) & BitAtPosition(index))
#define SetBitMacro(bitfield, index)		((bitfield) |= BitAtPosition(index))

#define	kYearShiftFactor		9
#define	kMonthShiftFactor		5

#define	kCompletedShiftFactor	7

#define	kRawDateYearMask		0xfe00
#define	kRawDateMonthMask		0x01e0
#define	kRawDateDayMask			0x001f

#define	kCompletedBitMask		0x80

#define	kMacBaseYear			1984
#define	kPilotBaseYear			1904

//CSyncRecord field IDs
enum
{		
	kDueDateFieldID		= 'DUDT',
	kPriorityFieldID	= 'PRIO',
	kCompletedFieldID	= 'DONE',
	kDescriptionFieldID	= 'DESC',
	kNoteFieldID		= 'NOTE'
};


#endif	//__ToDoCommonHeader__
