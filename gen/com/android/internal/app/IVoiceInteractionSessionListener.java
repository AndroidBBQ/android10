/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
public interface IVoiceInteractionSessionListener extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractionSessionListener. */
  public static class Default implements com.android.internal.app.IVoiceInteractionSessionListener
  {
    /**
         * Called when a voice session is shown.
         */
    @Override public void onVoiceSessionShown() throws android.os.RemoteException
    {
    }
    /**
         * Called when a voice session is hidden.
         */
    @Override public void onVoiceSessionHidden() throws android.os.RemoteException
    {
    }
    /**
         * Called when UI hints were received.
         */
    @Override public void onSetUiHints(android.os.Bundle args) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IVoiceInteractionSessionListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IVoiceInteractionSessionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IVoiceInteractionSessionListener interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IVoiceInteractionSessionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IVoiceInteractionSessionListener))) {
        return ((com.android.internal.app.IVoiceInteractionSessionListener)iin);
      }
      return new com.android.internal.app.IVoiceInteractionSessionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onVoiceSessionShown:
        {
          data.enforceInterface(descriptor);
          this.onVoiceSessionShown();
          return true;
        }
        case TRANSACTION_onVoiceSessionHidden:
        {
          data.enforceInterface(descriptor);
          this.onVoiceSessionHidden();
          return true;
        }
        case TRANSACTION_onSetUiHints:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSetUiHints(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IVoiceInteractionSessionListener
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
      /**
           * Called when a voice session is shown.
           */
      @Override public void onVoiceSessionShown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVoiceSessionShown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVoiceSessionShown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when a voice session is hidden.
           */
      @Override public void onVoiceSessionHidden() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVoiceSessionHidden, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVoiceSessionHidden();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when UI hints were received.
           */
      @Override public void onSetUiHints(android.os.Bundle args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetUiHints, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetUiHints(args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.app.IVoiceInteractionSessionListener sDefaultImpl;
    }
    static final int TRANSACTION_onVoiceSessionShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onVoiceSessionHidden = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSetUiHints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.app.IVoiceInteractionSessionListener impl) {
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
    public static com.android.internal.app.IVoiceInteractionSessionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when a voice session is shown.
       */
  public void onVoiceSessionShown() throws android.os.RemoteException;
  /**
       * Called when a voice session is hidden.
       */
  public void onVoiceSessionHidden() throws android.os.RemoteException;
  /**
       * Called when UI hints were received.
       */
  public void onSetUiHints(android.os.Bundle args) throws android.os.RemoteException;
}
