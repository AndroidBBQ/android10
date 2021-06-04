/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * Interface from MediaController2 to MediaSession2Service.
 * <p>
 * Keep this interface oneway. Otherwise a malicious app may implement fake version of this,
 * and holds calls from controller to make controller owner(s) frozen.
 * @hide
 */
public interface IMediaSession2Service extends android.os.IInterface
{
  /** Default implementation for IMediaSession2Service. */
  public static class Default implements android.media.IMediaSession2Service
  {
    @Override public void connect(android.media.Controller2Link caller, int seq, android.os.Bundle connectionRequest) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IMediaSession2Service
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IMediaSession2Service";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IMediaSession2Service interface,
     * generating a proxy if needed.
     */
    public static android.media.IMediaSession2Service asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IMediaSession2Service))) {
        return ((android.media.IMediaSession2Service)iin);
      }
      return new android.media.IMediaSession2Service.Stub.Proxy(obj);
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
        case TRANSACTION_connect:
        {
          data.enforceInterface(descriptor);
          android.media.Controller2Link _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.Controller2Link.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.connect(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IMediaSession2Service
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
      @Override public void connect(android.media.Controller2Link caller, int seq, android.os.Bundle connectionRequest) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((caller!=null)) {
            _data.writeInt(1);
            caller.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(seq);
          if ((connectionRequest!=null)) {
            _data.writeInt(1);
            connectionRequest.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_connect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connect(caller, seq, connectionRequest);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IMediaSession2Service sDefaultImpl;
    }
    static final int TRANSACTION_connect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.IMediaSession2Service impl) {
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
    public static android.media.IMediaSession2Service getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void connect(android.media.Controller2Link caller, int seq, android.os.Bundle connectionRequest) throws android.os.RemoteException;
}
