/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * {@hide}
 */
public interface IImsUtListener extends android.os.IInterface
{
  /** Default implementation for IImsUtListener. */
  public static class Default implements com.android.ims.internal.IImsUtListener
  {
    /**
         * Notifies the result of the supplementary service configuration udpate.
         */
    @Override public void utConfigurationUpdated(com.android.ims.internal.IImsUt ut, int id) throws android.os.RemoteException
    {
    }
    @Override public void utConfigurationUpdateFailed(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsReasonInfo error) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the result of the supplementary service configuration query.
         */
    @Override public void utConfigurationQueried(com.android.ims.internal.IImsUt ut, int id, android.os.Bundle ssInfo) throws android.os.RemoteException
    {
    }
    @Override public void utConfigurationQueryFailed(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsReasonInfo error) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the status of the call barring supplementary service.
         */
    @Override public void utConfigurationCallBarringQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsSsInfo[] cbInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the status of the call forwarding supplementary service.
         */
    @Override public void utConfigurationCallForwardQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsCallForwardInfo[] cfInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the status of the call waiting supplementary service.
         */
    @Override public void utConfigurationCallWaitingQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsSsInfo[] cwInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client when Supplementary Service indication is received
         *
         * @param ssData Details of SS request and response information
         */
    @Override public void onSupplementaryServiceIndication(android.telephony.ims.ImsSsData ssData) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsUtListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsUtListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsUtListener interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsUtListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsUtListener))) {
        return ((com.android.ims.internal.IImsUtListener)iin);
      }
      return new com.android.ims.internal.IImsUtListener.Stub.Proxy(obj);
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
        case TRANSACTION_utConfigurationUpdated:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.utConfigurationUpdated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_utConfigurationUpdateFailed:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.utConfigurationUpdateFailed(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_utConfigurationQueried:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.utConfigurationQueried(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_utConfigurationQueryFailed:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.utConfigurationQueryFailed(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_utConfigurationCallBarringQueried:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsSsInfo[] _arg2;
          _arg2 = data.createTypedArray(android.telephony.ims.ImsSsInfo.CREATOR);
          this.utConfigurationCallBarringQueried(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_utConfigurationCallForwardQueried:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsCallForwardInfo[] _arg2;
          _arg2 = data.createTypedArray(android.telephony.ims.ImsCallForwardInfo.CREATOR);
          this.utConfigurationCallForwardQueried(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_utConfigurationCallWaitingQueried:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUt _arg0;
          _arg0 = com.android.ims.internal.IImsUt.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsSsInfo[] _arg2;
          _arg2 = data.createTypedArray(android.telephony.ims.ImsSsInfo.CREATOR);
          this.utConfigurationCallWaitingQueried(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onSupplementaryServiceIndication:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsSsData _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsSsData.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSupplementaryServiceIndication(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsUtListener
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
           * Notifies the result of the supplementary service configuration udpate.
           */
      @Override public void utConfigurationUpdated(com.android.ims.internal.IImsUt ut, int id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationUpdated(ut, id);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void utConfigurationUpdateFailed(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsReasonInfo error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          if ((error!=null)) {
            _data.writeInt(1);
            error.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationUpdateFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationUpdateFailed(ut, id, error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the result of the supplementary service configuration query.
           */
      @Override public void utConfigurationQueried(com.android.ims.internal.IImsUt ut, int id, android.os.Bundle ssInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          if ((ssInfo!=null)) {
            _data.writeInt(1);
            ssInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationQueried, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationQueried(ut, id, ssInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void utConfigurationQueryFailed(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsReasonInfo error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          if ((error!=null)) {
            _data.writeInt(1);
            error.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationQueryFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationQueryFailed(ut, id, error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the status of the call barring supplementary service.
           */
      @Override public void utConfigurationCallBarringQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsSsInfo[] cbInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          _data.writeTypedArray(cbInfo, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationCallBarringQueried, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationCallBarringQueried(ut, id, cbInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the status of the call forwarding supplementary service.
           */
      @Override public void utConfigurationCallForwardQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsCallForwardInfo[] cfInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          _data.writeTypedArray(cfInfo, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationCallForwardQueried, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationCallForwardQueried(ut, id, cfInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the status of the call waiting supplementary service.
           */
      @Override public void utConfigurationCallWaitingQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsSsInfo[] cwInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((ut!=null))?(ut.asBinder()):(null)));
          _data.writeInt(id);
          _data.writeTypedArray(cwInfo, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_utConfigurationCallWaitingQueried, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().utConfigurationCallWaitingQueried(ut, id, cwInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client when Supplementary Service indication is received
           *
           * @param ssData Details of SS request and response information
           */
      @Override public void onSupplementaryServiceIndication(android.telephony.ims.ImsSsData ssData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((ssData!=null)) {
            _data.writeInt(1);
            ssData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSupplementaryServiceIndication, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSupplementaryServiceIndication(ssData);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsUtListener sDefaultImpl;
    }
    static final int TRANSACTION_utConfigurationUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_utConfigurationUpdateFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_utConfigurationQueried = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_utConfigurationQueryFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_utConfigurationCallBarringQueried = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_utConfigurationCallForwardQueried = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_utConfigurationCallWaitingQueried = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onSupplementaryServiceIndication = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsUtListener impl) {
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
    public static com.android.ims.internal.IImsUtListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies the result of the supplementary service configuration udpate.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:34:1:34:25")
  public void utConfigurationUpdated(com.android.ims.internal.IImsUt ut, int id) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:36:1:36:25")
  public void utConfigurationUpdateFailed(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsReasonInfo error) throws android.os.RemoteException;
  /**
       * Notifies the result of the supplementary service configuration query.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:42:1:42:25")
  public void utConfigurationQueried(com.android.ims.internal.IImsUt ut, int id, android.os.Bundle ssInfo) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:44:1:44:25")
  public void utConfigurationQueryFailed(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsReasonInfo error) throws android.os.RemoteException;
  /**
       * Notifies the status of the call barring supplementary service.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:50:1:50:25")
  public void utConfigurationCallBarringQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsSsInfo[] cbInfo) throws android.os.RemoteException;
  /**
       * Notifies the status of the call forwarding supplementary service.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:57:1:57:25")
  public void utConfigurationCallForwardQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsCallForwardInfo[] cfInfo) throws android.os.RemoteException;
  /**
       * Notifies the status of the call waiting supplementary service.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsUtListener.aidl:64:1:64:25")
  public void utConfigurationCallWaitingQueried(com.android.ims.internal.IImsUt ut, int id, android.telephony.ims.ImsSsInfo[] cwInfo) throws android.os.RemoteException;
  /**
       * Notifies client when Supplementary Service indication is received
       *
       * @param ssData Details of SS request and response information
       */
  public void onSupplementaryServiceIndication(android.telephony.ims.ImsSsData ssData) throws android.os.RemoteException;
}
