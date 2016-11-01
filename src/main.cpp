#include "c4d.h"
#include "tGBcogwheelClass.h"

// forward declarations
Bool RegisterGBcogwheel(void);

// Declare Crash handler
C4D_CrashHandler old_handler;

#define VersionString String("GearBuilder 1.02")

void SDKCrashHandler(CHAR *crashinfo)
{
	// don't forget to call the original handler!!!
	if (old_handler) (*old_handler)(crashinfo);
}

Bool PluginStart(void)
{
	// Installing the crash handler
	old_handler = C4DOS.CrashHandler;		// backup the original handler (must be called!)
	C4DOS.CrashHandler = SDKCrashHandler;	// insert the own handler
	
	GePrint("================================");
	GePrint(VersionString);

	// Register plugins
	if (!RegisterGBcogwheel()) return FALSE;

	return TRUE;
}

void PluginEnd(void)
{
	// Called when plugin is terminated
}

Bool PluginMessage(LONG id, void *data)
{
	// React to messages
	switch (id)
	{
		case C4DPL_INIT_SYS:
			// Don't start plugin without resources
			if (!resource.Init()) return FALSE;
			return TRUE;
	}

	return TRUE;
}
