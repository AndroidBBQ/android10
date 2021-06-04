/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.voice;
/**
 * @hide
 */
public interface IVoiceInteractionService extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractionService. */
  public static class Default implements android.service.voice.IVoiceInteractionService
  {
    @Override public void ready() throws android.os.RemoteException
    {
    }
    @Override public void soundModelsChanged() throws android.os.RemoteException
    {
    }
    @Override public void shutdown() throws android.os.RemoteException
    {
    }
    @Override public void launchVoiceAssistFromKeyguard() throws android.os.RemoteException
    {
    }
    @Override public void getActiveServiceSupportedActions(java.util.List<java.lang.String> voiceActions, com.android.internal.app.IVoiceActionCheckCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.voice.IVoiceInteractionService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.voice.IVoiceInteractionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.voice.IVoiceInteractionService interface,
     * generating a proxy if needed.
     */
    public static android.service.voice.IVoiceInteractionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.voice.IVoiceInteractionService))) {
        return ((android.service.voice.IVoiceInteractionService)iin);
      }
      return new android.service.voice.IVoiceInteractionService.Stub.Proxy(obj);
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
        case TRANSACTION_ready:
        {
          data.enforceInterface(descriptor);
          this.ready();
          return true;
        }
        case TRANSACTION_soundModelsChanged:
        {
          data.enforceInterface(descriptor);
          this.soundModelsChanged();
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          this.shutdown();
          return true;
        }
        case TRANSACTION_launchVoiceAssistFromKeyguard:
        {
          data.enforceInterface(descriptor);
          this.launchVoiceAssistFromKeyguard();
          return true;
        }
        case TRANSACTION_getActiveServiceSupportedActions:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          com.android.internal.app.IVoiceActionCheckCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceActionCheckCallback.Stub.asInterface(data.readStrongBinder());
          this.getActiveServiceSupportedActions(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.voice.IVoiceInteractionService
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
      @Override public void ready() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_ready, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().ready();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void soundModelsChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_soundModelsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().soundModelsChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void shutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void launchVoiceAssistFromKeyguard() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchVoiceAssistFromKeyguard, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().launchVoiceAssistFromKeyguard();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getActiveServiceSupportedActions(java.util.List<java.lang.String> voiceActions, com.android.internal.app.IVoiceActionCheckCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(voiceActions);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveServiceSupportedActions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getActiveServiceSupportedActions(voiceActions, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.voice.IVoiceInteractionService sDefaultImpl;
    }
    static final int TRANSACTION_ready = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_soundModelsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_launchVoiceAssistFromKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getActiveServiceSupportedActions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.service.voice.IVoiceInteractionService impl) {
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
    public static android.service.voice.IVoiceInteractionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void ready() throws android.os.RemoteException;
  public void soundModelsChanged() throws android.os.RemoteException;
  public void shutdown() throws android.os.RemoteException;
  public void launchVoiceAssistFromKeyguard() throws android.os.RemoteException;
  public void getActiveServiceSupportedActions(java.util.List<java.lang.String> voiceActions, com.android.internal.app.IVoiceActionCheckCallback callback) throws android.os.RemoteException;
}
