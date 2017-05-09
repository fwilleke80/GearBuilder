#ifndef TGBCOGWHEEL_H__
#define TGBCOGWHEEL_H__

#include "c4d.h"


// Unique Plugin ID obtained from www.plugincafe.com
static const Int32 ID_TGBCOGWHEEL	= 1024778;

// Help path
#define HelpFile GeGetPluginPath() + "help"


// Plugin Class declaration
class tGBcogwheel : public TagData
{
	INSTANCEOF(tGBcogwheel, TagData)
	
private:
	BaseBitmap *icon_motor;
	BaseBitmap *icon_cogwheel;
	BaseBitmap *icon_belt;
	BaseBitmap *icon_shareaxis;
	
public:
	virtual Bool Init(GeListNode *node);
	virtual Bool Message(GeListNode *node, Int32 type, void *data);
	virtual Bool GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc);
	
	virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags);
	virtual EXECUTIONRESULT Execute(BaseTag *tag, BaseDocument *doc, BaseObject *op, BaseThread *bt, Int32 priority, EXECUTIONFLAGS flags);
	virtual Bool Draw(BaseTag *tag, BaseObject *op, BaseDraw *bd, BaseDrawHelp *bh);
	
	// Icon storage
	BaseBitmap *GetIcon_Cogwheel()          { return icon_cogwheel; }
	BaseBitmap *GetIcon_Motor()             { return icon_motor; }
	BaseBitmap *GetIcon_Belt()              { return icon_belt; }
	BaseBitmap *GetIcon_ShareAxis()         { return icon_shareaxis; }
	void SetIcon_Cogwheel(BaseBitmap *b)    { icon_cogwheel = b; }
	void SetIcon_Motor(BaseBitmap *b)       { icon_motor = b; }
	void SetIcon_Belt(BaseBitmap *b)        { icon_belt = b; }
	void SetIcon_ShareAxis(BaseBitmap *b)   { icon_shareaxis = b; }
	
	static NodeData *Alloc() { return NewObjClear(tGBcogwheel); }
	

	// Init members
	tGBcogwheel() : icon_motor(nullptr), icon_cogwheel(nullptr), icon_belt(nullptr), icon_shareaxis(nullptr)
	{	}

	// Clear icon storage on destruction
	~tGBcogwheel()
	{
		BaseBitmap::Free(icon_motor);
		BaseBitmap::Free(icon_cogwheel);
		BaseBitmap::Free(icon_belt);
		BaseBitmap::Free(icon_shareaxis);
	}
};


#endif // TGBCOGWHEEL_H__