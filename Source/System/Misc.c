/****************************/
/*      MISC ROUTINES       */
/* (c)2003 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "game.h"

/****************************/
/*    CONSTANTS             */
/****************************/

#define	USE_MALLOC		1



/**********************/
/*     VARIABLES      */
/**********************/


uint32_t 	gSeed0 = 0, gSeed1 = 0, gSeed2 = 0;

float	gFramesPerSecond, gFramesPerSecondFrac;

int		gNumPointers = 0;

Boolean	gSlowCPU = false;


/**********************/
/*     PROTOTYPES     */
/**********************/

static void DoSerialDialog(unsigned char *out);



/****************** DO SYSTEM ERROR ***************/

void ShowSystemErr(long err)
{
Str255		numStr;

	Enter2D(true);


	/*if (gDisplayContext)
		GammaOn();*/
	MyFlushEvents();
	TurnOffISp();										// MUST TURN OFF INPUT SPROK TO GET KEYBOARD EVENTS!!!
	UseResFile(gMainAppRezFile);
	NumToString(err, numStr);
	DoAlert (numStr);
	
//	if (gOSX)
//		DebugStr("ShowSystemErr has been called");
	
	Exit2D();
	
	CleanQuit();
}

/****************** DO SYSTEM ERROR : NONFATAL ***************/
//
// nonfatal
//
void ShowSystemErr_NonFatal(long err)
{
Str255		numStr;

	Enter2D(true);


	/*if (gDisplayContext)
		GammaOn();*/
	MyFlushEvents();
	NumToString(err, numStr);
	DoAlert (numStr);
	
	Exit2D();
			
}


/*********************** DO ALERT *******************/

void DoAlert(const char* format, ...)
{
	Enter2D(true);

	char message[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(message, sizeof(message), format, args);
	va_end(args);

	printf("CMR Alert: %s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Billy Frontier", message, gSDLWindow);

	Exit2D();
}


/*********************** DO FATAL ALERT *******************/

void DoFatalAlert(const char* format, ...)
{
	Enter2D(true);

	char message[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(message, sizeof(message), format, args);
	va_end(args);

	printf("CMR Fatal Alert: %s\n", message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Billy Frontier", message, gSDLWindow);

	Exit2D();
	CleanQuit();
}


/************ CLEAN QUIT ***************/

void CleanQuit(void)
{	
static Boolean	beenHere = false;

	if (!beenHere)
	{
		beenHere = true;
		
		DeleteAllObjects();
		DisposeAllBG3DContainers();						// nuke all models
		DisposeAllSpriteGroups();						// nuke all sprites

		if (gGameViewInfoPtr)							// see if need to dispose this
			OGL_DisposeWindowSetup(&gGameViewInfoPtr);

		ShutdownSound();								// cleanup sound stuff
	}

	GameScreenToBlack();
	CleanupDisplay();								// unloads Draw Sprocket

#if 0
	if (gISPInitialized)							// unload ISp
		ISpShutdown();
#endif

	UseResFile(gMainAppRezFile);
	
	InitCursor();
	MyFlushEvents();

	SavePrefs();							// save prefs before bailing

	ExitToShell();		
}



#pragma mark -


/******************** MY RANDOM LONG **********************/
//
// My own random number generator that returns a LONG
//
// NOTE: call this instead of MyRandomShort if the value is going to be
//		masked or if it just doesnt matter since this version is quicker
//		without the 0xffff at the end.
//

unsigned long MyRandomLong(void)
{
  return gSeed2 ^= (((gSeed1 ^= (gSeed2>>5)*1568397607UL)>>7)+
                   (gSeed0 = (gSeed0+1)*3141592621UL))*2435386481UL;
}


/************************* RANDOM RANGE *************************/
//
// THE RANGE *IS* INCLUSIVE OF MIN AND MAX
//

uint16_t	RandomRange(uint16_t min, uint16_t max)
{
uint16_t		qdRdm;											// treat return value as 0-65536
uint32_t		range, t;

	qdRdm = MyRandomLong();
	range = max+1 - min;
	t = (qdRdm * range)>>16;	 							// now 0 <= t <= range
	
	return( t+min );
}



/************** RANDOM FLOAT ********************/
//
// returns a random float between 0 and 1
//

float RandomFloat(void)
{
unsigned long	r;
float	f;

	r = MyRandomLong() & 0xfff;		
	if (r == 0)
		return(0);

	f = (float)r;							// convert to float
	f = f * (1.0f/(float)0xfff);			// get # between 0..1
	return(f);
} 
 

/************** RANDOM FLOAT 2 ********************/
//
// returns a random float between -1 and +1
//

float RandomFloat2(void)
{
unsigned long	r;
float	f;

	r = MyRandomLong() & 0xfff;		
	if (r == 0)
		return(0);

	f = (float)r;							// convert to float
	f = f * (2.0f/(float)0xfff);			// get # between 0..2
	f -= 1.0f;								// get -1..+1
	return(f);
} 



/**************** SET MY RANDOM SEED *******************/

void SetMyRandomSeed(unsigned long seed)
{
	gSeed0 = seed;
	gSeed1 = 0;
	gSeed2 = 0;	
	
}

/**************** INIT MY RANDOM SEED *******************/

void InitMyRandomSeed(void)
{
	gSeed0 = 0x2a80ce30;
	gSeed1 = 0;
	gSeed2 = 0;	
}


#pragma mark -


/******************* FLOAT TO STRING *******************/

void FloatToString(float num, Str255 string)
{
Str255	sf;
long	i,f;

	i = num;						// get integer part
	
	
	f = (fabs(num)-fabs((float)i)) * 10000;		// reduce num to fraction only & move decimal --> 5 places	

	if ((i==0) && (num < 0))		// special case if (-), but integer is 0
	{
		string[0] = 2;
		string[1] = '-';
		string[2] = '0';
	}
	else
		NumToString(i,string);		// make integer into string
		
	NumToString(f,sf);				// make fraction into string
	
	string[++string[0]] = '.';		// add "." into string
	
	if (f >= 1)
	{
		if (f < 1000)
			string[++string[0]] = '0';	// add 1000's zero
		if (f < 100)
			string[++string[0]] = '0';	// add 100's zero
		if (f < 10)
			string[++string[0]] = '0';	// add 10's zero
	}
	
	for (i = 0; i < sf[0]; i++)
	{
		string[++string[0]] = sf[i+1];	// copy fraction into string
	}
}

/*********************** STRING TO FLOAT *************************/

float StringToFloat(Str255 textStr)
{
short	i;
short	length;
Byte	mode = 0;
long	integer = 0;
long	mantissa = 0,mantissaSize = 0;
float	f;
float	tens[8] = {1,10,100,1000,10000,100000,1000000,10000000};
char	c;
float	sign = 1;												// assume positive

	length = textStr[0];										// get string length

	if (length== 0)												// quick check for empty
		return(0);


			/* SCAN THE NUMBER */

	for (i = 1; i <= length; i++)
	{
		c  = textStr[i];										// get this char
		
		if (c == '-')											// see if negative
		{
			sign = -1;
			continue;
		}
		else
		if (c == '.')											// see if hit the decimal
		{
			mode = 1;
			continue;
		}
		else
		if ((c < '0') || (c > '9'))								// skip all but #'s
			continue;
	
	
		if (mode == 0)
			integer = (integer * 10) + (c - '0');
		else
		{
			mantissa = (mantissa * 10) + (c - '0');
			mantissaSize++;
		}
	}

			/* BUILT A FLOAT FROM IT */
			
	f = (float)integer + ((float)mantissa/tens[mantissaSize]);
	f *= sign;

	return(f);
}





#pragma mark -

/****************** ALLOC HANDLE ********************/

Handle	AllocHandle(long size)
{
Handle	hand;
OSErr	err;

	hand = NewHandle(size);							// alloc in APPL
	if (hand == nil)
	{
		DoAlert("AllocHandle: using temp mem");
		hand = TempNewHandle(size,&err);			// try TEMP mem
		if (hand == nil)
		{
			DoAlert("AllocHandle: failed!");
			return(nil);
		}
		else
			return(hand);							
	}
	return(hand);		
								
}



/****************** ALLOC PTR ********************/

void *AllocPtr(long size)
{
Ptr	pr;
uint32_t	*cookiePtr;

	size += 16;								// make room for our cookie & whatever else (also keep to 16-byte alignment!)

#if USE_MALLOC
	pr = malloc(size);
#else
	pr = NewPtr(size);
#endif	
	if (pr == nil)
		DoFatalAlert("AllocPtr: NewPtr failed");

	cookiePtr = (uint32_t *)pr;

	*cookiePtr++ = 'FACE';
	*cookiePtr++ = 'PTR2';
	*cookiePtr++ = 'PTR3';
	*cookiePtr = 'PTR4';

	pr += 16;
	
	gNumPointers++;
	
	return(pr);
}


/****************** ALLOC PTR CLEAR ********************/

void *AllocPtrClear(long size)
{
Ptr	pr;
uint32_t	*cookiePtr;

	size += 16;								// make room for our cookie & whatever else (also keep to 16-byte alignment!)

#if USE_MALLOC
	pr = calloc(1, size);
#else
	pr = NewPtrClear(size);						// alloc in Application
#endif	

	if (pr == nil)
		DoFatalAlert("AllocPtr: NewPtr failed");

	cookiePtr = (uint32_t *)pr;

	*cookiePtr++ = 'FACE';
	*cookiePtr++ = 'PTC2';
	*cookiePtr++ = 'PTC3';
	*cookiePtr = 'PTC4';

	pr += 16;
	
	gNumPointers++;
	
	return(pr);
}


/***************** SAFE DISPOSE PTR ***********************/

void SafeDisposePtr(void *ptr)
{
uint32_t	*cookiePtr;
Ptr		p = ptr;

	p -= 16;					// back up to pt to cookie
	
	cookiePtr = (uint32_t *)p;
	
	if (*cookiePtr != 'FACE')
		DoFatalAlert("SafeSafeDisposePtr: invalid cookie!");
		
	*cookiePtr = 0;
	
#if USE_MALLOC	
	free(p);
#else
	DisposePtr(p);
#endif
	
	gNumPointers--;
}



#pragma mark -


/******************* VERIFY SYSTEM ******************/

void VerifySystem(void)
{
	gOSX = true;
	gSlowCPU = false;

#if 0
OSErr	iErr;
long		 cpuFamily, cpuSpeed;
NumVersion	vers;
short		i;



		/* REQUIRE CARBONLIB 1.4 */

	iErr = Gestalt(gestaltCarbonVersion,(long *)&vers);
	if (iErr)
	{
		ShowSystemErr_NonFatal(iErr);
		goto carbonerr;
	}
	if (vers.stage == 1)
	{
		if (vers.nonRelRev < 0x40)
		{
carbonerr:		
			DoFatalAlert("This application requires CarbonLib 1.4 or newer.  Run Software Update to get the latest version");
		}
	}



			/* SEE IF PROCESSOR IS G4 OR NOT */

	gSlowCPU = false;														// assume not slow
				
	iErr = Gestalt(gestaltNativeCPUfamily,&cpuFamily);
	if (iErr != noErr)
		DoFatalAlert("VerifySystem: gestaltNativeCPUfamily failed!");
	
	if (cpuFamily >= gestaltCPUG4)
		gG4 = true;
	else
		gG4 = false;

	if (!gG4)																// if not G4, check processor speed to see if on really fast G3
	{
		iErr = Gestalt(gestaltProcClkSpeed,&cpuSpeed);							
		if (iErr == noErr)
		{
			if ((cpuSpeed/1000000) >= 600)										// must be at least 600mhz G3 for us to treat it like a G4
				gG4 = true;
			else
			if ((cpuSpeed/1000000) <= 450)										// if 450 or less then it's a slow G3
				gSlowCPU = true;
		}
	}


		/* DETERMINE IF RUNNING ON OS X */

	iErr = Gestalt(gestaltSystemVersion,(long *)&vers);
	if (iErr != noErr)
		DoFatalAlert("VerifySystem: gestaltSystemVersion failed!");
				
	if (vers.stage >= 0x10)													// see if at least OS 10
	{
		gOSX = true;
		
		if ((vers.stage == 0x10) && (vers.nonRelRev < 0x20))				// must be at least OS 10.1 !!!
		{
			if (!gGamePrefs.oldOSWarned)
			{
				DoAlert("This game requires MacOS 10.2 or later.  It might run for you on 10.1, but it will probably freeze up your computer at some point.");
				gGamePrefs.oldOSWarned = true;
			}
		}
	}
	else
	{
		gOSX = false;
		if (vers.stage < 9)						// check for OS 9
			DoFatalAlert("This game will not run on MacOS 8.");
		else
		if (!gGamePrefs.oldOSWarned)
		{
			DoAlert("This game requires MacOS 10.2 or later.  It might run for you on MacOS 9.2.2, however, we only support it on 10.2 or later.");
			gGamePrefs.oldOSWarned = true;
		}
	}
	

			/* CHECK TIME-BOMB */
	{
		unsigned long secs;
		DateTimeRec	d;

		GetDateTime(&secs);
		SecondsToDate(secs, &d);
		
		if ((d.year > 2003) ||
			((d.year == 2003) && (d.month > 7)))
		{
			DoFatalAlert("Sorry, but this beta has expired");
		}
	}



			/* CHECK OPENGL */
			
	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) 				// check for existance of OpenGL
		DoFatalAlert("This application needs OpenGL to function.  Please install OpenGL and try again.");
			

		/* CHECK SPROCKETS */
		
	if (!gOSX)
	{
		if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) ISpStartup) 							// check for existance of Input Sprocket
			DoFatalAlert("This application needs Input Sprocket to function.  Please install Game Sprockets and try again.");
	
		if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup) 							// check for existance of Draw Sprocket
			DoFatalAlert("This application needs Draw Sprocket to function.  Please install Game Sprockets and try again.");

	}
	
	
		/***********************************/
		/* SEE IF LITTLE-SNITCH IS RUNNING */
		/***********************************/
	
	{	
		ProcessSerialNumber psn = {kNoProcess, kNoProcess};
		ProcessInfoRec	info;
		
		Str255		s;
		const char snitch[] = "LittleSnitchDaemon";
		
		info.processName = s;
		info.processInfoLength = sizeof(ProcessInfoRec);
		info.processAppSpec = nil;
		
		while(GetNextProcess(&psn) == noErr)
		{
			char	pname[256];
			char	*matched;
		
			iErr = GetProcessInformation(&psn, &info);
			if (iErr)
				break;

			p2cstrcpy(pname, &s[0]);					// convert pstring to cstring
			
			matched = strstr (pname, "Snitc");			// does "Snitc" appear anywhere in the process name?
			if (matched != nil)
			{
				gLittleSnitch = true;
				break;
			}
		}
	}
#endif
}




#pragma mark -



/************** CALC FRAMES PER SECOND *****************/
//
// This version uses UpTime() which is only available on PCI Macs.
//

void CalcFramesPerSecond(void)
{
static UnsignedWide time;
UnsignedWide currTime;
unsigned long deltaTime;

	Microseconds(&currTime);
	deltaTime = currTime.lo - time.lo;

	gFramesPerSecond = 1000000.0f / deltaTime;

	if (gFramesPerSecond < MIN_FPS)			// (avoid divide by 0's later)
		gFramesPerSecond = MIN_FPS;

#if _DEBUG
	if (GetKeyState(SDL_SCANCODE_KP_PLUS))		// debug speed-up with KP_PLUS
		gFramesPerSecond = 10;
#endif

	gFramesPerSecondFrac = 1.0f/gFramesPerSecond;		// calc fractional for multiplication

	time = currTime;	// reset for next time interval
}


/********************* IS POWER OF 2 ****************************/

Boolean IsPowerOf2(int num)
{
int		i;
	
	i = 2;
	do
	{
		if (i == num)				// see if this power of 2 matches
			return(true);
		i *= 2;						// next power of 2	
	}while(i <= num);				// search until power is > number			

	return(false);
}

#pragma mark-

/******************* MY FLUSH EVENTS **********************/
//
// This call makes damed sure that there are no events anywhere in any event queue.
//

void MyFlushEvents(void)
{
	IMPLEMENT_ME_SOFT();
#if 0
//EventRecord 	theEvent;

	FlushEvents (everyEvent, REMOVE_ALL_EVENTS);	
	FlushEventQueue(GetMainEventQueue());

			/* POLL EVENT QUEUE TO BE SURE THINGS ARE FLUSHED OUT */
			
	while (GetNextEvent(mDownMask|mUpMask|keyDownMask|keyUpMask|autoKeyMask, &theEvent));


	FlushEvents (everyEvent, REMOVE_ALL_EVENTS);	
	FlushEventQueue(GetMainEventQueue());
#endif	
}
