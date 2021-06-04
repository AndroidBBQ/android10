/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * Interface from MediaSession2 to MediaController2.
 * <p>
 * Keep this interface oneway. Otherwise a malicious app may implement fake version of this,
 * and holds calls from session to make session owner(s) frozen.
 * @hide
 */// Code for AML only

public interface IMediaController2 extends android.os.IInterface
{
  /** Default implementation for IMediaController2. */
  public static class Default implements android.media.IMediaController2
  {
    @Override public void notifyConnected(int seq, android.os.Bundle connectionResult) throws android.os.RemoteException
    {
    }
    @Override public void notifyDisconnected(int seq) throws android.os.RemoteException
    {
    }
    @Override public void notifyPlaybackActiveChanged(int seq, boolean playbackActive) throws android.os.RemoteException
    {
    }
    @Override public void sendSessionCommand(int seq, android.media.Session2Command command, android.os.Bundle args, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
    }
    @Override public void cancelSessionCommand(int seq) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IMediaController2
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IMediaController2";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IMediaController2 interface,
     * generating a proxy if needed.
     */
    public static android.media.IMediaController2 asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IMediaController2))) {
        return ((android.media.IMediaController2)iin);
      }
      return new android.media.IMediaController2.Stub.Proxy(obj);
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
        case TRANSACTION_notifyConnected:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.notifyConnected(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyDisconnected:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyDisconnected(_arg0);
          return true;
        }
        case TRANSACTION_notifyPlaybackActiveChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.notifyPlaybackActiveChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendSessionCommand:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.media.Session2Command _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.Session2Command.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.ResultReceiver _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.sendSessionCommand(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_cancelSessionCommand:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.cancelSessionCommand(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IMediaController2
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
      @Override public void notifyConnected(int seq, android.os.Bundle connectionResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seq);
          if ((connectionResult!=null)) {
            _data.writeInt(1);
            connectionResult.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyConnected(seq, connectionResult);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyDisconnected(int seq) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seq);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDisconnected(seq);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyPlaybackActiveChanged(int seq, boolean playbackActive) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seq);
          _data.writeInt(((playbackActive)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPlaybackActiveChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPlaybackActiveChanged(seq, playbackActive);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendSessionCommand(int seq, android.media.Session2Command command, android.os.Bundle args, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seq);
          if ((command!=null)) {
            _data.writeInt(1);
            command.writeToParcel(_data, 0);
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
            getDefaultImpl().sendSessionCommand(seq, command, args, resultReceiver);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelSessionCommand(int seq) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seq);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSessionCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSessionCommand(seq);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IMediaController2 sDefaultImpl;
    }
    static final int TRANSACTION_notifyConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_notifyDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyPlaybackActiveChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_sendSessionCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_cancelSessionCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.media.IMediaController2 impl) {
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
    public static android.media.IMediaController2 getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void notifyConnected(int seq, android.os.Bundle connectionResult) throws android.os.RemoteException;
  public void notifyDisconnected(int seq) throws android.os.RemoteException;
  public void notifyPlaybackActiveChanged(int seq, boolean playbackActive) throws android.os.RemoteException;
  public void sendSessionCommand(int seq, android.media.Session2Command command, android.os.Bundle args, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  public void cancelSessionCommand(int seq) throws android.os.RemoteException;
}
