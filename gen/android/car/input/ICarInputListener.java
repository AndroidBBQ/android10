/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.input;
/**
 * Binder API for Input Service.
 *
 * @hide
 */
public interface ICarInputListener extends android.os.IInterface
{
  /** Default implementation for ICarInputListener. */
  public static class Default implements android.car.input.ICarInputListener
  {
    /** Called when key event has been received. */
    @Override public void onKeyEvent(android.view.KeyEvent keyEvent, int targetDisplay) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.input.ICarInputListener
  {
    private static final java.lang.String DESCRIPTOR = "android.car.input.ICarInputListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.input.ICarInputListener interface,
     * generating a proxy if needed.
     */
    public static android.car.input.ICarInputListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.input.ICarInputListener))) {
        return ((android.car.input.ICarInputListener)iin);
      }
      return new android.car.input.ICarInputListener.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_onKeyEvent:
        {
          data.enforceInterface(descriptor);
          android.view.KeyEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onKeyEvent(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.input.ICarInputListener
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      /** Called when key event has been received. */
      @Override public void onKeyEvent(android.view.KeyEvent keyEvent, int targetDisplay) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((keyEvent!=null)) {
            _data.writeInt(1);
            keyEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(targetDisplay);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onKeyEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onKeyEvent(keyEvent, targetDisplay);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.input.ICarInputListener sDefaultImpl;
    }
    static final int TRANSACTION_onKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.input.ICarInputListener impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static android.car.input.ICarInputListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Called when key event has been received. */
  public void onKeyEvent(android.view.KeyEvent keyEvent, int targetDisplay) throws android.os.RemoteException;
}
