/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote callback interface for call screening services.
 *
 * @see android.telecom.CallScreeningService
 *
 * {@hide}
 */
public interface ICallScreeningAdapter extends android.os.IInterface
{
  /** Default implementation for ICallScreeningAdapter. */
  public static class Default implements com.android.internal.telecom.ICallScreeningAdapter
  {
    @Override public void allowCall(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void silenceCall(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void disallowCall(java.lang.String callId, boolean shouldReject, boolean shouldAddToCallLog, boolean shouldShowNotification, android.content.ComponentName componentName) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.ICallScreeningAdapter
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.ICallScreeningAdapter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.ICallScreeningAdapter interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.ICallScreeningAdapter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.ICallScreeningAdapter))) {
        return ((com.android.internal.telecom.ICallScreeningAdapter)iin);
      }
      return new com.android.internal.telecom.ICallScreeningAdapter.Stub.Proxy(obj);
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
        case TRANSACTION_allowCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.allowCall(_arg0);
          return true;
        }
        case TRANSACTION_silenceCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.silenceCall(_arg0);
          return true;
        }
        case TRANSACTION_disallowCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          android.content.ComponentName _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.disallowCall(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.ICallScreeningAdapter
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
      @Override public void allowCall(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_allowCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().allowCall(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void silenceCall(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_silenceCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().silenceCall(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disallowCall(java.lang.String callId, boolean shouldReject, boolean shouldAddToCallLog, boolean shouldShowNotification, android.content.ComponentName componentName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((shouldReject)?(1):(0)));
          _data.writeInt(((shouldAddToCallLog)?(1):(0)));
          _data.writeInt(((shouldShowNotification)?(1):(0)));
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_disallowCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disallowCall(callId, shouldReject, shouldAddToCallLog, shouldShowNotification, componentName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.ICallScreeningAdapter sDefaultImpl;
    }
    static final int TRANSACTION_allowCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_silenceCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_disallowCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.telecom.ICallScreeningAdapter impl) {
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
    public static com.android.internal.telecom.ICallScreeningAdapter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void allowCall(java.lang.String callId) throws android.os.RemoteException;
  public void silenceCall(java.lang.String callId) throws android.os.RemoteException;
  public void disallowCall(java.lang.String callId, boolean shouldReject, boolean shouldAddToCallLog, boolean shouldShowNotification, android.content.ComponentName componentName) throws android.os.RemoteException;
}
