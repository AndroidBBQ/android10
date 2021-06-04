/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
public interface IVoiceInteractionSessionShowCallback extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractionSessionShowCallback. */
  public static class Default implements com.android.internal.app.IVoiceInteractionSessionShowCallback
  {
    @Override public void onFailed() throws android.os.RemoteException
    {
    }
    @Override public void onShown() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IVoiceInteractionSessionShowCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IVoiceInteractionSessionShowCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IVoiceInteractionSessionShowCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IVoiceInteractionSessionShowCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IVoiceInteractionSessionShowCallback))) {
        return ((com.android.internal.app.IVoiceInteractionSessionShowCallback)iin);
      }
      return new com.android.internal.app.IVoiceInteractionSessionShowCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onFailed:
        {
          data.enforceInterface(descriptor);
          this.onFailed();
          return true;
        }
        case TRANSACTION_onShown:
        {
          data.enforceInterface(descriptor);
          this.onShown();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IVoiceInteractionSessionShowCallback
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
      @Override public void onFailed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFailed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onShown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.app.IVoiceInteractionSessionShowCallback sDefaultImpl;
    }
    static final int TRANSACTION_onFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.internal.app.IVoiceInteractionSessionShowCallback impl) {
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
    public static com.android.internal.app.IVoiceInteractionSessionShowCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onFailed() throws android.os.RemoteException;
  public void onShown() throws android.os.RemoteException;
}
