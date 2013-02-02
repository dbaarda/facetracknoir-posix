#include "global-settings.h"

#if !(defined(__WIN32) || defined(_WIN32))
#   include <dlfcn.h>
#endif

SelectedLibraries* Libraries = NULL;

SelectedLibraries::~SelectedLibraries()
{
    if (pTracker) {
        pTracker->WaitForExit();
    }
    if (pSecondTracker) {
        pSecondTracker->WaitForExit();
    }

    if (pTracker) {
        delete pTracker;
        pTracker = NULL;
    }

    if (pSecondTracker) {
        delete pSecondTracker;
        pSecondTracker = NULL;
    }

    if (pFilter)
        delete pFilter;

    if (pProtocol)
        delete pProtocol;
}

SelectedLibraries::SelectedLibraries(IDynamicLibraryProvider* mainApp) :
    pTracker(NULL), pSecondTracker(NULL), pFilter(NULL), pProtocol(NULL)
{
    correct = false;
    if (!mainApp)
        return;
    NULLARY_DYNAMIC_FUNCTION ptr;
    DynamicLibrary* lib;

    lib = mainApp->current_tracker1();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pTracker = (ITracker*) ptr();
    }

    lib = mainApp->current_tracker2();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pSecondTracker = (ITracker*) ptr();
    }

    lib = mainApp->current_protocol();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pProtocol = (IProtocol*) ptr();
    }

    lib = mainApp->current_filter();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pFilter = (IFilter*) ptr();
    }

    // Check if the Protocol-server files were installed OK.
    // Some servers also create a memory-mapping, for Inter Process Communication.
    // The handle of the MainWindow is sent to 'The Game', so it can send a message back.

    if (pProtocol)
        if(!pProtocol->checkServerInstallationOK())
            return;

    // retrieve pointers to the User Interface and the main Application
    if (pTracker) {
        pTracker->StartTracker( mainApp->get_video_widget() );
    }
    if (pSecondTracker) {
        pSecondTracker->StartTracker( mainApp->get_video_widget() );
    }

    if (pFilter)
        pFilter->Initialize();

    if (pProtocol)
        pProtocol->Initialize();

    correct = true;
}

DynamicLibrary::DynamicLibrary(const char* filename)
{
    this->filename = filename;
#if defined(__WIN32) || defined(_WIN32)
    handle = new QLibrary(filename);
    Dialog = (SETTINGS_FUNCTION) handle->resolve("GetDialog" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
    Constructor = (NULLARY_DYNAMIC_FUNCTION) handle->resolve("GetConstructor" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
    Metadata = (METADATA_FUNCTION) handle->resolve("GetMetadata" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
#else
    handle = dlopen(filename, RTLD_LAZY |
#   ifdef __linux
                    RTLD_DEEPBIND
#   else
                    0
#   endif
                    );
    if (handle)
    {
        Dialog = (SETTINGS_FUNCTION) dlsym(handle, "GetDialog");
        Constructor = (NULLARY_DYNAMIC_FUNCTION) dlsym(handle, "GetConstructor");
        Metadata = (METADATA_FUNCTION) dlsym(handle, "GetMetadata");
    }
#endif
}

DynamicLibrary::~DynamicLibrary()
{
#if defined(__WIN32) || defined(_WIN32)
    handle->unload();
#else
    if (handle)
        (void) dlclose(handle);
#endif
}
