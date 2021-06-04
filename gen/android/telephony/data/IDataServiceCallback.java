/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.data;
/**
 * The call back interface
 * @hide
 */
public interface IDataServiceCallback extends android.os.IInterface
{
  /** Default implementation for IDataServiceCallback. */
  public static class Default implements android.telephony.data.IDataServiceCallback
  {
    @Override public void onSetupDataCallComplete(int result, android.telephony.data.DataCallResponse dataCallResponse) throws android.os.RemoteException
    {
    }
    @Override public void onDeactivateDataCallComplete(int result) throws android.os.RemoteException
    {
    }
    @Override public void onSetInitialAttachApnComplete(int result) throws android.os.RemoteException
    {
    }
    @Override public void onSetDataProfileComplete(int result) throws android.os.RemoteException
    {
    }
    @Override public void onRequestDataCallListComplete(int result, java.util.List<android.telephony.data.DataCallResponse> dataCallList) throws android.os.RemoteException
    {
    }
    @Override public void onDataCallListChanged(java.util.List<android.telephony.data.DataCallResponse> dataCallList) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.data.IDataServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.data.IDataServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.data.IDataServiceCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.data.IDataServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.data.IDataServiceCallback))) {
        return ((android.telephony.data.IDataServiceCallback)iin);
      }
      return new android.telephony.data.IDataServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSetupDataCallComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.data.DataCallResponse _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.data.DataCallResponse.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onSetupDataCallComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDeactivateDataCallComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onDeactivateDataCallComplete(_arg0);
          return true;
        }
        case TRANSACTION_onSetInitialAttachApnComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSetInitialAttachApnComplete(_arg0);
          return true;
        }
        case TRANSACTION_onSetDataProfileComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSetDataProfileComplete(_arg0);
          return true;
        }
        case TRANSACTION_onRequestDataCallListComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.telephony.data.DataCallResponse> _arg1;
          _arg1 = data.createTypedArrayList(android.telephony.data.DataCallResponse.CREATOR);
          this.onRequestDataCallListComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDataCallListChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.data.DataCallResponse> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.data.DataCallResponse.CREATOR);
          this.onDataCallListChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.data.IDataServiceCallback
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
      @Override public void onSetupDataCallComplete(int result, android.telephony.data.DataCallResponse dataCallResponse) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          if ((dataCallResponse!=null)) {
            _data.writeInt(1);
            dataCallResponse.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetupDataCallComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetupDataCallComplete(result, dataCallResponse);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDeactivateDataCallComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeactivateDataCallComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeactivateDataCallComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSetInitialAttachApnComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetInitialAttachApnComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetInitialAttachApnComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSetDataProfileComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetDataProfileComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetDataProfileComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRequestDataCallListComplete(int result, java.util.List<android.telephony.data.DataCallResponse> dataCallList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          _data.writeTypedList(dataCallList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRequestDataCallListComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRequestDataCallListComplete(result, dataCallList);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDataCallListChanged(java.util.List<android.telephony.data.DataCallResponse> dataCallList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(dataCallList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDataCallListChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDataCallListChanged(dataCallList);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.data.IDataServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSetupDataCallComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onDeactivateDataCallComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSetInitialAttachApnComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSetDataProfileComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onRequestDataCallListComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onDataCallListChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.telephony.data.IDataServiceCallback impl) {
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
    public static android.telephony.data.IDataServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSetupDataCallComplete(int result, android.telephony.data.DataCallResponse dataCallResponse) throws android.os.RemoteException;
  public void onDeactivateDataCallComplete(int result) throws android.os.RemoteException;
  public void onSetInitialAttachApnComplete(int result) throws android.os.RemoteException;
  public void onSetDataProfileComplete(int result) throws android.os.RemoteException;
  public void onRequestDataCallListComplete(int result, java.util.List<android.telephony.data.DataCallResponse> dataCallList) throws android.os.RemoteException;
  public void onDataCallListChanged(java.util.List<android.telephony.data.DataCallResponse> dataCallList) throws android.os.RemoteException;
}
