/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.voice;
/**
 * @hide
 */
public interface IVoiceInteractionSessionService extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractionSessionService. */
  public static class Default implements android.service.voice.IVoiceInteractionSessionService
  {
    @Override public void newSession(android.os.IBinder token, android.os.Bundle args, int startFlags) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.voice.IVoiceInteractionSessionService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.voice.IVoiceInteractionSessionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.voice.IVoiceInteractionSessionService interface,
     * generating a proxy if needed.
     */
    public static android.service.voice.IVoiceInteractionSessionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.voice.IVoiceInteractionSessionService))) {
        return ((android.service.voice.IVoiceInteractionSessionService)iin);
      }
      return new android.service.voice.IVoiceInteractionSessionService.Stub.Proxy(obj);
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
        case TRANSACTION_newSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.newSession(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.voice.IVoiceInteractionSessionService
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
      @Override public void newSession(android.os.IBinder token, android.os.Bundle args, int startFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(startFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_newSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().newSession(token, args, startFlags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.voice.IVoiceInteractionSessionService sDefaultImpl;
    }
    static final int TRANSACTION_newSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.voice.IVoiceInteractionSessionService impl) {
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
    public static android.service.voice.IVoiceInteractionSessionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void newSession(android.os.IBinder token, android.os.Bundle args, int startFlags) throws android.os.RemoteException;
}
