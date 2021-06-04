/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * An interface used by the Context Hub Service to invoke callbacks for lifecycle notifications of a
 * Context Hub and nanoapps, as well as for nanoapp messaging.
 *
 * @hide
 */
public interface IContextHubClientCallback extends android.os.IInterface
{
  /** Default implementation for IContextHubClientCallback. */
  public static class Default implements android.hardware.location.IContextHubClientCallback
  {
    // Callback invoked when receiving a message from a nanoapp.

    @Override public void onMessageFromNanoApp(android.hardware.location.NanoAppMessage message) throws android.os.RemoteException
    {
    }
    // Callback invoked when the attached Context Hub has reset.

    @Override public void onHubReset() throws android.os.RemoteException
    {
    }
    // Callback invoked when a nanoapp aborts at the attached Context Hub.

    @Override public void onNanoAppAborted(long nanoAppId, int abortCode) throws android.os.RemoteException
    {
    }
    // Callback invoked when a nanoapp is loaded at the attached Context Hub.

    @Override public void onNanoAppLoaded(long nanoAppId) throws android.os.RemoteException
    {
    }
    // Callback invoked when a nanoapp is unloaded from the attached Context Hub.

    @Override public void onNanoAppUnloaded(long nanoAppId) throws android.os.RemoteException
    {
    }
    // Callback invoked when a nanoapp is enabled at the attached Context Hub.

    @Override public void onNanoAppEnabled(long nanoAppId) throws android.os.RemoteException
    {
    }
    // Callback invoked when a nanoapp is disabled at the attached Context Hub.

    @Override public void onNanoAppDisabled(long nanoAppId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IContextHubClientCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IContextHubClientCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IContextHubClientCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IContextHubClientCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IContextHubClientCallback))) {
        return ((android.hardware.location.IContextHubClientCallback)iin);
      }
      return new android.hardware.location.IContextHubClientCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onMessageFromNanoApp:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.NanoAppMessage _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.location.NanoAppMessage.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onMessageFromNanoApp(_arg0);
          return true;
        }
        case TRANSACTION_onHubReset:
        {
          data.enforceInterface(descriptor);
          this.onHubReset();
          return true;
        }
        case TRANSACTION_onNanoAppAborted:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.onNanoAppAborted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onNanoAppLoaded:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onNanoAppLoaded(_arg0);
          return true;
        }
        case TRANSACTION_onNanoAppUnloaded:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onNanoAppUnloaded(_arg0);
          return true;
        }
        case TRANSACTION_onNanoAppEnabled:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onNanoAppEnabled(_arg0);
          return true;
        }
        case TRANSACTION_onNanoAppDisabled:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onNanoAppDisabled(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IContextHubClientCallback
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
      // Callback invoked when receiving a message from a nanoapp.

      @Override public void onMessageFromNanoApp(android.hardware.location.NanoAppMessage message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((message!=null)) {
            _data.writeInt(1);
            message.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageFromNanoApp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageFromNanoApp(message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback invoked when the attached Context Hub has reset.

      @Override public void onHubReset() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHubReset, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHubReset();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback invoked when a nanoapp aborts at the attached Context Hub.

      @Override public void onNanoAppAborted(long nanoAppId, int abortCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(nanoAppId);
          _data.writeInt(abortCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNanoAppAborted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNanoAppAborted(nanoAppId, abortCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback invoked when a nanoapp is loaded at the attached Context Hub.

      @Override public void onNanoAppLoaded(long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNanoAppLoaded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNanoAppLoaded(nanoAppId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback invoked when a nanoapp is unloaded from the attached Context Hub.

      @Override public void onNanoAppUnloaded(long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNanoAppUnloaded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNanoAppUnloaded(nanoAppId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback invoked when a nanoapp is enabled at the attached Context Hub.

      @Override public void onNanoAppEnabled(long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNanoAppEnabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNanoAppEnabled(nanoAppId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Callback invoked when a nanoapp is disabled at the attached Context Hub.

      @Override public void onNanoAppDisabled(long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNanoAppDisabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNanoAppDisabled(nanoAppId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.location.IContextHubClientCallback sDefaultImpl;
    }
    static final int TRANSACTION_onMessageFromNanoApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onHubReset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onNanoAppAborted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onNanoAppLoaded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onNanoAppUnloaded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onNanoAppEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onNanoAppDisabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.hardware.location.IContextHubClientCallback impl) {
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
    public static android.hardware.location.IContextHubClientCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Callback invoked when receiving a message from a nanoapp.

  public void onMessageFromNanoApp(android.hardware.location.NanoAppMessage message) throws android.os.RemoteException;
  // Callback invoked when the attached Context Hub has reset.

  public void onHubReset() throws android.os.RemoteException;
  // Callback invoked when a nanoapp aborts at the attached Context Hub.

  public void onNanoAppAborted(long nanoAppId, int abortCode) throws android.os.RemoteException;
  // Callback invoked when a nanoapp is loaded at the attached Context Hub.

  public void onNanoAppLoaded(long nanoAppId) throws android.os.RemoteException;
  // Callback invoked when a nanoapp is unloaded from the attached Context Hub.

  public void onNanoAppUnloaded(long nanoAppId) throws android.os.RemoteException;
  // Callback invoked when a nanoapp is enabled at the attached Context Hub.

  public void onNanoAppEnabled(long nanoAppId) throws android.os.RemoteException;
  // Callback invoked when a nanoapp is disabled at the attached Context Hub.

  public void onNanoAppDisabled(long nanoAppId) throws android.os.RemoteException;
}
