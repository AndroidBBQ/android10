/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * AIDL for the AudioService to signal audio focus listeners of focus updates.
 *
 * {@hide}
 */
public interface IAudioFocusDispatcher extends android.os.IInterface
{
  /** Default implementation for IAudioFocusDispatcher. */
  public static class Default implements android.media.IAudioFocusDispatcher
  {
    @Override public void dispatchAudioFocusChange(int focusChange, java.lang.String clientId) throws android.os.RemoteException
    {
    }
    @Override public void dispatchFocusResultFromExtPolicy(int requestResult, java.lang.String clientId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IAudioFocusDispatcher
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IAudioFocusDispatcher";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IAudioFocusDispatcher interface,
     * generating a proxy if needed.
     */
    public static android.media.IAudioFocusDispatcher asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IAudioFocusDispatcher))) {
        return ((android.media.IAudioFocusDispatcher)iin);
      }
      return new android.media.IAudioFocusDispatcher.Stub.Proxy(obj);
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
        case TRANSACTION_dispatchAudioFocusChange:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.dispatchAudioFocusChange(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dispatchFocusResultFromExtPolicy:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.dispatchFocusResultFromExtPolicy(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IAudioFocusDispatcher
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
      @Override public void dispatchAudioFocusChange(int focusChange, java.lang.String clientId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(focusChange);
          _data.writeString(clientId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchAudioFocusChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchAudioFocusChange(focusChange, clientId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchFocusResultFromExtPolicy(int requestResult, java.lang.String clientId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(requestResult);
          _data.writeString(clientId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchFocusResultFromExtPolicy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchFocusResultFromExtPolicy(requestResult, clientId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IAudioFocusDispatcher sDefaultImpl;
    }
    static final int TRANSACTION_dispatchAudioFocusChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_dispatchFocusResultFromExtPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.media.IAudioFocusDispatcher impl) {
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
    public static android.media.IAudioFocusDispatcher getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IAudioFocusDispatcher.aidl:26:1:26:25")
  public void dispatchAudioFocusChange(int focusChange, java.lang.String clientId) throws android.os.RemoteException;
  public void dispatchFocusResultFromExtPolicy(int requestResult, java.lang.String clientId) throws android.os.RemoteException;
}
