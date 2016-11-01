/////////////////////////////////////////////////////////////
//
// GearBuilder :: Cogwheel Tag
//
/////////////////////////////////////////////////////////////
//
// tGBcogwheel class declaration
//
/////////////////////////////////////////////////////////////
// 2009-2010 by www.c4d-jack.de
/////////////////////////////////////////////////////////////


// Plugin Class declaration
class tGBcogwheel : public TagData
{
	INSTANCEOF(tGBcogwheel,TagData)

	private:
		BaseBitmap *icon_motor;
		BaseBitmap *icon_cogwheel;
		BaseBitmap *icon_belt;
		BaseBitmap *icon_shareaxis;

	public:
		virtual Bool Init(GeListNode *node);
		virtual Bool Message(GeListNode* node, LONG type, void* data);
		virtual Bool GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc);

		virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags);
		virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, LONG priority, EXECUTIONFLAGS flags);
		virtual Bool Draw(BaseTag* tag, BaseObject* op, BaseDraw* bd, BaseDrawHelp* bh);

		// Icon storage
		BaseBitmap *GetIcon_Cogwheel(void)		{ return icon_cogwheel; }
		BaseBitmap *GetIcon_Motor(void)				{ return icon_motor; }
		BaseBitmap *GetIcon_Belt(void)				{ return icon_belt; }
		BaseBitmap *GetIcon_ShareAxis(void)		{ return icon_shareaxis; }
		void SetIcon_Cogwheel(BaseBitmap *b)	{ icon_cogwheel = b; }
		void SetIcon_Motor(BaseBitmap *b)			{ icon_motor = b; }
		void SetIcon_Belt(BaseBitmap *b)			{ icon_belt = b; }
		void SetIcon_ShareAxis(BaseBitmap *b)	{ icon_shareaxis = b; }

		static NodeData *Alloc(void) { return gNew tGBcogwheel; }

		// Clear icon storage on destruction
		~tGBcogwheel(void)
		{
			BaseBitmap *icon = icon_cogwheel;
			if (icon) BaseBitmap::Free(icon);
			icon = icon_motor;
			if (icon) BaseBitmap::Free(icon);
			icon = icon_belt;
			if (icon) BaseBitmap::Free(icon);
			icon = icon_shareaxis;
			if (icon) BaseBitmap::Free(icon);
		}
};

// Unique Plugin ID obtained from www.plugincafe.com
#define ID_TGBCOGWHEEL	1024778

// Help path
#define HelpFile GeGetPluginPath() + "help"
