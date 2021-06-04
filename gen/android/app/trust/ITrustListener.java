/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.trust;
/**
 * Private API to be notified about trust changes.
 *
 * {@hide}
 */
public interface ITrustListener extends android.os.IInterface
{
  /** Default implementation for ITrustListener. */
  public static class Default implements android.app.trust.ITrustListener
  {
    @Override public void onTrustChanged(boolean enabled, int userId, int flags) throws android.os.RemoteException
    {
    }
    @Override public void onTrustManagedChanged(boolean managed, int userId) throws android.os.RemoteException
    {
    }
    @Override public void onTrustError(java.lang.CharSequence message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.trust.ITrustListener
  {
    private static final java.lang.String DESCRIPTOR = "android.app.trust.ITrustListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.trust.ITrustListener interface,
     * generating a proxy if needed.
     */
    public static android.app.trust.ITrustListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.trust.ITrustListener))) {
        return ((android.app.trust.ITrustListener)iin);
      }
      return new android.app.trust.ITrustListener.Stub.Proxy(obj);
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
        case TRANSACTION_onTrustChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onTrustChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onTrustManagedChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onTrustManagedChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onTrustError:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onTrustError(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.trust.ITrustListener
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
      @Override public void onTrustChanged(boolean enabled, int userId, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeInt(userId);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustChanged(enabled, userId, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTrustManagedChanged(boolean managed, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((managed)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustManagedChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustManagedChanged(managed, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTrustError(java.lang.CharSequence message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (message!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(message, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustError(message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.trust.ITrustListener sDefaultImpl;
    }
    static final int TRANSACTION_onTrustChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onTrustManagedChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onTrustError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.app.trust.ITrustListener impl) {
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
    public static android.app.trust.ITrustListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onTrustChanged(boolean enabled, int userId, int flags) throws android.os.RemoteException;
  public void onTrustManagedChanged(boolean managed, int userId) throws android.os.RemoteException;
  public void onTrustError(java.lang.CharSequence message) throws android.os.RemoteException;
}
