/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.radio;
/**
 * API to the broadcast radio service.
 *
 * {@hide}
 */
public interface IRadioService extends android.os.IInterface
{
  /** Default implementation for IRadioService. */
  public static class Default implements android.hardware.radio.IRadioService
  {
    @Override public java.util.List<android.hardware.radio.RadioManager.ModuleProperties> listModules() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.hardware.radio.ITuner openTuner(int moduleId, android.hardware.radio.RadioManager.BandConfig bandConfig, boolean withAudio, android.hardware.radio.ITunerCallback callback) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.hardware.radio.ICloseHandle addAnnouncementListener(int[] enabledTypes, android.hardware.radio.IAnnouncementListener listener) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.radio.IRadioService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.radio.IRadioService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.radio.IRadioService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.radio.IRadioService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.radio.IRadioService))) {
        return ((android.hardware.radio.IRadioService)iin);
      }
      return new android.hardware.radio.IRadioService.Stub.Proxy(obj);
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
        case TRANSACTION_listModules:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.radio.RadioManager.ModuleProperties> _result = this.listModules();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_openTuner:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.radio.RadioManager.BandConfig _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.radio.RadioManager.BandConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          android.hardware.radio.ITunerCallback _arg3;
          _arg3 = android.hardware.radio.ITunerCallback.Stub.asInterface(data.readStrongBinder());
          android.hardware.radio.ITuner _result = this.openTuner(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_addAnnouncementListener:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          android.hardware.radio.IAnnouncementListener _arg1;
          _arg1 = android.hardware.radio.IAnnouncementListener.Stub.asInterface(data.readStrongBinder());
          android.hardware.radio.ICloseHandle _result = this.addAnnouncementListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.radio.IRadioService
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
      @Override public java.util.List<android.hardware.radio.RadioManager.ModuleProperties> listModules() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.radio.RadioManager.ModuleProperties> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listModules, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().listModules();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.radio.RadioManager.ModuleProperties.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.hardware.radio.ITuner openTuner(int moduleId, android.hardware.radio.RadioManager.BandConfig bandConfig, boolean withAudio, android.hardware.radio.ITunerCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.radio.ITuner _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(moduleId);
          if ((bandConfig!=null)) {
            _data.writeInt(1);
            bandConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((withAudio)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_openTuner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openTuner(moduleId, bandConfig, withAudio, callback);
          }
          _reply.readException();
          _result = android.hardware.radio.ITuner.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.hardware.radio.ICloseHandle addAnnouncementListener(int[] enabledTypes, android.hardware.radio.IAnnouncementListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.radio.ICloseHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(enabledTypes);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAnnouncementListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addAnnouncementListener(enabledTypes, listener);
          }
          _reply.readException();
          _result = android.hardware.radio.ICloseHandle.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.hardware.radio.IRadioService sDefaultImpl;
    }
    static final int TRANSACTION_listModules = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_openTuner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_addAnnouncementListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.hardware.radio.IRadioService impl) {
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
    public static android.hardware.radio.IRadioService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.util.List<android.hardware.radio.RadioManager.ModuleProperties> listModules() throws android.os.RemoteException;
  public android.hardware.radio.ITuner openTuner(int moduleId, android.hardware.radio.RadioManager.BandConfig bandConfig, boolean withAudio, android.hardware.radio.ITunerCallback callback) throws android.os.RemoteException;
  public android.hardware.radio.ICloseHandle addAnnouncementListener(int[] enabledTypes, android.hardware.radio.IAnnouncementListener listener) throws android.os.RemoteException;
}
