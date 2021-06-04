/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See SmsImplBase for more information.
 * {@hide}
 */
public interface IImsSmsListener extends android.os.IInterface
{
  /** Default implementation for IImsSmsListener. */
  public static class Default implements android.telephony.ims.aidl.IImsSmsListener
  {
    @Override public void onSendSmsResult(int token, int messageRef, int status, int reason) throws android.os.RemoteException
    {
    }
    @Override public void onSmsStatusReportReceived(int token, int messageRef, java.lang.String format, byte[] pdu) throws android.os.RemoteException
    {
    }
    @Override public void onSmsReceived(int token, java.lang.String format, byte[] pdu) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsSmsListener
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsSmsListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsSmsListener interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsSmsListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsSmsListener))) {
        return ((android.telephony.ims.aidl.IImsSmsListener)iin);
      }
      return new android.telephony.ims.aidl.IImsSmsListener.Stub.Proxy(obj);
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
        case TRANSACTION_onSendSmsResult:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.onSendSmsResult(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onSmsStatusReportReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          this.onSmsStatusReportReceived(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onSmsReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.onSmsReceived(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsSmsListener
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
      @Override public void onSendSmsResult(int token, int messageRef, int status, int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeInt(messageRef);
          _data.writeInt(status);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSendSmsResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSendSmsResult(token, messageRef, status, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSmsStatusReportReceived(int token, int messageRef, java.lang.String format, byte[] pdu) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeInt(messageRef);
          _data.writeString(format);
          _data.writeByteArray(pdu);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSmsStatusReportReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSmsStatusReportReceived(token, messageRef, format, pdu);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSmsReceived(int token, java.lang.String format, byte[] pdu) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeString(format);
          _data.writeByteArray(pdu);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSmsReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSmsReceived(token, format, pdu);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsSmsListener sDefaultImpl;
    }
    static final int TRANSACTION_onSendSmsResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSmsStatusReportReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSmsReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsSmsListener impl) {
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
    public static android.telephony.ims.aidl.IImsSmsListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSendSmsResult(int token, int messageRef, int status, int reason) throws android.os.RemoteException;
  public void onSmsStatusReportReceived(int token, int messageRef, java.lang.String format, byte[] pdu) throws android.os.RemoteException;
  public void onSmsReceived(int token, java.lang.String format, byte[] pdu) throws android.os.RemoteException;
}
