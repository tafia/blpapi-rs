/* Copyright 2012. Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:  The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef INCLUDED_BLPTHREADUTIL
#define INCLUDED_BLPTHREADUTIL

#ifdef _WIN32
#include <windows.h>
#define SLEEP(s) Sleep((s) * 1000)
#else
#include <pthread.h>
#include <unistd.h>
#define SLEEP(s) sleep(s)
#endif // _WIN32

namespace BloombergLP {

class Mutex
{
   // DATA
#ifdef _WIN32
    CRITICAL_SECTION d_lock;
#else
    pthread_mutex_t d_lock;
#endif

    // NOT IMPLEMENTED
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);

  public:

    // CREATORS
    Mutex();
        // Create a mutex initialized to an unlocked state.

    ~Mutex();
        // Destroy this mutex object.

    // MANIPULATORS
    void lock();
        // Acquire a lock on this mutex object.  If this object is currently
        // locked, then suspend execution of the current thread until a
        // lock can be acquired. Note that the behavior is undefined if the
        // calling thread already owns the lock on this mutex, and will likely
        // result in a deadlock.

    void unlock();
        // Release a lock on this mutex that was previously acquired through a
        // successful call to 'lock'. The behavior is undefined, unless the
        // calling thread currently owns the lock on this mutex.
};

class MutexGuard
{
    // DATA
    Mutex *d_mutex_p;

    // NOT IMPLEMENTED
    MutexGuard(const MutexGuard&);
    MutexGuard& operator=(const MutexGuard&);

public:
    // CREATORS
    MutexGuard(Mutex *mutex);
        // Create a proctor object, place the specified 'mutex' object
        // under management by this proctor, and invoke the 'lock()' method
        // on 'mutex'. Note that the lifetime of 'mutex' must be greater than
        // the lifetime of the newly created guard object.

    ~MutexGuard();
        // Destroy this proctor object and invoke the 'unlock()' method on the
        // mutex object that was passed to the constructor.

    Mutex* release();
        // Return a pointer to the 'mutex' object that was passed to
        // the constructor if it is still under management by this proctor, and
        // release this 'mutex' object from further management by this proctor.

};

#ifdef _WIN32

inline
Mutex::Mutex()
{
    InitializeCriticalSection(&d_lock);
}

inline
Mutex::~Mutex()
{
     DeleteCriticalSection(&d_lock);
}

inline
void Mutex::lock()
{
    EnterCriticalSection(&d_lock);
}

inline
void Mutex::unlock()
{
    LeaveCriticalSection(&d_lock);
}

#else

inline
Mutex::Mutex()
{
    pthread_mutex_init(&d_lock, 0);
}

inline
Mutex::~Mutex()
{
    pthread_mutex_destroy(&d_lock);
}

inline
void Mutex::lock()
{
    pthread_mutex_lock(&d_lock);
}

inline
void Mutex::unlock()
{
    pthread_mutex_unlock(&d_lock);
}

#endif // _WIN32

inline
MutexGuard::MutexGuard(Mutex *mutex)
        : d_mutex_p(mutex)
{
    if (d_mutex_p) {
        d_mutex_p->lock();
    }
}

inline
MutexGuard::~MutexGuard()
{
    if (d_mutex_p) {
        d_mutex_p->unlock();
    }
}

inline
Mutex* MutexGuard::release()
{
    Mutex *mutex_p = d_mutex_p;
    d_mutex_p = 0;
    return mutex_p;
}

} // namespace BloombergLP {

#endif // INCLUDED_BLPTHREADUTIL

