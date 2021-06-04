/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony;
/**
 * Callback to provide asynchronous CellInfo.
 * @hide
 */
public interface ICellInfoCallback extends android.os.IInterface
{
  /** Default implementation for ICellInfoCallback. */
  public static class Default implements android.telephony.ICellInfoCallback
  {
    @Override public void onCellInfo(java.util.List<android.telephony.CellInfo> state) throws android.os.RemoteException
    {
    }
    @Override public void onError(int errorCode, android.os.ParcelableException detail) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ICellInfoCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ICellInfoCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ICellInfoCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ICellInfoCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ICellInfoCallback))) {
        return ((android.telephony.ICellInfoCallback)iin);
      }
      return new android.telephony.ICellInfoCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onCellInfo:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.CellInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.CellInfo.CREATOR);
          this.onCellInfo(_arg0);
          return true;
        }
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ParcelableException _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelableException.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onError(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ICellInfoCallback
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
      @Override public void onCellInfo(java.util.List<android.telephony.CellInfo> state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCellInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCellInfo(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onError(int errorCode, android.os.ParcelableException detail) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(errorCode);
          if ((detail!=null)) {
            _data.writeInt(1);
            detail.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(errorCode, detail);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ICellInfoCallback sDefaultImpl;
    }
    static final int TRANSACTION_onCellInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.telephony.ICellInfoCallback impl) {
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
    public static android.telephony.ICellInfoCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onCellInfo(java.util.List<android.telephony.CellInfo> state) throws android.os.RemoteException;
  public void onError(int errorCode, android.os.ParcelableException detail) throws android.os.RemoteException;
}
