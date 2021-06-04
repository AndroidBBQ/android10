/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * System private API for direct alarm callbacks (non-broadcast deliver).  See the
 * AlarmManager#set() variants that take an AlarmReceiver callback object
 * rather than a PendingIntent.
 *
 * {@hide}
 */
public interface IAlarmListener extends android.os.IInterface
{
  /** Default implementation for IAlarmListener. */
  public static class Default implements android.app.IAlarmListener
  {
    @Override public void doAlarm(android.app.IAlarmCompleteListener callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IAlarmListener
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IAlarmListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IAlarmListener interface,
     * generating a proxy if needed.
     */
    public static android.app.IAlarmListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IAlarmListener))) {
        return ((android.app.IAlarmListener)iin);
      }
      return new android.app.IAlarmListener.Stub.Proxy(obj);
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
        case TRANSACTION_doAlarm:
        {
          data.enforceInterface(descriptor);
          android.app.IAlarmCompleteListener _arg0;
          _arg0 = android.app.IAlarmCompleteListener.Stub.asInterface(data.readStrongBinder());
          this.doAlarm(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IAlarmListener
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
      @Override public void doAlarm(android.app.IAlarmCompleteListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_doAlarm, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doAlarm(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IAlarmListener sDefaultImpl;
    }
    static final int TRANSACTION_doAlarm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.IAlarmListener impl) {
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
    public static android.app.IAlarmListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void doAlarm(android.app.IAlarmCompleteListener callback) throws android.os.RemoteException;
}
