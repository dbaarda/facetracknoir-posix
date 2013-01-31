/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "compat.h"
#include <exception>

#if defined(_WIN32) || defined(__WIN32)

PortableLockedShm::PortableLockedShm(const char* shmName, const char* mutexName, int mapSize)
{
    hMutex = CreateMutexA(NULL, false, mutexName);
    if (!hMutex)
        throw std::exception();
    hMapFile = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,
                 NULL,
                 PAGE_READWRITE,
                 0,
                 mapSize,
                 shmName);
    if (!hMapFile)
        throw std::exception();
    mem = MapViewOfFile(hMapFile,
                        FILE_MAP_READ | FILE_MAP_WRITE,
                        0,
                        0,
                        mapSize);
    if (!mem)
        throw std::exception();
}

PortableLockedShm::~PortableLockedShm()
{
    UnmapViewOfFile(mem);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);
}

void PortableLockedShm::lock()
{
    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
        throw std::exception();
}

void PortableLockedShm::unlock()
{
    (void) ReleaseMutex(hMutex);
}

#else
PortableLockedShm::PortableLockedShm(const char *shmName, const char *mutexName, int mapSize) : size(mapSize)
{
    char shm_filename[NAME_MAX];
    shm_filename[0] = '/';
    strncpy(shm_filename+1, shmName, NAME_MAX-2);
    shm_filename[NAME_MAX-1] = '\0';

    //(void) shm_unlink(shm_filename);

    fd = shm_open(shm_filename, O_RDWR | O_CREAT, 0600);

    if (fd < 0)
        throw std::exception();

    if (ftruncate(fd, mapSize) < 0)
        std::exception();

    mem = mmap(NULL, mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)0);

    if (mem == (void *)-1)
        throw std::exception();
}

PortableLockedShm::~PortableLockedShm()
{
    //(void) shm_unlink(shm_filename);

    (void) munmap(mem, size);
    (void) close(fd);
}

void PortableLockedShm::lock()
{
    if (flock(fd, LOCK_EX))
        throw std::exception();
}

void PortableLockedShm::unlock()
{
    if (flock(fd, LOCK_UN))
        throw std::exception();
}



#endif
