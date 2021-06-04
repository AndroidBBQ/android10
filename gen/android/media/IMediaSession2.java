/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * Interface from MediaController2 to MediaSession2.
 * <p>
 * Keep this interface oneway. Otherwise a malicious app may implement fake version of this,
 * and holds calls from session to make session owner(s) frozen.
 * @hide
 */// Code for AML only

public interface IMediaSession2 extends android.os.IInterface
{
  /** Default implementation for IMediaSession2. */
  public static class Default implements android.media.IMediaSession2
  {
    @Override public void connect(android.media.Controller2Link caller, int seq, android.os.Bundle connectionRequest) throws android.os.RemoteException
    {
    }
    @Override public void disconnect(android.media.Controller2Link caller, int seq) throws android.os.RemoteException
    {
    }
    @Override public void sendSessionCommand(android.media.Controller2Link caller, int seq, android.media.Session2Command sessionCommand, android.os.Bundle args, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
    }
    @Override public void cancelSessionCommand(android.media.Controller2Link caller, int seq) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IMediaSession2
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IMediaSession2";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IMediaSession2 interface,
     * generating a proxy if needed.
     */
    public static android.media.IMediaSession2 asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IMediaSession2))) {
        return ((android.media.IMediaSession2)iin);
      }
      return new android.media.IMediaSession2.Stub.Proxy(obj);
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
        case TRANSACTION_disconnect:
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
          this.disconnect(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendSessionCommand:
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
          android.media.Session2Command _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.media.Session2Command.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.os.ResultReceiver _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.sendSessionCommand(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_cancelSessionCommand:
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
          this.cancelSessionCommand(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IMediaSession2
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
      @Override public void disconnect(android.media.Controller2Link caller, int seq) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnect(caller, seq);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendSessionCommand(android.media.Controller2Link caller, int seq, android.media.Session2Command sessionCommand, android.os.Bundle args, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
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
          if ((sessionCommand!=null)) {
            _data.writeInt(1);
            sessionCommand.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((resultReceiver!=null)) {
            _data.writeInt(1);
            resultReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendSessionCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendSessionCommand(caller, seq, sessionCommand, args, resultReceiver);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelSessionCommand(android.media.Controller2Link caller, int seq) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSessionCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSessionCommand(caller, seq);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IMediaSession2 sDefaultImpl;
    }
    static final int TRANSACTION_connect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_sendSessionCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_cancelSessionCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.media.IMediaSession2 impl) {
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
    public static android.media.IMediaSession2 getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void connect(android.media.Controller2Link caller, int seq, android.os.Bundle connectionRequest) throws android.os.RemoteException;
  public void disconnect(android.media.Controller2Link caller, int seq) throws android.os.RemoteException;
  public void sendSessionCommand(android.media.Controller2Link caller, int seq, android.media.Session2Command sessionCommand, android.os.Bundle args, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  public void cancelSessionCommand(android.media.Controller2Link caller, int seq) throws android.os.RemoteException;
}
