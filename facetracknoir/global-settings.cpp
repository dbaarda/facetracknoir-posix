#include "global-settings.h"

SelectedLibraries* Libraries = NULL;

SelectedLibraries::~SelectedLibraries()
{
    if (pTracker && pTracker->NeedsTimeToFinish()) {
        pTracker->mutex.lock();
        pTracker->should_quit = true;
        pTracker->alert_finished.wait(&pTracker->mutex);
        pTracker->mutex.unlock();
    }
    if (pSecondTracker && pSecondTracker->NeedsTimeToFinish()) {
        pSecondTracker->mutex.lock();
        pSecondTracker->should_quit = true;
        pSecondTracker->alert_finished.wait(&pSecondTracker->mutex);
        pSecondTracker->mutex.unlock();
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

    //
    // Check if the Protocol-server files were installed OK.
    // Some servers also create a memory-mapping, for Inter Process Communication.
    // The handle of the MainWindow is sent to 'The Game', so it can send a message back.
    //
    // Sorry! -sh
#if 0
    if (pProtocol) {
        bool DLL_Ok = pProtocol->checkServerInstallationOK();
        if (!DLL_Ok) {
            QMessageBox::information(mainApp, "FaceTrackNoIR error", "Protocol is not (correctly) installed!");
        }
    }
#endif
}

DynamicLibrary::DynamicLibrary(QString filename)
{
    this->filename = filename;
    handle = new QLibrary(filename);
    Dialog = (SETTINGS_FUNCTION) handle->resolve("GetDialog" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
    Constructor = (NULLARY_DYNAMIC_FUNCTION) handle->resolve("GetConstructor" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
    Metadata = (METADATA_FUNCTION) handle->resolve("GetMetadata" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
}

DynamicLibrary::~DynamicLibrary()
{
    handle->unload();
}
