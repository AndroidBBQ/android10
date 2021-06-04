/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.user;
/**
 * Binder for CarUserNoticeService/CarService to pass IUserNotice binder to UserNotice UI.
 * UserNotice UI implements this binder.
 * @hide
*/
public interface IUserNoticeUI extends android.os.IInterface
{
  /** Default implementation for IUserNoticeUI. */
  public static class Default implements android.car.user.IUserNoticeUI
  {
    /**
         * CarUserNoticeService will use this call to pass IUserNotice binder which can be used
         * to notify dismissal of UI dialog.
         */
    @Override public void setCallbackBinder(android.car.user.IUserNotice binder) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.user.IUserNoticeUI
  {
    private static final java.lang.String DESCRIPTOR = "android.car.user.IUserNoticeUI";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.user.IUserNoticeUI interface,
     * generating a proxy if needed.
     */
    public static android.car.user.IUserNoticeUI asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.user.IUserNoticeUI))) {
        return ((android.car.user.IUserNoticeUI)iin);
      }
      return new android.car.user.IUserNoticeUI.Stub.Proxy(obj);
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
        case TRANSACTION_setCallbackBinder:
        {
          data.enforceInterface(descriptor);
          android.car.user.IUserNotice _arg0;
          _arg0 = android.car.user.IUserNotice.Stub.asInterface(data.readStrongBinder());
          this.setCallbackBinder(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.user.IUserNoticeUI
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
      /**
           * CarUserNoticeService will use this call to pass IUserNotice binder which can be used
           * to notify dismissal of UI dialog.
           */
      @Override public void setCallbackBinder(android.car.user.IUserNotice binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((binder!=null))?(binder.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallbackBinder, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallbackBinder(binder);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.user.IUserNoticeUI sDefaultImpl;
    }
    static final int TRANSACTION_setCallbackBinder = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.user.IUserNoticeUI impl) {
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
    public static android.car.user.IUserNoticeUI getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * CarUserNoticeService will use this call to pass IUserNotice binder which can be used
       * to notify dismissal of UI dialog.
       */
  public void setCallbackBinder(android.car.user.IUserNotice binder) throws android.os.RemoteException;
}
