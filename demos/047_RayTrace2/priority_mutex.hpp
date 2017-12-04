//! Class declaration: priority_mutex

#ifndef TRIPLEMUTEX_H
#define TRIPLEMUTEX_H

#include <mutex>

namespace Dg {

class priority_mutex
{
  public:

    //! The calling thread locks the mutex and is given a high priority.
    void high_lock();

    //! The calling thread attempts locks the mutex, and is given a high priority.
    //! @return false if the mutex is already locked.
    bool high_try_lock();

    //! The high priority calling thread unlocks the mutex. The thread should have previously
    //! locked the mutex with high_lock().
    void high_unlock();

    //! The calling thread locks the mutex and is given a low priority.
    void low_lock();

    //! The calling thread attempts locks the mutex, and is given a low priority.
    //! @return false if the mutex is already locked.
    bool low_try_lock();

    //! The low priority calling thread unlocks the mutex. The thread should have previously
    //! locked the mutex with low_lock().
    void low_unlock();

  private:

    std::mutex L, M, N;

};

} // namespace Dg


#endif