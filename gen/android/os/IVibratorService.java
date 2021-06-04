/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/** {@hide} */
public interface IVibratorService extends android.os.IInterface
{
  /** Default implementation for IVibratorService. */
  public static class Default implements android.os.IVibratorService
  {
    @Override public boolean hasVibrator() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean hasAmplitudeControl() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void vibrate(int uid, java.lang.String opPkg, android.os.VibrationEffect effect, android.media.AudioAttributes attributes, java.lang.String reason, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void cancelVibrate(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IVibratorService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IVibratorService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IVibratorService interface,
     * generating a proxy if needed.
     */
    public static android.os.IVibratorService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IVibratorService))) {
        return ((android.os.IVibratorService)iin);
      }
      return new android.os.IVibratorService.Stub.Proxy(obj);
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
        case TRANSACTION_hasVibrator:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasVibrator();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hasAmplitudeControl:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasAmplitudeControl();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_vibrate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.VibrationEffect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.VibrationEffect.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.media.AudioAttributes _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.media.AudioAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.IBinder _arg5;
          _arg5 = data.readStrongBinder();
          this.vibrate(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelVibrate:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.cancelVibrate(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IVibratorService
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
      @Override public boolean hasVibrator() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasVibrator, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasVibrator();
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
      @Override public boolean hasAmplitudeControl() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasAmplitudeControl, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasAmplitudeControl();
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
      @Override public void vibrate(int uid, java.lang.String opPkg, android.os.VibrationEffect effect, android.media.AudioAttributes attributes, java.lang.String reason, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(opPkg);
          if ((effect!=null)) {
            _data.writeInt(1);
            effect.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((attributes!=null)) {
            _data.writeInt(1);
            attributes.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(reason);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_vibrate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().vibrate(uid, opPkg, effect, attributes, reason, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelVibrate(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelVibrate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelVibrate(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.IVibratorService sDefaultImpl;
    }
    static final int TRANSACTION_hasVibrator = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_hasAmplitudeControl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_vibrate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_cancelVibrate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.os.IVibratorService impl) {
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
    public static android.os.IVibratorService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean hasVibrator() throws android.os.RemoteException;
  public boolean hasAmplitudeControl() throws android.os.RemoteException;
  public void vibrate(int uid, java.lang.String opPkg, android.os.VibrationEffect effect, android.media.AudioAttributes attributes, java.lang.String reason, android.os.IBinder token) throws android.os.RemoteException;
  public void cancelVibrate(android.os.IBinder token) throws android.os.RemoteException;
}
