/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware;
/** {@hide} */
public interface IConsumerIrService extends android.os.IInterface
{
  /** Default implementation for IConsumerIrService. */
  public static class Default implements android.hardware.IConsumerIrService
  {
    @Override public boolean hasIrEmitter() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void transmit(java.lang.String packageName, int carrierFrequency, int[] pattern) throws android.os.RemoteException
    {
    }
    @Override public int[] getCarrierFrequencies() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.IConsumerIrService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.IConsumerIrService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.IConsumerIrService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.IConsumerIrService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.IConsumerIrService))) {
        return ((android.hardware.IConsumerIrService)iin);
      }
      return new android.hardware.IConsumerIrService.Stub.Proxy(obj);
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
        case TRANSACTION_hasIrEmitter:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasIrEmitter();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_transmit:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int[] _arg2;
          _arg2 = data.createIntArray();
          this.transmit(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCarrierFrequencies:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getCarrierFrequencies();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.IConsumerIrService
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
      @Override public boolean hasIrEmitter() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasIrEmitter, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasIrEmitter();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void transmit(java.lang.String packageName, int carrierFrequency, int[] pattern) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(carrierFrequency);
          _data.writeIntArray(pattern);
          boolean _status = mRemote.transact(Stub.TRANSACTION_transmit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().transmit(packageName, carrierFrequency, pattern);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getCarrierFrequencies() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierFrequencies, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarrierFrequencies();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.hardware.IConsumerIrService sDefaultImpl;
    }
    static final int TRANSACTION_hasIrEmitter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_transmit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCarrierFrequencies = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.hardware.IConsumerIrService impl) {
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
    public static android.hardware.IConsumerIrService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean hasIrEmitter() throws android.os.RemoteException;
  public void transmit(java.lang.String packageName, int carrierFrequency, int[] pattern) throws android.os.RemoteException;
  public int[] getCarrierFrequencies() throws android.os.RemoteException;
}
