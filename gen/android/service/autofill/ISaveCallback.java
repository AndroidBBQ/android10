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
public interface ISaveCallback extends android.os.IInterface
{
  /** Default implementation for ISaveCallback. */
  public static class Default implements android.service.autofill.ISaveCallback
  {
    @Override public void onSuccess(android.content.IntentSender intentSender) throws android.os.RemoteException
    {
    }
    @Override public void onFailure(java.lang.CharSequence message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.autofill.ISaveCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.autofill.ISaveCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.autofill.ISaveCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.autofill.ISaveCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.autofill.ISaveCallback))) {
        return ((android.service.autofill.ISaveCallback)iin);
      }
      return new android.service.autofill.ISaveCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSuccess:
        {
          data.enforceInterface(descriptor);
          android.content.IntentSender _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.IntentSender.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSuccess(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onFailure:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onFailure(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.autofill.ISaveCallback
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
      @Override public void onSuccess(android.content.IntentSender intentSender) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intentSender!=null)) {
            _data.writeInt(1);
            intentSender.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuccess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuccess(intentSender);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onFailure(java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFailure, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFailure(message);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.service.autofill.ISaveCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.service.autofill.ISaveCallback impl) {
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
    public static android.service.autofill.ISaveCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSuccess(android.content.IntentSender intentSender) throws android.os.RemoteException;
  public void onFailure(java.lang.CharSequence message) throws android.os.RemoteException;
}
