#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <limits.h>
#include <unistd.h>

#include <XPLMPlugin.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMCamera.h>
#include <XPLMProcessing.h>

// using Wine name to ease things
#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

typedef struct PortableLockedShm {
    void* mem;
    int fd, size;
} PortableLockedShm;

typedef struct WineSHM {
    float rx, ry, rz, tx, ty, tz;
    char stop;
} WineSHM;

static PortableLockedShm* lck_posix = NULL;
static WineSHM* shm_posix = NULL;
static void *view_x, *view_y, *view_z, *view_heading, *view_pitch;
static float offset_x, offset_y, offset_z;

PortableLockedShm* PortableLockedShm_init(const char *shmName, const char *mutexName, int mapSize)
{
    PortableLockedShm* self = malloc(sizeof(PortableLockedShm));
    char shm_filename[NAME_MAX];
    shm_filename[0] = '/';
    strncpy(shm_filename+1, shmName, NAME_MAX-2);
    shm_filename[NAME_MAX-1] = '\0';
    sprintf(shm_filename + strlen(shm_filename), "%ld\n", (long) getuid());
    //(void) shm_unlink(shm_filename);
    
    self->fd = shm_open(shm_filename, O_RDWR | O_CREAT, 0600);
    if (ftruncate(self->fd, mapSize) == 0)
        self->mem = mmap(NULL, mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, self->fd, (off_t)0);
    else
        self->mem = (void*) -1;
    return self;
}

void PortableLockedShm_free(PortableLockedShm* self)
{
    //(void) shm_unlink(shm_filename);
    (void) munmap(self->mem, self->size);
    (void) close(self->fd);
    free(self);
}

void PortableLockedShm_lock(PortableLockedShm* self)
{
    flock(self->fd, LOCK_SH);
}

void PortableLockedShm_unlock(PortableLockedShm* self)
{
    flock(self->fd, LOCK_UN);
}

static void reinit_offset() {
    offset_x = XPLMGetDataf(view_x);
    offset_y = XPLMGetDataf(view_y);
    offset_z = XPLMGetDataf(view_z);
}

int write_head_position(
    XPLMDrawingPhase     inPhase,    
    int                  inIsBefore,    
    void *               inRefcon)
{
    if (lck_posix != NULL && shm_posix != NULL) {
        PortableLockedShm_lock(lck_posix);
        XPLMSetDataf(view_x, shm_posix->tx * 1e-2 + offset_x);
        XPLMSetDataf(view_y, shm_posix->ty * 1e-2 + offset_y);
        XPLMSetDataf(view_z, shm_posix->tz * 1e-2 + offset_z);
        XPLMSetDataf(view_heading, shm_posix->rx * 57.295781);
        XPLMSetDataf(view_pitch, shm_posix->ry * 57.295781);
        PortableLockedShm_unlock(lck_posix);
    }
    return 1;
}

PLUGIN_API int XPluginStart ( char * outName, char * outSignature, char * outDescription ) {
    view_x = XPLMFindDataRef("sim/aircraft/view/acf_peX");
    view_y = XPLMFindDataRef("sim/aircraft/view/acf_peY");
    view_z = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
    view_heading = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    view_pitch = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    if (view_x && view_y && view_z && view_heading && view_pitch) {
        lck_posix = PortableLockedShm_init(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
        if (lck_posix->mem == (void*)-1) {
            fprintf(stderr, "FTNOIR failed to init SHM #1!\n");
            return 0;
        }
        if (lck_posix->mem == NULL) {
            fprintf(stderr, "FTNOIR failed to init SHM #2!\n");
            return 0;
        }
        shm_posix = (WineSHM*) lck_posix->mem;
        memset(shm_posix, 0, sizeof(WineSHM));
        strcpy(outName, "FaceTrackNoIR");
        strcpy(outSignature, "FaceTrackNoIR - FreeTrack lives!");
        strcpy(outDescription, "Face tracking view control");
        fprintf(stderr, "FTNOIR init complete\n");
        return 1;
    }
    return 0;
}

#if 0
static int camera_callback(XPLMCameraPosition_t* outCameraPosition, int inIsLosingControl, void* inRefCon) {
    if (!inIsLosingControl && XPLMGetCycleNumber() > 0) {
        //XPLMReadCameraPosition(outCameraPosition);
        PortableLockedShm_lock(lck_posix);
        outCameraPosition->heading = shm_posix->rx * 57.295781;
        outCameraPosition->pitch = shm_posix->ry * 57.295781;
        outCameraPosition->roll = shm_posix->rz * 57.295781;
        outCameraPosition->x = XPLMGetDataf(view_x);
        outCameraPosition->y = XPLMGetDataf(view_y);
        outCameraPosition->z = XPLMGetDataf(view_z);
        PortableLockedShm_unlock(lck_posix);
        return 1;
    }
    return 0;
}
static float flight_loop (
    float                inElapsedSinceLastCall,    
    float                inElapsedTimeSinceLastFlightLoop,    
    int                  inCounter,    
    void *               inRefcon)
{
    XPLMControlCamera(xplm_ControlCameraForever, camera_callback, NULL);
    // don't want it called anymore
    return 0;
}
#endif

PLUGIN_API void XPluginStop ( void ) {
#if 0
    // crashes due to race
    if (lck_posix)
        PortableLockedShm_free(lck_posix);
    lck_posix = NULL;
    shm_posix = NULL;
#endif
}

PLUGIN_API void XPluginEnable ( void ) {
    reinit_offset();
    XPLMRegisterDrawCallback(write_head_position, xplm_Phase_LastScene, 1, NULL);
#if 0
    XPLMRegisterFlightLoopCallback(flight_loop, -1, NULL);
#endif
}

PLUGIN_API void XPluginDisable ( void ) {
    XPLMUnregisterDrawCallback(write_head_position, xplm_Phase_LastScene, 1, NULL);
    XPLMSetDataf(view_x, offset_x);
    XPLMSetDataf(view_y, offset_y);
    XPLMSetDataf(view_z, offset_z);
#if 0
    XPLMUnregisterFlightLoopCallback(flight_loop, NULL);
    if (XPLMIsCameraBeingControlled(NULL))
        XPLMDontControlCamera();
#endif
}

PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID    inFromWho,
    int             inMessage,
    void *          inParam)
{
    if (inMessage == XPLM_MSG_AIRPORT_LOADED)
        reinit_offset();
}
