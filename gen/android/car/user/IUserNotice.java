/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.user;
/**
 * Binder for UserNotice UI to notify status change to CarUserNoticeService/CarService.
 * This binder is implemented inside CarService.
 * @hide
*/
public interface IUserNotice extends android.os.IInterface
{
  /** Default implementation for IUserNotice. */
  public static class Default implements android.car.user.IUserNotice
  {
    /**
         * Notify CarUserNoticeService/CarSercice that UI dialog is dismissed.
         * CarUserNoticeService will unbind the UI servie to finish it.
         */
    @Override public void onDialogDismissed() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.user.IUserNotice
  {
    private static final java.lang.String DESCRIPTOR = "android.car.user.IUserNotice";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.user.IUserNotice interface,
     * generating a proxy if needed.
     */
    public static android.car.user.IUserNotice asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.user.IUserNotice))) {
        return ((android.car.user.IUserNotice)iin);
      }
      return new android.car.user.IUserNotice.Stub.Proxy(obj);
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
        case TRANSACTION_onDialogDismissed:
        {
          data.enforceInterface(descriptor);
          this.onDialogDismissed();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.user.IUserNotice
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
           * Notify CarUserNoticeService/CarSercice that UI dialog is dismissed.
           * CarUserNoticeService will unbind the UI servie to finish it.
           */
      @Override public void onDialogDismissed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDialogDismissed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDialogDismissed();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.user.IUserNotice sDefaultImpl;
    }
    static final int TRANSACTION_onDialogDismissed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.user.IUserNotice impl) {
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
    public static android.car.user.IUserNotice getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notify CarUserNoticeService/CarSercice that UI dialog is dismissed.
       * CarUserNoticeService will unbind the UI servie to finish it.
       */
  public void onDialogDismissed() throws android.os.RemoteException;
}
