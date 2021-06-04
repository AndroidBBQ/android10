/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * Interface to inform NetworkMonitor of decisions of app handling captive portal.
 * @hide
 */
public interface ICaptivePortal extends android.os.IInterface
{
  /** Default implementation for ICaptivePortal. */
  public static class Default implements android.net.ICaptivePortal
  {
    @Override public void appResponse(int response) throws android.os.RemoteException
    {
    }
    @Override public void logEvent(int eventId, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.ICaptivePortal
  {
    private static final java.lang.String DESCRIPTOR = "android.net.ICaptivePortal";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.ICaptivePortal interface,
     * generating a proxy if needed.
     */
    public static android.net.ICaptivePortal asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.ICaptivePortal))) {
        return ((android.net.ICaptivePortal)iin);
      }
      return new android.net.ICaptivePortal.Stub.Proxy(obj);
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
        case TRANSACTION_appResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.appResponse(_arg0);
          return true;
        }
        case TRANSACTION_logEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.logEvent(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.ICaptivePortal
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
      @Override public void appResponse(int response) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(response);
          boolean _status = mRemote.transact(Stub.TRANSACTION_appResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appResponse(response);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void logEvent(int eventId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(eventId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_logEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().logEvent(eventId, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.ICaptivePortal sDefaultImpl;
    }
    static final int TRANSACTION_appResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_logEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.ICaptivePortal impl) {
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
    public static android.net.ICaptivePortal getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void appResponse(int response) throws android.os.RemoteException;
  public void logEvent(int eventId, java.lang.String packageName) throws android.os.RemoteException;
}
