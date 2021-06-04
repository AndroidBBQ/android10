/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.lowpan;
/** {@hide} */
public interface ILowpanManager extends android.os.IInterface
{
  /** Default implementation for ILowpanManager. */
  public static class Default implements android.net.lowpan.ILowpanManager
  {
    @Override public android.net.lowpan.ILowpanInterface getInterface(java.lang.String name) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getInterfaceList() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addListener(android.net.lowpan.ILowpanManagerListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeListener(android.net.lowpan.ILowpanManagerListener listener) throws android.os.RemoteException
    {
    }
    @Override public void addInterface(android.net.lowpan.ILowpanInterface lowpan_interface) throws android.os.RemoteException
    {
    }
    @Override public void removeInterface(android.net.lowpan.ILowpanInterface lowpan_interface) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.lowpan.ILowpanManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.lowpan.ILowpanManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.lowpan.ILowpanManager interface,
     * generating a proxy if needed.
     */
    public static android.net.lowpan.ILowpanManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.lowpan.ILowpanManager))) {
        return ((android.net.lowpan.ILowpanManager)iin);
      }
      return new android.net.lowpan.ILowpanManager.Stub.Proxy(obj);
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
        case TRANSACTION_getInterface:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.lowpan.ILowpanInterface _result = this.getInterface(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getInterfaceList:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getInterfaceList();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_addListener:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanManagerListener _arg0;
          _arg0 = android.net.lowpan.ILowpanManagerListener.Stub.asInterface(data.readStrongBinder());
          this.addListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeListener:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanManagerListener _arg0;
          _arg0 = android.net.lowpan.ILowpanManagerListener.Stub.asInterface(data.readStrongBinder());
          this.removeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addInterface:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanInterface _arg0;
          _arg0 = android.net.lowpan.ILowpanInterface.Stub.asInterface(data.readStrongBinder());
          this.addInterface(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeInterface:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanInterface _arg0;
          _arg0 = android.net.lowpan.ILowpanInterface.Stub.asInterface(data.readStrongBinder());
          this.removeInterface(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.lowpan.ILowpanManager
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
      @Override public android.net.lowpan.ILowpanInterface getInterface(java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.lowpan.ILowpanInterface _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInterface(name);
          }
          _reply.readException();
          _result = android.net.lowpan.ILowpanInterface.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getInterfaceList() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInterfaceList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInterfaceList();
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
      @Override public void addListener(android.net.lowpan.ILowpanManagerListener listener) throws android.os.RemoteException
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
      @Override public void removeListener(android.net.lowpan.ILowpanManagerListener listener) throws android.os.RemoteException
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
      @Override public void addInterface(android.net.lowpan.ILowpanInterface lowpan_interface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((lowpan_interface!=null))?(lowpan_interface.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addInterface(lowpan_interface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeInterface(android.net.lowpan.ILowpanInterface lowpan_interface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((lowpan_interface!=null))?(lowpan_interface.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeInterface(lowpan_interface);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.lowpan.ILowpanManager sDefaultImpl;
    }
    static final int TRANSACTION_getInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getInterfaceList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_addListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_removeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_addInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.net.lowpan.ILowpanManager impl) {
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
    public static android.net.lowpan.ILowpanManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /* Keep this in sync with Context.LOWPAN_SERVICE */
  public static final String LOWPAN_SERVICE_NAME = "lowpan";
  public android.net.lowpan.ILowpanInterface getInterface(java.lang.String name) throws android.os.RemoteException;
  public java.lang.String[] getInterfaceList() throws android.os.RemoteException;
  public void addListener(android.net.lowpan.ILowpanManagerListener listener) throws android.os.RemoteException;
  public void removeListener(android.net.lowpan.ILowpanManagerListener listener) throws android.os.RemoteException;
  public void addInterface(android.net.lowpan.ILowpanInterface lowpan_interface) throws android.os.RemoteException;
  public void removeInterface(android.net.lowpan.ILowpanInterface lowpan_interface) throws android.os.RemoteException;
}
