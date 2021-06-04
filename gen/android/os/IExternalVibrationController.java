/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * {@hide}
 */
public interface IExternalVibrationController extends android.os.IInterface
{
  /** Default implementation for IExternalVibrationController. */
  public static class Default implements android.os.IExternalVibrationController
  {
    /**
         * A method to ask a currently playing vibration to mute (i.e. not vibrate).
         *
         * This method is only valid from the time that
         * {@link IExternalVibratorService#onExternalVibrationStart} returns until
         * {@link IExternalVibratorService#onExternalVibrationStop} returns.
         *
         * @return whether the mute operation was successful
         */
    @Override public boolean mute() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * A method to ask a currently playing vibration to unmute (i.e. start vibrating).
         *
         * This method is only valid from the time that
         * {@link IExternalVibratorService#onExternalVibrationStart} returns until
         * {@link IExternalVibratorService#onExternalVibrationStop} returns.
         *
         * @return whether the unmute operation was successful
         */
    @Override public boolean unmute() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IExternalVibrationController
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IExternalVibrationController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IExternalVibrationController interface,
     * generating a proxy if needed.
     */
    public static android.os.IExternalVibrationController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IExternalVibrationController))) {
        return ((android.os.IExternalVibrationController)iin);
      }
      return new android.os.IExternalVibrationController.Stub.Proxy(obj);
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
        case TRANSACTION_mute:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.mute();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unmute:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.unmute();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IExternalVibrationController
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
           * A method to ask a currently playing vibration to mute (i.e. not vibrate).
           *
           * This method is only valid from the time that
           * {@link IExternalVibratorService#onExternalVibrationStart} returns until
           * {@link IExternalVibratorService#onExternalVibrationStop} returns.
           *
           * @return whether the mute operation was successful
           */
      @Override public boolean mute() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_mute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().mute();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * A method to ask a currently playing vibration to unmute (i.e. start vibrating).
           *
           * This method is only valid from the time that
           * {@link IExternalVibratorService#onExternalVibrationStart} returns until
           * {@link IExternalVibratorService#onExternalVibrationStop} returns.
           *
           * @return whether the unmute operation was successful
           */
      @Override public boolean unmute() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unmute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unmute();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.IExternalVibrationController sDefaultImpl;
    }
    static final int TRANSACTION_mute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unmute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.os.IExternalVibrationController impl) {
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
    public static android.os.IExternalVibrationController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * A method to ask a currently playing vibration to mute (i.e. not vibrate).
       *
       * This method is only valid from the time that
       * {@link IExternalVibratorService#onExternalVibrationStart} returns until
       * {@link IExternalVibratorService#onExternalVibrationStop} returns.
       *
       * @return whether the mute operation was successful
       */
  public boolean mute() throws android.os.RemoteException;
  /**
       * A method to ask a currently playing vibration to unmute (i.e. start vibrating).
       *
       * This method is only valid from the time that
       * {@link IExternalVibratorService#onExternalVibrationStart} returns until
       * {@link IExternalVibratorService#onExternalVibrationStop} returns.
       *
       * @return whether the unmute operation was successful
       */
  public boolean unmute() throws android.os.RemoteException;
}
