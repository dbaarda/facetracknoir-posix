/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2011	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*********************************************************************************/
/*
	Modifications (last one on top):
		20130101 - WVR: Added "None" to filter-listbox to remove "use advanced filtering".
		20121209 - WVR: Pre-v170 DLLs will not be added to the Listbox. Initial selection was changed (made case-insensitive).
		20121014 - WVR: Added second Tracker Source for Arduino solution. The two will be mutually exclusive.
		20120929 - WVR: Disable button Filter-settings when StartTracker.
		20120918 - WVR: When AutoStart is TRUE, the program is not directly minimized any more.
						This now depends on the AutoMinimize time. Fixed the 'not showing' of the MIB.
						Also disable combo and buttons after 'Start'.
		20120917 - WVR: Added Mouse-buttons to ShortKeys.
		20120717 - WVR: FunctionConfig is now used for the Curves, instead of BezierConfig.
        20120427 - WVR: The Protocol-code was already in separate DLLs, but the ListBox was still filled 'statically'. Now, a Dir() of the
						EXE-folder is done, to locate Protocol-DLLs. The Icons were also moved to the DLLs
		20120317 - WVR: The Filter and Tracker-code was moved to separate DLLs. The calling-method
						was changed accordingly. The save() and LoadSettings() functions were adapted.
						The face-tracker member-functions NotifyZeroed and refreshVideo were added, as 
						requested by Stanislaw.
		20110813 - WVR: Changed the presentation of the raw inputs: now a decimal digit will even show when '0'.
		20110404 - WVR: Migrated the FlightGear protocol to a separate DLL. The rest must follow...
		20110401 - WVR: The about-dialog was shown 'misplaced'. It was corrected.
		20110328 - WVR: Added the display for output-pose.
		20110207 - WVR: RadioButtons for 'Stop engine' added. It is now possible to choose Stop or Keep tracking.
		20110109 - WVR: Added minimizeTaskBar option added. It is now possible to choose minimized or tray.
*/
#include "facetracknoir.h"
#include "tracker.h"
#include <ftnoir_tracker_ht/ht-api.h>
#include <QDebug>

#if defined(__WIN32) || defined(_WIN32)
#   include <windows.h>
#endif

#if defined(__APPLE__)
#   define SONAME "dylib"
#elif defined(_WIN32) || defined(__WIN32)
#   define SONAME "dll"
#else
#   define SONAME "so"
#endif

#include <iostream>

#if defined(__WIN32) || defined(_WIN32)
#define DIRECTINPUT_VERSION 0x0800
#include <dshow.h>
#include <dinput.h>

KeybindingWorkerDummy::~KeybindingWorkerDummy() {
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorkerDummy::KeybindingWorkerDummy(FaceTrackNoIR& w, Key keyCenter, Key keyInhibit, Key keyStartStop, Key keyZero)
: kCenter(keyCenter), kInhibit(keyInhibit), kStartStop(keyStartStop), kZero(keyZero), window(w), should_quit(true), din(0), dinkeyboard(0)
{
    if (DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&din, NULL) != DI_OK) {
        qDebug() << "setup DirectInput8 Creation failed!" << GetLastError();
        return;
    }
    if (din->CreateDevice(GUID_SysKeyboard, &dinkeyboard, NULL) != DI_OK) {
        din->Release();
        din = 0;
        qDebug() << "setup CreateDevice function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
        qDebug() << "setup SetDataFormat function failed!" << GetLastError();
        dinkeyboard->Release();
        dinkeyboard = 0;
        din->Release();
        din = 0;
    }
    
    if (dinkeyboard->SetCooperativeLevel(window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup SetCooperativeLevel function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->Acquire() != DI_OK)
    {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup dinkeyboard Acquire failed!" << GetLastError();
        return;
    }
    qDebug() << "keycodes bound:" << kCenter.keycode << kInhibit.keycode << kStartStop.keycode << kZero.keycode;
    should_quit = false;
}

#define PROCESS_KEY(k, s) \
    if (isKeyPressed(&k, keystate) && (!k.ever_pressed ? (k.timer.start(), k.ever_pressed = true) : k.timer.restart() > 100)) \
        window.s();

static bool isKeyPressed( const Key *key, const BYTE *keystate ) {
    bool shift;
    bool ctrl;
    bool alt;

    if (keystate[key->keycode] & 0x80) {
        shift = ( (keystate[DIK_LSHIFT] & 0x80) || (keystate[DIK_RSHIFT] & 0x80) );
        ctrl  = ( (keystate[DIK_LCONTROL] & 0x80) || (keystate[DIK_RCONTROL] & 0x80) );
        alt   = ( (keystate[DIK_LALT] & 0x80) || (keystate[DIK_RALT] & 0x80) );

        //
        // If one of the modifiers is needed and not pressed, return false.
        //
        if (key->shift && !shift) return false;
        if (key->ctrl && !ctrl) return false;
        if (key->alt && !alt) return false;

        //
        // All is well!
        //
        return true;
    }
    return false;
}

void KeybindingWorkerDummy::run() {
    BYTE keystate[256];
    while (!should_quit)
    {
        if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
            qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
            Sleep(100);
            continue;
        }
        
        PROCESS_KEY(kCenter, shortcutRecentered);
        PROCESS_KEY(kInhibit, shortcutInhibit);
        PROCESS_KEY(kZero, shortcutZero);
        PROCESS_KEY(kStartStop, shortcutStartStop);
        
        Sleep(100);
    }
}
#else
#endif

#ifdef _MSC_VER
#   define LIB_PREFIX ""
#else
#   define LIB_PREFIX "lib"
#endif

//
// Setup the Main Dialog
//
FaceTrackNoIR::FaceTrackNoIR(QWidget *parent, Qt::WFlags flags) : 
    QMainWindow(parent, flags),
    pTrackerDialog(NULL),
    pSecondTrackerDialog(NULL),
    pProtocolDialog(NULL),
    pFilterDialog(NULL),
    trayIcon(NULL),
    trayIconMenu(NULL),
#if defined(__WIN32) || defined(_WIN32)
    keybindingWorker(NULL),
#endif
    keyCenter(),
    keyZero(),
    keyStartStop(),
    keyInhibit(),
    looping(false)
{	
    GlobalPose = new HeadPoseData();
	cameraDetected = false;

	//
	// Initialize Widget handles, to prevent memory-access errors.
	//
	_keyboard_shortcuts = 0;
	_preferences = 0;
	_keyboard_shortcuts = 0;
	_curve_config = 0;

	tracker = 0;
//	_display = 0;
	l = 0;
	trayIcon = 0;

	setupFaceTrackNoIR();

	//
	// Read the AutoStartTracking value from the registry. If it is '1', start the Tracker and Minimize...
	//
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	if (settings.value ( "AutoStartTracking", 0 ).toBool()) {
		startTracker();
	}

    //Q_INIT_RESOURCE(PoseWidget);
	_pose_display = new GLWidget(ui.widget4logo, 0);
    _pose_display->rotateBy(0, 0, 0);

	ui.lblX->setVisible(false);
	ui.lblY->setVisible(false);
	ui.lblZ->setVisible(false);
	ui.lblRotX->setVisible(false);
	ui.lblRotY->setVisible(false);
	ui.lblRotZ->setVisible(false);

	ui.lcdNumOutputPosX->setVisible(false);
	ui.lcdNumOutputPosY->setVisible(false);
	ui.lcdNumOutputPosZ->setVisible(false);
	ui.lcdNumOutputRotX->setVisible(false);
	ui.lcdNumOutputRotY->setVisible(false);
	ui.lcdNumOutputRotZ->setVisible(false);
}

/** sets up all objects and connections to buttons */
void FaceTrackNoIR::setupFaceTrackNoIR() {
    ui.setupUi(this);

    // if we simply place a global variable with THeadPoseData,
    // it gets initialized and pulls in QSettings before
    // main() starts. program can and will crash.

	ui.headPoseWidget->show();
	ui.video_frame->hide();

	// menu objects will be connected with the functions in FaceTrackNoIR class
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.btnLoad, SIGNAL(clicked()), this, SLOT(open()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(save()));
	connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(save()));
	connect(ui.actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(ui.btnSaveAs, SIGNAL(clicked()), this, SLOT(saveAs()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(exit()));

	connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
	connect(ui.actionKeyboard_Shortcuts, SIGNAL(triggered()), this, SLOT(showKeyboardShortcuts()));
	connect(ui.actionCurve_Configuration, SIGNAL(triggered()), this, SLOT(showCurveConfiguration()));
	connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));
	connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(showKeyboardShortcuts()));

	connect(ui.actionSupport, SIGNAL(triggered()), this, SLOT(openurl_support()));
	connect(ui.actionYour_Support, SIGNAL(triggered()), this, SLOT(openurl_donation()));
	connect(ui.btnDonate, SIGNAL(clicked()), this, SLOT(openurl_donation()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));

	connect(ui.actionVideoWidget, SIGNAL(triggered()), this, SLOT(showVideoWidget()));
	connect(ui.actionHeadPoseWidget, SIGNAL(triggered()), this, SLOT(showHeadPoseWidget()));
	connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showTrackerSettings()));
	connect(ui.btnShowSecondTrackerSettings, SIGNAL(clicked()), this, SLOT(showSecondTrackerSettings()));
	connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showServerControls()));
	connect(ui.btnShowFilterControls, SIGNAL(clicked()), this, SLOT(showFilterControls()));

	// Connect checkboxes
	connect(ui.chkInvertYaw, SIGNAL(stateChanged(int)), this, SLOT(setInvertYaw(int)));
	connect(ui.chkInvertRoll, SIGNAL(stateChanged(int)), this, SLOT(setInvertRoll(int)));
	connect(ui.chkInvertPitch, SIGNAL(stateChanged(int)), this, SLOT(setInvertPitch(int)));
	connect(ui.chkInvertX, SIGNAL(stateChanged(int)), this, SLOT(setInvertX(int)));
	connect(ui.chkInvertY, SIGNAL(stateChanged(int)), this, SLOT(setInvertY(int)));
	connect(ui.chkInvertZ, SIGNAL(stateChanged(int)), this, SLOT(setInvertZ(int)));

	// button methods connect with methods in this class
	connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
	connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));

	//read the camera-name, using DirectShow
	GetCameraNameDX();
	
	//Create the system-tray and connect the events for that.
	createIconGroupBox();
	createActions();
	createTrayIcon();

    if (trayIcon)
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	//Load the tracker-settings, from the INI-file
	loadSettings();

    if (trayIcon)
        trayIcon->show();

	connect(ui.iconcomboProtocol, SIGNAL(currentIndexChanged(int)), this, SLOT(protocolSelected(int)));
	connect(ui.iconcomboProfile, SIGNAL(currentIndexChanged(int)), this, SLOT(profileSelected(int)));
	connect(ui.iconcomboTrackerSource, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingSourceSelected(int)));
	connect(ui.iconcomboFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(filterSelected(int)));

	//Setup the timer for automatically minimizing after StartTracker.
	timMinimizeFTN = new QTimer(this);
    connect(timMinimizeFTN, SIGNAL(timeout()), this, SLOT(showMinimized()));

	//Setup the timer for showing the headpose.
	timUpdateHeadPose = new QTimer(this);
    connect(timUpdateHeadPose, SIGNAL(timeout()), this, SLOT(showHeadPose()));
	ui.txtTracking->setVisible(false);
	ui.txtAxisReverse->setVisible(false);
    settingsDirty = false;
}

/** destructor stops the engine and quits the faceapi **/
FaceTrackNoIR::~FaceTrackNoIR() {

	//
	// Stop the tracker, by simulating a button-push
	//
	stopTracker();

	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard, QMessageBox::Discard );

		switch (ret) {
			case QMessageBox::Save:
				saveAs();
				break;
			case QMessageBox::Discard:
				// Don't Save was clicked
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
}

//
// Update the Settings, after a value has changed. This way, the Tracker does not have to re-start.
//
void FaceTrackNoIR::updateSettings() {
	if ( tracker != NULL ) {
		tracker->loadSettings();
	}
}

//
// Get a pointer to the video-widget, to use in the DLL
//
QFrame *FaceTrackNoIR::get_video_widget() {
	return ui.video_frame;
}

/** read the name of the first video-capturing device at start up **/
/** FaceAPI can only use this first one... **/
void FaceTrackNoIR::GetCameraNameDX() {
#if 0
////	ui.widget->setCameraName("No video-capturing device was found in your system: check if it's connected!");

	ui.cameraName->setText("No video-capturing device was found in your system: check if it's connected!");

	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		qDebug() << "GetWDM says: CoCreateInstance Failed!";
		return;
	}

	qDebug() << "GetWDM says: CoCreateInstance succeeded!";
	
	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) {
		qDebug() << "GetWDM says: CreateClassEnumerator succeeded!";

		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		if (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))	{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					QString str((QChar*)varName.bstrVal, wcslen(varName.bstrVal));
					qDebug() << "GetWDM says: Moniker found:" << str;
					ui.cameraName->setText(str);
				}
				VariantClear(&varName);

				////// To create an instance of the filter, do the following:
				////IBaseFilter *pFilter;
				////hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
				////	(void**)&pFilter);
				// Now add the filter to the graph. 
				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
#endif
}

//
// Open an INI-file with the QFileDialog
// If succesfull, the settings in it will be read
//
void FaceTrackNoIR::open() {
     QFileDialog dialog(this);
     dialog.setFileMode(QFileDialog::ExistingFile);
     
	 QString fileName = dialog.getOpenFileName(
								this,
                                 tr("Select one FTNoir settings file"),
								 QCoreApplication::applicationDirPath() + "/Settings/",
                                 tr("Settings file (*.ini);;All Files (*)"),
                                               NULL);

	//
	// If a file was selected, save it's name and read it's contents.
	//
	if (! fileName.isEmpty() ) {
		QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
        settings.setValue ("SettingsFile", QFileInfo(fileName).absoluteFilePath());
		loadSettings();
    }
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FaceTrackNoIR::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Tracking" );
	iniFile.setValue ( "invertYaw", ui.chkInvertYaw->isChecked() );
	iniFile.setValue ( "invertPitch", ui.chkInvertPitch->isChecked() );
	iniFile.setValue ( "invertRoll", ui.chkInvertRoll->isChecked() );
	iniFile.setValue ( "invertX", ui.chkInvertX->isChecked() );
	iniFile.setValue ( "invertY", ui.chkInvertY->isChecked() );
	iniFile.setValue ( "invertZ", ui.chkInvertZ->isChecked() );
	iniFile.endGroup ();

	iniFile.beginGroup ( "GameProtocol" );
    {
        DynamicLibrary* proto = dlopen_protocols.value( ui.iconcomboProtocol->currentIndex(), (DynamicLibrary*) NULL);
        iniFile.setValue ( "DLL",  proto == NULL ? "" : proto->filename);
    }
	iniFile.endGroup ();

	iniFile.beginGroup ( "TrackerSource" );
    {
        DynamicLibrary* tracker = dlopen_trackers.value( ui.iconcomboTrackerSource->currentIndex(), (DynamicLibrary*) NULL);
        iniFile.setValue ( "DLL",  tracker == NULL ? "" : tracker->filename);
    }
    {
        DynamicLibrary* tracker = dlopen_trackers.value( ui.cbxSecondTrackerSource->currentIndex() - 1, (DynamicLibrary*) NULL);
        iniFile.setValue ( "2ndDLL",  tracker == NULL ? "" : tracker->filename);
    }
	iniFile.endGroup ();

	//
	// Save the name of the filter in the INI-file.
	//
	iniFile.beginGroup ( "Filter" );
    {
        DynamicLibrary* filter = dlopen_filters.value( ui.iconcomboFilter->currentIndex(), (DynamicLibrary*) NULL);
        iniFile.setValue ( "DLL",  filter == NULL ? "" : filter->filename);
    }
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Get the new name of the INI-file and save the settings to it.
//
// The user may choose to overwrite an existing file. This will be deleted, before copying the current file to it.
//
void FaceTrackNoIR::saveAs()
{
	//
	// Get the current filename of the INI-file.
	//
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	QString oldFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();

	//
	// Get the new filename of the INI-file.
	//
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
													oldFile,
//													QCoreApplication::applicationDirPath() + "/Settings",
													tr("Settings file (*.ini);;All Files (*)"));
	if (!fileName.isEmpty()) {

		//
		// Remove the file, if it already exists.
		//
		QFileInfo newFileInfo ( fileName );
		if ((newFileInfo.exists()) && (oldFile != fileName)) {
			QFile newFileFile ( fileName );
			newFileFile.remove();
		}

		//
		// Copy the current INI-file to the new name.
		//
		QFileInfo oldFileInfo ( oldFile );
		if (oldFileInfo.exists()) {
			QFile oldFileFile ( oldFile );
			oldFileFile.copy( fileName );
		}

		//
		// Write the new name to the Registry and save the other INI-values.
		//
		settings.setValue ("SettingsFile", fileName);
		save();

		//
		// Reload the settings, to get the GUI right again...
		//
		loadSettings();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FaceTrackNoIR::loadSettings() {
    if (looping)
        return;
    looping = true;
	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    qDebug() << "Config file now" << currentFile;
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	//
	// Put the filename in the window-title.
	//
    QFileInfo pathInfo ( currentFile );
    setWindowTitle ( "FaceTrackNoIR (1.8 pre-alpha) - " + pathInfo.fileName() );

	//
	// Get a List of all the INI-files in the (currently active) Settings-folder.
	//
	QDir settingsDir( pathInfo.dir() );
    QStringList filters;
    filters << "*.ini";
	iniFileList.clear();
	iniFileList = settingsDir.entryList( filters, QDir::Files, QDir::Name );
	
	//
	// Add strings to the Listbox.
	//
	ui.iconcomboProfile->clear();
	for ( int i = 0; i < iniFileList.size(); i++) {
        ui.iconcomboProfile->addItem(QIcon(":/images/settings16.png"), iniFileList.at(i));
		if (iniFileList.at(i) == pathInfo.fileName()) {
            ui.iconcomboProfile->setItemIcon(i, QIcon(":/images/settingsopen16.png"));
			ui.iconcomboProfile->setCurrentIndex( i );
		}
	}

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Tracking" );
    ui.chkInvertYaw->setChecked (iniFile.value ( "invertYaw", 0 ).toBool());
	ui.chkInvertPitch->setChecked (iniFile.value ( "invertPitch", 0 ).toBool());
	ui.chkInvertRoll->setChecked (iniFile.value ( "invertRoll", 0 ).toBool());
	ui.chkInvertX->setChecked (iniFile.value ( "invertX", 0 ).toBool());
	ui.chkInvertY->setChecked (iniFile.value ( "invertY", 0 ).toBool());
	ui.chkInvertZ->setChecked (iniFile.value ( "invertZ", 0 ).toBool());
	iniFile.endGroup ();

	//
	// Read the currently selected Protocol from the INI-file.
	// If the setting "DLL" isn't found (pre-1.7 version of INI), then the setting 'Selection' is evaluated.
	//
	iniFile.beginGroup ( "GameProtocol" );
    QString selectedProtocolName = iniFile.value ( "DLL", "" ).toString();
    iniFile.endGroup ();

    //
	// Find the Index of the DLL and set the selection.
	//
    for ( int i = 0; i < dlopen_protocols.size(); i++) {
        if (dlopen_protocols.at(i)->filename.compare( selectedProtocolName, Qt::CaseInsensitive ) == 0) {
			ui.iconcomboProtocol->setCurrentIndex( i );
			break;
		}
	}

    //
    // Read the currently selected Tracker from the INI-file.
    // If the setting "DLL" isn't found (pre-1.7 version), then the setting 'Selection' is evaluated.
    //
    iniFile.beginGroup ( "TrackerSource" );
    QString selectedTrackerName = iniFile.value ( "DLL", "" ).toString();
    qDebug() << "loadSettings says: selectedTrackerName = " << selectedTrackerName;
    QString secondTrackerName = iniFile.value ( "2ndDLL", "None" ).toString();
    qDebug() << "loadSettings says: secondTrackerName = " << secondTrackerName;
    iniFile.endGroup ();

    for ( int i = 0; i < dlopen_trackers.size(); i++) {
        DynamicLibrary* foo = dlopen_trackers.at(i);
        if (foo && foo->filename.compare( selectedTrackerName, Qt::CaseInsensitive ) == 0) {
			ui.iconcomboTrackerSource->setCurrentIndex( i );
		}
        if (foo && foo->filename.compare( secondTrackerName, Qt::CaseInsensitive ) == 0) {
            ui.cbxSecondTrackerSource->setCurrentIndex( i + 1 );
		}
	}

	//
	// Read the currently selected Filter from the INI-file.
	//
	iniFile.beginGroup ( "Filter" );
    QString selectedFilterName = iniFile.value ( "DLL", "" ).toString();
	qDebug() << "createIconGroupBox says: selectedFilterName = " << selectedFilterName;
	iniFile.endGroup ();

	//
	// Find the Index of the DLL and set the selection.
	//
    for ( int i = 0; i < dlopen_filters.size(); i++) {
        DynamicLibrary* foo = dlopen_filters.at(i);
        if (foo && foo->filename.compare( selectedFilterName, Qt::CaseInsensitive ) == 0) {
            ui.iconcomboFilter->setCurrentIndex( i );
			break;
		}
	}

	settingsDirty = false;
    looping = false;
}

/** show support page in web-browser **/
void FaceTrackNoIR::openurl_support() {
	QDesktopServices::openUrl(QUrl("http://facetracknoir.sourceforge.net/manual/manual.htm", QUrl::TolerantMode));
}

/** show donations page in web-browser **/
void FaceTrackNoIR::openurl_donation() {
	QDesktopServices::openUrl(QUrl("http://facetracknoir.sourceforge.net/information_links/donate.htm", QUrl::TolerantMode));
}

/** show about dialog **/
void FaceTrackNoIR::about() {

	QPoint offsetpos(100, 100);
	aboutDialog.move(this->pos() + offsetpos);
	aboutDialog.show();

	/** ABOUT DIALOG **/
	aboutDialog.setBaseSize(270, 440);

	aboutDialog.setMaximumWidth(270);
	aboutDialog.setMaximumHeight(440);

	aboutDialog.setMinimumWidth(270);
	aboutDialog.setMinimumHeight(440);
    aboutDialog.setStyleSheet("background:#fff url(:/uielements/aboutfacetracknoir.png) no-repeat;");
}

/** start tracking the face **/
void FaceTrackNoIR::startTracker( ) {	
    bindKeyboardShortcuts();

	// 
	// Disable buttons
	//
	ui.iconcomboProfile->setEnabled ( false );
	ui.btnLoad->setEnabled ( false );
	ui.btnSave->setEnabled ( false );
	ui.btnSaveAs->setEnabled ( false );
	ui.btnShowFilterControls->setEnabled ( false );

	//
	// Create the Tracker and setup
	//

    if (Libraries)
        delete Libraries;
    Libraries = new SelectedLibraries(this);

    if (!Libraries->correct)
    {
        QMessageBox::warning(this, "Something went wrong", "Tracking can't be initialized, probably protocol prerequisites missing", QMessageBox::Ok, QMessageBox::NoButton);
        stopTracker();
        return;
    }
    
#if defined(_WIN32) || defined(__WIN32)
    keybindingWorker = new KeybindingWorker(*this, keyCenter, keyInhibit, keyStartStop, keyZero);
    keybindingWorker->start();
#endif

    if (tracker) {
        tracker->wait();
        delete tracker;
    }

	tracker = new Tracker ( this );

	//
	// Setup the Tracker and send the settings.
	// This is necessary, because the events are only triggered 'on change'
	//
	tracker->setInvertYaw (ui.chkInvertYaw->isChecked() );
	tracker->setInvertPitch (ui.chkInvertPitch->isChecked() );
	tracker->setInvertRoll (ui.chkInvertRoll->isChecked() );
	tracker->setInvertX (ui.chkInvertX->isChecked() );
	tracker->setInvertY (ui.chkInvertY->isChecked() );
	tracker->setInvertZ (ui.chkInvertZ->isChecked() );

    tracker->start();

	//
	// Register the Tracker instance with the Tracker Dialog (if open)
	//
    if (pTrackerDialog && Libraries->pTracker) {
        pTrackerDialog->registerTracker( Libraries->pTracker );
	}

	ui.headPoseWidget->show();

	// 
	ui.btnStartTracker->setEnabled ( false );
	ui.btnStopTracker->setEnabled ( true );

	// Enable/disable Protocol-server Settings
	ui.iconcomboTrackerSource->setEnabled ( false );
	ui.cbxSecondTrackerSource->setEnabled ( false );
	ui.iconcomboProtocol->setEnabled ( false );
    ui.btnShowServerControls->setEnabled ( false );
	ui.iconcomboFilter->setEnabled ( false );

	//
	// Update the camera-name, FaceAPI can only use the 1st one found!
	//
	GetCameraNameDX();

	//
	// Get the TimeOut value for minimizing FaceTrackNoIR
	// Only start the Timer if value > 0
	//
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	int timevalue = settings.value ( "AutoMinimizeTime", 0 ).toInt() * 1000;
	if (timevalue > 0) {

		bool minimizeTaskBar = settings.value ( "MinimizeTaskBar", 1 ).toBool();
		if (minimizeTaskBar) {
			connect(timMinimizeFTN, SIGNAL(timeout()), this, SLOT(showMinimized()));
		}
		else {
			connect(timMinimizeFTN, SIGNAL(timeout()), this, SLOT(hide()));
		}

		timMinimizeFTN->setSingleShot( true );
		timMinimizeFTN->start(timevalue);
	}

	//
	// Start the timer to update the head-pose (digits and 'man in black')
	//
	timUpdateHeadPose->start(50);

	ui.lblX->setVisible(true);
	ui.lblY->setVisible(true);
	ui.lblZ->setVisible(true);
	ui.lblRotX->setVisible(true);
	ui.lblRotY->setVisible(true);
	ui.lblRotZ->setVisible(true);

	ui.lcdNumOutputPosX->setVisible(true);
	ui.lcdNumOutputPosY->setVisible(true);
	ui.lcdNumOutputPosZ->setVisible(true);
	ui.lcdNumOutputRotX->setVisible(true);
	ui.lcdNumOutputRotY->setVisible(true);
	ui.lcdNumOutputRotZ->setVisible(true);
}

/** stop tracking the face **/
void FaceTrackNoIR::stopTracker( ) {	

#if defined(_WIN32) || defined(__WIN32)
    if (keybindingWorker)
    {
        keybindingWorker->should_quit = true;
        keybindingWorker->wait();
        delete keybindingWorker;
        keybindingWorker = NULL;
    }
#endif
	//
	// Stop displaying the head-pose.
	//
	timUpdateHeadPose->stop();
    _pose_display->rotateBy(0, 0, 0);

	ui.lblX->setVisible(false);
	ui.lblY->setVisible(false);
	ui.lblZ->setVisible(false);
	ui.lblRotX->setVisible(false);
	ui.lblRotY->setVisible(false);
	ui.lblRotZ->setVisible(false);

	ui.lcdNumOutputPosX->setVisible(false);
	ui.lcdNumOutputPosY->setVisible(false);
	ui.lcdNumOutputPosZ->setVisible(false);
	ui.lcdNumOutputRotX->setVisible(false);
	ui.lcdNumOutputRotY->setVisible(false);
	ui.lcdNumOutputRotZ->setVisible(false);
	ui.txtTracking->setVisible(false);
	ui.txtAxisReverse->setVisible(false);

	//
	// Delete the tracker (after stopping things and all).
	//
    if ( tracker ) {
        qDebug() << "Done with tracking";
        tracker->should_quit = true;
        tracker->wait();

		qDebug() << "stopTracker says: Deleting tracker!";
		delete tracker;
		qDebug() << "stopTracker says: Tracker deleted!";
		tracker = 0;
        if (Libraries) {
            delete Libraries;
            Libraries = NULL;
        }
	}

    //
    // UnRegister the Tracker instance with the Tracker Dialog (if open)
    //
    if (pTrackerDialog) {
        pTrackerDialog->unRegisterTracker();
    }
    if (pProtocolDialog) {
        pProtocolDialog->unRegisterProtocol();
    }
	ui.btnStartTracker->setEnabled ( true );
	ui.btnStopTracker->setEnabled ( false );
//	ui.btnShowEngineControls->setEnabled ( false );
	ui.iconcomboProtocol->setEnabled ( true );
	ui.iconcomboTrackerSource->setEnabled ( true );
	ui.cbxSecondTrackerSource->setEnabled ( true );
	ui.iconcomboFilter->setEnabled ( true );

	// Enable/disable Protocol-server Settings
	ui.btnShowServerControls->setEnabled ( true );
	ui.video_frame->hide();

	// 
	ui.iconcomboProfile->setEnabled ( true );
	ui.btnLoad->setEnabled ( true );
	ui.btnSave->setEnabled ( true );
	ui.btnSaveAs->setEnabled ( true );
	ui.btnShowFilterControls->setEnabled ( true );

	//
	// Stop the timer, so it won't go off again...
	//
	timMinimizeFTN->stop();
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertYaw( int invert ) {
    if (tracker)
        tracker->setInvertYaw ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertPitch( int invert ) {
    if (tracker)
        tracker->setInvertPitch ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertRoll( int invert ) {
    if (tracker)
        tracker->setInvertRoll ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertX( int invert ) {
    if (tracker)
        tracker->setInvertX ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertY( int invert ) {
    if (tracker)
        tracker->setInvertY ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** set the invert from the checkbox **/
void FaceTrackNoIR::setInvertZ( int invert ) {
    if (tracker)
        tracker->setInvertZ ( (invert != 0)?true:false );
	settingsDirty = true;
}

/** Show the headpose in the widget (triggered by timer) **/
void FaceTrackNoIR::showHeadPose() {
THeadPoseData newdata;

	ui.lblX->setVisible(true);
	ui.lblY->setVisible(true);
	ui.lblZ->setVisible(true);
	ui.lblRotX->setVisible(true);
	ui.lblRotY->setVisible(true);
	ui.lblRotZ->setVisible(true);

	ui.lcdNumOutputPosX->setVisible(true);
	ui.lcdNumOutputPosY->setVisible(true);
	ui.lcdNumOutputPosZ->setVisible(true);
	ui.lcdNumOutputRotX->setVisible(true);
	ui.lcdNumOutputRotY->setVisible(true);
	ui.lcdNumOutputRotZ->setVisible(true);

	//
	// Get the pose and also display it.
	// Updating the pose from within the Tracker-class caused crashes...
	//
    tracker->getHeadPose(&newdata);
	ui.lcdNumX->display(QString("%1").arg(newdata.x, 0, 'f', 1));
	ui.lcdNumY->display(QString("%1").arg(newdata.y, 0, 'f', 1));
	ui.lcdNumZ->display(QString("%1").arg(newdata.z, 0, 'f', 1));

	ui.lcdNumRotX->display(QString("%1").arg(newdata.yaw, 0, 'f', 1));
	ui.lcdNumRotY->display(QString("%1").arg(newdata.pitch, 0, 'f', 1));
	ui.lcdNumRotZ->display(QString("%1").arg(newdata.roll, 0, 'f', 1));

    ui.txtTracking->setVisible(tracker->getTrackingActive());
    ui.txtAxisReverse->setVisible(tracker->getAxisReverse());

	//
	// Get the output-pose and also display it.
	//
	if (_pose_display) {
        tracker->getOutputHeadPose(&newdata);
		_pose_display->rotateBy(newdata.pitch, newdata.yaw, newdata.roll);

		ui.lcdNumOutputPosX->display(QString("%1").arg(newdata.x, 0, 'f', 1));
		ui.lcdNumOutputPosY->display(QString("%1").arg(newdata.y, 0, 'f', 1));
		ui.lcdNumOutputPosZ->display(QString("%1").arg(newdata.z, 0, 'f', 1));

		ui.lcdNumOutputRotX->display(QString("%1").arg(newdata.yaw, 0, 'f', 1));
		ui.lcdNumOutputRotY->display(QString("%1").arg(newdata.pitch, 0, 'f', 1));
		ui.lcdNumOutputRotZ->display(QString("%1").arg(newdata.roll, 0, 'f', 1));
	}
}

/** toggles Video Widget **/
void FaceTrackNoIR::showVideoWidget() {
	if(ui.video_frame->isHidden())
		ui.video_frame->show();
	else
		ui.video_frame->hide();
}

/** toggles Video Widget **/
void FaceTrackNoIR::showHeadPoseWidget() {
	if(ui.headPoseWidget->isHidden())
		ui.headPoseWidget->show();
	else
		ui.headPoseWidget->hide();
}

/** toggles Engine Controls Dialog **/
void FaceTrackNoIR::showTrackerSettings() {
	if (pTrackerDialog) {
		delete pTrackerDialog;
		pTrackerDialog = NULL;
	}

    DynamicLibrary* lib = dlopen_trackers.value(ui.iconcomboTrackerSource->currentIndex(), (DynamicLibrary*) NULL);

    if (lib) {
        pTrackerDialog = (ITrackerDialog*) lib->Dialog();
        if (pTrackerDialog) {
            pTrackerDialog->Initialize(this);
            if (Libraries && Libraries->pTracker)
                pTrackerDialog->registerTracker(Libraries->pTracker);
        }
    }
}

// Show the Settings dialog for the secondary Tracker
void FaceTrackNoIR::showSecondTrackerSettings() {
    if (pSecondTrackerDialog) {
        delete pSecondTrackerDialog;
        pSecondTrackerDialog = NULL;
    }

    DynamicLibrary* lib = dlopen_trackers.value(ui.cbxSecondTrackerSource->currentIndex() - 1, (DynamicLibrary*) NULL);

    if (lib) {
        pSecondTrackerDialog = (ITrackerDialog*) lib->Dialog();
        if (pSecondTrackerDialog) {
            pSecondTrackerDialog->Initialize(this);
            if (Libraries && Libraries->pSecondTracker)
                pSecondTrackerDialog->registerTracker(Libraries->pSecondTracker);
        }
    }
}

/** toggles Server Controls Dialog **/
void FaceTrackNoIR::showServerControls() {
    if (pProtocolDialog) {
        delete pProtocolDialog;
        pProtocolDialog = NULL;
    }

    DynamicLibrary* lib = dlopen_protocols.value(ui.iconcomboProtocol->currentIndex(), (DynamicLibrary*) NULL);

    if (lib && lib->Dialog) {
        pProtocolDialog = (IProtocolDialog*) lib->Dialog();
        if (pProtocolDialog) {
            pProtocolDialog->Initialize(this);
        }
    }
}

/** toggles Filter Controls Dialog **/
void FaceTrackNoIR::showFilterControls() {
    if (pFilterDialog) {
        delete pFilterDialog;
        pFilterDialog = NULL;
    }

    DynamicLibrary* lib = dlopen_filters.value(ui.iconcomboFilter->currentIndex(), (DynamicLibrary*) NULL);

    if (lib && lib->Dialog) {
        pFilterDialog = (IFilterDialog*) lib->Dialog();
        if (pFilterDialog) {
            pFilterDialog->Initialize(this, Libraries ? Libraries->pFilter : NULL);
        }
    }
}

/** toggles FaceTrackNoIR Preferences Dialog **/
void FaceTrackNoIR::showPreferences() {

	// Create if new
	if (!_preferences)
    {
        _preferences = new PreferencesDialog( this, this, Qt::Dialog );
    }

	// Show if already created
	if (_preferences) {
		_preferences->show();
		_preferences->raise();
	}
}

/** toggles Keyboard Shortcut Dialog **/
void FaceTrackNoIR::showKeyboardShortcuts() {

	// Create if new
	if (!_keyboard_shortcuts)
    {
        _keyboard_shortcuts = new KeyboardShortcutDialog( this, this, Qt::Dialog );
    }

	// Show if already created
	if (_keyboard_shortcuts) {
		_keyboard_shortcuts->show();
		_keyboard_shortcuts->raise();
	}
}

/** toggles Curve Configuration Dialog **/
void FaceTrackNoIR::showCurveConfiguration() {

	// Create if new
	if (!_curve_config)
    {
        _curve_config = new CurveConfigurationDialog( this, this, Qt::Dialog );
    }

	// Show if already created
	if (_curve_config) {
		_curve_config->show();
		_curve_config->raise();
	}
}

/** exit application **/
void FaceTrackNoIR::exit() {
	QCoreApplication::exit(0);
}

//
// Setup the icons for the comboBoxes
//
void FaceTrackNoIR::createIconGroupBox()
{
	QDir settingsDir( QCoreApplication::applicationDirPath() );

    {
        QStringList protocols = settingsDir.entryList( QStringList() << (LIB_PREFIX "ftnoir-proto-*." SONAME), QDir::Files, QDir::Name );
        for ( int i = 0; i < protocols.size(); i++) {
            QIcon icon;
            QString longName;
            QString str = protocols.at(i);
            DynamicLibrary* lib = new DynamicLibrary(str.toLatin1().constData());
            qDebug() << "Loading" << str;
            std::cout.flush();
            Metadata* meta;
            if (!lib->Metadata || ((meta = lib->Metadata()), !meta))
            {
                delete lib;
                continue;
            }
            meta->getFullName(&longName);
            meta->getIcon(&icon);
            delete meta;
            dlopen_protocols.push_back(lib);
            ui.iconcomboProtocol->addItem(icon, longName);
        }
    }

    {
        ui.cbxSecondTrackerSource->addItem(QIcon(), "None");
        QStringList trackers = settingsDir.entryList( QStringList() << (LIB_PREFIX "ftnoir-tracker-*." SONAME), QDir::Files, QDir::Name );
        for ( int i = 0; i < trackers.size(); i++) {
            QIcon icon;
            QString longName;
            QString str = trackers.at(i);
            DynamicLibrary* lib = new DynamicLibrary(str.toLatin1().constData());
            qDebug() << "Loading" << str;
            std::cout.flush();
            Metadata* meta;
            if (!lib->Metadata || ((meta = lib->Metadata()), !meta))
            {
                delete lib;
                continue;
            }
            meta->getFullName(&longName);
            meta->getIcon(&icon);
            delete meta;
            dlopen_trackers.push_back(lib);
            ui.iconcomboTrackerSource->addItem(icon, longName);
            ui.cbxSecondTrackerSource->addItem(icon, longName);
        }
    }

    {
        dlopen_filters.push_back((DynamicLibrary*) NULL);
        ui.iconcomboFilter->addItem(QIcon(), "None");
        QStringList filters = settingsDir.entryList( QStringList() << (LIB_PREFIX "ftnoir-filter-*." SONAME), QDir::Files, QDir::Name );
        for ( int i = 0; i < filters.size(); i++) {
            QIcon icon;
            QString fullName;
            QString str = filters.at(i);
            DynamicLibrary* lib = new DynamicLibrary(str.toLatin1().constData());
            qDebug() << "Loading" << str;
            std::cout.flush();
            Metadata* meta;
            if (!lib->Metadata || ((meta = lib->Metadata()), !meta))
            {
                delete lib;
                continue;
            }
            meta->getFullName(&fullName);
            meta->getIcon(&icon);
            delete meta;
            dlopen_filters.push_back(lib);
            ui.iconcomboFilter->addItem(icon, fullName);
        }
    }

	connect(ui.iconcomboProtocol, SIGNAL(currentIndexChanged(int)), this, SLOT(protocolSelected(int)));
    connect(ui.iconcomboTrackerSource, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingSourceSelected(int)));
    connect(ui.iconcomboFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(filterSelected(int)));
	connect(ui.cbxSecondTrackerSource, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingSourceSelected(int)));
}

//
// Create the Actions in the System tray and connect them to Application events
//
void FaceTrackNoIR::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize FaceTrackNoIR"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    //maximizeAction = new QAction(tr("Ma&ximize"), this);
    //connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore FaceTrackNoIR"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit FaceTrackNoIR"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

//
// Create the SystemTray and set the default Icon
//
void FaceTrackNoIR::createTrayIcon()
{
	if (QSystemTrayIcon::isSystemTrayAvailable()) {
		trayIconMenu = new QMenu(this);
		trayIconMenu->addAction(minimizeAction);
		trayIconMenu->addAction(restoreAction);
		trayIconMenu->addSeparator();
		trayIconMenu->addAction(quitAction);

		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setContextMenu(trayIconMenu);

        //trayIcon->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/images/FaceTrackNoIR.png"));
	}
}

//
// Handle SystemTray events
//
void FaceTrackNoIR::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
     switch (reason) {
     case QSystemTrayIcon::Trigger:
     case QSystemTrayIcon::DoubleClick:
         //ui.iconcomboProtocol->setCurrentIndex((ui.iconcomboProtocol->currentIndex() + 1)
         //                              % ui.iconcomboProtocol->count());
         break;
     ////case QSystemTrayIcon::MiddleClick:
     ////    showMessage();
     ////    break;
     default:
         ;
     }
 }

//
// Handle changes of the Protocol selection
//
void FaceTrackNoIR::protocolSelected(int index)
{
	settingsDirty = true;
	ui.btnShowServerControls->setEnabled ( true );

	//
	// Set the Icon for the tray and update the Icon for the Settings button.
	//
	QIcon icon = ui.iconcomboProtocol->itemIcon(index);
	if (trayIcon != 0) {
		trayIcon->setIcon(icon);
	    trayIcon->setToolTip(ui.iconcomboProtocol->itemText(index));
		trayIcon->show();
		trayIcon->showMessage( "FaceTrackNoIR", ui.iconcomboProtocol->itemText(index));
	}
    //setWindowIcon(QIcon(":/images/FaceTrackNoIR.png"));
    //breaks with transparency -sh
    //ui.btnShowServerControls->setIcon(icon);]
}

//
// Handle changes of the Tracking Source selection
//
void FaceTrackNoIR::trackingSourceSelected(int index)
{
	settingsDirty = true;
	ui.btnShowEngineControls->setEnabled ( true );
}

//
// Handle changes of the Profile selection
//
void FaceTrackNoIR::profileSelected(int index)
{
	//
	// Read the current INI-file setting, to get the folder in which it's located...
	//
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QFileInfo pathInfo ( currentFile );

	//
	// Save the name of the INI-file in the Registry.
	//
    settings.setValue ("SettingsFile", pathInfo.absolutePath() + "/" + iniFileList.value(ui.iconcomboProfile->currentIndex(), ""));
	loadSettings();
}

//
// Handle changes of the Filter selection
//
void FaceTrackNoIR::filterSelected(int index)
{
	settingsDirty = true;

	//QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	//QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	//QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	ui.btnShowFilterControls->setEnabled ( true );
}

//
// Constructor for FaceTrackNoIR=Preferences-dialog
//
PreferencesDialog::PreferencesDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	mainApp = ftnoir;											// Preserve a pointer to FTNoIR

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	connect(ui.spinAutoMinimizeTime, SIGNAL(valueChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkAutoStartTracking, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.radioMinimize, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
PreferencesDialog::~PreferencesDialog() {
	qDebug() << "~PreferencesDialog() says: started";
}

//
// OK clicked on server-dialog
//
void PreferencesDialog::doOK() {
	save();
	this->close();
}

// override show event
void PreferencesDialog::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void PreferencesDialog::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void PreferencesDialog::loadSettings() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	ui.spinAutoMinimizeTime->setValue( settings.value ( "AutoMinimizeTime", 0 ).toInt() );
	ui.chkAutoStartTracking->setChecked( settings.value ( "AutoStartTracking", 0 ).toBool() );
	ui.radioMinimize->setChecked( settings.value ( "MinimizeTaskBar", 1 ).toBool() );

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void PreferencesDialog::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	settings.setValue( "AutoMinimizeTime", ui.spinAutoMinimizeTime->value() );
	settings.setValue( "AutoStartTracking", ui.chkAutoStartTracking->isChecked() );
	settings.setValue( "MinimizeTaskBar", ui.radioMinimize->isChecked() );

	//
	// Send a message to the main program, to update the Settings (for the tracker)
	//
	mainApp->updateSettings();

	settingsDirty = false;
}

//**************************************************************************************************//
//**************************************************************************************************//
//
// Constructor for Keyboard-shortcuts-dialog
//
KeyboardShortcutDialog::KeyboardShortcutDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	mainApp = ftnoir;											// Preserve a pointer to FTNoIR

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	connect(ui.cbxCenterKey, SIGNAL(currentIndexChanged(int)), this, SLOT(keyChanged( int )));
	connect(ui.chkCenterShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkCenterCtrl, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkCenterAlt, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	connect(ui.cbxGameZeroKey, SIGNAL(currentIndexChanged(int)), this, SLOT(keyChanged( int )));
	connect(ui.chkGameZeroShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkGameZeroCtrl, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkGameZeroAlt, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	connect(ui.cbxStartStopKey, SIGNAL(currentIndexChanged(int)), this, SLOT(keyChanged( int )));
	connect(ui.chkStartStopShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkStartStopCtrl, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkStartStopAlt, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.radioSetZero, SIGNAL(toggled(bool)), this, SLOT(keyChanged(bool)));

	connect(ui.cbxInhibitKey, SIGNAL(currentIndexChanged(int)), this, SLOT(keyChanged( int )));
	connect(ui.chkInhibitShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitCtrl, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitAlt, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	// Also add events for the Axis-checkboxes
	connect(ui.chkInhibitShift, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitYaw, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitRoll, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitX, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitY, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));
	connect(ui.chkInhibitZ, SIGNAL(stateChanged(int)), this, SLOT(keyChanged(int)));

	// Clear the Lists with key-descriptions and keycodes and build the Lists
	// The strings will all be added to the ListBoxes for each Shortkey
	//

	// Add strings to the Listboxes.
	//

    for ( int i = 0; i < global_key_sequences.size(); i++) {
        ui.cbxCenterKey->addItem(global_key_sequences.at(i));
        ui.cbxGameZeroKey->addItem(global_key_sequences.at(i));
        ui.cbxStartStopKey->addItem(global_key_sequences.at(i));
        ui.cbxInhibitKey->addItem(global_key_sequences.at(i));
	}

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
KeyboardShortcutDialog::~KeyboardShortcutDialog() {
	qDebug() << "~KeyboardShortcutDialog() says: started";
}

//
// OK clicked on server-dialog
//
void KeyboardShortcutDialog::doOK() {
	save();
	this->close();
    mainApp->bindKeyboardShortcuts();
}

// override show event
void KeyboardShortcutDialog::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void KeyboardShortcutDialog::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

void FaceTrackNoIR::bindKeyboardShortcuts()
{
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)
    iniFile.beginGroup ( "KB_Shortcuts" );
    int idxCenter = iniFile.value("Key_index_Center", 0).toInt();
    int idxGameZero = iniFile.value("Key_index_GameZero", 0).toInt();
    int idxStartStop = iniFile.value("Key_index_StartStop", 0).toInt();
    int idxInhibit = iniFile.value("Key_index_Inhibit", 0).toInt();
    
#if !defined(_WIN32) && !defined(__WIN32)
    if (keyCenter) {
        delete keyCenter;
        keyCenter = NULL;
    }

    if (keyZero) {
        delete keyZero;
        keyZero = NULL;
    }

    if (keyStartStop) {
        delete keyStartStop;
        keyStartStop = NULL;
    }

    if (keyInhibit) {
        delete keyInhibit;
        keyInhibit = NULL;
    }

    if (idxCenter > 0)
    {
        QString seq(global_key_sequences.value(idxCenter, ""));
        if (!seq.isEmpty())
        {
            if (iniFile.value("Shift_Center", false).toBool())
                seq = "Shift+" + seq;
            if (iniFile.value("Alt_Center", false).toBool())
                seq = "Alt+" + seq;
            if (iniFile.value("Ctrl_Center", false).toBool())
                seq = "Ctrl+" + seq;
            keyCenter = new QxtGlobalShortcut(QKeySequence(seq));
            connect(keyCenter, SIGNAL(activated()), this, SLOT(shortcutRecentered()));
        }
    }

    if (idxGameZero > 0)
    {
        QString seq(global_key_sequences.value(idxGameZero, ""));
        if (!seq.isEmpty())
        {
            if (iniFile.value("Shift_GameZero", false).toBool())
                seq = "Shift+" + seq;
            if (iniFile.value("Alt_GameZero", false).toBool())
                seq = "Alt+" + seq;
            if (iniFile.value("Ctrl_GameZero", false).toBool())
                seq = "Ctrl+" + seq;
        }
        keyZero = new QxtGlobalShortcut(QKeySequence(seq));
        connect(keyZero, SIGNAL(activated()), this, SLOT(shortcutZero()));
    }

    if (idxStartStop > 0)
    {
        QString seq(global_key_sequences.value(idxStartStop, ""));
        if (!seq.isEmpty())
        {
            if (iniFile.value("Shift_StartStop", false).toBool())
                seq = "Shift+" + seq;
            if (iniFile.value("Alt_StartStop", false).toBool())
                seq = "Alt+" + seq;
            if (iniFile.value("Ctrl_StartStop", false).toBool())
                seq = "Ctrl+" + seq;
        }
        keyStartStop = new QxtGlobalShortcut(QKeySequence(seq));
        connect(keyStartStop, SIGNAL(activated()), this, SLOT(shortcutStartStop()));
    }

    if (idxInhibit > 0)
    {
        QString seq(global_key_sequences.value(idxInhibit, ""));
        if (!seq.isEmpty())
        {
            if (iniFile.value("Shift_Inhibit", false).toBool())
                seq = "Shift+" + seq;
            if (iniFile.value("Alt_Inhibit", false).toBool())
                seq = "Alt+" + seq;
            if (iniFile.value("Ctrl_Inhibit", false).toBool())
                seq = "Ctrl+" + seq;
        }
        keyInhibit = new QxtGlobalShortcut(QKeySequence(seq));
        connect(keyInhibit, SIGNAL(activated()), this, SLOT(shortcutInhibit()));
    }
#else
    keyCenter.keycode = keyZero.keycode = keyInhibit.keycode = keyStartStop.keycode = 0;
    keyCenter.shift = keyCenter.alt = keyCenter.ctrl = 0;
    keyZero.shift = keyZero.alt = keyZero.ctrl = 0;
    keyInhibit.shift = keyInhibit.alt = keyInhibit.ctrl = 0;
    keyStartStop.shift = keyStartStop.alt = keyStartStop.ctrl = 0;
    if (idxCenter > 0 && idxCenter < global_windows_key_sequences.size())
        keyCenter.keycode = global_windows_key_sequences[idxCenter];
    if (idxGameZero > 0 && idxCenter < global_windows_key_sequences.size())
        keyZero.keycode = global_windows_key_sequences[idxGameZero];
    if (idxInhibit > 0 && idxInhibit < global_windows_key_sequences.size())
        keyInhibit.keycode = global_windows_key_sequences[idxInhibit];
    if (idxStartStop > 0 && idxStartStop < global_windows_key_sequences.size())
        keyStartStop.keycode = global_windows_key_sequences[idxStartStop];
    
    keyCenter.shift = iniFile.value("Shift_Center", false).toBool();
    keyCenter.alt = iniFile.value("Alt_Center", false).toBool();
    keyCenter.ctrl = iniFile.value("Ctrl_Center", false).toBool();
    
    keyInhibit.shift = iniFile.value("Shift_Inhibit", false).toBool();
    keyInhibit.alt = iniFile.value("Alt_Inhibit", false).toBool();
    keyInhibit.ctrl = iniFile.value("Ctrl_Inhibit", false).toBool();
    
    keyZero.shift = iniFile.value("Shift_GameZero", false).toBool();
    keyZero.alt = iniFile.value("Alt_GameZero", false).toBool();
    keyZero.ctrl = iniFile.value("Ctrl_GameZero", false).toBool();
    
    keyStartStop.shift = iniFile.value("Shift_StartStop", false).toBool();
    keyStartStop.alt = iniFile.value("Alt_StartStop", false).toBool();
    keyStartStop.ctrl = iniFile.value("Ctrl_StartStop", false).toBool();
#endif
    iniFile.endGroup ();
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void KeyboardShortcutDialog::loadSettings() {
	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "KB_Shortcuts" );
	
    ui.chkCenterShift->setChecked (iniFile.value ( "Shift_Center", 0 ).toBool());
	ui.chkCenterCtrl->setChecked (iniFile.value ( "Ctrl_Center", 0 ).toBool());
	ui.chkCenterAlt->setChecked (iniFile.value ( "Alt_Center", 0 ).toBool());

    ui.cbxCenterKey->setCurrentIndex(iniFile.value("Key_index_Center", 0).toInt());

	ui.chkGameZeroShift->setChecked (iniFile.value ( "Shift_GameZero", 0 ).toBool());
	ui.chkGameZeroCtrl->setChecked (iniFile.value ( "Ctrl_GameZero", 0 ).toBool());
	ui.chkGameZeroAlt->setChecked (iniFile.value ( "Alt_GameZero", 0 ).toBool());
    ui.cbxGameZeroKey->setCurrentIndex(iniFile.value("Key_index_GameZero", 0).toInt());

	ui.chkStartStopShift->setChecked (iniFile.value ( "Shift_StartStop", 0 ).toBool());
	ui.chkStartStopCtrl->setChecked (iniFile.value ( "Ctrl_StartStop", 0 ).toBool());
	ui.chkStartStopAlt->setChecked (iniFile.value ( "Alt_StartStop", 0 ).toBool());
    ui.cbxStartStopKey->setCurrentIndex(iniFile.value("Key_index_StartStop", 0).toInt());

	ui.radioSetZero->setChecked (iniFile.value ( "SetZero", 1 ).toBool());
	ui.radioSetFreeze->setChecked(!ui.radioSetZero->isChecked());

	ui.chkInhibitShift->setChecked (iniFile.value ( "Shift_Inhibit", 0 ).toBool());
	ui.chkInhibitCtrl->setChecked (iniFile.value ( "Ctrl_Inhibit", 0 ).toBool());
	ui.chkInhibitAlt->setChecked (iniFile.value ( "Alt_Inhibit", 0 ).toBool());
    ui.cbxInhibitKey->setCurrentIndex(iniFile.value("Key_index_Inhibit", 0).toInt());

	ui.chkInhibitPitch->setChecked (iniFile.value ( "Inhibit_Pitch", 0 ).toBool());
	ui.chkInhibitYaw->setChecked (iniFile.value ( "Inhibit_Yaw", 0 ).toBool());
	ui.chkInhibitRoll->setChecked (iniFile.value ( "Inhibit_Roll", 0 ).toBool());
	ui.chkInhibitX->setChecked (iniFile.value ( "Inhibit_X", 0 ).toBool());
	ui.chkInhibitY->setChecked (iniFile.value ( "Inhibit_Y", 0 ).toBool());
	ui.chkInhibitZ->setChecked (iniFile.value ( "Inhibit_Z", 0 ).toBool());


	// Reverse Axis
	ui.chkEnableReverseAxis->setChecked (iniFile.value ( "Enable_ReverseAxis", 0 ).toBool());
	ui.spinYawAngle4ReverseAxis->setValue( iniFile.value ( "RA_Yaw", 40 ).toInt() );
	ui.spinZ_Pos4ReverseAxis->setValue( iniFile.value ( "RA_ZPos", -20 ).toInt() );
	ui.spinZ_PosWhenReverseAxis->setValue( iniFile.value ( "RA_ToZPos", 50 ).toInt() );

	iniFile.endGroup ();

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void KeyboardShortcutDialog::save() {

	qDebug() << "save() says: started";

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "KB_Shortcuts" );
    iniFile.setValue ( "Key_index_Center", ui.cbxCenterKey->currentIndex() );
	iniFile.setValue ( "Shift_Center", ui.chkCenterShift->isChecked() );
	iniFile.setValue ( "Ctrl_Center", ui.chkCenterCtrl->isChecked() );
	iniFile.setValue ( "Alt_Center", ui.chkCenterAlt->isChecked() );

    iniFile.setValue ( "Key_index_GameZero", ui.cbxGameZeroKey->currentIndex() );
	iniFile.setValue ( "Shift_GameZero", ui.chkGameZeroShift->isChecked() );
	iniFile.setValue ( "Ctrl_GameZero", ui.chkGameZeroCtrl->isChecked() );
	iniFile.setValue ( "Alt_GameZero", ui.chkGameZeroAlt->isChecked() );

    iniFile.setValue ( "Key_index_StartStop", ui.cbxStartStopKey->currentIndex() );
	iniFile.setValue ( "Shift_StartStop", ui.chkStartStopShift->isChecked() );
	iniFile.setValue ( "Ctrl_StartStop", ui.chkStartStopCtrl->isChecked() );
	iniFile.setValue ( "Alt_StartStop", ui.chkStartStopAlt->isChecked() );
	iniFile.setValue ( "SetZero", ui.radioSetZero->isChecked() );

    iniFile.setValue ( "Key_index_Inhibit", ui.cbxInhibitKey->currentIndex() );
	iniFile.setValue ( "Shift_Inhibit", ui.chkInhibitShift->isChecked() );
	iniFile.setValue ( "Ctrl_Inhibit", ui.chkInhibitCtrl->isChecked() );
	iniFile.setValue ( "Alt_Inhibit", ui.chkInhibitAlt->isChecked() );

	iniFile.setValue ( "Inhibit_Pitch", ui.chkInhibitPitch->isChecked() );
	iniFile.setValue ( "Inhibit_Yaw", ui.chkInhibitYaw->isChecked() );
	iniFile.setValue ( "Inhibit_Roll", ui.chkInhibitRoll->isChecked() );
	iniFile.setValue ( "Inhibit_X", ui.chkInhibitX->isChecked() );
	iniFile.setValue ( "Inhibit_Y", ui.chkInhibitY->isChecked() );
	iniFile.setValue ( "Inhibit_Z", ui.chkInhibitZ->isChecked() );

	// Reverse Axis
	iniFile.setValue ( "Enable_ReverseAxis", ui.chkEnableReverseAxis->isChecked() );
	iniFile.setValue( "RA_Yaw", ui.spinYawAngle4ReverseAxis->value() );
	iniFile.setValue( "RA_ZPos", ui.spinZ_Pos4ReverseAxis->value() );
	iniFile.setValue( "RA_ToZPos", ui.spinZ_PosWhenReverseAxis->value() );

	iniFile.endGroup ();

	settingsDirty = false;

	//
	// Send a message to the main program, to update the Settings (for the tracker)
	//
	mainApp->updateSettings();
}

//**************************************************************************************************//
//**************************************************************************************************//
//
// Constructor for Curve-configuration-dialog
//
CurveConfigurationDialog::CurveConfigurationDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	ui.setupUi( this );

	QPoint offsetpos(120, 30);
	this->move(parent->pos() + offsetpos);

	mainApp = ftnoir;											// Preserve a pointer to FTNoIR

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();

    ui.qFunctionX->setConfig(GlobalPose->X.curvePtr, currentFile);
	connect(ui.qFunctionX, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));

    ui.qFunctionY->setConfig(GlobalPose->Y.curvePtr, currentFile);
	connect(ui.qFunctionY, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));

    ui.qFunctionZ->setConfig(GlobalPose->Z.curvePtr, currentFile);
	connect(ui.qFunctionZ, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));

    ui.qFunctionYaw->setConfig(GlobalPose->Yaw.curvePtr, currentFile);
	connect(ui.qFunctionYaw, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));
	//
	// There are 2 curves for Pitch: Up and Down. Users have indicated that, to be able to use visual Flight controls, it is necessary to have a 'slow' curve for Down...
	//

    ui.qFunctionPitch->setConfig(GlobalPose->Pitch.curvePtr, currentFile);
	connect(ui.qFunctionPitch, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));

    ui.qFunctionPitchDown->setConfig(GlobalPose->Pitch.curvePtrAlt, currentFile);
	connect(ui.qFunctionPitchDown, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));

    ui.qFunctionRoll->setConfig(GlobalPose->Roll.curvePtr, currentFile);
	connect(ui.qFunctionRoll, SIGNAL(CurveChanged(bool)), this, SLOT(curveChanged(bool)));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
CurveConfigurationDialog::~CurveConfigurationDialog() {
	qDebug() << "~CurveConfigurationDialog() says: started";
}

//
// OK clicked on server-dialog
//
void CurveConfigurationDialog::doOK() {
	save();
	this->close();
}

// override show event
void CurveConfigurationDialog::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void CurveConfigurationDialog::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void CurveConfigurationDialog::loadSettings() {
	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Tracking" );
    iniFile.endGroup ();

	ui.qFunctionYaw->loadSettings(currentFile);
	ui.qFunctionPitch->loadSettings(currentFile);
	ui.qFunctionPitchDown->loadSettings(currentFile);
	ui.qFunctionRoll->loadSettings(currentFile);

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void CurveConfigurationDialog::save() {

	qDebug() << "save() says: started";

    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();

    ui.qFunctionYaw->saveSettings(currentFile);
	ui.qFunctionPitch->saveSettings(currentFile);
	ui.qFunctionPitchDown->saveSettings(currentFile);
	ui.qFunctionRoll->saveSettings(currentFile);

	ui.qFunctionX->saveSettings(currentFile);
    ui.qFunctionY->saveSettings(currentFile);
	ui.qFunctionZ->saveSettings(currentFile);

	settingsDirty = false;

	//
	// Send a message to the main program, to update the Settings (for the tracker)
	//
	mainApp->updateSettings();
}

void FaceTrackNoIR::shortcutRecentered()
{
    if (tracker)
    {
#if defined(__WIN32) || defined(_WIN32)
        MessageBeep(MB_OK);
#else
        QApplication::beep();
#endif
        qDebug() << "Center";
        tracker->do_center = true;
    }
}

void FaceTrackNoIR::shortcutZero()
{
    if (tracker)
    {
        tracker->do_game_zero = true;
    }
}

void FaceTrackNoIR::shortcutStartStop()
{
    if (tracker)
    {
        tracker->do_tracking = !tracker->do_tracking;
        qDebug() << "do-tracking" << tracker->do_tracking;
    }
}

void FaceTrackNoIR::shortcutInhibit()
{
    if (tracker)
    {
        tracker->do_inhibit = true;
    }
}
