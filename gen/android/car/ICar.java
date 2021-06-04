/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/** @hide */
public interface ICar extends android.os.IInterface
{
  /** Default implementation for ICar. */
  public static class Default implements android.car.ICar
  {
    // All oneway methods are called from system server and should be placed in top positions.
    // Do not change the order of oneway methods as system server make binder call based on this
    // order.
    /**
         * IBinder is ICarServiceHelper but passed as IBinder due to aidl hidden.
         *
         * This should be the 1st method. Do not change the order.
         */
    @Override public void setCarServiceHelper(android.os.IBinder helper) throws android.os.RemoteException
    {
    }
    /**
         * Notify lock / unlock of user id to car service.
         * unlocked: 1 if unlocked 0 otherwise.
         *
         * This should be the 2nd method. Do not change the order.
         */
    @Override public void setUserLockStatus(int userHandle, int unlocked) throws android.os.RemoteException
    {
    }
    /**
         * Notify of user switching.  This is called only for foreground users when the user is starting
         * to boot.
         *
         * @param userHandle -  user handle of new user.
         *
         * This should be the 3rd method. Do not change the order.
         */
    @Override public void onSwitchUser(int userHandle) throws android.os.RemoteException
    {
    }
    @Override public android.os.IBinder getCarService(java.lang.String serviceName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getCarConnectionType() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.ICar
  {
    private static final java.lang.String DESCRIPTOR = "android.car.ICar";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.ICar interface,
     * generating a proxy if needed.
     */
    public static android.car.ICar asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.ICar))) {
        return ((android.car.ICar)iin);
      }
      return new android.car.ICar.Stub.Proxy(obj);
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
        case TRANSACTION_setCarServiceHelper:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.setCarServiceHelper(_arg0);
          return true;
        }
        case TRANSACTION_setUserLockStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setUserLockStatus(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSwitchUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSwitchUser(_arg0);
          return true;
        }
        case TRANSACTION_getCarService:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _result = this.getCarService(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_getCarConnectionType:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCarConnectionType();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.ICar
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
      // All oneway methods are called from system server and should be placed in top positions.
      // Do not change the order of oneway methods as system server make binder call based on this
      // order.
      /**
           * IBinder is ICarServiceHelper but passed as IBinder due to aidl hidden.
           *
           * This should be the 1st method. Do not change the order.
           */
      @Override public void setCarServiceHelper(android.os.IBinder helper) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(helper);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCarServiceHelper, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCarServiceHelper(helper);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notify lock / unlock of user id to car service.
           * unlocked: 1 if unlocked 0 otherwise.
           *
           * This should be the 2nd method. Do not change the order.
           */
      @Override public void setUserLockStatus(int userHandle, int unlocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          _data.writeInt(unlocked);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserLockStatus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserLockStatus(userHandle, unlocked);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notify of user switching.  This is called only for foreground users when the user is starting
           * to boot.
           *
           * @param userHandle -  user handle of new user.
           *
           * This should be the 3rd method. Do not change the order.
           */
      @Override public void onSwitchUser(int userHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSwitchUser, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSwitchUser(userHandle);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public android.os.IBinder getCarService(java.lang.String serviceName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(serviceName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarService(serviceName);
          }
          _reply.readException();
          _result = _reply.readStrongBinder();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getCarConnectionType() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarConnectionType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarConnectionType();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.car.ICar sDefaultImpl;
    }
    static final int TRANSACTION_setCarServiceHelper = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setUserLockStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSwitchUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getCarService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getCarConnectionType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.car.ICar impl) {
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
    public static android.car.ICar getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // All oneway methods are called from system server and should be placed in top positions.
  // Do not change the order of oneway methods as system server make binder call based on this
  // order.
  /**
       * IBinder is ICarServiceHelper but passed as IBinder due to aidl hidden.
       *
       * This should be the 1st method. Do not change the order.
       */
  public void setCarServiceHelper(android.os.IBinder helper) throws android.os.RemoteException;
  /**
       * Notify lock / unlock of user id to car service.
       * unlocked: 1 if unlocked 0 otherwise.
       *
       * This should be the 2nd method. Do not change the order.
       */
  public void setUserLockStatus(int userHandle, int unlocked) throws android.os.RemoteException;
  /**
       * Notify of user switching.  This is called only for foreground users when the user is starting
       * to boot.
       *
       * @param userHandle -  user handle of new user.
       *
       * This should be the 3rd method. Do not change the order.
       */
  public void onSwitchUser(int userHandle) throws android.os.RemoteException;
  public android.os.IBinder getCarService(java.lang.String serviceName) throws android.os.RemoteException;
  public int getCarConnectionType() throws android.os.RemoteException;
}
