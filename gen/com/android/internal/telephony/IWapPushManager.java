/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface IWapPushManager extends android.os.IInterface
{
  /** Default implementation for IWapPushManager. */
  public static class Default implements com.android.internal.telephony.IWapPushManager
  {
    /**
         * Processes WAP push message and triggers the receiver application registered
         * in the application ID table.
         */
    @Override public int processMessage(java.lang.String app_id, java.lang.String content_type, android.content.Intent intent) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Add receiver application into the application ID table.
         * Returns true if inserting the information is successfull. Inserting the duplicated
         * record in the application ID table is not allowed. Use update/delete method.
         */
    @Override public boolean addPackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name, int app_type, boolean need_signature, boolean further_processing) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Updates receiver application that is last added.
         * Returns true if updating the information is successfull.
         */
    @Override public boolean updatePackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name, int app_type, boolean need_signature, boolean further_processing) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Delites receiver application information.
         * Returns true if deleting is successfull.
         */
    @Override public boolean deletePackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.IWapPushManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.IWapPushManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.IWapPushManager interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.IWapPushManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.IWapPushManager))) {
        return ((com.android.internal.telephony.IWapPushManager)iin);
      }
      return new com.android.internal.telephony.IWapPushManager.Stub.Proxy(obj);
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
        case TRANSACTION_processMessage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.Intent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _result = this.processMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          boolean _result = this.addPackage(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updatePackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          boolean _result = this.updatePackage(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_deletePackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          boolean _result = this.deletePackage(_arg0, _arg1, _arg2, _arg3);
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
    private static class Proxy implements com.android.internal.telephony.IWapPushManager
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
           * Processes WAP push message and triggers the receiver application registered
           * in the application ID table.
           */
      @Override public int processMessage(java.lang.String app_id, java.lang.String content_type, android.content.Intent intent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(app_id);
          _data.writeString(content_type);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_processMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().processMessage(app_id, content_type, intent);
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
      /**
           * Add receiver application into the application ID table.
           * Returns true if inserting the information is successfull. Inserting the duplicated
           * record in the application ID table is not allowed. Use update/delete method.
           */
      @Override public boolean addPackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name, int app_type, boolean need_signature, boolean further_processing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(x_app_id);
          _data.writeString(content_type);
          _data.writeString(package_name);
          _data.writeString(class_name);
          _data.writeInt(app_type);
          _data.writeInt(((need_signature)?(1):(0)));
          _data.writeInt(((further_processing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addPackage(x_app_id, content_type, package_name, class_name, app_type, need_signature, further_processing);
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
           * Updates receiver application that is last added.
           * Returns true if updating the information is successfull.
           */
      @Override public boolean updatePackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name, int app_type, boolean need_signature, boolean further_processing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(x_app_id);
          _data.writeString(content_type);
          _data.writeString(package_name);
          _data.writeString(class_name);
          _data.writeInt(app_type);
          _data.writeInt(((need_signature)?(1):(0)));
          _data.writeInt(((further_processing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updatePackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updatePackage(x_app_id, content_type, package_name, class_name, app_type, need_signature, further_processing);
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
           * Delites receiver application information.
           * Returns true if deleting is successfull.
           */
      @Override public boolean deletePackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(x_app_id);
          _data.writeString(content_type);
          _data.writeString(package_name);
          _data.writeString(class_name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deletePackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().deletePackage(x_app_id, content_type, package_name, class_name);
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
      public static com.android.internal.telephony.IWapPushManager sDefaultImpl;
    }
    static final int TRANSACTION_processMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_updatePackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_deletePackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.internal.telephony.IWapPushManager impl) {
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
    public static com.android.internal.telephony.IWapPushManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Processes WAP push message and triggers the receiver application registered
       * in the application ID table.
       */
  public int processMessage(java.lang.String app_id, java.lang.String content_type, android.content.Intent intent) throws android.os.RemoteException;
  /**
       * Add receiver application into the application ID table.
       * Returns true if inserting the information is successfull. Inserting the duplicated
       * record in the application ID table is not allowed. Use update/delete method.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/IWapPushManager.aidl:33:1:33:25")
  public boolean addPackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name, int app_type, boolean need_signature, boolean further_processing) throws android.os.RemoteException;
  /**
       * Updates receiver application that is last added.
       * Returns true if updating the information is successfull.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/IWapPushManager.aidl:42:1:42:25")
  public boolean updatePackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name, int app_type, boolean need_signature, boolean further_processing) throws android.os.RemoteException;
  /**
       * Delites receiver application information.
       * Returns true if deleting is successfull.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/IWapPushManager.aidl:51:1:51:25")
  public boolean deletePackage(java.lang.String x_app_id, java.lang.String content_type, java.lang.String package_name, java.lang.String class_name) throws android.os.RemoteException;
}
