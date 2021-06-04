/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * {@hide}
 */
public interface IGnssStatusListener extends android.os.IInterface
{
  /** Default implementation for IGnssStatusListener. */
  public static class Default implements android.location.IGnssStatusListener
  {
    @Override public void onGnssStarted() throws android.os.RemoteException
    {
    }
    @Override public void onGnssStopped() throws android.os.RemoteException
    {
    }
    @Override public void onFirstFix(int ttff) throws android.os.RemoteException
    {
    }
    @Override public void onSvStatusChanged(int svCount, int[] svidWithFlags, float[] cn0s, float[] elevations, float[] azimuths, float[] carrierFreqs) throws android.os.RemoteException
    {
    }
    @Override public void onNmeaReceived(long timestamp, java.lang.String nmea) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.IGnssStatusListener
  {
    private static final java.lang.String DESCRIPTOR = "android.location.IGnssStatusListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.IGnssStatusListener interface,
     * generating a proxy if needed.
     */
    public static android.location.IGnssStatusListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.IGnssStatusListener))) {
        return ((android.location.IGnssStatusListener)iin);
      }
      return new android.location.IGnssStatusListener.Stub.Proxy(obj);
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
        case TRANSACTION_onGnssStarted:
        {
          data.enforceInterface(descriptor);
          this.onGnssStarted();
          return true;
        }
        case TRANSACTION_onGnssStopped:
        {
          data.enforceInterface(descriptor);
          this.onGnssStopped();
          return true;
        }
        case TRANSACTION_onFirstFix:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onFirstFix(_arg0);
          return true;
        }
        case TRANSACTION_onSvStatusChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _arg1;
          _arg1 = data.createIntArray();
          float[] _arg2;
          _arg2 = data.createFloatArray();
          float[] _arg3;
          _arg3 = data.createFloatArray();
          float[] _arg4;
          _arg4 = data.createFloatArray();
          float[] _arg5;
          _arg5 = data.createFloatArray();
          this.onSvStatusChanged(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onNmeaReceived:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onNmeaReceived(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.IGnssStatusListener
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
      @Override public void onGnssStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGnssStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGnssStarted();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGnssStopped() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGnssStopped, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGnssStopped();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFirstFix(int ttff) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(ttff);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFirstFix, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFirstFix(ttff);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSvStatusChanged(int svCount, int[] svidWithFlags, float[] cn0s, float[] elevations, float[] azimuths, float[] carrierFreqs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(svCount);
          _data.writeIntArray(svidWithFlags);
          _data.writeFloatArray(cn0s);
          _data.writeFloatArray(elevations);
          _data.writeFloatArray(azimuths);
          _data.writeFloatArray(carrierFreqs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSvStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSvStatusChanged(svCount, svidWithFlags, cn0s, elevations, azimuths, carrierFreqs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNmeaReceived(long timestamp, java.lang.String nmea) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          _data.writeString(nmea);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNmeaReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNmeaReceived(timestamp, nmea);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.location.IGnssStatusListener sDefaultImpl;
    }
    static final int TRANSACTION_onGnssStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGnssStopped = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onFirstFix = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSvStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onNmeaReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.location.IGnssStatusListener impl) {
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
    public static android.location.IGnssStatusListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onGnssStarted() throws android.os.RemoteException;
  public void onGnssStopped() throws android.os.RemoteException;
  public void onFirstFix(int ttff) throws android.os.RemoteException;
  public void onSvStatusChanged(int svCount, int[] svidWithFlags, float[] cn0s, float[] elevations, float[] azimuths, float[] carrierFreqs) throws android.os.RemoteException;
  public void onNmeaReceived(long timestamp, java.lang.String nmea) throws android.os.RemoteException;
}
