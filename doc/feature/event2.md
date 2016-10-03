Event Handling v2
============

This specification outlines the archicture for event handling
introduced in Equalizer 2.0.

## Requirements

* Modern event definitions using EventI/OCommand


## Data Flow

* The (agl,glx,wgl,qt) EventHandler receives a system event
* The event handler decodes the event into a PointerEvent, KeyEvent,
  SizeEvent, ButtonEvent, AxisEvent or stateless event
* The event handler calls (agl,glx,wgl,qt)
  ```Window::processEvent( event type, native event, [, translated event ])```
* The system window performs system operations if necessary, and calls
  ```GLWindow::processEvent( event type, [, translated event ])```
* The GL window performs OpenGL procession if necessary, and calls
  ```Window::processEvent( event type, [, translated event ])```
* The eq::Window performs window updates and transforms the event to the
  respective Channel coordinates (for PointerEvent) or calls
  ```Config::sendEvent( event type ) << event```
* The eq::Channel performs channel updates and calls
  ```Config::sendEvent( event type ) << event```
* Config::sendEvent transmits the event to the application node
* The application calls Config::handleEvents, which receives the event
  and calls ```Config::handleEvent( EventICommand )```
* Config::handleEvent decodes the command and calls one of the
  ```Config::handeEvent( event type, event )```
