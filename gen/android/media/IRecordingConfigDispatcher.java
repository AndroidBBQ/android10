/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * AIDL for the RecordingActivity monitor in AudioService to signal audio recording updates.
 *
 * {@hide}
 */
public interface IRecordingConfigDispatcher extends android.os.IInterface
{
  /** Default implementation for IRecordingConfigDispatcher. */
  public static class Default implements android.media.IRecordingConfigDispatcher
  {
    @Override public void dispatchRecordingConfigChange(java.util.List<android.media.AudioRecordingConfiguration> configs) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IRecordingConfigDispatcher
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IRecordingConfigDispatcher";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IRecordingConfigDispatcher interface,
     * generating a proxy if needed.
     */
    public static android.media.IRecordingConfigDispatcher asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IRecordingConfigDispatcher))) {
        return ((android.media.IRecordingConfigDispatcher)iin);
      }
      return new android.media.IRecordingConfigDispatcher.Stub.Proxy(obj);
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
        case TRANSACTION_dispatchRecordingConfigChange:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.media.AudioRecordingConfiguration> _arg0;
          _arg0 = data.createTypedArrayList(android.media.AudioRecordingConfiguration.CREATOR);
          this.dispatchRecordingConfigChange(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IRecordingConfigDispatcher
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
      @Override public void dispatchRecordingConfigChange(java.util.List<android.media.AudioRecordingConfiguration> configs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(configs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchRecordingConfigChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchRecordingConfigChange(configs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IRecordingConfigDispatcher sDefaultImpl;
    }
    static final int TRANSACTION_dispatchRecordingConfigChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.IRecordingConfigDispatcher impl) {
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
    public static android.media.IRecordingConfigDispatcher getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void dispatchRecordingConfigChange(java.util.List<android.media.AudioRecordingConfiguration> configs) throws android.os.RemoteException;
}
