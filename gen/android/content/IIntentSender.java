/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content;
/** @hide */
public interface IIntentSender extends android.os.IInterface
{
  /** Default implementation for IIntentSender. */
  public static class Default implements android.content.IIntentSender
  {
    @Override public void send(int code, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder whitelistToken, android.content.IIntentReceiver finishedReceiver, java.lang.String requiredPermission, android.os.Bundle options) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.IIntentSender
  {
    private static final java.lang.String DESCRIPTOR = "android.content.IIntentSender";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.IIntentSender interface,
     * generating a proxy if needed.
     */
    public static android.content.IIntentSender asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.IIntentSender))) {
        return ((android.content.IIntentSender)iin);
      }
      return new android.content.IIntentSender.Stub.Proxy(obj);
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
        case TRANSACTION_send:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          android.content.IIntentReceiver _arg4;
          _arg4 = android.content.IIntentReceiver.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg5;
          _arg5 = data.readString();
          android.os.Bundle _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.send(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.IIntentSender
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
      @Override public void send(int code, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder whitelistToken, android.content.IIntentReceiver finishedReceiver, java.lang.String requiredPermission, android.os.Bundle options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          _data.writeStrongBinder(whitelistToken);
          _data.writeStrongBinder((((finishedReceiver!=null))?(finishedReceiver.asBinder()):(null)));
          _data.writeString(requiredPermission);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_send, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().send(code, intent, resolvedType, whitelistToken, finishedReceiver, requiredPermission, options);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.content.IIntentSender sDefaultImpl;
    }
    static final int TRANSACTION_send = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.content.IIntentSender impl) {
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
    public static android.content.IIntentSender getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void send(int code, android.content.Intent intent, java.lang.String resolvedType, android.os.IBinder whitelistToken, android.content.IIntentReceiver finishedReceiver, java.lang.String requiredPermission, android.os.Bundle options) throws android.os.RemoteException;
}
