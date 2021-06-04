/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.carrier;
/**
 * Callback interface definition for the Carrier Messaging Service client to get informed of the
 * result of various API invocations.
 * @hide
 */
public interface ICarrierMessagingCallback extends android.os.IInterface
{
  /** Default implementation for ICarrierMessagingCallback. */
  public static class Default implements android.service.carrier.ICarrierMessagingCallback
  {
    @Override public void onFilterComplete(int result) throws android.os.RemoteException
    {
    }
    @Override public void onSendSmsComplete(int result, int messageRef) throws android.os.RemoteException
    {
    }
    @Override public void onSendMultipartSmsComplete(int result, int[] messageRefs) throws android.os.RemoteException
    {
    }
    @Override public void onSendMmsComplete(int result, byte[] sendConfPdu) throws android.os.RemoteException
    {
    }
    @Override public void onDownloadMmsComplete(int result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.carrier.ICarrierMessagingCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.carrier.ICarrierMessagingCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.carrier.ICarrierMessagingCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.carrier.ICarrierMessagingCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.carrier.ICarrierMessagingCallback))) {
        return ((android.service.carrier.ICarrierMessagingCallback)iin);
      }
      return new android.service.carrier.ICarrierMessagingCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onFilterComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onFilterComplete(_arg0);
          return true;
        }
        case TRANSACTION_onSendSmsComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onSendSmsComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSendMultipartSmsComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _arg1;
          _arg1 = data.createIntArray();
          this.onSendMultipartSmsComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSendMmsComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.onSendMmsComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDownloadMmsComplete:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onDownloadMmsComplete(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.carrier.ICarrierMessagingCallback
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
      @Override public void onFilterComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFilterComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFilterComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSendSmsComplete(int result, int messageRef) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          _data.writeInt(messageRef);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSendSmsComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSendSmsComplete(result, messageRef);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSendMultipartSmsComplete(int result, int[] messageRefs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          _data.writeIntArray(messageRefs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSendMultipartSmsComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSendMultipartSmsComplete(result, messageRefs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSendMmsComplete(int result, byte[] sendConfPdu) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          _data.writeByteArray(sendConfPdu);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSendMmsComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSendMmsComplete(result, sendConfPdu);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDownloadMmsComplete(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDownloadMmsComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDownloadMmsComplete(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.carrier.ICarrierMessagingCallback sDefaultImpl;
    }
    static final int TRANSACTION_onFilterComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSendSmsComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSendMultipartSmsComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSendMmsComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onDownloadMmsComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.service.carrier.ICarrierMessagingCallback impl) {
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
    public static android.service.carrier.ICarrierMessagingCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onFilterComplete(int result) throws android.os.RemoteException;
  public void onSendSmsComplete(int result, int messageRef) throws android.os.RemoteException;
  public void onSendMultipartSmsComplete(int result, int[] messageRefs) throws android.os.RemoteException;
  public void onSendMmsComplete(int result, byte[] sendConfPdu) throws android.os.RemoteException;
  public void onDownloadMmsComplete(int result) throws android.os.RemoteException;
}
