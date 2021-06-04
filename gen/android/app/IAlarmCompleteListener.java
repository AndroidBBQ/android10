/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * Callback from app into system process to indicate that processing of
 * a direct-call alarm has completed.
 * {@hide}
 */
public interface IAlarmCompleteListener extends android.os.IInterface
{
  /** Default implementation for IAlarmCompleteListener. */
  public static class Default implements android.app.IAlarmCompleteListener
  {
    @Override public void alarmComplete(android.os.IBinder who) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IAlarmCompleteListener
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IAlarmCompleteListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IAlarmCompleteListener interface,
     * generating a proxy if needed.
     */
    public static android.app.IAlarmCompleteListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IAlarmCompleteListener))) {
        return ((android.app.IAlarmCompleteListener)iin);
      }
      return new android.app.IAlarmCompleteListener.Stub.Proxy(obj);
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
        case TRANSACTION_alarmComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.alarmComplete(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IAlarmCompleteListener
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
      @Override public void alarmComplete(android.os.IBinder who) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(who);
          boolean _status = mRemote.transact(Stub.TRANSACTION_alarmComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().alarmComplete(who);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.IAlarmCompleteListener sDefaultImpl;
    }
    static final int TRANSACTION_alarmComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.IAlarmCompleteListener impl) {
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
    public static android.app.IAlarmCompleteListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void alarmComplete(android.os.IBinder who) throws android.os.RemoteException;
}
