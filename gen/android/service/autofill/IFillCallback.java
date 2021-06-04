/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.autofill;
/**
 * Interface to receive the result of a save request.
 *
 * @hide
 */
public interface IFillCallback extends android.os.IInterface
{
  /** Default implementation for IFillCallback. */
  public static class Default implements android.service.autofill.IFillCallback
  {
    @Override public void onCancellable(android.os.ICancellationSignal cancellation) throws android.os.RemoteException
    {
    }
    @Override public void onSuccess(android.service.autofill.FillResponse response) throws android.os.RemoteException
    {
    }
    @Override public void onFailure(int requestId, java.lang.CharSequence message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.autofill.IFillCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.autofill.IFillCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.autofill.IFillCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.autofill.IFillCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.autofill.IFillCallback))) {
        return ((android.service.autofill.IFillCallback)iin);
      }
      return new android.service.autofill.IFillCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onCancellable:
        {
          data.enforceInterface(descriptor);
          android.os.ICancellationSignal _arg0;
          _arg0 = android.os.ICancellationSignal.Stub.asInterface(data.readStrongBinder());
          this.onCancellable(_arg0);
          return true;
        }
        case TRANSACTION_onSuccess:
        {
          data.enforceInterface(descriptor);
          android.service.autofill.FillResponse _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.autofill.FillResponse.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onFailure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.CharSequence _arg1;
          if (0!=data.readInt()) {
            _arg1 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onFailure(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.autofill.IFillCallback
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
      @Override public void onCancellable(android.os.ICancellationSignal cancellation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cancellation!=null))?(cancellation.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCancellable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCancellable(cancellation);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSuccess(android.service.autofill.FillResponse response) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((response!=null)) {
            _data.writeInt(1);
            response.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuccess(response);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFailure(int requestId, java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(requestId);
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFailure(requestId, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.autofill.IFillCallback sDefaultImpl;
    }
    static final int TRANSACTION_onCancellable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.service.autofill.IFillCallback impl) {
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
    public static android.service.autofill.IFillCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onCancellable(android.os.ICancellationSignal cancellation) throws android.os.RemoteException;
  public void onSuccess(android.service.autofill.FillResponse response) throws android.os.RemoteException;
  public void onFailure(int requestId, java.lang.CharSequence message) throws android.os.RemoteException;
}
