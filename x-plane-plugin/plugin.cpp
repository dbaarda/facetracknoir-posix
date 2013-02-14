#include <stdlib.h>
#include <string.h>
#include "compat/compat.h"
#include "compat/compat.cpp"
#include "ftnoir_protocol_wine/fttypes.h"

#include <XPLMPlugin.h>
#include <XPLMCamera.h>

// using Wine name to ease things
#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

static PortableLockedShm* lck_posix = NULL;
static WineSHM* shm_posix = NULL;
static bool start = false;

static int camera_callback(XPLMCameraPosition_t* outCameraPosition, int inIsLosingControl, void* inRefCon) {
	if (lck_posix->mem != (void*) -1 && lck_posix->mem != NULL && outCameraPosition && !inIsLosingControl) {
		lck_posix->lock();
		outCameraPosition->heading = shm_posix->rx;
		outCameraPosition->pitch   = shm_posix->ry;
		outCameraPosition->roll    = shm_posix->rz;
		outCameraPosition->x       = shm_posix->tx;
		outCameraPosition->y       = shm_posix->ty;
		outCameraPosition->zoom    = shm_posix->tz;
		outCameraPosition->z       = 0;
		lck_posix->unlock();
		return 1;
	}
	return 0;
}

PLUGIN_API int XPluginStart ( char * outName, char * outSignature, char * outDescription ) {
	lck_posix = new PortableLockedShm(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(PortableLockedShm));
        if(lck_posix->mem == (void*)-1)
                return 0;
	if (lck_posix->mem == NULL)
		return 0;
	shm_posix = (WineSHM*) lck_posix->mem;
	return 1;
}

PLUGIN_API void XPluginStop ( void ) {
	if (lck_posix)
		delete lck_posix;
	lck_posix = NULL;
	shm_posix = NULL;
}

PLUGIN_API void XPluginEnable ( void ) {
	if (!start) {
		start = true;
		XPLMControlCamera(xplm_ControlCameraForever, camera_callback, NULL);
	}
}

PLUGIN_API void XPluginDisable ( void ) {
	if (start) {
		start = false;
		XPLMDontControlCamera();
	}
}

PLUGIN_API void XPluginReceiveMessage ( XPLMPluginID inFrom, long inMessage, void * inParam ) {
}
