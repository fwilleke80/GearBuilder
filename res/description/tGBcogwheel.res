/////////////////////////////////////////////////////////////
//
// GearBuilder :: Cogwheel Tag Resource
//
/////////////////////////////////////////////////////////////
//
// Cogwheel Tag :: Main Tag Description Resource
//
/////////////////////////////////////////////////////////////
// 2009-2010 by www.c4d-jack.de
/////////////////////////////////////////////////////////////

CONTAINER tGBcogwheel
{
	NAME tGBcogwheel;
	INCLUDE Texpression;

	GROUP COGWHEEL_PROPERTIES
	{
		DEFAULT 1;

		LONG	COGWHEEL_MODE				{ CYCLE { COGWHEEL_MODE_MOTOR; COGWHEEL_MODE_DRIVEN; } }
		
		GROUP COGWHEEL_GROUP_MOTOR
		{
			DEFAULT 1;

			BOOL	COGWHEEL_MOTOR_ACTIVE		{  }
			REAL	COGWHEEL_MOTOR_SPEED		{ UNIT DEGREE; STEP 1.0; }
			REAL	COGWHEEL_MOTOR_OFFSET		{ UNIT DEGREE; STEP 0.1; }
		}
		
		GROUP	COGWHEEL_GROUP_CONNECTION
		{
			DEFAULT 1;

			LINK	COGWHEEL_CONNECT_DRIVERTAG	{ ACCEPT { 1024778; } }
			LONG	COGWHEEL_CONNECT_TYPE				{ CYCLE { COGWHEEL_CONNECT_TYPE_COGWHEEL; COGWHEEL_CONNECT_TYPE_BELT; COGWHEEL_CONNECT_TYPE_SHARE; } }
			REAL	COGWHEEL_CONNECT_OFFSET			{ UNIT DEGREE; STEP 0.1; }

			GROUP
			{
				COLUMNS 2;
				
				BOOL	COGWHEEL_CONNECT_CONSTRAIN	{  }
				BOOL	COGWHEEL_CONNECT_INVERT			{  }
			}
			
			SEPARATOR													{  }
			
			REAL	COGWHEEL_CONNECT_BELT_LENGTH	{ UNIT METER; }
		}
		
		GROUP COGWHEEL_GROUP_PROPERTIES
		{
			DEFAULT 1;

			LINK	COGWHEEL_PROP_LINK					{ ACCEPT { 5188; 5181; 5170; } }
			LONG	COGWHEEL_PROP_RADIUSGETFROM	{ CYCLE { GETFROM_INNER; GETFROM_MIDDLE; GETFROM_OUTER; } }
			REAL	COGWHEEL_PROP_RADIUS				{ UNIT METER; MIN 0.001; STEP 1.0; }
			LONG	COGWHEEL_PROP_TOOTHCOUNT		{ MIN 2; }
		}
		
		GROUP COGWHEEL_GROUP_VIS
		{
			DEFAULT 1;
			
			BOOL	COGWHEEL_VIS_ENABLED		{  }
			
			GROUP
			{
				LAYOUTGROUP;
				COLUMNS 2;
				
				GROUP
				{
					
					BOOL	COGWHEEL_VIS_DRAWCIRCLE		{  }
				}

				GROUP
				{
					BOOL	COGWHEEL_VIS_DRAWCONNECTION	{  }
				}
			}
		}
		
		SEPARATOR							{ LINE; }
		
		BUTTON	COGWHEEL_CMD_OPENHELP		{  }
	}
}
