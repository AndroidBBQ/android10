/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.media;
/**
 * Binder interface to callback the volume key events.
 *
 * @hide
 */
public interface ICarVolumeCallback extends android.os.IInterface
{
  /** Default implementation for ICarVolumeCallback. */
  public static class Default implements android.car.media.ICarVolumeCallback
  {
    /**
         * This is called whenever a group volume is changed.
         * The changed-to volume index is not included, the caller is encouraged to
         * get the current group volume index via CarAudioManager.
         */
    @Override public void onGroupVolumeChanged(int zoneId, int groupId, int flags) throws android.os.RemoteException
    {
    }
    /**
         * This is called whenever the master mute state is changed.
         * The changed-to master mute state is not included, the caller is encouraged to
         * get the current master mute state via AudioManager.
         */
    @Override public void onMasterMuteChanged(int zoneId, int flags) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.media.ICarVolumeCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.car.media.ICarVolumeCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.media.ICarVolumeCallback interface,
     * generating a proxy if needed.
     */
    public static android.car.media.ICarVolumeCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.media.ICarVolumeCallback))) {
        return ((android.car.media.ICarVolumeCallback)iin);
      }
      return new android.car.media.ICarVolumeCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onGroupVolumeChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onGroupVolumeChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onMasterMuteChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onMasterMuteChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.media.ICarVolumeCallback
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
           * This is called whenever a group volume is changed.
           * The changed-to volume index is not included, the caller is encouraged to
           * get the current group volume index via CarAudioManager.
           */
      @Override public void onGroupVolumeChanged(int zoneId, int groupId, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(groupId);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGroupVolumeChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGroupVolumeChanged(zoneId, groupId, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * This is called whenever the master mute state is changed.
           * The changed-to master mute state is not included, the caller is encouraged to
           * get the current master mute state via AudioManager.
           */
      @Override public void onMasterMuteChanged(int zoneId, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMasterMuteChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMasterMuteChanged(zoneId, flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.media.ICarVolumeCallback sDefaultImpl;
    }
    static final int TRANSACTION_onGroupVolumeChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onMasterMuteChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.media.ICarVolumeCallback impl) {
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
    public static android.car.media.ICarVolumeCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * This is called whenever a group volume is changed.
       * The changed-to volume index is not included, the caller is encouraged to
       * get the current group volume index via CarAudioManager.
       */
  public void onGroupVolumeChanged(int zoneId, int groupId, int flags) throws android.os.RemoteException;
  /**
       * This is called whenever the master mute state is changed.
       * The changed-to master mute state is not included, the caller is encouraged to
       * get the current master mute state via AudioManager.
       */
  public void onMasterMuteChanged(int zoneId, int flags) throws android.os.RemoteException;
}
