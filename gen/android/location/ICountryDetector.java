/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * The API for detecting the country where the user is.
 *
 * {@hide}
 */
public interface ICountryDetector extends android.os.IInterface
{
  /** Default implementation for ICountryDetector. */
  public static class Default implements android.location.ICountryDetector
  {
    /**
         * Start detecting the country that the user is in.
         * @return the country if it is available immediately, otherwise null will be returned.
         */
    @Override public android.location.Country detectCountry() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Add a listener to receive the notification when the country is detected or changed.
         */
    @Override public void addCountryListener(android.location.ICountryListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Remove the listener
         */
    @Override public void removeCountryListener(android.location.ICountryListener listener) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.ICountryDetector
  {
    private static final java.lang.String DESCRIPTOR = "android.location.ICountryDetector";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.ICountryDetector interface,
     * generating a proxy if needed.
     */
    public static android.location.ICountryDetector asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.ICountryDetector))) {
        return ((android.location.ICountryDetector)iin);
      }
      return new android.location.ICountryDetector.Stub.Proxy(obj);
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
        case TRANSACTION_detectCountry:
        {
          data.enforceInterface(descriptor);
          android.location.Country _result = this.detectCountry();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_addCountryListener:
        {
          data.enforceInterface(descriptor);
          android.location.ICountryListener _arg0;
          _arg0 = android.location.ICountryListener.Stub.asInterface(data.readStrongBinder());
          this.addCountryListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeCountryListener:
        {
          data.enforceInterface(descriptor);
          android.location.ICountryListener _arg0;
          _arg0 = android.location.ICountryListener.Stub.asInterface(data.readStrongBinder());
          this.removeCountryListener(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.ICountryDetector
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
      /**
           * Start detecting the country that the user is in.
           * @return the country if it is available immediately, otherwise null will be returned.
           */
      @Override public android.location.Country detectCountry() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.location.Country _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_detectCountry, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().detectCountry();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.location.Country.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Add a listener to receive the notification when the country is detected or changed.
           */
      @Override public void addCountryListener(android.location.ICountryListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addCountryListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addCountryListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Remove the listener
           */
      @Override public void removeCountryListener(android.location.ICountryListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeCountryListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeCountryListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.location.ICountryDetector sDefaultImpl;
    }
    static final int TRANSACTION_detectCountry = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addCountryListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeCountryListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.location.ICountryDetector impl) {
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
    public static android.location.ICountryDetector getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Start detecting the country that the user is in.
       * @return the country if it is available immediately, otherwise null will be returned.
       */
  public android.location.Country detectCountry() throws android.os.RemoteException;
  /**
       * Add a listener to receive the notification when the country is detected or changed.
       */
  public void addCountryListener(android.location.ICountryListener listener) throws android.os.RemoteException;
  /**
       * Remove the listener
       */
  public void removeCountryListener(android.location.ICountryListener listener) throws android.os.RemoteException;
}
