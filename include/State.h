#ifndef INCLUDE_STATE_H_
#define INCLUDE_STATE_H_
#include <string>

using namespace std;

class State {
 protected:
  const char* stateName = "";

  bool finished = false;

 public:
  virtual ~State() = default;

  // Called when entering the state
  virtual void onEnter() = 0;

  // Called when exiting the state
  virtual void onExit() = 0;

  // Main action of the state
  virtual void handle() = 0;

  virtual const char* getName() const {return stateName;}

  // Everytime we reset everything
  virtual void reset() = 0;

  bool hasFinished() const { return finished; }

  void setFinished(bool finished) { this->finished = finished; }
};

#endif  // INCLUDE_STATE_H_
