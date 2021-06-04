/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.midi;
/** @hide */
public interface IMidiDeviceOpenCallback extends android.os.IInterface
{
  /** Default implementation for IMidiDeviceOpenCallback. */
  public static class Default implements android.media.midi.IMidiDeviceOpenCallback
  {
    @Override public void onDeviceOpened(android.media.midi.IMidiDeviceServer server, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.midi.IMidiDeviceOpenCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.midi.IMidiDeviceOpenCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.midi.IMidiDeviceOpenCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.midi.IMidiDeviceOpenCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.midi.IMidiDeviceOpenCallback))) {
        return ((android.media.midi.IMidiDeviceOpenCallback)iin);
      }
      return new android.media.midi.IMidiDeviceOpenCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onDeviceOpened:
        {
          data.enforceInterface(descriptor);
          android.media.midi.IMidiDeviceServer _arg0;
          _arg0 = android.media.midi.IMidiDeviceServer.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.onDeviceOpened(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.midi.IMidiDeviceOpenCallback
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
      @Override public void onDeviceOpened(android.media.midi.IMidiDeviceServer server, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((server!=null))?(server.asBinder()):(null)));
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceOpened, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceOpened(server, token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.midi.IMidiDeviceOpenCallback sDefaultImpl;
    }
    static final int TRANSACTION_onDeviceOpened = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.midi.IMidiDeviceOpenCallback impl) {
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
    public static android.media.midi.IMidiDeviceOpenCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDeviceOpened(android.media.midi.IMidiDeviceServer server, android.os.IBinder token) throws android.os.RemoteException;
}
