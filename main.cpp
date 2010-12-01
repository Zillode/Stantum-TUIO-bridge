//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib, "winmm.lib")
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>


#define RES_X 800
#define RES_Y 480
#define OSC_HOST_LITTLE_ENDIAN 1
#define MAX_FINGERS 50

#ifdef WIN32
#include "libsmt-windows/SMT.h"
#include <conio.h>
#include <winsock2.h>
#include <cstdio>
#include <string.h>
#include <windows.h>
#include <process.h>
#endif

#ifndef WIN32
#include "libsmt-linux/SMT.h"
#include <pthread.h>
#endif

#include "TuioServer.h"

const char host[] = "localhost";
const int port =  3333;

TuioServer *tuio_server;
static int run = 1;
static int fseq = 0;
static int res_width = 0;
static int res_height = 0;
static std::vector<int> v(0);
static float old_x[MAX_FINGERS];
static float old_y[MAX_FINGERS];
static double old_time[MAX_FINGERS];
static float old_speed[MAX_FINGERS];


#ifdef WIN32
HANDLE hMutex;
#else
pthread_t threadAlive;
pthread_mutex_t mutex_tuioserver = PTHREAD_MUTEX_INITIALIZER;
#endif

void addAlives();
float distance(float dX0, float dY0, float dX1, float dY1);


// myCallback will receive all events from the sensor
void myCallback(SMT_EVENT message, SMT_SENSOR sensor, SMT_CURSOR cursor) {
	int c = (int)SMT_GetCursorID(cursor) % MAX_FINGERS;
	float x = 0;
	float y = 0;
	float dx = 0;
	float dy = 0;
	float dt = 0;
	float xspeed = 0;
	float yspeed = 0;
	float m = 0;
	float speed = 0;
	float dist = 0;
	struct timeval curr_time;
	gettimeofday(&curr_time, NULL);
	double seconds = curr_time.tv_sec + ((double)curr_time.tv_usec / 1000000);
#ifdef WIN32
	WaitForSingleObject( hMutex, INFINITE );
#else
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
			old_x[c] = x;
			old_y[c] = y;
			old_time[c] = seconds;
			old_speed[c] = 0;
			v.push_back(c);
			tuio_server->addCurSeq(fseq);
			addAlives();
			tuio_server->addCurSet(c, x, y, 0, 0, 0);
			tuio_server->sendCurMessages();
			break;
		case SMT_CURSOR_MOVE:
			dt = seconds - old_time[c];
			x = (float)SMT_GetCursorX(cursor) / res_width;
			y = (float)SMT_GetCursorY(cursor) / res_height;
			dx = x - old_x[c];
			dy = y - old_y[c];
			dist = sqrt(dx*dx+dy*dy);
			speed = dist/dt;
			xspeed = dx/dt;
			yspeed = dy/dt;
			m = (speed-old_speed[c])/dt;
			//dx = (x - old_x[c]) / dt;
			//dy = (y - old_y[c]) / dt;
			//speed = distance(old_x[c], old_y[c], x, y) / dt;
			//m = (speed - old_speed[c]) / dt;

			//printf("Secs: %f %f %f\n", seconds, old_time[c], dt);
			old_x[c] = x;
			old_y[c] = y;
			old_time[c] = seconds;
			old_speed[c] = speed;
			printf("C: %d %f\n", c, old_time[c]);
			//printf("Secs: %f %f %f", seconds, old_time[c], dt);

			printf("--- %f %f %f %f %f\n", x, y, xspeed, yspeed, m);
			tuio_server->addCurSeq(fseq);
			addAlives();
			tuio_server->addCurSet(c, x, y, xspeed, yspeed, m);
			tuio_server->sendCurMessages();
			break;
		case SMT_CURSOR_UP:
			printf("UP\n");
			for (unsigned int i=0; i<v.size(); i++) {
				if (v[i] == c) {
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
#ifdef WIN32
	ReleaseMutex(hMutex);
#else
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

float distance(float dX0, float dY0, float dX1, float dY1) {
	return sqrt((dX1 - dX0)*(dX1 - dX0) +
			(dY1 - dY0)*(dY1 - dY0));
}

#ifdef WIN32
void functionThreadAlive(void *p) {
	while (true) {
		WaitForSingleObject( hMutex, INFINITE );
#else
		void *functionThreadAlive(void *p) {
			while (true) {
				pthread_mutex_lock(&mutex_tuioserver);
#endif
				++fseq;
				tuio_server->addCurSeq(fseq);
				addAlives();
				tuio_server->sendCurMessages();
#ifdef WIN32
				ReleaseMutex( hMutex );
				Sleep(1000);
#else
				pthread_mutex_unlock(&mutex_tuioserver);
				sleep(1);
#endif
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
#ifdef WIN32
			SMT_SENSOR s = SMT_Open(0, 0, RES_X, RES_Y, myCallback, 0);
#else
			SMT_SENSOR s = SMT_Open(0, RES_X, RES_Y, myCallback, 0);
#endif
			if (!s) {
				// connection will fail if device is already connected to
				printf("failed opening connection to SMK\n");
				return 0;
			}
			tuio_server = new TuioServer(host, port);
#ifdef WIN32
			hMutex = CreateMutex( NULL, FALSE, NULL );
			_beginthread( functionThreadAlive, 0, NULL );
#else
			pthread_create( &threadAlive, NULL, &functionThreadAlive, NULL);
#endif
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
