/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.notification;
/** @hide */
public interface IStatusBarNotificationHolder extends android.os.IInterface
{
  /** Default implementation for IStatusBarNotificationHolder. */
  public static class Default implements android.service.notification.IStatusBarNotificationHolder
  {
    /** Fetch the held StatusBarNotification. This method should only be called once per Holder */
    @Override public android.service.notification.StatusBarNotification get() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.notification.IStatusBarNotificationHolder
  {
    private static final java.lang.String DESCRIPTOR = "android.service.notification.IStatusBarNotificationHolder";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.notification.IStatusBarNotificationHolder interface,
     * generating a proxy if needed.
     */
    public static android.service.notification.IStatusBarNotificationHolder asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.notification.IStatusBarNotificationHolder))) {
        return ((android.service.notification.IStatusBarNotificationHolder)iin);
      }
      return new android.service.notification.IStatusBarNotificationHolder.Stub.Proxy(obj);
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
        case TRANSACTION_get:
        {
          data.enforceInterface(descriptor);
          android.service.notification.StatusBarNotification _result = this.get();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.notification.IStatusBarNotificationHolder
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
      /** Fetch the held StatusBarNotification. This method should only be called once per Holder */
      @Override public android.service.notification.StatusBarNotification get() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.service.notification.StatusBarNotification _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_get, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().get();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.service.notification.StatusBarNotification.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.service.notification.IStatusBarNotificationHolder sDefaultImpl;
    }
    static final int TRANSACTION_get = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.notification.IStatusBarNotificationHolder impl) {
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
    public static android.service.notification.IStatusBarNotificationHolder getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Fetch the held StatusBarNotification. This method should only be called once per Holder */
  public android.service.notification.StatusBarNotification get() throws android.os.RemoteException;
}
