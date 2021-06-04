/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/** @hide */
public interface IInstantAppResolver extends android.os.IInterface
{
  /** Default implementation for IInstantAppResolver. */
  public static class Default implements android.app.IInstantAppResolver
  {
    @Override public void getInstantAppResolveInfoList(android.content.Intent sanitizedIntent, int[] hostDigestPrefix, int userId, java.lang.String token, int sequence, android.os.IRemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getInstantAppIntentFilterList(android.content.Intent sanitizedIntent, int[] hostDigestPrefix, int userId, java.lang.String token, android.os.IRemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IInstantAppResolver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IInstantAppResolver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IInstantAppResolver interface,
     * generating a proxy if needed.
     */
    public static android.app.IInstantAppResolver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IInstantAppResolver))) {
        return ((android.app.IInstantAppResolver)iin);
      }
      return new android.app.IInstantAppResolver.Stub.Proxy(obj);
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
        case TRANSACTION_getInstantAppResolveInfoList:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int[] _arg1;
          _arg1 = data.createIntArray();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          android.os.IRemoteCallback _arg5;
          _arg5 = android.os.IRemoteCallback.Stub.asInterface(data.readStrongBinder());
          this.getInstantAppResolveInfoList(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_getInstantAppIntentFilterList:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int[] _arg1;
          _arg1 = data.createIntArray();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.IRemoteCallback _arg4;
          _arg4 = android.os.IRemoteCallback.Stub.asInterface(data.readStrongBinder());
          this.getInstantAppIntentFilterList(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IInstantAppResolver
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
      @Override public void getInstantAppResolveInfoList(android.content.Intent sanitizedIntent, int[] hostDigestPrefix, int userId, java.lang.String token, int sequence, android.os.IRemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sanitizedIntent!=null)) {
            _data.writeInt(1);
            sanitizedIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeIntArray(hostDigestPrefix);
          _data.writeInt(userId);
          _data.writeString(token);
          _data.writeInt(sequence);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInstantAppResolveInfoList, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getInstantAppResolveInfoList(sanitizedIntent, hostDigestPrefix, userId, token, sequence, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getInstantAppIntentFilterList(android.content.Intent sanitizedIntent, int[] hostDigestPrefix, int userId, java.lang.String token, android.os.IRemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sanitizedIntent!=null)) {
            _data.writeInt(1);
            sanitizedIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeIntArray(hostDigestPrefix);
          _data.writeInt(userId);
          _data.writeString(token);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInstantAppIntentFilterList, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getInstantAppIntentFilterList(sanitizedIntent, hostDigestPrefix, userId, token, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IInstantAppResolver sDefaultImpl;
    }
    static final int TRANSACTION_getInstantAppResolveInfoList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getInstantAppIntentFilterList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.app.IInstantAppResolver impl) {
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
    public static android.app.IInstantAppResolver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void getInstantAppResolveInfoList(android.content.Intent sanitizedIntent, int[] hostDigestPrefix, int userId, java.lang.String token, int sequence, android.os.IRemoteCallback callback) throws android.os.RemoteException;
  public void getInstantAppIntentFilterList(android.content.Intent sanitizedIntent, int[] hostDigestPrefix, int userId, java.lang.String token, android.os.IRemoteCallback callback) throws android.os.RemoteException;
}
