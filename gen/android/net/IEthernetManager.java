/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * Interface that answers queries about, and allows changing
 * ethernet configuration.
 *//** {@hide} */
public interface IEthernetManager extends android.os.IInterface
{
  /** Default implementation for IEthernetManager. */
  public static class Default implements android.net.IEthernetManager
  {
    @Override public java.lang.String[] getAvailableInterfaces() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.IpConfiguration getConfiguration(java.lang.String iface) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setConfiguration(java.lang.String iface, android.net.IpConfiguration config) throws android.os.RemoteException
    {
    }
    @Override public boolean isAvailable(java.lang.String iface) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void addListener(android.net.IEthernetServiceListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeListener(android.net.IEthernetServiceListener listener) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.IEthernetManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.IEthernetManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.IEthernetManager interface,
     * generating a proxy if needed.
     */
    public static android.net.IEthernetManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.IEthernetManager))) {
        return ((android.net.IEthernetManager)iin);
      }
      return new android.net.IEthernetManager.Stub.Proxy(obj);
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
        case TRANSACTION_getAvailableInterfaces:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getAvailableInterfaces();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getConfiguration:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.IpConfiguration _result = this.getConfiguration(_arg0);
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
        case TRANSACTION_setConfiguration:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.IpConfiguration _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.IpConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setConfiguration(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isAvailable:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isAvailable(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addListener:
        {
          data.enforceInterface(descriptor);
          android.net.IEthernetServiceListener _arg0;
          _arg0 = android.net.IEthernetServiceListener.Stub.asInterface(data.readStrongBinder());
          this.addListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeListener:
        {
          data.enforceInterface(descriptor);
          android.net.IEthernetServiceListener _arg0;
          _arg0 = android.net.IEthernetServiceListener.Stub.asInterface(data.readStrongBinder());
          this.removeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.IEthernetManager
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
      @Override public java.lang.String[] getAvailableInterfaces() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableInterfaces, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableInterfaces();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.IpConfiguration getConfiguration(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.IpConfiguration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getConfiguration(iface);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.IpConfiguration.CREATOR.createFromParcel(_reply);
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
      @Override public void setConfiguration(java.lang.String iface, android.net.IpConfiguration config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConfiguration(iface, config);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isAvailable(java.lang.String iface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAvailable(iface);
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
      @Override public void addListener(android.net.IEthernetServiceListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeListener(android.net.IEthernetServiceListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.IEthernetManager sDefaultImpl;
    }
    static final int TRANSACTION_getAvailableInterfaces = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_addListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.net.IEthernetManager impl) {
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
    public static android.net.IEthernetManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.lang.String[] getAvailableInterfaces() throws android.os.RemoteException;
  public android.net.IpConfiguration getConfiguration(java.lang.String iface) throws android.os.RemoteException;
  public void setConfiguration(java.lang.String iface, android.net.IpConfiguration config) throws android.os.RemoteException;
  public boolean isAvailable(java.lang.String iface) throws android.os.RemoteException;
  public void addListener(android.net.IEthernetServiceListener listener) throws android.os.RemoteException;
  public void removeListener(android.net.IEthernetServiceListener listener) throws android.os.RemoteException;
}
