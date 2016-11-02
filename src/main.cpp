#include "c4d.h"
#include "main.h"


Bool PluginStart()
{
	if (!RegisterGBcogwheel()) return false;

	return true;
}

void PluginEnd()
{
}

Bool PluginMessage(Int32 id, void *data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			// Don't start plugin without resources
			if (!resource.Init()) return false;
			return true;
	}

	return true;
}
