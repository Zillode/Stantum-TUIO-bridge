#include <stdio.h>
#include <signal.h>
#ifdef WIN32
#include <dos.h>
#include <conio.h>
// Include SMT header for windows here
#endif
#include "TuioServer.h"
#include <iostream>
#include <vector>
#ifndef WIN32
#include "libsmt-linux/SMT.h"
#include <pthread.h>
#endif

const char host[] = "localhost";
const int port =  3333;

TuioServer *tuio_server;
static int run = 1;
static int classID = 2;
static int fseq = 0;
static int res_width = 0;
static int res_height = 0;
static std::vector<int> v(0);
#ifndef WIN32
pthread_t threadAlive;
pthread_mutex_t mutex_tuioserver = PTHREAD_MUTEX_INITIALIZER;
#endif

void addAlives();

// myCallback will receive all events from the sensor
void myCallback(SMT_EVENT message, SMT_SENSOR sensor, SMT_CURSOR cursor) {
	float x = 0;
	float y = 0;
#ifndef WIN32
	pthread_mutex_lock(&mutex_tuioserver);
#endif
	++fseq;
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
		x = (float)SMT_GetCursorX(cursor) / res_width;
		y = (float)SMT_GetCursorY(cursor) / res_height;
		v.push_back(SMT_GetCursorID(cursor));
		tuio_server->addCurSeq(fseq);
		addAlives();
		tuio_server->addCurSet(SMT_GetCursorID(cursor), x, y, 0, 0, 0);
		tuio_server->sendCurMessages();
		break;
	case SMT_CURSOR_MOVE:
		x = (float)SMT_GetCursorX(cursor) / res_width;
		y = (float)SMT_GetCursorY(cursor) / res_height;
		tuio_server->addCurSeq(fseq);
		addAlives();
		tuio_server->addCurSet(SMT_GetCursorID(cursor), x, y, 0, 0, 0);
		tuio_server->sendCurMessages();
		break;
	case SMT_CURSOR_UP:
		for (int i=0; i<v.size(); i++) {
			if (v[i] == SMT_GetCursorID(cursor)) {
				v.erase(v.begin()+i);
				break;
			}
		}
		tuio_server->addCurSeq(fseq);
		addAlives();
		tuio_server->sendCurMessages();
		break;
	case SMT_CURSOR_DESTROY:
		break;
	case SMT_SENSOR_DISCONNECT:
		break;
	default:
		break;
	}
#ifndef WIN32
	pthread_mutex_unlock(&mutex_tuioserver);
#endif
}

void addAlives() {
	int aliveSize = v.size();
	int *aliveList = new int[aliveSize];
	for (int i=0; i<aliveSize; i++) {
		aliveList[i] = v[i];
	}
	tuio_server->addCurAlive(aliveList,aliveSize);
	delete [] aliveList;
}

#ifndef WIN32
void *functionThreadAlive(void *) {
	while (true) {
	pthread_mutex_lock(&mutex_tuioserver);
	++fseq;
	tuio_server->addCurSeq(fseq);
	addAlives();
	tuio_server->sendCurMessages();
	pthread_mutex_unlock(&mutex_tuioserver);
	sleep(1);
	}
}
#endif

void sigintHandler(int i) {
	run = 0;
}
int main(int argc, char *argv[]) {
	// set up signal handler to exit cleanly
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);
#ifdef WIN32
	signal(SIGBREAK, sigintHandler);
#endif
	signal(SIGABRT, sigintHandler);
	printf("opening connection to SMK...\n");
	// we call SMT_Open with a NULL ID to connect to the first SMK device detected
	// myCallback will be called for each event that occurs
	SMT_SENSOR s = SMT_Open(0, 800, 480, myCallback, 0);
	if (!s) {
		// connection will fail if device is already connected to
		printf("failed opening connection to SMK\n");
		return 0;
	}
	tuio_server = new TuioServer(host, port);
#ifndef WIN32
	pthread_create( &threadAlive, NULL, &functionThreadAlive, NULL);
#endif
	printf("Press CTRL+C to Quit.\n");
	while (run) {
		// separate frames with newlines
		// call SMT_Update to receive new data from the SMK
		// SMT_Update will return 0 when connection is lost
		if (!SMT_Update(s)) run = 0;
#ifdef WIN32
		if (kbhit() && getch()==0x1B) {
			run = 0;
		}
#endif
	}
	printf("closing connection to SMK...\n");
	// close connection to SMK before exit
	SMT_Close(s);
	return 0;
}
