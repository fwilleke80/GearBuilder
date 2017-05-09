#include "c4d.h"
#include "main.h"


#define PLUGIN_VERSION	String("1.04")


Bool PluginStart()
{
	GePrint("GearBuilder " + PLUGIN_VERSION);
	return RegisterGBcogwheel();
}

void PluginEnd()
{ }

Bool PluginMessage(Int32 id, void *data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			// Don't start plugin without resources
			return resource.Init();
	}

	return true;
}
