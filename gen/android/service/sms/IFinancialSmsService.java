/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.sms;
/**
 * Service used by financial apps to read sms messages.
 *
 * @hide
 */
public interface IFinancialSmsService extends android.os.IInterface
{
  /** Default implementation for IFinancialSmsService. */
  public static class Default implements android.service.sms.IFinancialSmsService
  {
    @Override public void getSmsMessages(android.os.RemoteCallback callback, android.os.Bundle params) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.sms.IFinancialSmsService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.sms.IFinancialSmsService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.sms.IFinancialSmsService interface,
     * generating a proxy if needed.
     */
    public static android.service.sms.IFinancialSmsService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.sms.IFinancialSmsService))) {
        return ((android.service.sms.IFinancialSmsService)iin);
      }
      return new android.service.sms.IFinancialSmsService.Stub.Proxy(obj);
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
        case TRANSACTION_getSmsMessages:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.getSmsMessages(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.sms.IFinancialSmsService
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
      @Override public void getSmsMessages(android.os.RemoteCallback callback, android.os.Bundle params) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSmsMessages, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getSmsMessages(callback, params);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.sms.IFinancialSmsService sDefaultImpl;
    }
    static final int TRANSACTION_getSmsMessages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.sms.IFinancialSmsService impl) {
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
    public static android.service.sms.IFinancialSmsService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void getSmsMessages(android.os.RemoteCallback callback, android.os.Bundle params) throws android.os.RemoteException;
}
