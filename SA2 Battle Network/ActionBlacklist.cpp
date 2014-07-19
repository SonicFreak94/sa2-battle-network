#include "Common.h"
#include "ActionBlacklist.h"

bool isHoldAction(uchar action)
{
	switch(action)
	{
		case 19:
		case 20:
		case 21:
		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 33:
		case 34:
		case 35:
			return true;
			
		default:
			return false;
	}
}