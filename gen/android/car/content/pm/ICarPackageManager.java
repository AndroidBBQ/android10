/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.content.pm;
/** @hide */
public interface ICarPackageManager extends android.os.IInterface
{
  /** Default implementation for ICarPackageManager. */
  public static class Default implements android.car.content.pm.ICarPackageManager
  {
    @Override public void setAppBlockingPolicy(java.lang.String packageName, android.car.content.pm.CarAppBlockingPolicy policy, int flags) throws android.os.RemoteException
    {
    }
    @Override public boolean isActivityDistractionOptimized(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isServiceDistractionOptimized(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isActivityBackedBySafeActivity(android.content.ComponentName activityName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setEnableActivityBlocking(boolean enable) throws android.os.RemoteException
    {
    }
    @Override public void restartTask(int taskId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.content.pm.ICarPackageManager
  {
    private static final java.lang.String DESCRIPTOR = "android.car.content.pm.ICarPackageManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.content.pm.ICarPackageManager interface,
     * generating a proxy if needed.
     */
    public static android.car.content.pm.ICarPackageManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.content.pm.ICarPackageManager))) {
        return ((android.car.content.pm.ICarPackageManager)iin);
      }
      return new android.car.content.pm.ICarPackageManager.Stub.Proxy(obj);
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
        case TRANSACTION_setAppBlockingPolicy:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.car.content.pm.CarAppBlockingPolicy _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.content.pm.CarAppBlockingPolicy.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.setAppBlockingPolicy(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isActivityDistractionOptimized:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isActivityDistractionOptimized(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isServiceDistractionOptimized:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isServiceDistractionOptimized(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isActivityBackedBySafeActivity:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isActivityBackedBySafeActivity(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setEnableActivityBlocking:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setEnableActivityBlocking(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restartTask:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.restartTask(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.content.pm.ICarPackageManager
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
      @Override public void setAppBlockingPolicy(java.lang.String packageName, android.car.content.pm.CarAppBlockingPolicy policy, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((policy!=null)) {
            _data.writeInt(1);
            policy.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppBlockingPolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAppBlockingPolicy(packageName, policy, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isActivityDistractionOptimized(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(className);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActivityDistractionOptimized, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActivityDistractionOptimized(packageName, className);
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
      @Override public boolean isServiceDistractionOptimized(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(className);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isServiceDistractionOptimized, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isServiceDistractionOptimized(packageName, className);
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
      @Override public boolean isActivityBackedBySafeActivity(android.content.ComponentName activityName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((activityName!=null)) {
            _data.writeInt(1);
            activityName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActivityBackedBySafeActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActivityBackedBySafeActivity(activityName);
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
      @Override public void setEnableActivityBlocking(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEnableActivityBlocking, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setEnableActivityBlocking(enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void restartTask(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restartTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restartTask(taskId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.content.pm.ICarPackageManager sDefaultImpl;
    }
    static final int TRANSACTION_setAppBlockingPolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isActivityDistractionOptimized = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isServiceDistractionOptimized = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isActivityBackedBySafeActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setEnableActivityBlocking = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_restartTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.car.content.pm.ICarPackageManager impl) {
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
    public static android.car.content.pm.ICarPackageManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setAppBlockingPolicy(java.lang.String packageName, android.car.content.pm.CarAppBlockingPolicy policy, int flags) throws android.os.RemoteException;
  public boolean isActivityDistractionOptimized(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException;
  public boolean isServiceDistractionOptimized(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException;
  public boolean isActivityBackedBySafeActivity(android.content.ComponentName activityName) throws android.os.RemoteException;
  public void setEnableActivityBlocking(boolean enable) throws android.os.RemoteException;
  public void restartTask(int taskId) throws android.os.RemoteException;
}
