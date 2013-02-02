#include "stdafx.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "headtracker-ftnoir.h"
#include "ftnoir_tracker_ht.h"
#include "ftnoir_tracker_ht_dll.h"
#include "ui_trackercontrols.h"
#include "facetracknoir/global-settings.h"

#define WIDGET_WIDTH 250
#define WIDGET_HEIGHT 188

// delicious copypasta
static QList<QString> get_camera_names(void) {
#if 0
	QList<QString> ret;
	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return ret;
	}
	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) {
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
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
					ret.append(str);
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
	return ret;
 #endif
    return QList<QString>();
}

typedef struct {
	int width;
	int height;
} resolution_tuple;

static resolution_tuple resolution_choices[] = {
	{ 640, 480 },
	{ 320, 240 },
	{ 320, 200 },
	{ 0, 0 }
};

static void load_settings(ht_config_t* config, Tracker* tracker)
{
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );

	iniFile.beginGroup( "HT-Tracker" );
	config->classification_delay = 4000;
	config->field_of_view = iniFile.value("fov", 69).toFloat();
	config->pyrlk_pyramids = 3;
	config->pyrlk_win_size_w = config->pyrlk_win_size_h = 21;
	config->max_keypoints = 250;
	config->keypoint_quality = 12;
	config->keypoint_distance = 2.3f;
	config->keypoint_3distance = 6;
	//config->force_width = 640;
	//config->force_height = 480;
	config->force_fps = iniFile.value("fps", 0).toInt();
	config->camera_index = iniFile.value("camera-index", -1).toInt();
	config->ransac_num_iters = 100;
    config->ransac_max_reprojection_error = 6.0f;
    config->ransac_max_inlier_error = 6.0f;
    config->ransac_max_mean_error = 5.5f;
    config->ransac_abs_max_mean_error = 11.0f;
	config->debug = 0;
	config->ransac_min_features = 0.75f;
    int res = iniFile.value("resolution", 0).toInt() - 1;
    if (res < 0 || res >= (int)(sizeof(*resolution_choices) / sizeof(resolution_tuple)))
		res = 0;
	resolution_tuple r = resolution_choices[res];
	config->force_width = r.width;
	config->force_height = r.height;
	if (tracker)
	{
		tracker->enableRX = iniFile.value("enable-rx", true).toBool();
		tracker->enableRY = iniFile.value("enable-ry", true).toBool();
		tracker->enableRZ = iniFile.value("enable-rz", true).toBool();
		tracker->enableTX = iniFile.value("enable-tx", true).toBool();
		tracker->enableTY = iniFile.value("enable-ty", true).toBool();
		tracker->enableTZ = iniFile.value("enable-tz", true).toBool();
	}
	iniFile.endGroup();
}

Tracker::Tracker() : lck_shm(HT_SHM_NAME, HT_MUTEX_NAME, sizeof(ht_shm_t))
{
	videoWidget = NULL;
	layout = NULL;
	enableRX = enableRY = enableRZ = enableTX = enableTY = enableTZ = true;
    shm = (ht_shm_t*) lck_shm.mem;
	load_settings(&shm->config, this);
}

Tracker::~Tracker()
{
    subprocess.kill();
    shm->terminate = true;
	if (layout)
		delete layout;
	if (videoWidget)
		delete videoWidget;
}

void Tracker::StartTracker(QFrame* videoframe)
{
    videoframe->setAttribute(Qt::WA_NativeWindow);
    videoframe->show();
    videoWidget = new VideoWidget(videoframe);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(videoWidget);
    if (videoframe->layout())
        delete videoframe->layout();
    videoframe->setLayout(layout);
    videoWidget->resize(WIDGET_WIDTH, WIDGET_HEIGHT);
    videoWidget->show();
    this->layout = layout;
    load_settings(&shm->config, this);
    shm->frame.channels = shm->frame.width = shm->frame.height = 0;
    shm->pause = shm->terminate = shm->running = false;
    shm->timer = 0;
    subprocess.setWorkingDirectory(QCoreApplication::applicationDirPath() + "/tracker-ht");
#if defined(_WIN32) || defined(__WIN32)
    subprocess.start("\"" + QCoreApplication::applicationDirPath() + "/tracker-ht/headtracker-ftnoir" + "\"");
#else
    subprocess.start(QCoreApplication::applicationDirPath() + "/tracker-ht/headtracker-ftnoir");
#endif
}

bool Tracker::GiveHeadPoseData(THeadPoseData* data)
{
	bool ret = false;

    lck_shm.lock();
    shm->timer = 0;
    if (shm->frame.width > 0)
    {
        //memcpy(foo, shm->frame.frame, shm->frame.width * shm->frame.height * 3);
        videoWidget->update(shm->frame.frame, shm->frame.width, shm->frame.height);
    }
    if (shm->result.filled) {
        if (enableRX)
            data->yaw = shm->result.rotx * 57.295781;
        if (enableRY)
            data->pitch = shm->result.roty * 57.295781;
        if (enableRZ)
            data->roll = shm->result.rotz * 57.295781;
        if (enableTX)
            data->x = shm->result.tx;
        if (enableTY)
            data->y = shm->result.ty;
        if (enableTZ)
            data->z = shm->result.tz;
        ret = true;
    }
    lck_shm.unlock();

	return ret;
}

//-----------------------------------------------------------------------------
void TrackerDll::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = "HT 0.7";
}

void TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = "HT";
}

void TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = "";
}

void TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/ht.ico");
}


//-----------------------------------------------------------------------------
//#pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
	return new TrackerDll;
}

//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT void* CALLING_CONVENTION GetConstructor()
{
    return (ITracker*) new Tracker;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT void* CALLING_CONVENTION GetDialog( )
{
    return (ITrackerDialog*) new TrackerControls;
}

TrackerControls::TrackerControls()
{
	ui.setupUi(this);
	loadSettings();
	connect(ui.cameraName, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.cameraFPS, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.cameraFOV, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.rx, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.ry, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.rz, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.tx, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.ty, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.tz, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.buttonCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.buttonOK, SIGNAL(clicked()), this, SLOT(doOK()));
	settingsDirty = false;
}

TrackerControls::~TrackerControls()
{
}

void TrackerControls::showEvent(QShowEvent *event)
{
}

void TrackerControls::Initialize(QWidget* parent)
{
	show();
}

void TrackerControls::loadSettings()
{
	ui.cameraName->clear();
	QList<QString> names = get_camera_names();
	names.prepend("Any available");
	ui.cameraName->addItems(names);
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );
	iniFile.beginGroup( "HT-Tracker" );
	ui.cameraName->setCurrentIndex(iniFile.value("camera-index", -1).toInt() + 1);
	ui.cameraFOV->setValue(iniFile.value("fov", 69).toFloat());
	int fps;
	switch (iniFile.value("fps", 0).toInt())
	{
	default:
	case 0:
		fps = 0;
		break;
	case 30:
		fps = 1;
		break;
	case 60:
		fps = 2;
		break;
	case 120:
		fps = 3;
		break;
	}
	ui.cameraFPS->setCurrentIndex(fps);
	ui.rx->setCheckState(iniFile.value("enable-rx", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.ry->setCheckState(iniFile.value("enable-ry", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.rz->setCheckState(iniFile.value("enable-rz", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.tx->setCheckState(iniFile.value("enable-tx", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.ty->setCheckState(iniFile.value("enable-ty", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.tz->setCheckState(iniFile.value("enable-tz", true).toBool() ? Qt::Checked : Qt::Unchecked);
    ui.resolution->setCurrentIndex(iniFile.value("resolution", 0).toInt());
	iniFile.endGroup();
	settingsDirty = false;
}

void TrackerControls::save()
{
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );

	iniFile.beginGroup( "HT-Tracker" );
	iniFile.setValue("fov", ui.cameraFOV->value());
	int fps;
	switch (ui.cameraFPS->currentIndex())
	{
	case 0:
	default:
		fps = 0;
		break;
	case 1:
		fps = 30;
		break;
	case 2:
		fps = 60;
		break;
	case 3:
		fps = 120;
		break;
	}
	iniFile.setValue("fps", fps);
	iniFile.setValue("camera-index", ui.cameraName->currentIndex() - 1);
	iniFile.setValue("enable-rx", ui.rx->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-ry", ui.ry->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-rz", ui.rz->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-tx", ui.tx->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-ty", ui.ty->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-tz", ui.tz->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("resolution", ui.resolution->currentIndex());
	iniFile.endGroup();
	settingsDirty = false;
}

void TrackerControls::doOK()
{
	save();
	this->close();
}

void TrackerControls::doCancel()
{
	if (settingsDirty) {
		int ret = QMessageBox::question ( this,
										  "Settings have changed",
										  "Do you want to save the settings?",
										  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
										  QMessageBox::Discard );

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
