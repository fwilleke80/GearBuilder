/////////////////////////////////////////////////////////////
//
// SceneDocumentor :: Cogwheel Tag
//
/////////////////////////////////////////////////////////////
//
// tGBcogwheel ID definitions
//
/////////////////////////////////////////////////////////////
// 2009-2010 by www.c4d-jack.de
/////////////////////////////////////////////////////////////

#ifndef _tGBcogwheel_H_
#define _tGBcogwheel_H_

enum
{
	COGWHEEL_PROPERTIES							= 10000,

	COGWHEEL_MODE										= 10001,
	COGWHEEL_MODE_MOTOR							= 10002,
	COGWHEEL_MODE_DRIVEN						= 10003,

	COGWHEEL_GROUP_MOTOR						= 10010,
	COGWHEEL_MOTOR_ACTIVE						= 10011,
	COGWHEEL_MOTOR_SPEED						= 10012,
	COGWHEEL_MOTOR_OFFSET						= 10013,

	COGWHEEL_GROUP_CONNECTION				= 10020,
	COGWHEEL_CONNECT_DRIVERTAG			= 10021,
	COGWHEEL_CONNECT_TYPE						= 10022,
	COGWHEEL_CONNECT_TYPE_COGWHEEL	= 10023,
	COGWHEEL_CONNECT_TYPE_SHARE			= 10024,
	COGWHEEL_CONNECT_TYPE_BELT			= 10025,
	COGWHEEL_CONNECT_OFFSET					= 10026,
	COGWHEEL_CONNECT_CONSTRAIN			= 10027,
	COGWHEEL_CONNECT_INVERT					= 10028,
	COGWHEEL_CONNECT_BELT_LENGTH		= 10029,

	COGWHEEL_GROUP_PROPERTIES				= 10030,
	COGWHEEL_PROP_RADIUS						= 10031,
	COGWHEEL_PROP_LINK							= 10032,
	COGWHEEL_PROP_TOOTHCOUNT				= 10033,
	COGWHEEL_PROP_RADIUSGETFROM			= 10034,
		GETFROM_INNER										= 1,
		GETFROM_OUTER										= 2,
		GETFROM_MIDDLE									= 3,

	COGWHEEL_GROUP_VIS							= 10040,
	COGWHEEL_VIS_ENABLED						= 10041,
	COGWHEEL_VIS_DRAWCIRCLE					= 10042,
	COGWHEEL_VIS_DRAWCONNECTION			= 10043,

	COGWHEEL_CMD_OPENHELP						= 10050
};

#endif
