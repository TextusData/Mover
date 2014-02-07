/* State.h -*- c++ -*- 
 * Copyright (C) 2010 Ross Biro
 *
 * State.h represents the current state of a state machine.
 * We have to be careful to separate the state from the variables
 * that make up the state.  The state really just consists of a pointer
 * to the transtition functions.  The rest of the variables that make
 * up the state are stored in a derived class.
 *
 */

#ifndef TEXTUS_EVENT_STATE_H_
#define TEXTUS_EVENT_STATE_H_

#include "textus/base/Base.h"
#include "textus/base/AutoDelete.h"
#include "textus/event/Message.h"
#include "textus/network/server/ServerTemplate.h"

#include <map>

namespace textus { namespace event {

using namespace std;

class State: virtual public textus::network::server::Processor {
public:
  typedef int (State::*TransitionMember)(Message *m);
  typedef map<uint32_t, TransitionMember> TransitionMap;
  typedef map<int, TransitionMap > TransitionMapMap;
  enum definedStates {
    ErrorState = -1,
    StartState = 0,
    EndState = 1,
    LastReservedState = 10000, // all states less than this are reserved.
  };

private:
  int current_state;
  textus::base::AutoDelete<TransitionMapMap> simple_transitions;

protected:
  virtual const TransitionMap *transitions() {
    Synchronized(this);
    assert(simple_transitions != NULL);
    TransitionMapMap::iterator i = simple_transitions->find(current_state);
    if (i == simple_transitions->end()) {
      return NULL;
    }
    return &(i->second);
  }

  int addTransitionMap(int state, const TransitionMap t) {
    Synchronized(this);
    if (simple_transitions == NULL) {
      simple_transitions = new TransitionMapMap();
    }
    if (simple_transitions == NULL) {
      return -1;
    }
    (*simple_transitions)[state] = t;
    return 0;
  }

  int addTransition(int state, uint32_t mt, TransitionMember member) {
    Synchronized(this);
    if (simple_transitions == NULL) {
      simple_transitions = new TransitionMapMap();
    }
    if (simple_transitions == NULL) {
      return -1;
    }
    ((*simple_transitions)[state])[mt] = member;
    return 0;
  }

public:
  State(): current_state(StartState) {}
  virtual ~State() {}

  virtual void processMessage(Message *m) {
    Synchronized(this);
    if (current_state == EndState) {
      return;
    }
    int old_state = current_state;
    uint32_t type = m->type();
    const TransitionMap *tmap = transitions();
    assert(tmap != NULL); // if we have no transitions, we are in an invalid state.
    TransitionMap::const_iterator i = tmap->find(type);
    if ( i == tmap->end() ) {
      unhandledMessage(m);
      current_state = ErrorState;
    } else {
      const TransitionMember tm = i->second;
      current_state = (this->*tm)(m);
    }
    onStateChange(old_state, current_state);
    signal();
    return;
  }

  virtual void unhandledMessage(Message *m) {
    LOG(WARNING) << "Unahndled Message in state " << this << ": " << m << "\n";
  }

  void waitForState(int s) {
    Synchronized(this);
    while (state() != s && state() != ErrorState && state() != EndState ) {
      wait();
    }
  }

  int state() {
    Synchronized(this);
    return current_state;
  }

  void setState(int s) {
    Synchronized(this);
    int old_state = current_state;
    current_state = s;
    onStateChange(old_state, current_state);
    signal();
  }

  virtual void onStateChange(int old, int nw) {
    // Default to do nothing.  The actions should occur
    // in the transition function above.  But this allows
    // for outside observers.
  }
  
};

}} //namespace

#endif // TEXTUS_EVENT_STATE_H_
