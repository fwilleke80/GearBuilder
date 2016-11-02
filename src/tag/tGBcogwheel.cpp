#include "c4d.h"
#include "c4d_symbols.h"
#include "tGBcogwheel.h"
#include "customgui_priority.h"


// Unique Plugin ID obtained from www.plugincafe.com
static const Int32 ID_TGBCOGWHEEL	1024778

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
	virtual Bool Message(GeListNode* node, Int32 type, void* data);
	virtual Bool GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc);
	
	virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags);
	virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags);
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


// Shows or hides a description from the node.
Bool ShowDescription(GeListNode *node, Description *descr, Int32 MyDescID, Bool DoHide)
{
	AutoAlloc<AtomArray> ar; if(!ar) return false;
	ar->Append(static_cast<C4DAtom*>(node));

	BaseContainer *bc = descr->GetParameterI(DescLevel(MyDescID), ar);
	if(bc) bc->SetBool(DESC_HIDE, !DoHide);
	else return false;

	return true;
}

// Calculate length of belt
Float GetBeltLength(const Float r1, const Float r2, const Float d)
{
	Float r_small = FMin(r1, r2);
	Float r_big = FMax(r1, r2);
	return pi * (r_small + r_big) + 2 * acos(r_big - r_small) + 2 * Sqrt(d*d - (r_big - r_small) * (r_big - r_small));
}


EXECUTIONRESULT tGBcogwheel::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
	if (!doc) return EXECUTIONRESULT_USERBREAK;
	if (!op) return EXECUTIONRESULT_USERBREAK;
	if (!tag) return EXECUTIONRESULT_USERBREAK;

	BaseContainer *bc = tag->GetDataInstance();
	if (!bc) return EXECUTIONRESULT_USERBREAK;

	Int32 Mode = bc->GetLong(COGWHEEL_MODE);

	// Motor mode
	if (Mode == COGWHEEL_MODE_MOTOR)
	{
		// Only calculate rotation if motor is active
		if (bc->GetBool(COGWHEEL_MOTOR_ACTIVE))
		{

			// Get motor settings
			Float Speed = bc->GetFloat(COGWHEEL_MOTOR_SPEED);
			Float Offset = bc->GetFloat(COGWHEEL_MOTOR_OFFSET);

			// Get current scene time in seconds
			Float Time = doc->GetTime().Get();

			//Calculate rotation
			Vector Rotation = op->GetRelRot();
			Rotation = Vector(Rotation.x, Rotation.y, Speed * Time + Offset);
			op->SetRelRot(Rotation);
		}
	}
	// Driven mode
	else
	{
		// Variables
		Float cRadius = RCO 0.0;
		Float Radius = RCO 0.0;
		Vector cRotation = Vector(RCO 0.0);
		Vector Rotation = Vector(RCO 0.0);
		Float Offset = RCO 0.0;
		Int32 Type = 0;

		// Name tag after object
		tag->SetName(op->GetName());

		BaseTag	*cTag = (BaseTag*)bc->GetLink(COGWHEEL_CONNECT_DRIVERTAG, doc);
		if (!cTag) return EXECUTIONRESULT_USERBREAK;
		BaseContainer *tc = cTag->GetDataInstance();
		if (!tc) return EXECUTIONRESULT_USERBREAK;
		BaseObject *cObj = cTag->GetObject();
		if (!cObj) return EXECUTIONRESULT_USERBREAK;

		// Connection Type
		Type = bc->GetLong(COGWHEEL_CONNECT_TYPE);

		// Get Radius of both cogwheels
		Radius = bc->GetFloat(COGWHEEL_PROP_RADIUS);
		cRadius = tc->GetFloat(COGWHEEL_PROP_RADIUS);

		// Get rotation of connected cogwheel
		cRotation = cObj->GetRelRot();

		// Get current rotation of this cogwheel
		Rotation = op->GetRelRot();

		// Get Offset
		Offset = bc->GetFloat(COGWHEEL_CONNECT_OFFSET);

		// Calculate Rotation
		switch (Type)
		{
			case COGWHEEL_CONNECT_TYPE_COGWHEEL:
				// Cogwheel connection
				Rotation = Vector(Rotation.x, Rotation.y, (cRotation.z * (cRadius / Radius)) * -1 + Offset);
				break;

			case COGWHEEL_CONNECT_TYPE_SHARE:
				// Co-rotation on same axis
				Rotation = Vector(Rotation.x, Rotation.y, cRotation.z + Offset);
				break;

			case COGWHEEL_CONNECT_TYPE_BELT:
				// Co-rotation via Belt connection
				Rotation = Vector(Rotation.x, Rotation.y, cRotation.z * (cRadius / Radius) + Offset);
				break;
		}

		// Invert rotation
		if (bc->GetBool(COGWHEEL_CONNECT_INVERT))
		{
			Rotation.z *= RCO -1.0;
		}

		// Set Rotation
		op->SetRelRot(Rotation);

		// Calculate position, if "Same Axis" or "Same Plane" is enabled
		Bool Constrain = bc->GetBool(COGWHEEL_CONNECT_CONSTRAIN);

		if (Constrain && Mode != COGWHEEL_MODE_MOTOR)
		{
			Matrix m = op->GetMg();				// Get global Matrix of this wheel
			Matrix cm = cObj->GetMg();		// Get global Matrix of connected wheel

			Vector Pos = m.off * !cm;			// Transform position to connected wheel's local space

			if (Type == COGWHEEL_CONNECT_TYPE_SHARE)
			{
				Pos.x = RCO 0.0;						// Set X and Y coord to 0
				Pos.y = RCO 0.0;
			}
			else
			{
				Pos.z = RCO 0.0;
			}

			m.off = Pos * cm;							// Transform position back to global space

			op->SetMg(m);									// Set global Matrix of object
		}

		// Set Belt length info
		if (bc->GetLong(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN && bc->GetLong(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_BELT)
		{
			bc->SetFloat(COGWHEEL_CONNECT_BELT_LENGTH, GetBeltLength(cRadius, Radius, Len(cObj->GetMg().off - op->GetMg().off)));
		}
	}

	return EXECUTIONRESULT_OK;
}

Bool tGBcogwheel::Init(GeListNode *node)
{
	// Get pointer to tag's Container
	BaseTag			*tag  = (BaseTag*)node;
	BaseContainer	*data = tag->GetDataInstance();

	// Cogwheel mode
	data->SetLong(COGWHEEL_MODE, COGWHEEL_MODE_DRIVEN);

	// Motor settings
	data->SetBool(COGWHEEL_MOTOR_ACTIVE, true);
	data->SetFloat(COGWHEEL_MOTOR_SPEED, Rad(RCO 45.0));

	// Connection Settings
	data->SetLong(COGWHEEL_CONNECT_TYPE, COGWHEEL_CONNECT_TYPE_COGWHEEL);

	// Cogwheel Properties
	data->SetFloat(COGWHEEL_PROP_RADIUS, RCO 100.0);
	data->SetLong(COGWHEEL_PROP_RADIUSGETFROM, GETFROM_INNER);

	// Priority settings
	GeData d;
	if (node->GetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_GET_0))
	{
		PriorityData *pd = (PriorityData*)d.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);
		if (pd) pd->SetPriorityValue(PRIORITYVALUE_CAMERADEPENDENT, GeData(true));
		node->SetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_SET_0);
	}

	// Load icons for tag

  // Motor
 	BaseBitmap *icon = BaseBitmap::Alloc();
	if (!icon) return false;
	Filename fn = GeGetPluginPath() + "res" + "tGBmotor.tif";
	icon->Init(fn);
	SetIcon_Motor(icon);

	// Cogwheel
	icon = BaseBitmap::Alloc();
	if (!icon) return false;
	fn = GeGetPluginPath() + "res" + "tGBcogwheel.tif";
	icon->Init(fn);
	SetIcon_Cogwheel(icon);

	// Belt
	icon = BaseBitmap::Alloc();
	if (!icon) return false;
	fn = GeGetPluginPath() + "res" + "tGBbelt.tif";
	icon->Init(fn);
	SetIcon_Belt(icon);

	// Share Axis
	icon = BaseBitmap::Alloc();
	if (!icon) return false;
	fn = GeGetPluginPath() + "res" + "tGBshareaxis.tif";
	icon->Init(fn);
	SetIcon_ShareAxis(icon);

	return true;
}

Bool tGBcogwheel::GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags)
{
	// Load description (user interface)
	if (!description->LoadDescription(ID_TGBCOGWHEEL)) return false;
	flags |= DESCFLAGS_DESC_LOADED;

	// Get pointer to tag's Container
	BaseTag			*tag  = (BaseTag*)node;
	BaseContainer	*data = tag->GetDataInstance();
	BaseDocument	*doc = tag->GetDocument();
	if (!doc) return true;

	BaseObject *pObj = data->GetObjectLink(COGWHEEL_PROP_LINK, doc);
	if (pObj)
	{
		Float Radius = RCO 0.0;
		Int32 tCount = 2;

		if (pObj->GetType() == Osplinecogwheel)
		{
			switch (data->GetLong(COGWHEEL_PROP_RADIUSGETFROM))
			{
				case GETFROM_INNER:
					Radius = pObj->GetDataInstance()->GetFloat(PRIM_COGWHEEL_IRAD);
					break;
				case GETFROM_MIDDLE:
					Radius = pObj->GetDataInstance()->GetFloat(PRIM_COGWHEEL_MRAD);
					break;
				case GETFROM_OUTER:
					Radius = pObj->GetDataInstance()->GetFloat(PRIM_COGWHEEL_ORAD);
					break;
			}
			tCount = pObj->GetDataInstance()->GetLong(PRIM_COGWHEEL_TEETH);
		}
		else if (pObj->GetType() == Osplinecircle)
		{
			Radius = pObj->GetDataInstance()->GetFloat(PRIM_CIRCLE_RADIUS);
		}
		else if (pObj->GetType() == Ocylinder)
		{
			Radius = pObj->GetDataInstance()->GetFloat(PRIM_CYLINDER_RADIUS);
		}

		data->SetFloat(COGWHEEL_PROP_RADIUS, Radius);
		data->SetLong(COGWHEEL_PROP_TOOTHCOUNT, tCount);
	}

	// Show Motor or Connection group, depending on tag type
	ShowDescription(node, description, COGWHEEL_GROUP_MOTOR, data->GetLong(COGWHEEL_MODE) == COGWHEEL_MODE_MOTOR);
	ShowDescription(node, description, COGWHEEL_GROUP_CONNECTION, data->GetLong(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN);

	// Show Belt length info, if connection type is set to "Belt"
	ShowDescription(node, description, COGWHEEL_CONNECT_BELT_LENGTH, false /*data->GetLong(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN && data->GetLong(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_BELT*/);

	return true;
}

Bool tGBcogwheel::Message(GeListNode* node, Int32 type, void* data)
{
	BaseTag *tag = (BaseTag*)node; if(!tag) return true;
	BaseContainer *bc = tag->GetDataInstance(); if (!bc) return true;
	BaseDocument *doc = tag->GetDocument(); if (!doc) return true;
	BaseObject *Obj = tag->GetObject(); if (!Obj) return true;

	switch (type)
	{
		case MSG_DESCRIPTION_COMMAND:
		{
			// If Button clicked, create Null Object
			DescriptionCommand *dc = (DescriptionCommand*) data;
			switch (dc->id[0].id)
			{
				case COGWHEEL_CMD_OPENHELP:
				{
					if (!GeExecuteFile(HelpFile + "index.html"))
						GeOutString(GeLoadString(IDS_HELP_ERROR), GEMB_OK);
				}
			}
			break;
		}

		case MSG_GETCUSTOMICON:
		{
			// Set custom icon, depending on tag connection mode
			GetCustomIconData *cid = (GetCustomIconData*)data;
			IconData *id = cid->dat;

			if (id) {
				Int32 Mode = bc->GetLong(COGWHEEL_MODE);
				if (Mode == COGWHEEL_MODE_DRIVEN)
				{
					switch (bc->GetLong(COGWHEEL_CONNECT_TYPE))
					{
						case COGWHEEL_CONNECT_TYPE_COGWHEEL:
							id->bmp = this->GetIcon_Cogwheel();
							break;
						case COGWHEEL_CONNECT_TYPE_SHARE:
							id->bmp = this->GetIcon_ShareAxis();
							break;
						case COGWHEEL_CONNECT_TYPE_BELT:
							id->bmp = this->GetIcon_Belt();
							break;
					}
				}
				else
					id->bmp = tGBcogwheel::GetIcon_Motor();

				// Adjust image dimensions
				id->x = 0;
				id->y = 0;
				id->w = 32;
				id->h = 32;

				// Yep, we changed something
				cid->filled = true;
			}
			break;
		}
	}

	return SUPER::Message(node,type,data);
}

Bool tGBcogwheel::Draw(BaseTag* tag, BaseObject* op, BaseDraw* bd, BaseDrawHelp* bh)
{
	// Variables
	Matrix mCircle = Matrix();
	Vector col = Vector();
	BaseTag *cTag = nullptr;			// Connected (driver) tag
	BaseObject *cObj = nullptr;	// Connected (driver) object
	Int32 Mode = 0;
	Float Radius = RCO 0.0;
	BaseDocument *doc = nullptr;
	BaseContainer *bc = nullptr;

	if (!op || !tag) goto UnhappyEnding;
	doc = tag->GetDocument();
	if (!doc) goto UnhappyEnding;
	bc = tag->GetDataInstance();
	if (!bc) goto UnhappyEnding;

	if (bc->GetBool(COGWHEEL_VIS_ENABLED) != true) goto HappyEnding;

	// Settings from Tag
	Mode = bc->GetLong(COGWHEEL_MODE);
	Radius = bc->GetFloat(COGWHEEL_PROP_RADIUS);

	// Draw Circle
	bd->SetMatrix_Matrix(nullptr, Matrix());
	mCircle = op->GetMg();
	mCircle.v1 = !(mCircle.v1) * Radius;
	mCircle.v2 = !(mCircle.v2) * Radius;

	if (bc->GetBool(COGWHEEL_VIS_DRAWCIRCLE))
	{
		if (Mode == COGWHEEL_MODE_MOTOR)
			col = Vector(RCO 0.95, RCO 0.05, RCO 0.0);
		else if (Mode == COGWHEEL_MODE_MOTOR)
			col = Vector(RCO 0.2, RCO 0.2, RCO 1.0);
		bd->SetPen(col);
		bd->DrawCircle(mCircle);
	}

	// Draw Connection
	if (Mode == COGWHEEL_MODE_DRIVEN && bc->GetBool(COGWHEEL_VIS_DRAWCONNECTION))
	{
		cTag = (BaseTag*)bc->GetLink(COGWHEEL_CONNECT_DRIVERTAG, doc);
		if (!cTag) goto UnhappyEnding;
		cObj = (BaseObject*)cTag->GetObject();
		if (!cObj) goto UnhappyEnding;

		Vector p1 = Vector(op->GetMg().off);
		Vector p2 = Vector(cObj->GetMg().off);

		Float cRadius = cTag->GetDataInstance()->GetFloat(COGWHEEL_PROP_RADIUS);
		cRadius = (Radius + cRadius) / RCO 2.0;

		if (bc->GetLong(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_COGWHEEL)
			col = Vector(RCO 1.0, RCO 1.0, RCO 0.0);
		else if (bc->GetLong(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_BELT)
			col = Vector(RCO 1.0, RCO 0.5, RCO 0.0);
		else
			col = Vector(RCO 0.5, RCO 1.0, RCO 0.2);

		bd->SetPen(col);
		bd->DrawLine2D(bd->WS(p1), bd->WS(p2));

		Vector p = bd->WS(p1);
		bd->DrawCircle2D(p.x, p.y, RCO 5.0);
	}

HappyEnding:
	return true;

UnhappyEnding:
	return false;
}

Bool tGBcogwheel::GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc)
{
	// Enable and disable ('gray out') user controls
	BaseDocument *doc = node->GetDocument();
	BaseContainer &data = *((BaseObject*)node)->GetDataInstance();

	Bool Property_Object_Linked = (true && data.GetObjectLink(COGWHEEL_PROP_LINK, doc));
	Bool Property_Object_Is_Cogwheel = false;
	if (Property_Object_Linked)
		Property_Object_Is_Cogwheel = (data.GetObjectLink(COGWHEEL_PROP_LINK, doc)->GetType() == Osplinecogwheel);

	switch (id[0].id)
	{
		case COGWHEEL_VIS_DRAWCIRCLE:
			return data.GetBool(COGWHEEL_VIS_ENABLED);
		case COGWHEEL_VIS_DRAWCONNECTION:
			return data.GetBool(COGWHEEL_VIS_ENABLED) && data.GetLong(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN;
		case COGWHEEL_PROP_RADIUS:
			return (!Property_Object_Linked);
		case COGWHEEL_PROP_RADIUSGETFROM:
			return (Property_Object_Linked);
		case COGWHEEL_PROP_TOOTHCOUNT:
			return (!Property_Object_Linked && !Property_Object_Is_Cogwheel);
	}

	return true;
}


Bool RegisterGBcogwheel(void)
{
	// decide by name if the plugin shall be registered - just for user convenience
	String name = GeLoadString(IDS_TGBCOGWHEEL); if (!name.Content()) return true;
	return RegisterTagPlugin(ID_TGBCOGWHEEL, name, TAG_VISIBLE|TAG_EXPRESSION, tGBcogwheel::Alloc, "tGBcogwheel", AutoBitmap("tGBcogwheel.tif"), 0);
}
