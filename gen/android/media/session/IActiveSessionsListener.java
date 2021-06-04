/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.session;
/**
 * Listens for changes to the list of active sessions.
 * @hide
 */
public interface IActiveSessionsListener extends android.os.IInterface
{
  /** Default implementation for IActiveSessionsListener. */
  public static class Default implements android.media.session.IActiveSessionsListener
  {
    @Override public void onActiveSessionsChanged(java.util.List<android.media.session.MediaSession.Token> sessions) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.session.IActiveSessionsListener
  {
    private static final java.lang.String DESCRIPTOR = "android.media.session.IActiveSessionsListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.session.IActiveSessionsListener interface,
     * generating a proxy if needed.
     */
    public static android.media.session.IActiveSessionsListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.session.IActiveSessionsListener))) {
        return ((android.media.session.IActiveSessionsListener)iin);
      }
      return new android.media.session.IActiveSessionsListener.Stub.Proxy(obj);
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
        case TRANSACTION_onActiveSessionsChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.session.MediaSession.Token> _arg0;
          _arg0 = data.createTypedArrayList(android.media.session.MediaSession.Token.CREATOR);
          this.onActiveSessionsChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.session.IActiveSessionsListener
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
      @Override public void onActiveSessionsChanged(java.util.List<android.media.session.MediaSession.Token> sessions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(sessions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onActiveSessionsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onActiveSessionsChanged(sessions);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.session.IActiveSessionsListener sDefaultImpl;
    }
    static final int TRANSACTION_onActiveSessionsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.session.IActiveSessionsListener impl) {
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
    public static android.media.session.IActiveSessionsListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onActiveSessionsChanged(java.util.List<android.media.session.MediaSession.Token> sessions) throws android.os.RemoteException;
}
