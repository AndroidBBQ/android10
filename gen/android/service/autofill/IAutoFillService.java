/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.autofill;
/**
 * Interface from the system to an auto fill service.
 *
 * @hide
 */
public interface IAutoFillService extends android.os.IInterface
{
  /** Default implementation for IAutoFillService. */
  public static class Default implements android.service.autofill.IAutoFillService
  {
    @Override public void onConnectedStateChanged(boolean connected) throws android.os.RemoteException
    {
    }
    @Override public void onFillRequest(android.service.autofill.FillRequest request, android.service.autofill.IFillCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onSaveRequest(android.service.autofill.SaveRequest request, android.service.autofill.ISaveCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.autofill.IAutoFillService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.autofill.IAutoFillService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.autofill.IAutoFillService interface,
     * generating a proxy if needed.
     */
    public static android.service.autofill.IAutoFillService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.autofill.IAutoFillService))) {
        return ((android.service.autofill.IAutoFillService)iin);
      }
      return new android.service.autofill.IAutoFillService.Stub.Proxy(obj);
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
        case TRANSACTION_onConnectedStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onConnectedStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onFillRequest:
        {
          data.enforceInterface(descriptor);
          android.service.autofill.FillRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.autofill.FillRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.service.autofill.IFillCallback _arg1;
          _arg1 = android.service.autofill.IFillCallback.Stub.asInterface(data.readStrongBinder());
          this.onFillRequest(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSaveRequest:
        {
          data.enforceInterface(descriptor);
          android.service.autofill.SaveRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.autofill.SaveRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.service.autofill.ISaveCallback _arg1;
          _arg1 = android.service.autofill.ISaveCallback.Stub.asInterface(data.readStrongBinder());
          this.onSaveRequest(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.autofill.IAutoFillService
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
      @Override public void onConnectedStateChanged(boolean connected) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((connected)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectedStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectedStateChanged(connected);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFillRequest(android.service.autofill.FillRequest request, android.service.autofill.IFillCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFillRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFillRequest(request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSaveRequest(android.service.autofill.SaveRequest request, android.service.autofill.ISaveCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSaveRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSaveRequest(request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.autofill.IAutoFillService sDefaultImpl;
    }
    static final int TRANSACTION_onConnectedStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onFillRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSaveRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.service.autofill.IAutoFillService impl) {
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
    public static android.service.autofill.IAutoFillService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onConnectedStateChanged(boolean connected) throws android.os.RemoteException;
  public void onFillRequest(android.service.autofill.FillRequest request, android.service.autofill.IFillCallback callback) throws android.os.RemoteException;
  public void onSaveRequest(android.service.autofill.SaveRequest request, android.service.autofill.ISaveCallback callback) throws android.os.RemoteException;
}
