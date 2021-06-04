/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * {@hide}
 */
public interface ICountryListener extends android.os.IInterface
{
  /** Default implementation for ICountryListener. */
  public static class Default implements android.location.ICountryListener
  {
    @Override public void onCountryDetected(android.location.Country country) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.ICountryListener
  {
    private static final java.lang.String DESCRIPTOR = "android.location.ICountryListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.ICountryListener interface,
     * generating a proxy if needed.
     */
    public static android.location.ICountryListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.ICountryListener))) {
        return ((android.location.ICountryListener)iin);
      }
      return new android.location.ICountryListener.Stub.Proxy(obj);
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
        case TRANSACTION_onCountryDetected:
        {
          data.enforceInterface(descriptor);
          android.location.Country _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.Country.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onCountryDetected(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.ICountryListener
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
      @Override public void onCountryDetected(android.location.Country country) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((country!=null)) {
            _data.writeInt(1);
            country.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCountryDetected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCountryDetected(country);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.location.ICountryListener sDefaultImpl;
    }
    static final int TRANSACTION_onCountryDetected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.location.ICountryListener impl) {
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
    public static android.location.ICountryListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onCountryDetected(android.location.Country country) throws android.os.RemoteException;
}
