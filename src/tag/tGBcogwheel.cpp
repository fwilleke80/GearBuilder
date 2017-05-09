#include "c4d.h"
#include "tGBcogwheel_class.h"
#include "customgui_priority.h"
#include "main.h"
#include "c4d_symbols.h"
#include "tGBcogwheel.h"


// Shows or hides a description from the node.
static Bool ShowDescription(GeListNode *node, Description *descr, Int32 MyDescID, Bool DoHide)
{
	AutoAlloc<AtomArray> ar;
	if(!ar)
		return false;
	ar->Append(static_cast<C4DAtom*>(node));

	BaseContainer *bc = descr->GetParameterI(DescLevel(MyDescID), ar);
	if (!bc)
		return false;

	bc->SetBool(DESC_HIDE, !DoHide);

	return true;
}

// Calculate length of belt
static inline Float GetBeltLength(const Float r1, const Float r2, const Float d)
{
	Float r_small = FMin(r1, r2);
	Float r_big = FMax(r1, r2);
	return PI * (r_small + r_big) + 2 * ACos(r_big - r_small) + 2 * Sqrt(d*d - (r_big - r_small) * (r_big - r_small));
}


EXECUTIONRESULT tGBcogwheel::Execute(BaseTag *tag, BaseDocument *doc, BaseObject *op, BaseThread *bt, Int32 priority, EXECUTIONFLAGS flags)
{
	if (!doc || !op || !tag) return EXECUTIONRESULT_USERBREAK;

	BaseContainer *bc = tag->GetDataInstance();
	if (!bc) return EXECUTIONRESULT_USERBREAK;

	Int32 cogwheelMode = bc->GetInt32(COGWHEEL_MODE);

	// Motor mode
	if (cogwheelMode == COGWHEEL_MODE_MOTOR)
	{
		// Only calculate rotation if motor is active
		if (bc->GetBool(COGWHEEL_MOTOR_ACTIVE))
		{

			// Get motor settings
			Float speed = bc->GetFloat(COGWHEEL_MOTOR_SPEED);
			Float offset = bc->GetFloat(COGWHEEL_MOTOR_OFFSET);

			// Get current scene time in seconds
			Float time = doc->GetTime().Get();

			//Calculate rotation
			Vector rotation = op->GetRelRot();
			rotation = Vector(rotation.x, rotation.y, speed * time + offset);
			op->SetRelRot(rotation);
		}
	}
	// Driven mode
	else
	{
		// Variables
		Float   cRadius = 0.0;
		Float   radius = 0.0;
		Vector  cRotation = Vector();
		Vector  rotation = Vector();
		Float   offset = 0.0;
		Int32   type = 0;

		// Name tag after object
		tag->SetName(op->GetName());

		BaseTag	*cTag = (BaseTag*)bc->GetLink(COGWHEEL_CONNECT_DRIVERTAG, doc);
		if (!cTag)
			return EXECUTIONRESULT_USERBREAK;
		BaseContainer *tc = cTag->GetDataInstance();
		if (!tc)
			return EXECUTIONRESULT_USERBREAK;
		BaseObject *cObj = cTag->GetObject();
		if (!cObj)
			return EXECUTIONRESULT_USERBREAK;

		// Connection Type
		type = bc->GetInt32(COGWHEEL_CONNECT_TYPE);

		// Get Radius of both cogwheels
		radius = ClampValue(bc->GetFloat(COGWHEEL_PROP_RADIUS), 0.0001, MAXVALUE_FLOAT);
		cRadius = tc->GetFloat(COGWHEEL_PROP_RADIUS);

		// Get rotation of connected cogwheel
		cRotation = cObj->GetRelRot();

		// Get current rotation of this cogwheel
		rotation = op->GetRelRot();

		// Get Offset
		offset = bc->GetFloat(COGWHEEL_CONNECT_OFFSET);

		// Calculate Rotation
		switch (type)
		{
			case COGWHEEL_CONNECT_TYPE_COGWHEEL:
				// Cogwheel connection
				rotation = Vector(rotation.x, rotation.y, (cRotation.z * (cRadius / radius)) * -1 + offset);
				break;

			case COGWHEEL_CONNECT_TYPE_SHARE:
				// Co-rotation on same axis
				rotation = Vector(rotation.x, rotation.y, cRotation.z + offset);
				break;

			case COGWHEEL_CONNECT_TYPE_BELT:
				// Co-rotation via Belt connection
				rotation = Vector(rotation.x, rotation.y, cRotation.z * (cRadius / radius) + offset);
				break;
		}

		// Invert rotation
		if (bc->GetBool(COGWHEEL_CONNECT_INVERT))
		{
			rotation.z *= -1.0;
		}

		// Set Rotation
		op->SetRelRot(rotation);

		// Calculate position, if "Same Axis" or "Same Plane" is enabled
		Bool constrain = bc->GetBool(COGWHEEL_CONNECT_CONSTRAIN);

		if (constrain && cogwheelMode != COGWHEEL_MODE_MOTOR)
		{
			Matrix m(op->GetMg());        // Get global Matrix of this wheel
			Matrix cm(cObj->GetMg());     // Get global Matrix of connected wheel
			Vector pos(~cm * m.off);      // Transform position to connected wheel's local space

			if (type == COGWHEEL_CONNECT_TYPE_SHARE)
			{
				// Set X and Y coord to 0
				pos.x = 0.0;
				pos.y = 0.0;
			}
			else
			{
				pos.z = 0.0;
			}

			m.off = cm * pos;		// Transform position back to global space

			op->SetMg(m);				// Set global Matrix of object
		}

		// Set Belt length info
		if (bc->GetInt32(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN && bc->GetInt32(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_BELT)
		{
			bc->SetFloat(COGWHEEL_CONNECT_BELT_LENGTH, GetBeltLength(cRadius, radius, Len(cObj->GetMg().off - op->GetMg().off)));
		}
	}

	return EXECUTIONRESULT_OK;
}

Bool tGBcogwheel::Init(GeListNode *node)
{
	// Get pointer to tag's Container
	BaseTag *tag  = static_cast<BaseTag*>(node);
	BaseContainer	*data = tag->GetDataInstance();

	// Cogwheel mode
	data->SetInt32(COGWHEEL_MODE, COGWHEEL_MODE_DRIVEN);

	// Motor settings
	data->SetBool(COGWHEEL_MOTOR_ACTIVE, true);
	data->SetFloat(COGWHEEL_MOTOR_SPEED, Rad(45.0));

	// Connection Settings
	data->SetInt32(COGWHEEL_CONNECT_TYPE, COGWHEEL_CONNECT_TYPE_COGWHEEL);

	// Cogwheel Properties
	data->SetFloat(COGWHEEL_PROP_RADIUS, 100.0);
	data->SetInt32(COGWHEEL_PROP_RADIUSGETFROM, GETFROM_INNER);

	// Priority settings
	GeData d;
	if (node->GetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_GET_0))
	{
		PriorityData *pd = (PriorityData*)d.GetCustomDataType(CUSTOMGUI_PRIORITY_DATA);
		if (pd) pd->SetPriorityValue(PRIORITYVALUE_CAMERADEPENDENT, GeData(true));
		node->SetParameter(DescLevel(EXPRESSION_PRIORITY), d, DESCFLAGS_SET_0);
	}

  // Load Motor icon
 	BaseBitmap *icon = BaseBitmap::Alloc();
	if (!icon)
		return false;
	Filename fn = GeGetPluginPath() + "res" + "tGBmotor.tif";
	icon->Init(fn);
	SetIcon_Motor(icon);

	// Load Cogwheel icon
	icon = BaseBitmap::Alloc();
	if (!icon)
		return false;
	fn = GeGetPluginPath() + "res" + "tGBcogwheel.tif";
	icon->Init(fn);
	SetIcon_Cogwheel(icon);

	// Load Belt icon
	icon = BaseBitmap::Alloc();
	if (!icon)
		return false;
	fn = GeGetPluginPath() + "res" + "tGBbelt.tif";
	icon->Init(fn);
	SetIcon_Belt(icon);

	// Load Share Axis icon
	icon = BaseBitmap::Alloc();
	if (!icon)
		return false;
	fn = GeGetPluginPath() + "res" + "tGBshareaxis.tif";
	icon->Init(fn);
	SetIcon_ShareAxis(icon);

	return SUPER::Init(node);
}

Bool tGBcogwheel::GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags)
{
	// Load description (user interface)
	if (!description->LoadDescription(ID_TGBCOGWHEEL))
		return false;
	flags |= DESCFLAGS_DESC_LOADED;

	// Get pointer to tag's Container
	BaseTag       *tag  = static_cast<BaseTag*>(node);
	BaseContainer *data = tag->GetDataInstance();
	BaseDocument  *doc = tag->GetDocument();
	if (!doc)
		return true;

	BaseObject *pObj = data->GetObjectLink(COGWHEEL_PROP_LINK, doc);
	if (pObj)
	{
		Float radius = 0.0;
		Int32 tCount = 2;

		if (pObj->GetType() == Osplinecogwheel)
		{
			switch (data->GetInt32(COGWHEEL_PROP_RADIUSGETFROM))
			{
				case GETFROM_INNER:
					radius = pObj->GetDataInstance()->GetFloat(PRIM_COGWHEEL_IRAD);
					break;
				case GETFROM_MIDDLE:
					radius = pObj->GetDataInstance()->GetFloat(PRIM_COGWHEEL_MRAD);
					break;
				case GETFROM_OUTER:
					radius = pObj->GetDataInstance()->GetFloat(PRIM_COGWHEEL_ORAD);
					break;
			}
			tCount = pObj->GetDataInstance()->GetInt32(PRIM_COGWHEEL_TEETH);
		}
		else if (pObj->GetType() == Osplinecircle)
		{
			radius = pObj->GetDataInstance()->GetFloat(PRIM_CIRCLE_RADIUS);
		}
		else if (pObj->GetType() == Ocylinder)
		{
			radius = pObj->GetDataInstance()->GetFloat(PRIM_CYLINDER_RADIUS);
		}

		data->SetFloat(COGWHEEL_PROP_RADIUS, radius);
		data->SetInt32(COGWHEEL_PROP_TOOTHCOUNT, tCount);
	}

	// Show Motor or Connection group, depending on tag type
	ShowDescription(node, description, COGWHEEL_GROUP_MOTOR, data->GetInt32(COGWHEEL_MODE) == COGWHEEL_MODE_MOTOR);
	ShowDescription(node, description, COGWHEEL_GROUP_CONNECTION, data->GetInt32(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN);

	// Show Belt length info, if connection type is set to "Belt"
	ShowDescription(node, description, COGWHEEL_CONNECT_BELT_LENGTH, false);

	return SUPER::GetDDescription(node, description, flags);
}

Bool tGBcogwheel::Message(GeListNode *node, Int32 type, void *data)
{
	BaseTag *tag = static_cast<BaseTag*>(node);
	if (!tag)
		return true;
	BaseContainer *bc = tag->GetDataInstance();
	if (!bc)
		return true;
	BaseDocument *doc = tag->GetDocument();
	if (!doc)
		return true;

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
				Int32 Mode = bc->GetInt32(COGWHEEL_MODE);
				if (Mode == COGWHEEL_MODE_DRIVEN)
				{
					switch (bc->GetInt32(COGWHEEL_CONNECT_TYPE))
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

	return SUPER::Message(node, type, data);
}

Bool tGBcogwheel::Draw(BaseTag *tag, BaseObject *op, BaseDraw *bd, BaseDrawHelp *bh)
{
	// Variables
	Matrix mCircle;
	Vector col;
	BaseTag *cTag = nullptr;     // Connected (driver) tag
	BaseObject *cObj = nullptr;  // Connected (driver) object
	Int32 Mode = 0;
	Float Radius = 0.0;
	BaseDocument *doc = nullptr;
	BaseContainer *bc = nullptr;

	if (!op || !tag)
		return false;
	doc = tag->GetDocument();
	if (!doc)
		return false;
	bc = tag->GetDataInstance();
	if (!bc)
		return false;

	if (bc->GetBool(COGWHEEL_VIS_ENABLED) != true)
		return true;

	// Settings from Tag
	Mode = bc->GetInt32(COGWHEEL_MODE);
	Radius = bc->GetFloat(COGWHEEL_PROP_RADIUS);

	// Draw Circle
	bd->SetMatrix_Matrix(nullptr, Matrix());
	mCircle = op->GetMg();
	mCircle.v1 = !(mCircle.v1) * Radius;
	mCircle.v2 = !(mCircle.v2) * Radius;

	if (bc->GetBool(COGWHEEL_VIS_DRAWCIRCLE))
	{
		if (Mode == COGWHEEL_MODE_MOTOR)
			col = Vector(0.95, 0.05, 0.0);
		else if (Mode == COGWHEEL_MODE_MOTOR)
			col = Vector(0.2, 0.2, 1.0);
		bd->SetPen(col);
		bd->DrawCircle(mCircle);
	}

	// Draw Connection
	if (Mode == COGWHEEL_MODE_DRIVEN && bc->GetBool(COGWHEEL_VIS_DRAWCONNECTION))
	{
		cTag = (BaseTag*)bc->GetLink(COGWHEEL_CONNECT_DRIVERTAG, doc);
		if (!cTag)
			return false;
		cObj = (BaseObject*)cTag->GetObject();
		if (!cObj)
			return false;

		Vector p1 = Vector(op->GetMg().off);
		Vector p2 = Vector(cObj->GetMg().off);

		Float cRadius = cTag->GetDataInstance()->GetFloat(COGWHEEL_PROP_RADIUS);
		cRadius = (Radius + cRadius) / 2.0;

		if (bc->GetInt32(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_COGWHEEL)
			col = Vector(1.0, 1.0, 0.0);
		else if (bc->GetInt32(COGWHEEL_CONNECT_TYPE) == COGWHEEL_CONNECT_TYPE_BELT)
			col = Vector(1.0, 0.5, 0.0);
		else
			col = Vector(0.5, 1.0, 0.2);

		bd->SetPen(col);
		bd->DrawLine2D(bd->WS(p1), bd->WS(p2));

		Vector p = bd->WS(p1);
		bd->DrawCircle2D((Int32)p.x, (Int32)p.y, 5.0);
	}

	return SUPER::Draw(tag, op, bd, bh);
}

Bool tGBcogwheel::GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc)
{
	if (!node)
		return false;
	
	// Enable and disable ('gray out') user controls
	BaseDocument *doc = node->GetDocument();
	if (!doc)
		return false;
	BaseContainer *data = static_cast<BaseObject*>(node)->GetDataInstance();
	if (!data)
		return false;

	Bool propertyObject = (true && data->GetObjectLink(COGWHEEL_PROP_LINK, doc));
	Bool propertyObjectIsCogwheel = false;
	if (propertyObject)
		propertyObjectIsCogwheel = (data->GetObjectLink(COGWHEEL_PROP_LINK, doc)->GetType() == Osplinecogwheel);

	switch (id[0].id)
	{
		case COGWHEEL_VIS_DRAWCIRCLE:
			return data->GetBool(COGWHEEL_VIS_ENABLED);
		case COGWHEEL_VIS_DRAWCONNECTION:
			return data->GetBool(COGWHEEL_VIS_ENABLED) && data->GetInt32(COGWHEEL_MODE) == COGWHEEL_MODE_DRIVEN;
		case COGWHEEL_PROP_RADIUS:
			return (!propertyObject);
		case COGWHEEL_PROP_RADIUSGETFROM:
			return (propertyObject);
		case COGWHEEL_PROP_TOOTHCOUNT:
			return (!propertyObject && !propertyObjectIsCogwheel);
	}

	return SUPER::GetDEnabling(node, id, t_data, flags, itemdesc);
}


Bool RegisterGBcogwheel()
{
	return RegisterTagPlugin(ID_TGBCOGWHEEL, GeLoadString(IDS_TGBCOGWHEEL), TAG_VISIBLE|TAG_EXPRESSION, tGBcogwheel::Alloc, "tGBcogwheel", AutoBitmap("tGBcogwheel.tif"), 0);
}
