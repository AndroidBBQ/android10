/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/** @hide */
public interface IHardwarePropertiesManager extends android.os.IInterface
{
  /** Default implementation for IHardwarePropertiesManager. */
  public static class Default implements android.os.IHardwarePropertiesManager
  {
    @Override public float[] getDeviceTemperatures(java.lang.String callingPackage, int type, int source) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.CpuUsageInfo[] getCpuUsages(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public float[] getFanSpeeds(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IHardwarePropertiesManager
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IHardwarePropertiesManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IHardwarePropertiesManager interface,
     * generating a proxy if needed.
     */
    public static android.os.IHardwarePropertiesManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IHardwarePropertiesManager))) {
        return ((android.os.IHardwarePropertiesManager)iin);
      }
      return new android.os.IHardwarePropertiesManager.Stub.Proxy(obj);
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
        case TRANSACTION_getDeviceTemperatures:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          float[] _result = this.getDeviceTemperatures(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeFloatArray(_result);
          return true;
        }
        case TRANSACTION_getCpuUsages:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.CpuUsageInfo[] _result = this.getCpuUsages(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getFanSpeeds:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          float[] _result = this.getFanSpeeds(_arg0);
          reply.writeNoException();
          reply.writeFloatArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IHardwarePropertiesManager
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
      @Override public float[] getDeviceTemperatures(java.lang.String callingPackage, int type, int source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(type);
          _data.writeInt(source);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceTemperatures, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDeviceTemperatures(callingPackage, type, source);
          }
          _reply.readException();
          _result = _reply.createFloatArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.CpuUsageInfo[] getCpuUsages(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.CpuUsageInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCpuUsages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCpuUsages(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.os.CpuUsageInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public float[] getFanSpeeds(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFanSpeeds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFanSpeeds(callingPackage);
          }
          _reply.readException();
          _result = _reply.createFloatArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.IHardwarePropertiesManager sDefaultImpl;
    }
    static final int TRANSACTION_getDeviceTemperatures = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getCpuUsages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getFanSpeeds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.os.IHardwarePropertiesManager impl) {
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
    public static android.os.IHardwarePropertiesManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public float[] getDeviceTemperatures(java.lang.String callingPackage, int type, int source) throws android.os.RemoteException;
  public android.os.CpuUsageInfo[] getCpuUsages(java.lang.String callingPackage) throws android.os.RemoteException;
  public float[] getFanSpeeds(java.lang.String callingPackage) throws android.os.RemoteException;
}
