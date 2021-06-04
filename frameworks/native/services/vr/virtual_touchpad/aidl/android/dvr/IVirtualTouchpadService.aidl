package android.dvr;

/** @hide */
interface IVirtualTouchpadService
{
  const String SERVICE_NAME = "virtual_touchpad";

  /**
   * Initialize the virtual touchpad.
   */
  void attach() = 0;

  /**
   * Shut down the virtual touchpad.
   */
  void detach() = 1;

  /**
   * Generate a simulated touch event.
   *
   * @param touchpad Selects touchpad.
   * @param x Horizontal touch position.
   * @param y Vertical touch position.
   * @param pressure Touch pressure; use 0.0 for no touch (lift or hover).
   *
   * Position values in the range [0.0, 1.0) map to the screen.
   */
  void touch(int touchpad, float x, float y, float pressure) = 2;

  /**
   * Generate a simulated touchpad button state event.
   *
   * @param touchpad Selects touchpad.
   * @param buttons A union of MotionEvent BUTTON_* values.
   */
  void buttonState(int touchpad, int buttons) = 3;

  /**
   * Generate a simulated scroll event.
   *
   * @param touchpad Selects touchpad.
   * @param x Horizontal scroll increment.
   * @param y Vertical scroll increment.
   *
   * Scroll values are in the range [-1.0, 1.0].
   */
  void scroll(int touchpad, float x, float y) = 4;
}
