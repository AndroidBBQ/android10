/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms;
/**
 * @hide
 */
public interface IGroupCallCallback extends android.os.IInterface
{
  /** Default implementation for IGroupCallCallback. */
  public static class Default implements android.telephony.mbms.IGroupCallCallback
  {
    @Override public void onError(int errorCode, java.lang.String message) throws android.os.RemoteException
    {
    }
    @Override public void onGroupCallStateChanged(int state, int reason) throws android.os.RemoteException
    {
    }
    @Override public void onBroadcastSignalStrengthUpdated(int signalStrength) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.IGroupCallCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.IGroupCallCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.IGroupCallCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.IGroupCallCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.IGroupCallCallback))) {
        return ((android.telephony.mbms.IGroupCallCallback)iin);
      }
      return new android.telephony.mbms.IGroupCallCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onError(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGroupCallStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onGroupCallStateChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onBroadcastSignalStrengthUpdated:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onBroadcastSignalStrengthUpdated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.IGroupCallCallback
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
      @Override public void onError(int errorCode, java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(errorCode);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(errorCode, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGroupCallStateChanged(int state, int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGroupCallStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGroupCallStateChanged(state, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onBroadcastSignalStrengthUpdated(int signalStrength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(signalStrength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBroadcastSignalStrengthUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBroadcastSignalStrengthUpdated(signalStrength);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.mbms.IGroupCallCallback sDefaultImpl;
    }
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGroupCallStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onBroadcastSignalStrengthUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.telephony.mbms.IGroupCallCallback impl) {
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
    public static android.telephony.mbms.IGroupCallCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onError(int errorCode, java.lang.String message) throws android.os.RemoteException;
  public void onGroupCallStateChanged(int state, int reason) throws android.os.RemoteException;
  public void onBroadcastSignalStrengthUpdated(int signalStrength) throws android.os.RemoteException;
}
