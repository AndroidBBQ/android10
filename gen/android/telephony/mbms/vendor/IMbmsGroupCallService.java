/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms.vendor;
/**
 * @hide
 */
public interface IMbmsGroupCallService extends android.os.IInterface
{
  /** Default implementation for IMbmsGroupCallService. */
  public static class Default implements android.telephony.mbms.vendor.IMbmsGroupCallService
  {
    @Override public int initialize(android.telephony.mbms.IMbmsGroupCallSessionCallback callback, int subId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void stopGroupCall(int subId, long tmgi) throws android.os.RemoteException
    {
    }
    @Override public void updateGroupCall(int subscriptionId, long tmgi, java.util.List saiList, java.util.List frequencyList) throws android.os.RemoteException
    {
    }
    @Override public int startGroupCall(int subscriptionId, long tmgi, java.util.List saiList, java.util.List frequencyList, android.telephony.mbms.IGroupCallCallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void dispose(int subId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.vendor.IMbmsGroupCallService
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.vendor.IMbmsGroupCallService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.vendor.IMbmsGroupCallService interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.vendor.IMbmsGroupCallService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.vendor.IMbmsGroupCallService))) {
        return ((android.telephony.mbms.vendor.IMbmsGroupCallService)iin);
      }
      return new android.telephony.mbms.vendor.IMbmsGroupCallService.Stub.Proxy(obj);
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
        case TRANSACTION_initialize:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.IMbmsGroupCallSessionCallback _arg0;
          _arg0 = android.telephony.mbms.IMbmsGroupCallSessionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.initialize(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopGroupCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          this.stopGroupCall(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateGroupCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.util.List _arg2;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg2 = data.readArrayList(cl);
          java.util.List _arg3;
          _arg3 = data.readArrayList(cl);
          this.updateGroupCall(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startGroupCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.util.List _arg2;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg2 = data.readArrayList(cl);
          java.util.List _arg3;
          _arg3 = data.readArrayList(cl);
          android.telephony.mbms.IGroupCallCallback _arg4;
          _arg4 = android.telephony.mbms.IGroupCallCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.startGroupCall(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_dispose:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.dispose(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.vendor.IMbmsGroupCallService
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
      @Override public int initialize(android.telephony.mbms.IMbmsGroupCallSessionCallback callback, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_initialize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().initialize(callback, subId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void stopGroupCall(int subId, long tmgi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeLong(tmgi);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopGroupCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopGroupCall(subId, tmgi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateGroupCall(int subscriptionId, long tmgi, java.util.List saiList, java.util.List frequencyList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subscriptionId);
          _data.writeLong(tmgi);
          _data.writeList(saiList);
          _data.writeList(frequencyList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateGroupCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateGroupCall(subscriptionId, tmgi, saiList, frequencyList);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int startGroupCall(int subscriptionId, long tmgi, java.util.List saiList, java.util.List frequencyList, android.telephony.mbms.IGroupCallCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subscriptionId);
          _data.writeLong(tmgi);
          _data.writeList(saiList);
          _data.writeList(frequencyList);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startGroupCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startGroupCall(subscriptionId, tmgi, saiList, frequencyList, callback);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void dispose(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispose, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispose(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.telephony.mbms.vendor.IMbmsGroupCallService sDefaultImpl;
    }
    static final int TRANSACTION_initialize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopGroupCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_updateGroupCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_startGroupCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_dispose = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.telephony.mbms.vendor.IMbmsGroupCallService impl) {
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
    public static android.telephony.mbms.vendor.IMbmsGroupCallService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int initialize(android.telephony.mbms.IMbmsGroupCallSessionCallback callback, int subId) throws android.os.RemoteException;
  public void stopGroupCall(int subId, long tmgi) throws android.os.RemoteException;
  public void updateGroupCall(int subscriptionId, long tmgi, java.util.List saiList, java.util.List frequencyList) throws android.os.RemoteException;
  public int startGroupCall(int subscriptionId, long tmgi, java.util.List saiList, java.util.List frequencyList, android.telephony.mbms.IGroupCallCallback callback) throws android.os.RemoteException;
  public void dispose(int subId) throws android.os.RemoteException;
}
