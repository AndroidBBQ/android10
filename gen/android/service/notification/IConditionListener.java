/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.notification;
/** @hide */
public interface IConditionListener extends android.os.IInterface
{
  /** Default implementation for IConditionListener. */
  public static class Default implements android.service.notification.IConditionListener
  {
    @Override public void onConditionsReceived(android.service.notification.Condition[] conditions) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.notification.IConditionListener
  {
    private static final java.lang.String DESCRIPTOR = "android.service.notification.IConditionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.notification.IConditionListener interface,
     * generating a proxy if needed.
     */
    public static android.service.notification.IConditionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.notification.IConditionListener))) {
        return ((android.service.notification.IConditionListener)iin);
      }
      return new android.service.notification.IConditionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onConditionsReceived:
        {
          data.enforceInterface(descriptor);
          android.service.notification.Condition[] _arg0;
          _arg0 = data.createTypedArray(android.service.notification.Condition.CREATOR);
          this.onConditionsReceived(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.notification.IConditionListener
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
      @Override public void onConditionsReceived(android.service.notification.Condition[] conditions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(conditions, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConditionsReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConditionsReceived(conditions);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.notification.IConditionListener sDefaultImpl;
    }
    static final int TRANSACTION_onConditionsReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.notification.IConditionListener impl) {
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
    public static android.service.notification.IConditionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onConditionsReceived(android.service.notification.Condition[] conditions) throws android.os.RemoteException;
}
