/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.data;
/**
 * {@hide}
 */
public interface IDataService extends android.os.IInterface
{
  /** Default implementation for IDataService. */
  public static class Default implements android.telephony.data.IDataService
  {
    @Override public void createDataServiceProvider(int slotId) throws android.os.RemoteException
    {
    }
    @Override public void removeDataServiceProvider(int slotId) throws android.os.RemoteException
    {
    }
    @Override public void setupDataCall(int slotId, int accessNetwork, android.telephony.data.DataProfile dataProfile, boolean isRoaming, boolean allowRoaming, int reason, android.net.LinkProperties linkProperties, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void deactivateDataCall(int slotId, int cid, int reason, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setInitialAttachApn(int slotId, android.telephony.data.DataProfile dataProfile, boolean isRoaming, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setDataProfile(int slotId, java.util.List<android.telephony.data.DataProfile> dps, boolean isRoaming, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void requestDataCallList(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void registerForDataCallListChanged(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterForDataCallListChanged(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.data.IDataService
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.data.IDataService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.data.IDataService interface,
     * generating a proxy if needed.
     */
    public static android.telephony.data.IDataService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.data.IDataService))) {
        return ((android.telephony.data.IDataService)iin);
      }
      return new android.telephony.data.IDataService.Stub.Proxy(obj);
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
        case TRANSACTION_createDataServiceProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.createDataServiceProvider(_arg0);
          return true;
        }
        case TRANSACTION_removeDataServiceProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeDataServiceProvider(_arg0);
          return true;
        }
        case TRANSACTION_setupDataCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.data.DataProfile _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.data.DataProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          int _arg5;
          _arg5 = data.readInt();
          android.net.LinkProperties _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          android.telephony.data.IDataServiceCallback _arg7;
          _arg7 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.setupDataCall(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_deactivateDataCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.telephony.data.IDataServiceCallback _arg3;
          _arg3 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.deactivateDataCall(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_setInitialAttachApn:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.data.DataProfile _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.data.DataProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.telephony.data.IDataServiceCallback _arg3;
          _arg3 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.setInitialAttachApn(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_setDataProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.telephony.data.DataProfile> _arg1;
          _arg1 = data.createTypedArrayList(android.telephony.data.DataProfile.CREATOR);
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.telephony.data.IDataServiceCallback _arg3;
          _arg3 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.setDataProfile(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_requestDataCallList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.data.IDataServiceCallback _arg1;
          _arg1 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.requestDataCallList(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_registerForDataCallListChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.data.IDataServiceCallback _arg1;
          _arg1 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.registerForDataCallListChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unregisterForDataCallListChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.data.IDataServiceCallback _arg1;
          _arg1 = android.telephony.data.IDataServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterForDataCallListChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.data.IDataService
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
      @Override public void createDataServiceProvider(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createDataServiceProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createDataServiceProvider(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeDataServiceProvider(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeDataServiceProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeDataServiceProvider(slotId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setupDataCall(int slotId, int accessNetwork, android.telephony.data.DataProfile dataProfile, boolean isRoaming, boolean allowRoaming, int reason, android.net.LinkProperties linkProperties, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(accessNetwork);
          if ((dataProfile!=null)) {
            _data.writeInt(1);
            dataProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((isRoaming)?(1):(0)));
          _data.writeInt(((allowRoaming)?(1):(0)));
          _data.writeInt(reason);
          if ((linkProperties!=null)) {
            _data.writeInt(1);
            linkProperties.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupDataCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupDataCall(slotId, accessNetwork, dataProfile, isRoaming, allowRoaming, reason, linkProperties, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deactivateDataCall(int slotId, int cid, int reason, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(cid);
          _data.writeInt(reason);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_deactivateDataCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deactivateDataCall(slotId, cid, reason, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setInitialAttachApn(int slotId, android.telephony.data.DataProfile dataProfile, boolean isRoaming, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          if ((dataProfile!=null)) {
            _data.writeInt(1);
            dataProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((isRoaming)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInitialAttachApn, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInitialAttachApn(slotId, dataProfile, isRoaming, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDataProfile(int slotId, java.util.List<android.telephony.data.DataProfile> dps, boolean isRoaming, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeTypedList(dps);
          _data.writeInt(((isRoaming)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataProfile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDataProfile(slotId, dps, isRoaming, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestDataCallList(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestDataCallList, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestDataCallList(slotId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void registerForDataCallListChanged(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerForDataCallListChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerForDataCallListChanged(slotId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unregisterForDataCallListChanged(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterForDataCallListChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterForDataCallListChanged(slotId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.data.IDataService sDefaultImpl;
    }
    static final int TRANSACTION_createDataServiceProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeDataServiceProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setupDataCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_deactivateDataCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setInitialAttachApn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setDataProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_requestDataCallList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_registerForDataCallListChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_unregisterForDataCallListChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.telephony.data.IDataService impl) {
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
    public static android.telephony.data.IDataService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void createDataServiceProvider(int slotId) throws android.os.RemoteException;
  public void removeDataServiceProvider(int slotId) throws android.os.RemoteException;
  public void setupDataCall(int slotId, int accessNetwork, android.telephony.data.DataProfile dataProfile, boolean isRoaming, boolean allowRoaming, int reason, android.net.LinkProperties linkProperties, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
  public void deactivateDataCall(int slotId, int cid, int reason, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
  public void setInitialAttachApn(int slotId, android.telephony.data.DataProfile dataProfile, boolean isRoaming, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
  public void setDataProfile(int slotId, java.util.List<android.telephony.data.DataProfile> dps, boolean isRoaming, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
  public void requestDataCallList(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
  public void registerForDataCallListChanged(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
  public void unregisterForDataCallListChanged(int slotId, android.telephony.data.IDataServiceCallback callback) throws android.os.RemoteException;
}
