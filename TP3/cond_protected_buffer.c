#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "circular_buffer.h"
#include "protected_buffer.h"
#include "utils.h"

// Initialise the protected buffer structure above.
protected_buffer_t * cond_protected_buffer_init(int length) {
  protected_buffer_t * b;
  b = (protected_buffer_t *)malloc(sizeof(protected_buffer_t));
  b->buffer = circular_buffer_init(length);
  // Initialize the synchronization components
  pthread_mutex_init(&(b->mutex), NULL);
  pthread_cond_init(&(b->condEmpty), NULL);
  pthread_cond_init(&(b->condFull), NULL);
  return b;
}

// Extract an element from buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void * cond_protected_buffer_get(protected_buffer_t * b){
  void * d;

  // Enter mutual exclusion
  pthread_mutex_lock(&(b->mutex));
  // Wait until there is a full slot to get data from the unprotected
  // circular buffer (circular_buffer_get).
  while ((d = circular_buffer_get(b->buffer)) == NULL){
    pthread_cond_wait(&(b->condFull), &(b->mutex));
  }
  // Signal or broadcast that an empty slot is available in the
  // unprotected circular buffer (if needed)
  pthread_cond_broadcast(&(b->condEmpty));

  print_task_activity ("get", d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&(b->mutex));
  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, the method call blocks until it is.
void cond_protected_buffer_put(protected_buffer_t * b, void * d){

  // Enter mutual exclusionss
  pthread_mutex_lock(&(b->mutex));
  // Wait until there is an empty slot to put data in the unprotected
  // circular buffer (circular_buffer_put).
  while (circular_buffer_put(b->buffer, d) == 0) {
    pthread_cond_wait(&(b->condEmpty), &(b->mutex));
  }
  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed)
  pthread_cond_broadcast(&(b->condFull));

  circular_buffer_put(b->buffer, d);
  print_task_activity ("put", d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&(b->mutex));

}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, return NULL. Otherwise, return the element.
void * cond_protected_buffer_remove(protected_buffer_t * b){
  void * d;

  pthread_mutex_lock(&(b->mutex));
  d = circular_buffer_get(b->buffer);
  if (d != NULL) {
    pthread_cond_broadcast(&(b->condEmpty));
  }

  // Signal or broadcast that an empty slot is available in the
  // unprotected circular buffer (if needed)

  print_task_activity ("remove", d);

  pthread_mutex_unlock(&(b->mutex));
  return d;
}

// Insert an element into buffer. If the attempted operation is
// not possible immedidately, return 0. Otherwise, return 1.
int cond_protected_buffer_add(protected_buffer_t * b, void * d){
  int done;

  // Enter mutual exclusion
  pthread_mutex_lock(&(b->mutex));
  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed)
  done = circular_buffer_put(b->buffer, d);

  if (!done) d = NULL;

  pthread_cond_broadcast(&(b->condFull));

  print_task_activity ("add", d);

  pthread_mutex_unlock(&(b->mutex));
  // Leave mutual exclusion
  return done;
}

// Extract an element from buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return the element if
// successful. Otherwise, return NULL.
void * cond_protected_buffer_poll(protected_buffer_t * b, struct timespec *abstime){
  void * d = NULL;
  int    rc = 0;

  // Enter mutual exclusion
  pthread_mutex_lock(&(b->mutex));
  // Wait until there is an empty slot to put data in the unprotected
  // circular buffer (circular_buffer_put) but waits no longer than
  // the given timeout.

  while ((d = circular_buffer_get(b->buffer)) == NULL){
    rc = pthread_cond_timedwait(&(b->condEmpty), &(b->mutex),abstime);
    if (rc == ETIMEDOUT)break;
  }
  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed)
  if (rc != ETIMEDOUT) {
    pthread_cond_broadcast(&(b->condEmpty));
  }
  print_task_activity ("poll", d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&(b->mutex));
  return d;
}

// Insert an element into buffer. If the attempted operation is not
// possible immedidately, the method call blocks until it is, but
// waits no longer than the given timeout. Return 0 if not
// successful. Otherwise, return 1.
int cond_protected_buffer_offer(protected_buffer_t * b, void * d, struct timespec * abstime){
  int rc = 0;
  int done = 0;

  // Enter mutual exclusion
  pthread_mutex_lock(&(b->mutex));
  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed) but waits no longer than
  // the given timeout.
  while ((done = circular_buffer_put(b->buffer,d)) == 0){
    rc = pthread_cond_timedwait(&(b->condFull), &(b->mutex),abstime);
    if (rc == ETIMEDOUT)break;
  }
  // Signal or broadcast that a full slot is available in the
  // unprotected circular buffer (if needed)
  if (rc != ETIMEDOUT)
    pthread_cond_broadcast(&(b->condEmpty));

  if (!done) d = NULL;
  print_task_activity ("offer", d);

  // Leave mutual exclusion
  pthread_mutex_unlock(&(b->mutex));
  return done;
}
