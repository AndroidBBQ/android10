/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.fingerprint;
/**
 * Callback when clients become active or inactive.
 * @hide
 */
public interface IFingerprintClientActiveCallback extends android.os.IInterface
{
  /** Default implementation for IFingerprintClientActiveCallback. */
  public static class Default implements android.hardware.fingerprint.IFingerprintClientActiveCallback
  {
    @Override public void onClientActiveChanged(boolean isActive) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.fingerprint.IFingerprintClientActiveCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.fingerprint.IFingerprintClientActiveCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.fingerprint.IFingerprintClientActiveCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.fingerprint.IFingerprintClientActiveCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.fingerprint.IFingerprintClientActiveCallback))) {
        return ((android.hardware.fingerprint.IFingerprintClientActiveCallback)iin);
      }
      return new android.hardware.fingerprint.IFingerprintClientActiveCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onClientActiveChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onClientActiveChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.fingerprint.IFingerprintClientActiveCallback
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
      @Override public void onClientActiveChanged(boolean isActive) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isActive)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onClientActiveChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onClientActiveChanged(isActive);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.fingerprint.IFingerprintClientActiveCallback sDefaultImpl;
    }
    static final int TRANSACTION_onClientActiveChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.fingerprint.IFingerprintClientActiveCallback impl) {
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
    public static android.hardware.fingerprint.IFingerprintClientActiveCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onClientActiveChanged(boolean isActive) throws android.os.RemoteException;
}
