/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.onemedia;
public interface IPlayerCallback extends android.os.IInterface
{
  /** Default implementation for IPlayerCallback. */
  public static class Default implements com.android.onemedia.IPlayerCallback
  {
    @Override public void onSessionChanged(android.media.session.MediaSession.Token session) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.onemedia.IPlayerCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.onemedia.IPlayerCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.onemedia.IPlayerCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.onemedia.IPlayerCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.onemedia.IPlayerCallback))) {
        return ((com.android.onemedia.IPlayerCallback)iin);
      }
      return new com.android.onemedia.IPlayerCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSessionChanged:
        {
          data.enforceInterface(descriptor);
          android.media.session.MediaSession.Token _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.session.MediaSession.Token.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSessionChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.onemedia.IPlayerCallback
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
      @Override public void onSessionChanged(android.media.session.MediaSession.Token session) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((session!=null)) {
            _data.writeInt(1);
            session.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionChanged(session);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.onemedia.IPlayerCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSessionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.onemedia.IPlayerCallback impl) {
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
    public static com.android.onemedia.IPlayerCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSessionChanged(android.media.session.MediaSession.Token session) throws android.os.RemoteException;
}
