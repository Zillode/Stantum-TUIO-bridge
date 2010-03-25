/*!	
	\file		SMT.h
	\brief		Low level API for communication with Stantum MultiTouch sensors
	\author		Stantum
	\version	1.0	
	\date		March 13th, 2008
*/

#ifdef WIN32
	#ifdef STATIC_LIBRARY
		#define EXPORT
	#else
		#ifdef DLL_EXPORT
			#define EXPORT __declspec(dllexport)
		#else
			#define	EXPORT __declspec(dllimport)
		#endif
	#endif
#else
	#define EXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct SMT_SENSOR_PRIVATE;

/*!
	\brief		Type of a Sensor object. It is an opaque pointer to a private structure
*/
typedef struct SMT_SENSOR_PRIVATE * SMT_SENSOR;

struct SMT_CURSOR_PRIVATE;

/*!
	\brief		Type of a Cursor object. It is an opaque pointer to a private structure
*/
typedef struct SMT_CURSOR_PRIVATE * SMT_CURSOR;

/*!
	\brief		Type of Event. This will be passed to the user callback at each incoming event
*/
typedef enum
{
	SMT_CURSOR_UP = 1,		/*!< Contact released from touchscreen */
	SMT_CURSOR_DOWN,		/*!< Contact on touchscreen */
	SMT_CURSOR_MOVE,		/*!< Contact moved across touchscreen */
	
	SMT_CURSOR_CREATE,		/*!< New cursor created */
	SMT_CURSOR_DESTROY,		/*!< Cursor destroyed */
	
	SMT_SENSOR_CONNECT,		/*!< Connection made to a SMT sensor */
	SMT_SENSOR_DISCONNECT	/*!< Connection lost to a SMT sensor */ 
} SMT_EVENT;


/*!
	\brief		A data type that packs dimension infos for the sensor
*/
typedef struct
{
	int width;	/*!< Width value (unit depends on context) */
	int height;	/*!< Height value (unit depends on context) */
} SMT_DIMENSION;


/*!
	\brief		Prototype for the User Callback that will receive all SMK 15.4 events
	\param	msg		The event that occurred
	\param	sensor	The sensor object associated with the event
	\param	cursor	The cursor object associated with the event. NULL if event is SMT_EVENT::SMT_SENSOR_CONNECT or SMT_EVENT::SMT_SENSOR_DISCONNECT
*/
typedef void (*SMT_CALLBACK)(SMT_EVENT msg, SMT_SENSOR sensor, SMT_CURSOR cursor);

/*!
	\name	SMT Connection management
	@{
*/

/*!
	\brief		A function that enumerates the list of SMK sensors attached to the platform
	\param	smt_id	A pointer to an array of int. This will be filled on return with the IDs of existing sensors. If NULL, no array is filled, and the function returns the number of attached sensors.
	\param	size	The maximum size of the smt_ids array, in terms of how many int values it can contain. If size is not large enough to hold the sensor list, the function does not fill the array and returns the number of attached sensors.
	\return		Number of attached sensors
*/
EXPORT int SMT_Enum(int *smt_id, int size);

/*!
	\brief		A function that creates a connection to an SMK device attached to the platform, and returns a sensor object to be used in subsequent calls relating to this device.
	\param	id	The ID of the device to connect to. If 0, the first device found on the platform USB busses is chosen.
	\param	width	The width for the device touchscreen area. All cursors belonging to this sensor will have their X coordinate scaled between 0 and width-1. If 0, function returns 0.
	\param	height	The height for the device touchscreen area. All cursors belonging to this sensor will have their Y coordinate scaled between 0 and height-1. If 0, function returns 0.
	\param	callback	The user callback that will be called for all multitouch events related to this SMK device. This can be NULL.
	\param	userdata	Optional user data to be associated with the sensor. This will be stored in the sensor object and can be retrieved in subsequent calls.
	\return		A sensor object associated with the SMK device. NULL if the device wasn’t found, or if the connection could not be made, or the passed scaling factors were invalid.
*/
EXPORT SMT_SENSOR SMT_Open(int id, int width, int height, SMT_CALLBACK callback, void *userdata);

/*!
	\brief		A function that closes the connection to an SMK device.
	\param	sensor	The sensor object whose connection should be closed.
	
	 This call should be made to cleanly close the connection before the program exits, or to free the SMK device for a future connection.
*/
EXPORT void SMT_Close(SMT_SENSOR sensor);

/*!
	\brief		A function that fetches a frame of data from a sensor object.
	\param	sensor	The sensor object that needs updating.
	\return		1 if update request was successful. 0 if failed.

	This will update the internal list of cursors and their states. This will trigger a call to the user callback (if not NULL) for each event that was detected. This should be called in the application’s main loop.
	When SMT_Update(SMT_SENSOR sensor) returns 0, the user must call SMT_Close(SMT_SENSOR sensor) on the sensor object before attempting to recreate the connection.
*/
EXPORT int SMT_Update(SMT_SENSOR sensor);

/*!
	@}
	\name	SMT Sensor attributes
	@{
*/

/*!
	\brief		A function that returns the ID of a sensor object.
	\param	sensor	The sensor object.
	\return		The ID of the sensor object. 0 is sensor is invalid.
*/
EXPORT int SMT_GetSensorID(SMT_SENSOR sensor);

/*!
	\brief		A function that fetches the serial number of a sensor object.
	\param	sensor		The sensor object.
	\param	serial_nbr	An array that will store the serial number on return of the function.
	\return		1 if successful, 0 if failed.
*/
EXPORT int SMT_GetSensorSerialNbr(SMT_SENSOR sensor, unsigned char serial_nbr[8]);

/*!
	\brief		A function that fetches the dimensions of the virtual window associated with the sensor.
	\param	sensor	The sensor object.
	\param	dim		A structure that will hold the dimensions on return of the function.
	\return		1 if successful, 0 if failed.
*/
EXPORT int SMT_GetSensorWindowDimension(SMT_SENSOR sensor, SMT_DIMENSION *dim);

/*!
	\brief		A function that changes the dimensions of the virtual window associated with the sensor.
	\param	sensor		The sensor object.
	\param	width		The width value for the sensor.
	\param	height		The height value for the sensor.
	\return		1 if successful, 0 if failed.
	
	This will effectively change the coordinates of all existing cursors to the new range. No SMT_EVENT::SMT_CURSOR_MOVE event are triggered by this coordinate remapping.
*/
EXPORT int SMT_SetSensorWindowDimension(SMT_SENSOR sensor, int width, int height);

/*!
	\brief		A function that fetches the dimensions of the SMK device in millimeters. The result is stored in an SMT_DIMENSION structure..
	\param	sensor	The sensor object.
	\param	dim		A structure that will hold the dimensions on return of the function.
	\return		1 if successful, 0 if failed.
*/
EXPORT int SMT_GetSensorMatrixDimension(SMT_SENSOR sensor, SMT_DIMENSION *dim);

/*!
	\brief		A function that fetches the resolution of the SMK device in cells. The result is stored in an SMT_DIMENSION structure..
	\param	sensor	The sensor object.
	\param	dim		A structure that will hold the resolution on return of the function.
	\return		1 if successful, 0 if failed.
*/
EXPORT int SMT_GetSensorMatrixResolution(SMT_SENSOR sensor, SMT_DIMENSION *dim);

/*!
	\brief		A function that fetches the user callback associated with the sensor object.
	\param	sensor	The sensor object.
	\return		The callback associated with the sensor. NULL if no callback exists for this sensor, or the sensor object is invalid. 
*/
EXPORT SMT_CALLBACK SMT_GetSensorCallback(SMT_SENSOR sensor);

/*!
	\brief		A function that sets the user callback for a sensor object.
	\param	sensor	The sensor object.
	\param	callback	The user callback to associate with the sensor.
	\return		1 if successful, 0 if failed.
	
	The callback will be called for each event related to this sensor. If callback is NULL, the application will not be notified of any events. 	
*/
EXPORT int SMT_SetSensorCallback(SMT_SENSOR sensor, SMT_CALLBACK callback);

/*!
	\brief		A function that returns the user data associated with a sensor.
	\param	sensor	The sensor object.
	\return		The user data associated with the sensor. NULL if sensor object is invalid.
*/
EXPORT void * SMT_GetSensorUserData(SMT_SENSOR sensor);

/*!
	\brief		A function that sets the user data associated with a sensor. 
	\param	sensor	The sensor object.
	\param	userdata	The user data to associate with the sensor.
	\return		1 if successful, 0 if sensor object is invalid. 
*/
EXPORT int SMT_SetSensorUserData(SMT_SENSOR sensor, void *userdata);

/*!
	@}
	\name	SMT Cursors management
	@{
*/

/*!
	\brief		A function that returns the first of all cursors associated with the sensor.
	\param	sensor	The sensor object.
	\return		The first existing cursor object associated with the sensor. NULL if no cursors exist. 
	
	Cursors are stored in a linked list when created, and removed from the list upon destruction. 	
*/
EXPORT SMT_CURSOR SMT_GetSensorCursors(SMT_SENSOR sensor);

/*!
	\brief		A function that returns the next cursor in the cursors list.
	\param	cursor	The cursor object.
	\return		The next cursor object in the list. NULL if cursor is last in the list.	
*/
EXPORT SMT_CURSOR SMT_GetCursorNext(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the previous cursor in the cursors list.
	\param	cursor	The cursor object.
	\return		The previous cursor object in the list. NULL if cursor is first in the list.	
*/
EXPORT SMT_CURSOR SMT_GetCursorPrev(SMT_CURSOR cursor);

/*!
	\brief		A function that returns an integer ID that uniquely identifies a cursor.
	\param	cursor	The cursor object.
	\return		The ID for the cursor. 0 if cursor object is invalid.	
*/
EXPORT int SMT_GetCursorID(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the sensor object a cursor belongs to.
	\param	cursor	The cursor object.
	\return		The sensor the cursor belongs to. NULL if cursor object is invalid.	
*/
EXPORT SMT_SENSOR SMT_GetCursorSensor(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the X coordinate of a cursor.
	\param	cursor	The cursor object.
	\return		The X coordinate of the cursor. This is scaled by the virtual window associated with the parent sensor. X is between 0 and width-1 where width is the width of the virtual window. 0 is returned if cursor object is invalid.	
*/
EXPORT int SMT_GetCursorX(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the Y coordinate of a cursor.
	\param	cursor	The cursor object.
	\return		The Y coordinate of the cursor. This is scaled by the virtual window associated with the parent sensor. Y is between 0 and height-1 where width is the width of the virtual window. 0 is returned if cursor object is invalid.	
*/
EXPORT int SMT_GetCursorY(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the current state of a cursor.
	\param	cursor	The cursor object.
	\return		The current state of the cursor. Possible return values are SMT_EVENT::SMT_CURSOR_UP, SMT_EVENT::SMT_CURSOR_DOWN, or 0 if cursor object is invalid.
*/
EXPORT SMT_EVENT SMT_GetCursorState(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the number of clicks that occured for a cursor since its creation or its last SMT_EVENT::SMT_CURSOR_MOVE event.
	\param	cursor	The cursor object.
	\return		The number of clicks that occurred for the cursor.
	
	The click number attribute allows differentiation between a click and a multiclick, which both trigger SMT_EVENT::SMT_CURSOR_DOWN events.
*/
EXPORT int SMT_GetCursorClickNbr(SMT_CURSOR cursor);

/*!
	\brief		A function that returns the user data associated with a cursor.
	\param	cursor	The cursor object.
	\return		The user data associated with the cursor. NULL if no user data was associated with the cursor, or if cursor object is invalid.	
*/
EXPORT void * SMT_GetCursorUserData(SMT_CURSOR cursor);

/*!
	\brief		A function that sets the user data associated with a cursor.
	\param	cursor	The cursor object.
	\param	userdata	The user data to be associated with the cursor.
	\return		1 if successful, 0 if cursor object is invalid.
*/
EXPORT int SMT_SetCursorUserData(SMT_CURSOR cursor, void *userdata);

/*
	@}
*/

#ifdef __cplusplus
}
#endif
