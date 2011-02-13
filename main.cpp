#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#ifdef WIN32
#include <SMT.h>
#include <conio.h>
#include <ctype.h>
#endif
#ifndef WIN32
#include "libsmt-linux/SMT.h"
#endif

#define DEBUG(...) do { if (VERBOSE) printf(__VA_ARGS__); } while(0)

#define VERBOSE 0
#define RES_X 800
#define RES_Y 480
#define OSC_HOST_LITTLE_ENDIAN 1
#define MAX_FINGERS 50

#include "TUIO/Tuiocursor.h"
#include "TUIO/TuioServer.h"

const char host[] = "localhost";
const int port =  3333;

using namespace TUIO;

static TuioServer *tuio_server = NULL;
static TuioCursor *tuio_mapping[MAX_FINGERS];
static int res_width = 0;
static int res_height = 0;
static int run = 1;

// myCallback will receive all events from the sensor
void myCallback(SMT_EVENT message, SMT_SENSOR sensor, SMT_CURSOR cursor) {
	int c = (int)SMT_GetCursorID(cursor) % MAX_FINGERS;
	TuioCursor *match = NULL;
	float x = 0;
	float y = 0;
	TuioTime currentTime = TuioTime::getSessionTime();
	SMT_DIMENSION window_dim, real_dim;
	// analyze message and print relevant information
	switch (message) {
		case SMT_SENSOR_CONNECT:
			SMT_GetSensorWindowDimension(sensor, &window_dim);
			SMT_GetSensorMatrixDimension(sensor, &real_dim);
			printf("Connected to sensor %d. Window : %dpx x %dpx. Physical : %dmm x %dmm\n", SMT_GetSensorID(sensor), window_dim.width, window_dim.height, real_dim.width, real_dim.height);
			res_width = window_dim.width;
			res_height = window_dim.height;
			break;
		case SMT_CURSOR_CREATE:
			break;
		case SMT_CURSOR_DOWN:
			DEBUG("DOWN\n");
			x = (float)SMT_GetCursorX(cursor) / res_width;
			y = (float)SMT_GetCursorY(cursor) / res_height;
			tuio_server->initFrame(currentTime);
			match = tuio_mapping[c];
			if (match!=NULL) {
				tuio_server->removeTuioCursor(match);
				tuio_mapping[c] = NULL;
			}
			tuio_mapping[c] = tuio_server->addTuioCursor(x,y);
			tuio_server->commitFrame();
			break;
		case SMT_CURSOR_MOVE:
			DEBUG("MOVE\n");
			x = (float)SMT_GetCursorX(cursor) / res_width;
			y = (float)SMT_GetCursorY(cursor) / res_height;
			tuio_server->initFrame(currentTime);
			match = tuio_mapping[c];
			if (match==NULL)
				tuio_mapping[c] = tuio_server->addTuioCursor(x,y);
			else
				tuio_server->updateTuioCursor(match,x,y);
			tuio_server->commitFrame();
			break;
		case SMT_CURSOR_UP:
			DEBUG("UP\n");
			tuio_server->initFrame(currentTime);
			match = tuio_mapping[c];
			if (match!=NULL) {
				tuio_server->removeTuioCursor(match);
				tuio_mapping[c] = NULL;
			}
			tuio_server->commitFrame();
			break;
		case SMT_CURSOR_DESTROY:
			break;
		case SMT_SENSOR_DISCONNECT:
			break;
		default:
			break;
	}
}

void sigintHandler(int i) {
	run = 0;
}
int main(int argc, char *argv[]) {
	// set up signal handler to exit cleanly
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);
	signal(SIGABRT, sigintHandler);
	printf("opening connection to SMK...\n");
	// we call SMT_Open with a NULL ID to connect to the first SMK device detected
	// myCallback will be called for each event that occurs
//#ifdef OTHER_STANTUM_V0.5
//	SMT_SENSOR s = SMT_Open(0, 0, RES_X, RES_Y, myCallback, 0);
//#else
	SMT_SENSOR s = SMT_Open(0, RES_X, RES_Y, myCallback, 0);
//#endif
	if (!s) {
		// connection will fail if device is already connected to
		printf("failed opening connection to SMK\n");
		return 0;
	}
	tuio_server = new TuioServer(host, port);
	printf("Press CTRL+C to Quit.\n");
	while (run) {
		// separate frames with newlines
		// call SMT_Update to receive new data from the SMK
		// SMT_Update will return 0 when connection is lost
		if (!SMT_Update(s)) run = 0;
#ifdef WIN32
		if (_kbhit() && _getch()==0x1B) {
			run = 0;
		}
#endif
	}
	printf("closing connection to SMK...\n");
	// close connection to SMK before exit
	SMT_Close(s);
	return 0;
}
