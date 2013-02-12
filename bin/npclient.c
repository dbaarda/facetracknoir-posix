/*
 * NPClient.dll
 *
 */

#define NP_DECLSPEC __declspec(dllexport)

#define NP_EXPORT(t, n, ...) t __declspec(dllexport) __stdcall n(__VA_ARGS__)

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

#if 0
static FILE *debug_stream = fopen("c:\\NPClient.log", "a");
#define dbg_report(...) if (debug_stream) { fprintf(debug_stream, __VA_ARGS__); fflush(debug_stream); }
#else
#define dbg_report(...)
#endif

typedef struct {
	unsigned short id;
	unsigned char table[8];
} tir_keys_t;

static tir_keys_t keys[] = {
	{ 9601, { 42, 10, 86, 177, 146, 101, 39, 115 } },
	{ 11001, { 139, 166, 84, 143, 2, 93, 242, 113 } },
	{ 12601, { 233, 43, 154, 154, 144, 187, 82, 114 } },
	{ 5502, { 112, 26, 16, 126, 227, 36, 133, 143 } },
	{ 15002, { 249, 212, 54, 89, 231, 12, 206, 61 } },
	{ 1003, { 95, 209, 220, 182, 101, 47, 87, 90 } },
	{ 1526, { 56, 4, 1, 44, 68, 196, 209, 95 } },
	{ 9602, { 211, 154, 115, 28, 51, 212, 127, 129 } },
	{ 8106, { 14, 120, 40, 51, 97, 103, 34, 19 } },
	{ 7502, { 179, 22, 54, 235, 185, 5, 79, 164 } },
	{ 1525, { 220, 130, 247, 223, 194, 39, 234, 206 } },
	{ 9002, { 198, 121, 137, 40, 13, 170, 13, 91 } },
	{ 1303, { 160, 202, 247, 167, 167, 236, 20, 8 } },
	{ 1304, { 0, 119, 237, 169, 222, 144, 65, 129 } },
	{ 1305, { 25, 25, 161, 11, 237, 26, 112, 91 } },
	{ 2307, { 149, 84, 188, 175, 21, 197, 57, 149 } },
	{ 1575, { 220, 77, 77, 189, 136, 129, 182, 107 } },
	{ 1004, { 232, 72, 77, 170, 170, 251, 102, 89 } },
	{ 14203, { 188, 6, 50, 63, 183, 43, 37, 234 } },
	{ 12602, { 16, 18, 249, 33, 115, 153, 205, 86 } },
	{ 8107, { 55, 197, 21, 120, 51, 199, 145, 148 } },
	{ 2310, { 30, 178, 132, 199, 68, 152, 33, 75 } },
	{ 2375, { 109, 168, 243, 191, 192, 36, 218, 52 } },
	{ 7402, { 109, 168, 243, 191, 192, 36, 218, 52 } },
	{ 7403, { 215, 152, 81, 23, 150, 232, 232, 75 } },
	{ 1005, { 121, 210, 176, 175, 34, 247, 166, 243 } },
	{ 1006, { 155, 252, 136, 246, 143, 134, 86, 5 } },
	{ 1007, { 82, 131, 117, 27, 111, 64, 18, 28 } },
	{ 8108, { 237, 22, 160, 171, 111, 223, 251, 45 } },
	{ 2825, { 235, 226, 239, 152, 19, 225, 232, 110 } },
	{ 2775, { 186, 235, 241, 213, 70, 113, 28, 216 } },
	{ 2950, { 115, 184, 35, 231, 215, 135, 252, 215 } },
	{ 7503, { 171, 17, 148, 182, 153, 179, 226, 212 } },
	{ 7504, { 12, 180, 60, 81, 106, 185, 35, 6 } },
	{ 8109, { 116, 205, 94, 38, 41, 161, 62, 137 } },
	{ 0, { 0, 0, 0, 0, 0, 0, 0, 0 } }
};

static unsigned char *table = NULL;

#define TIRSHMNAM "FT_SharedMem"
#define TIRMTXNAM "FT_Mutext"

typedef struct tir_data{
  short status;
  short frame;
  unsigned int cksum;
  float roll, pitch, yaw;
  float tx, ty, tz;
  float padding[9];
} tir_data_t;

typedef struct tir_signature{
    char DllSignature[200];
    char AppSignature[200];
} tir_signature_t;

typedef struct TFreeTrackData {
	int DataID;
	int CamWidth;
    int CamHeight;
    // virtual pose
    float Yaw;   // positive yaw to the left
    float Pitch; // positive pitch up
    float Roll;  // positive roll to the left
    float X;
    float Y;
    float Z;
    // raw pose with no smoothing, sensitivity, response curve etc. 
    float RawYaw;
    float RawPitch;
    float RawRoll;
    float RawX;
    float RawY;
    float RawZ;
    // raw points, sorted by Y, origin top left corner
    float X1;
    float Y1;
    float X2;
    float Y2;
    float X3;
    float Y3;
    float X4;
    float Y4;
} TIRSHM;

static TCHAR tirshmtch[] = TEXT(TIRSHMNAM);
static TCHAR tirmtxtch[] = TEXT(TIRMTXNAM);

static TIRSHM *tirshm = NULL;
static HANDLE tirmtx = NULL, tirmap = NULL;

static void ftir_ensure_mapped(HANDLE *tirmap, TIRSHM **tirshm, HANDLE *tirmtx) {
	if (!*tirmtx)
		*tirmtx = CreateMutex(NULL, 0, tirmtxtch);
	if (!*tirmap)
		*tirmap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TIRSHM), tirshmtch);
	if (!*tirmap)
		return;
	if (!*tirshm)
		*tirshm = (TIRSHM *) MapViewOfFile(*tirmap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(TIRSHM));
}

static void game_data_get_desc(int id) {
	table = NULL;
	int i;
	for (i = 0; keys[i].id; i++) {
		if (keys[i].id == id) {
			table = keys[i].table;
			break;
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	dbg_report("DllMain (0x%p, %d, %p)\n", hinstDLL, fdwReason, lpvReserved);
    switch (fdwReason)
    {
	case DLL_PROCESS_ATTACH: {
            dbg_report("Attach request\n");
            DisableThreadLibraryCalls(hinstDLL);
            break;
	}
		case DLL_PROCESS_DETACH:
            //ltr_shutdown();
	    if (tirmtx)
	    	CloseHandle(tirmtx);
	    if (tirshm)
	    	UnmapViewOfFile(tirshm);
	    if (tirmap)
		CloseHandle(tirmap);
	    tirmtx = NULL;
	    tirmap = NULL;
	    tirshm = NULL;
	    dbg_report("Detach\n");
            break;
    }
    return TRUE;
}
/******************************************************************
 *		NPPriv_ClientNotify (NPCLIENT.1)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_ClientNotify, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}
/******************************************************************
 *		NPPriv_GetLastError (NPCLIENT.2)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_GetLastError, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}
/******************************************************************
 *		NPPriv_SetData (NPCLIENT.3)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_SetData, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}
/******************************************************************
 *		NPPriv_SetLastError (NPCLIENT.4)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_SetLastError, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}
/******************************************************************
 *		NPPriv_SetParameter (NPCLIENT.5)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_SetParameter, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}
/******************************************************************
 *		NPPriv_SetSignature (NPCLIENT.6)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_SetSignature, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}
/******************************************************************
 *		NPPriv_SetVersion (NPCLIENT.7)
 *
 *
 */
//

NP_EXPORT(int, NPPriv_SetVersion, )
{
	dbg_report("stub\n");
	/* @stub in .spec */
	return 0;
}

static float limit_num(float min, float val, float max)
{
  if(val < min) return min;
  if(val > max) return max;
  return val;
}

static unsigned int cksum(unsigned char buf[], unsigned int size)
{
  if((size == 0) || (buf == NULL)){
    return 0;
  }
  int rounds = size >> 2;
  int rem = size % 4;

  int c = size;
  int a0, a2;

  while(rounds != 0){
    a0 = *(short int*)buf;
    a2 = *(short int*)(buf+2);
    buf += 4;
    c += a0;
    a2 ^= (c << 5);
    a2 <<= 11;
    c ^= a2;
    c += (c >> 11);
    --rounds;
  }
  switch(rem){
    case 3:
        a0 = *(short int*)buf;
        a2 = *(signed char*)(buf+2);
        c += a0;
        a2 = (a2 << 2) ^ c;
        c ^= (a2 << 16);
        a2 = (c >> 11);
      break;
    case 2:
        a2 = *(short int*)buf;
        c += a2;
        c ^= (c << 11);
        a2 = (c >> 17);
      break;
    case 1:
        a2 = *(signed char*)(buf);
        c += a2;
        c ^= (c << 10);
        a2 = (c >> 1);
      break;
    default:
      break;
  }
  if(rem != 0){
    c+=a2;
  }

  c ^= (c << 3);
  c += (c >> 5);
  c ^= (c << 4);
  c += (c >> 17);
  c ^= (c << 25);
  c += (c >> 6);

  return (unsigned int)c;
}

static void enhance(unsigned char buf[], unsigned int size,
             unsigned char table[], unsigned int table_size)
{
  unsigned int table_ptr = 0;
  unsigned char var = 0x88;
  unsigned char tmp;
  if((size <= 0) || (table_size <= 0) ||
     (buf == NULL) || (table == NULL))
     return;
  do{
    tmp = buf[--size];
    buf[size] = tmp ^ table[table_ptr] ^ var;
    var += size + tmp;
    table_ptr = (++table_ptr) % table_size;
  }while(size != 0);
}


/******************************************************************
 *		NP_GetData (NPCLIENT.8)
 *
 *
 */
//

NP_EXPORT(int, NP_GetData, tir_data_t * data)
{
  static int frame = 0;
  //dbg_report("NP_GetData\n");
  ftir_ensure_mapped(&tirmap, &tirshm, &tirmtx);
  float r = 0, p = 0, y = 0, tx = 0, ty = 0, tz = 0;
  int res = 0;
  int tracking = 0;
  if (tirmtx && WaitForSingleObject(tirmtx, 5) == WAIT_OBJECT_0) {
	if (tirshm) {
		y = tirshm->Yaw;
		p = tirshm->Pitch;
		r = tirshm->Roll;
		tx = tirshm->X;
		ty = tirshm->Y;
		tz = tirshm->Z;
		tracking = tirshm->CamHeight && tirshm->CamWidth && tirshm->DataID;
	}
	ReleaseMutex(tirmtx);
  }
  //dbg_report("POSE %d; %f %f %f %f %f %f\n", tracking, y, p, r, tx, ty, tz);
  data->status = !tracking;
  data->frame = tracking ? (((frame++) & 1) ? 1 : 0) : 0;
  data->cksum = 0;
  data->roll = r * 8191;
  data->pitch = p * 8191;
  data->yaw = y * 8191;
  data->tx = limit_num(-16383.0f, 100 * tx, 16383);
  data->ty = limit_num(-16383.0f, 100 * ty, 16383);
  data->tz = limit_num(-16383.0f, 100 * tz, 16383);
  int i;
  for(i = 0; i < 9; ++i){
    data->padding[i] = 0.0;
  } 
  data->cksum = cksum((unsigned char*)data, sizeof(tir_data_t));
  if(table){
    enhance((unsigned char*)data, sizeof(tir_data_t), table, sizeof(table));
  }
  return 0;
}
/******************************************************************
 *		NP_GetParameter (NPCLIENT.9)
 *
 *
 */
//

NP_EXPORT(int, NP_GetParameter, int arg0, int arg1)
{
    //dbg_report("GetParameter request: %d %d\n", arg0, arg1);
	return (int) 0;
}

/******************************************************************
 *		NP_GetSignature (NPCLIENT.10)
 *
 *
 */

static volatile unsigned char part2_2[200] = {
	0xe3, 0xe5, 0x8e, 0xe8, 0x06, 0xd4, 0xab,
	0xcf, 0xfa, 0x51, 0xa6, 0x84, 0x69, 0x52,
	0x21, 0xde, 0x6b, 0x71, 0xe6, 0xac, 0xaa,
	0x16, 0xfc, 0x89, 0xd6, 0xac, 0xe7, 0xf8,
	0x7c, 0x09, 0x6a, 0x8b, 0x8b, 0x64, 0x0b,
	0x7c, 0xc3, 0x61, 0x7f, 0xc2, 0x97, 0xd3,
	0x33, 0xd9, 0x99, 0x59, 0xbe, 0xed, 0xdc,
	0x2c, 0x5d, 0x93, 0x5c, 0xd4, 0xdd, 0xdf,
	0x8b, 0xd5, 0x1d, 0x46, 0x95, 0xbd, 0x10,
	0x5a, 0xa9, 0xd1, 0x9f, 0x71, 0x70, 0xd3,
	0x94, 0x3c, 0x71, 0x5d, 0x53, 0x1c, 0x52,
	0xe4, 0xc0, 0xf1, 0x7f, 0x87, 0xd0, 0x70,
	0xa4, 0x04, 0x07, 0x05, 0x69, 0x2a, 0x16,
	0x15, 0x55, 0x85, 0xa6, 0x30, 0xc8, 0xb6,
	0x00
};


static volatile unsigned char part1_2[200] = {
	0x6d, 0x0b, 0xab, 0x56, 0x74, 0xe6, 0x1c,
	0xff, 0x24, 0xe8, 0x34, 0x8f, 0x00, 0x63,
	0xed, 0x47, 0x5d, 0x9b, 0xe1, 0xe0, 0x1d,
	0x02, 0x31, 0x22, 0x89, 0xac, 0x1f, 0xc0,
	0xbd, 0x29, 0x13, 0x23, 0x3e, 0x98, 0xdd,
	0xd0, 0x2a, 0x98, 0x7d, 0x29, 0xff, 0x2a,
	0x7a, 0x86, 0x6c, 0x39, 0x22, 0x3b, 0x86,
	0x86, 0xfa, 0x78, 0x31, 0xc3, 0x54, 0xa4,
	0x78, 0xaa, 0xc3, 0xca, 0x77, 0x32, 0xd3,
	0x67, 0xbd, 0x94, 0x9d, 0x7e, 0x6d, 0x31,
	0x6b, 0xa1, 0xc3, 0x14, 0x8c, 0x17, 0xb5,
	0x64, 0x51, 0x5b, 0x79, 0x51, 0xa8, 0xcf,
	0x5d, 0x1a, 0xb4, 0x84, 0x9c, 0x29, 0xf0,
	0xe6, 0x69, 0x73, 0x66, 0x0e, 0x4b, 0x3c,
	0x7d, 0x99, 0x8b, 0x4e, 0x7d, 0xaf, 0x86,
	0x92, 0xff
};

static volatile unsigned char part2_1[200] = {
	0x8b, 0x84, 0xfc, 0x8c, 0x71, 0xb5, 0xd9,
	0xaa, 0xda, 0x32, 0xc7, 0xe9, 0x0c, 0x20,
	0x40, 0xd4, 0x4b, 0x02, 0x89, 0xca, 0xde,
	0x61, 0x9d, 0xfb, 0xb3, 0x8c, 0x97, 0x8a,
	0x13, 0x6a, 0x0f, 0xf8, 0xf8, 0x0d, 0x65,
	0x1b, 0xe3, 0x05, 0x1e, 0xb6, 0xf6, 0xd9,
	0x13, 0xad, 0xeb, 0x38, 0xdd, 0x86, 0xfc,
	0x59, 0x2e, 0xf6, 0x2e, 0xf4, 0xb0, 0xb0,
	0xfd, 0xb0, 0x70, 0x23, 0xfb, 0xc9, 0x1a,
	0x50, 0x89, 0x92, 0xf0, 0x01, 0x09, 0xa1,
	0xfd, 0x5b, 0x19, 0x29, 0x73, 0x59, 0x2b,
	0x81, 0x83, 0x9e, 0x11, 0xf3, 0xa2, 0x1f,
	0xc8, 0x24, 0x53, 0x60, 0x0a, 0x42, 0x78,
	0x7a, 0x39, 0xea, 0xc1, 0x59, 0xad, 0xc5,
	0x00
};

static volatile unsigned char part1_1[200] = {
	0x1d, 0x79, 0xce, 0x35, 0x1d, 0x95, 0x79,
	0xdf, 0x4c, 0x8d, 0x55, 0xeb, 0x20, 0x17,
	0x9f, 0x26, 0x3e, 0xf0, 0x88, 0x8e, 0x7a,
	0x08, 0x11, 0x52, 0xfc, 0xd8, 0x3f, 0xb9,
	0xd2, 0x5c, 0x61, 0x03, 0x56, 0xfd, 0xbc,
	0xb4, 0x0a, 0xf1, 0x13, 0x5d, 0x90, 0x0a,
	0x0e, 0xee, 0x09, 0x19, 0x45, 0x5a, 0xeb,
	0xe3, 0xf0, 0x58, 0x5f, 0xac, 0x23, 0x84,
	0x1f, 0xc5, 0xe3, 0xa6, 0x18, 0x5d, 0xb8,
	0x47, 0xdc, 0xe6, 0xf2, 0x0b, 0x03, 0x55,
	0x61, 0xab, 0xe3, 0x57, 0xe3, 0x67, 0xcc,
	0x16, 0x38, 0x3c, 0x11, 0x25, 0x88, 0x8a,
	0x24, 0x7f, 0xf7, 0xeb, 0xf2, 0x5d, 0x82,
	0x89, 0x05, 0x53, 0x32, 0x6b, 0x28, 0x54,
	0x13, 0xf6, 0xe7, 0x21, 0x1a, 0xc6, 0xe3,
	0xe1, 0xff
};

//

NP_EXPORT(int, NP_GetSignature, tir_signature_t * sig)
{
  dbg_report("GetSignature request\n");
  int i;

  for (i = 0; i < 200; i++)
	  sig->DllSignature[i] = part1_2[i] ^ part1_1[i];
  for (i = 0; i < 200; i++)
	  sig->AppSignature[i] = part2_1[i] ^ part2_2[i];
  memset(sig->DllSignature + strlen(sig->DllSignature), 0, 200 - strlen(sig->DllSignature));
  memset(sig->AppSignature + strlen(sig->AppSignature), 0, 200 - strlen(sig->AppSignature));
  return 0;
}
/******************************************************************
 *		NP_QueryVersion (NPCLIENT.11)
 *
 *
 */
//

NP_EXPORT(int, NP_QueryVersion, unsigned short * version)
{
    dbg_report("QueryVersion request\n");
	*version=0x0500;
	return 0;
}
/******************************************************************
 *		NP_ReCenter (NPCLIENT.12)
 *
 *
 */
//

NP_EXPORT(int, NP_ReCenter, void)
{
  dbg_report("ReCenter request\n");
  return 0;
}

/******************************************************************
 *		NP_RegisterProgramProfileID (NPCLIENT.13)
 *
 *
 */
//

NP_EXPORT(int, NP_RegisterProgramProfileID, unsigned short id)
{
  ftir_ensure_mapped(&tirmap, &tirshm, &tirmtx);
  dbg_report("RegisterProgramProfileID request: %d\n", id);
  game_data_get_desc(id);
  dbg_report("Table: %02X %02X %02X %02X %02X %02X %02X %02X\n", table[0],table[1],table[2],table[3],table[4], table[5], table[6], table[7]);
  return 0;
}
/******************************************************************
 *		NP_RegisterWindowHandle (NPCLIENT.14)
 *
 *
 */
//

NP_EXPORT(int, NP_RegisterWindowHandle, HWND hwnd)
{
    dbg_report("RegisterWindowHandle request: 0x%X\n", hwnd);
	return (int) 0;
}
/******************************************************************
 *		NP_RequestData (NPCLIENT.15)
 *
 *
 */
//

NP_EXPORT(int, NP_RequestData, unsigned short req)
{
    dbg_report("RequestData request: %d\n", req);
	return (int) 0;
}
/******************************************************************
 *		NP_SetParameter (NPCLIENT.16)
 *
 *
 */
//

NP_EXPORT(int, NP_SetParameter, int arg0, int arg1)
{
    dbg_report("SetParameter request: %d %d\n", arg0, arg1);
	return (int) 0;
}
/******************************************************************
 *		NP_StartCursor (NPCLIENT.17)
 *
 *
 */
//

NP_EXPORT(int, NP_StartCursor, void)
{
    dbg_report("StartCursor request\n");
	return (int) 0;
}
/******************************************************************
 *		NP_StartDataTransmission (NPCLIENT.18)
 *
 *
 */
//

NP_EXPORT(int, NP_StartDataTransmission, void)
{
  ftir_ensure_mapped(&tirmap, &tirshm, &tirmtx);
  dbg_report("StartDataTransmission request %p %p %p\n", tirmap, tirshm, tirmtx);
  return 0;
}
/******************************************************************
 *		NP_StopCursor (NPCLIENT.19)
 *
 *
 */
//

NP_EXPORT(int, NP_StopCursor, void)
{
    dbg_report("StopCursor request\n");
	return (int) 0;
}
/******************************************************************
 *		NP_StopDataTransmission (NPCLIENT.20)
 *
 *
 */
//

NP_EXPORT(int, NP_StopDataTransmission, void)
{
  return 0;
}
/******************************************************************
 *		NP_UnregisterWindowHandle (NPCLIENT.21)
 *
 *
 */
//

NP_EXPORT(int, NP_UnregisterWindowHandle, void)
{
    dbg_report("UnregisterWindowHandle request\n");
	return (int) 0;
}
