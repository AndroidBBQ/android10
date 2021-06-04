/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * System private API for talking with the alarm manager service.
 *
 * {@hide}
 */
public interface IAlarmManager extends android.os.IInterface
{
  /** Default implementation for IAlarmManager. */
  public static class Default implements android.app.IAlarmManager
  {
    /** windowLength == 0 means exact; windowLength < 0 means the let the OS decide */
    @Override public void set(java.lang.String callingPackage, int type, long triggerAtTime, long windowLength, long interval, int flags, android.app.PendingIntent operation, android.app.IAlarmListener listener, java.lang.String listenerTag, android.os.WorkSource workSource, android.app.AlarmManager.AlarmClockInfo alarmClock) throws android.os.RemoteException
    {
    }
    @Override public boolean setTime(long millis) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setTimeZone(java.lang.String zone) throws android.os.RemoteException
    {
    }
    @Override public void remove(android.app.PendingIntent operation, android.app.IAlarmListener listener) throws android.os.RemoteException
    {
    }
    @Override public long getNextWakeFromIdleTime() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public android.app.AlarmManager.AlarmClockInfo getNextAlarmClock(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public long currentNetworkTimeMillis() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IAlarmManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IAlarmManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IAlarmManager interface,
     * generating a proxy if needed.
     */
    public static android.app.IAlarmManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IAlarmManager))) {
        return ((android.app.IAlarmManager)iin);
      }
      return new android.app.IAlarmManager.Stub.Proxy(obj);
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
        case TRANSACTION_set:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          long _arg3;
          _arg3 = data.readLong();
          long _arg4;
          _arg4 = data.readLong();
          int _arg5;
          _arg5 = data.readInt();
          android.app.PendingIntent _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          android.app.IAlarmListener _arg7;
          _arg7 = android.app.IAlarmListener.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg8;
          _arg8 = data.readString();
          android.os.WorkSource _arg9;
          if ((0!=data.readInt())) {
            _arg9 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg9 = null;
          }
          android.app.AlarmManager.AlarmClockInfo _arg10;
          if ((0!=data.readInt())) {
            _arg10 = android.app.AlarmManager.AlarmClockInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg10 = null;
          }
          this.set(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTime:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          boolean _result = this.setTime(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setTimeZone:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTimeZone(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_remove:
        {
          data.enforceInterface(descriptor);
          android.app.PendingIntent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.IAlarmListener _arg1;
          _arg1 = android.app.IAlarmListener.Stub.asInterface(data.readStrongBinder());
          this.remove(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getNextWakeFromIdleTime:
        {
          data.enforceInterface(descriptor);
          long _result = this.getNextWakeFromIdleTime();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getNextAlarmClock:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.AlarmManager.AlarmClockInfo _result = this.getNextAlarmClock(_arg0);
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
        case TRANSACTION_currentNetworkTimeMillis:
        {
          data.enforceInterface(descriptor);
          long _result = this.currentNetworkTimeMillis();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IAlarmManager
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
      /** windowLength == 0 means exact; windowLength < 0 means the let the OS decide */
      @Override public void set(java.lang.String callingPackage, int type, long triggerAtTime, long windowLength, long interval, int flags, android.app.PendingIntent operation, android.app.IAlarmListener listener, java.lang.String listenerTag, android.os.WorkSource workSource, android.app.AlarmManager.AlarmClockInfo alarmClock) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(type);
          _data.writeLong(triggerAtTime);
          _data.writeLong(windowLength);
          _data.writeLong(interval);
          _data.writeInt(flags);
          if ((operation!=null)) {
            _data.writeInt(1);
            operation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeString(listenerTag);
          if ((workSource!=null)) {
            _data.writeInt(1);
            workSource.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((alarmClock!=null)) {
            _data.writeInt(1);
            alarmClock.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_set, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().set(callingPackage, type, triggerAtTime, windowLength, interval, flags, operation, listener, listenerTag, workSource, alarmClock);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean setTime(long millis) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(millis);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setTime(millis);
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
      @Override public void setTimeZone(java.lang.String zone) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(zone);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTimeZone, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTimeZone(zone);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void remove(android.app.PendingIntent operation, android.app.IAlarmListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((operation!=null)) {
            _data.writeInt(1);
            operation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_remove, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remove(operation, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getNextWakeFromIdleTime() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNextWakeFromIdleTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNextWakeFromIdleTime();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.app.AlarmManager.AlarmClockInfo getNextAlarmClock(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.AlarmManager.AlarmClockInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNextAlarmClock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNextAlarmClock(userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.AlarmManager.AlarmClockInfo.CREATOR.createFromParcel(_reply);
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
      @Override public long currentNetworkTimeMillis() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_currentNetworkTimeMillis, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().currentNetworkTimeMillis();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.app.IAlarmManager sDefaultImpl;
    }
    static final int TRANSACTION_set = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setTimeZone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_remove = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getNextWakeFromIdleTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getNextAlarmClock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_currentNetworkTimeMillis = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.app.IAlarmManager impl) {
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
    public static android.app.IAlarmManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** windowLength == 0 means exact; windowLength < 0 means the let the OS decide */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IAlarmManager.aidl:32:1:32:25")
  public void set(java.lang.String callingPackage, int type, long triggerAtTime, long windowLength, long interval, int flags, android.app.PendingIntent operation, android.app.IAlarmListener listener, java.lang.String listenerTag, android.os.WorkSource workSource, android.app.AlarmManager.AlarmClockInfo alarmClock) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IAlarmManager.aidl:36:1:36:25")
  public boolean setTime(long millis) throws android.os.RemoteException;
  public void setTimeZone(java.lang.String zone) throws android.os.RemoteException;
  public void remove(android.app.PendingIntent operation, android.app.IAlarmListener listener) throws android.os.RemoteException;
  public long getNextWakeFromIdleTime() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IAlarmManager.aidl:41:1:41:25")
  public android.app.AlarmManager.AlarmClockInfo getNextAlarmClock(int userId) throws android.os.RemoteException;
  public long currentNetworkTimeMillis() throws android.os.RemoteException;
}
