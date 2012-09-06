#ifndef VOS_H_INCLUDED
#define VOS_H_INCLUDED

// threads
void *thread_create (void *proc, void *arg); // creates a new thread (and returns it handler), or return NULL (on ERROR)
//void  thread_suspend(void *thread); // win-only?
//void  thread_resume (void *thread); // win-only?
void  thread_destory(void *thread);

// mutex [ reentrant mutex ]
void *mutex_create ();
void  mutex_lock   (void *mutex);
void  mutex_unlock (void *mutex);
void  mutex_destroy(void * mutex);

// time & timers
void msleep(int msec); // msec { usleep(1000*msec); }
unsigned long os_ticks(); //GetTickCount()


// exec programs - to get out in a buffer
int os_exec(char *cmd, int (*on_out)(), void *arg); // on_out ret 0 = ok, continue

#endif // VOS_H_INCLUDED
