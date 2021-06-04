/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See MmTelFeature#Listener for more information.
 * {@hide}
 */
public interface IImsMmTelListener extends android.os.IInterface
{
  /** Default implementation for IImsMmTelListener. */
  public static class Default implements android.telephony.ims.aidl.IImsMmTelListener
  {
    @Override public void onIncomingCall(com.android.ims.internal.IImsCallSession c, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onRejectedCall(android.telephony.ims.ImsCallProfile callProfile, android.telephony.ims.ImsReasonInfo reason) throws android.os.RemoteException
    {
    }
    @Override public void onVoiceMessageCountUpdate(int count) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsMmTelListener
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsMmTelListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsMmTelListener interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsMmTelListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsMmTelListener))) {
        return ((android.telephony.ims.aidl.IImsMmTelListener)iin);
      }
      return new android.telephony.ims.aidl.IImsMmTelListener.Stub.Proxy(obj);
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
        case TRANSACTION_onIncomingCall:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsCallSession _arg0;
          _arg0 = com.android.ims.internal.IImsCallSession.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onIncomingCall(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onRejectedCall:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.ims.ImsReasonInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onRejectedCall(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onVoiceMessageCountUpdate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onVoiceMessageCountUpdate(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsMmTelListener
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
      @Override public void onIncomingCall(com.android.ims.internal.IImsCallSession c, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onIncomingCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onIncomingCall(c, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRejectedCall(android.telephony.ims.ImsCallProfile callProfile, android.telephony.ims.ImsReasonInfo reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callProfile!=null)) {
            _data.writeInt(1);
            callProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((reason!=null)) {
            _data.writeInt(1);
            reason.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRejectedCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRejectedCall(callProfile, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVoiceMessageCountUpdate(int count) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(count);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVoiceMessageCountUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVoiceMessageCountUpdate(count);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsMmTelListener sDefaultImpl;
    }
    static final int TRANSACTION_onIncomingCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onRejectedCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onVoiceMessageCountUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsMmTelListener impl) {
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
    public static android.telephony.ims.aidl.IImsMmTelListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onIncomingCall(com.android.ims.internal.IImsCallSession c, android.os.Bundle extras) throws android.os.RemoteException;
  public void onRejectedCall(android.telephony.ims.ImsCallProfile callProfile, android.telephony.ims.ImsReasonInfo reason) throws android.os.RemoteException;
  public void onVoiceMessageCountUpdate(int count) throws android.os.RemoteException;
}
