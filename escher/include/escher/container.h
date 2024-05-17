#ifndef ESCHER_CONTAINER_H
#define ESCHER_CONTAINER_H

/* Container is the entry point of a program using Escher.
 *
 * A container contains one or multiple App, and is responsible for running them
 * together. Currently Container displays a single App fullscreen, but can
 * switch to any other App.
 *
 * When writing an Escher program, you typically subclass Container, and your
 * subclass owns one or more App. You then call "run()" on your container. */

#include <escher/app.h>
#include <escher/run_loop.h>
#include <escher/window.h>
#include <ion/events.h>

namespace Escher {

class Container : public RunLoop {
  friend class App;

 public:
  Container();
  virtual ~Container();
  Container(const Container& other) = delete;
  Container(Container&& other) = delete;
  Container& operator=(const Container& other) = delete;
  Container& operator=(Container&& other) = delete;
  virtual void* currentAppBuffer() = 0;
  virtual void run();
  bool dispatchEvent(Ion::Events::Event event) override;
  virtual void switchToBuiltinApp(App::Snapshot* snapshot);

 protected:
  static App* activeApp() { return s_activeApp; }
  virtual Window* window() = 0;

 private:
  int numberOfTimers() override;
  Timer* timerAtIndex(int i) override;
  virtual int numberOfContainerTimers();
  virtual Timer* containerTimerAtIndex(int i);
  static App* s_activeApp;
};

inline App* App::app() { return Container::activeApp(); }

}  // namespace Escher

#endif
