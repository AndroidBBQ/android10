/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See {@link ImsManager#RegistrationCallback} for more information.
 *
 * {@hide}
 */
public interface IImsRegistrationCallback extends android.os.IInterface
{
  /** Default implementation for IImsRegistrationCallback. */
  public static class Default implements android.telephony.ims.aidl.IImsRegistrationCallback
  {
    @Override public void onRegistered(int imsRadioTech) throws android.os.RemoteException
    {
    }
    @Override public void onRegistering(int imsRadioTech) throws android.os.RemoteException
    {
    }
    @Override public void onDeregistered(android.telephony.ims.ImsReasonInfo info) throws android.os.RemoteException
    {
    }
    @Override public void onTechnologyChangeFailed(int imsRadioTech, android.telephony.ims.ImsReasonInfo info) throws android.os.RemoteException
    {
    }
    @Override public void onSubscriberAssociatedUriChanged(android.net.Uri[] uris) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsRegistrationCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsRegistrationCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsRegistrationCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsRegistrationCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsRegistrationCallback))) {
        return ((android.telephony.ims.aidl.IImsRegistrationCallback)iin);
      }
      return new android.telephony.ims.aidl.IImsRegistrationCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onRegistered:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onRegistered(_arg0);
          return true;
        }
        case TRANSACTION_onRegistering:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onRegistering(_arg0);
          return true;
        }
        case TRANSACTION_onDeregistered:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDeregistered(_arg0);
          return true;
        }
        case TRANSACTION_onTechnologyChangeFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onTechnologyChangeFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSubscriberAssociatedUriChanged:
        {
          data.enforceInterface(descriptor);
          android.net.Uri[] _arg0;
          _arg0 = data.createTypedArray(android.net.Uri.CREATOR);
          this.onSubscriberAssociatedUriChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsRegistrationCallback
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
      @Override public void onRegistered(int imsRadioTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(imsRadioTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRegistered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRegistered(imsRadioTech);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRegistering(int imsRadioTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(imsRadioTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRegistering, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRegistering(imsRadioTech);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDeregistered(android.telephony.ims.ImsReasonInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeregistered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeregistered(info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTechnologyChangeFailed(int imsRadioTech, android.telephony.ims.ImsReasonInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(imsRadioTech);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTechnologyChangeFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTechnologyChangeFailed(imsRadioTech, info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSubscriberAssociatedUriChanged(android.net.Uri[] uris) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(uris, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSubscriberAssociatedUriChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSubscriberAssociatedUriChanged(uris);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsRegistrationCallback sDefaultImpl;
    }
    static final int TRANSACTION_onRegistered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onRegistering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onDeregistered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onTechnologyChangeFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onSubscriberAssociatedUriChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsRegistrationCallback impl) {
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
    public static android.telephony.ims.aidl.IImsRegistrationCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onRegistered(int imsRadioTech) throws android.os.RemoteException;
  public void onRegistering(int imsRadioTech) throws android.os.RemoteException;
  public void onDeregistered(android.telephony.ims.ImsReasonInfo info) throws android.os.RemoteException;
  public void onTechnologyChangeFailed(int imsRadioTech, android.telephony.ims.ImsReasonInfo info) throws android.os.RemoteException;
  public void onSubscriberAssociatedUriChanged(android.net.Uri[] uris) throws android.os.RemoteException;
}
