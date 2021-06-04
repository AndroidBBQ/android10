/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * Activity Recognition Hardware provider interface.
 * This interface can be used to implement hardware based activity recognition.
 *
 * @hide
 */
public interface IActivityRecognitionHardware extends android.os.IInterface
{
  /** Default implementation for IActivityRecognitionHardware. */
  public static class Default implements android.hardware.location.IActivityRecognitionHardware
  {
    /**
         * Gets an array of supported activities by hardware.
         */
    @Override public java.lang.String[] getSupportedActivities() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns true if the given activity is supported, false otherwise.
         */
    @Override public boolean isActivitySupported(java.lang.String activityType) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Registers a sink with Hardware Activity-Recognition.
         */
    @Override public boolean registerSink(android.hardware.location.IActivityRecognitionHardwareSink sink) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Unregisters a sink with Hardware Activity-Recognition.
         */
    @Override public boolean unregisterSink(android.hardware.location.IActivityRecognitionHardwareSink sink) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Enables tracking of a given activity/event type, if the activity is supported.
         */
    @Override public boolean enableActivityEvent(java.lang.String activityType, int eventType, long reportLatencyNs) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Disables tracking of a given activity/eventy type.
         */
    @Override public boolean disableActivityEvent(java.lang.String activityType, int eventType) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Requests hardware for all the activity events detected up to the given point in time.
         */
    @Override public boolean flush() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IActivityRecognitionHardware
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IActivityRecognitionHardware";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IActivityRecognitionHardware interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IActivityRecognitionHardware asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IActivityRecognitionHardware))) {
        return ((android.hardware.location.IActivityRecognitionHardware)iin);
      }
      return new android.hardware.location.IActivityRecognitionHardware.Stub.Proxy(obj);
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
        case TRANSACTION_getSupportedActivities:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getSupportedActivities();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_isActivitySupported:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isActivitySupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_registerSink:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.IActivityRecognitionHardwareSink _arg0;
          _arg0 = android.hardware.location.IActivityRecognitionHardwareSink.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.registerSink(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterSink:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.IActivityRecognitionHardwareSink _arg0;
          _arg0 = android.hardware.location.IActivityRecognitionHardwareSink.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.unregisterSink(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableActivityEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          boolean _result = this.enableActivityEvent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableActivityEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.disableActivityEvent(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_flush:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.flush();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IActivityRecognitionHardware
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
           * Gets an array of supported activities by hardware.
           */
      @Override public java.lang.String[] getSupportedActivities() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedActivities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSupportedActivities();
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
      /**
           * Returns true if the given activity is supported, false otherwise.
           */
      @Override public boolean isActivitySupported(java.lang.String activityType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(activityType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActivitySupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActivitySupported(activityType);
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
      /**
           * Registers a sink with Hardware Activity-Recognition.
           */
      @Override public boolean registerSink(android.hardware.location.IActivityRecognitionHardwareSink sink) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sink!=null))?(sink.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerSink, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerSink(sink);
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
      /**
           * Unregisters a sink with Hardware Activity-Recognition.
           */
      @Override public boolean unregisterSink(android.hardware.location.IActivityRecognitionHardwareSink sink) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((sink!=null))?(sink.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterSink, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unregisterSink(sink);
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
      /**
           * Enables tracking of a given activity/event type, if the activity is supported.
           */
      @Override public boolean enableActivityEvent(java.lang.String activityType, int eventType, long reportLatencyNs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(activityType);
          _data.writeInt(eventType);
          _data.writeLong(reportLatencyNs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableActivityEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableActivityEvent(activityType, eventType, reportLatencyNs);
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
      /**
           * Disables tracking of a given activity/eventy type.
           */
      @Override public boolean disableActivityEvent(java.lang.String activityType, int eventType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(activityType);
          _data.writeInt(eventType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableActivityEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disableActivityEvent(activityType, eventType);
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
      /**
           * Requests hardware for all the activity events detected up to the given point in time.
           */
      @Override public boolean flush() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_flush, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().flush();
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
      public static android.hardware.location.IActivityRecognitionHardware sDefaultImpl;
    }
    static final int TRANSACTION_getSupportedActivities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isActivitySupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_registerSink = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_unregisterSink = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_enableActivityEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_disableActivityEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_flush = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.hardware.location.IActivityRecognitionHardware impl) {
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
    public static android.hardware.location.IActivityRecognitionHardware getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Gets an array of supported activities by hardware.
       */
  public java.lang.String[] getSupportedActivities() throws android.os.RemoteException;
  /**
       * Returns true if the given activity is supported, false otherwise.
       */
  public boolean isActivitySupported(java.lang.String activityType) throws android.os.RemoteException;
  /**
       * Registers a sink with Hardware Activity-Recognition.
       */
  public boolean registerSink(android.hardware.location.IActivityRecognitionHardwareSink sink) throws android.os.RemoteException;
  /**
       * Unregisters a sink with Hardware Activity-Recognition.
       */
  public boolean unregisterSink(android.hardware.location.IActivityRecognitionHardwareSink sink) throws android.os.RemoteException;
  /**
       * Enables tracking of a given activity/event type, if the activity is supported.
       */
  public boolean enableActivityEvent(java.lang.String activityType, int eventType, long reportLatencyNs) throws android.os.RemoteException;
  /**
       * Disables tracking of a given activity/eventy type.
       */
  public boolean disableActivityEvent(java.lang.String activityType, int eventType) throws android.os.RemoteException;
  /**
       * Requests hardware for all the activity events detected up to the given point in time.
       */
  public boolean flush() throws android.os.RemoteException;
}
