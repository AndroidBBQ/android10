/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.media;
/**
 * Binder interface for {@link android.car.media.CarAudioManager}.
 * Check {@link android.car.media.CarAudioManager} APIs for expected behavior of each call.
 *
 * @hide
 */
public interface ICarAudio extends android.os.IInterface
{
  /** Default implementation for ICarAudio. */
  public static class Default implements android.car.media.ICarAudio
  {
    @Override public boolean isDynamicRoutingEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setGroupVolume(int zoneId, int groupId, int index, int flags) throws android.os.RemoteException
    {
    }
    @Override public int getGroupMaxVolume(int zoneId, int groupId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getGroupMinVolume(int zoneId, int groupId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getGroupVolume(int zoneId, int groupId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setFadeTowardFront(float value) throws android.os.RemoteException
    {
    }
    @Override public void setBalanceTowardRight(float value) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String[] getExternalSources() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.car.media.CarAudioPatchHandle createAudioPatch(java.lang.String sourceAddress, int usage, int gainInMillibels) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void releaseAudioPatch(android.car.media.CarAudioPatchHandle patch) throws android.os.RemoteException
    {
    }
    @Override public int getVolumeGroupCount(int zoneId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getVolumeGroupIdForUsage(int zoneId, int usage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int[] getUsagesForVolumeGroupId(int zoneId, int groupId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int[] getAudioZoneIds() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getZoneIdForUid(int uid) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean setZoneIdForUid(int zoneId, int uid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean clearZoneIdForUid(int uid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getZoneIdForDisplayPortId(byte displayPortId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * IBinder is ICarVolumeCallback but passed as IBinder due to aidl hidden.
         */
    @Override public void registerVolumeCallback(android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    @Override public void unregisterVolumeCallback(android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.media.ICarAudio
  {
    private static final java.lang.String DESCRIPTOR = "android.car.media.ICarAudio";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.media.ICarAudio interface,
     * generating a proxy if needed.
     */
    public static android.car.media.ICarAudio asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.media.ICarAudio))) {
        return ((android.car.media.ICarAudio)iin);
      }
      return new android.car.media.ICarAudio.Stub.Proxy(obj);
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
        case TRANSACTION_isDynamicRoutingEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDynamicRoutingEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setGroupVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.setGroupVolume(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGroupMaxVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getGroupMaxVolume(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getGroupMinVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getGroupMinVolume(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getGroupVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getGroupVolume(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setFadeTowardFront:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.setFadeTowardFront(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setBalanceTowardRight:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.setBalanceTowardRight(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getExternalSources:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getExternalSources();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_createAudioPatch:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.car.media.CarAudioPatchHandle _result = this.createAudioPatch(_arg0, _arg1, _arg2);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_releaseAudioPatch:
        {
          data.enforceInterface(descriptor);
          android.car.media.CarAudioPatchHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.media.CarAudioPatchHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.releaseAudioPatch(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVolumeGroupCount:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getVolumeGroupCount(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getVolumeGroupIdForUsage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getVolumeGroupIdForUsage(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getUsagesForVolumeGroupId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int[] _result = this.getUsagesForVolumeGroupId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getAudioZoneIds:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getAudioZoneIds();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getZoneIdForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getZoneIdForUid(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setZoneIdForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setZoneIdForUid(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_clearZoneIdForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.clearZoneIdForUid(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getZoneIdForDisplayPortId:
        {
          data.enforceInterface(descriptor);
          byte _arg0;
          _arg0 = data.readByte();
          int _result = this.getZoneIdForDisplayPortId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_registerVolumeCallback:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.registerVolumeCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterVolumeCallback:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.unregisterVolumeCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.media.ICarAudio
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
      @Override public boolean isDynamicRoutingEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDynamicRoutingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDynamicRoutingEnabled();
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
      @Override public void setGroupVolume(int zoneId, int groupId, int index, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(groupId);
          _data.writeInt(index);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGroupVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGroupVolume(zoneId, groupId, index, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getGroupMaxVolume(int zoneId, int groupId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(groupId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupMaxVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupMaxVolume(zoneId, groupId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getGroupMinVolume(int zoneId, int groupId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(groupId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupMinVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupMinVolume(zoneId, groupId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getGroupVolume(int zoneId, int groupId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(groupId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupVolume(zoneId, groupId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFadeTowardFront(float value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFadeTowardFront, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFadeTowardFront(value);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setBalanceTowardRight(float value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBalanceTowardRight, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBalanceTowardRight(value);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String[] getExternalSources() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getExternalSources, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getExternalSources();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.car.media.CarAudioPatchHandle createAudioPatch(java.lang.String sourceAddress, int usage, int gainInMillibels) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.media.CarAudioPatchHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(sourceAddress);
          _data.writeInt(usage);
          _data.writeInt(gainInMillibels);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createAudioPatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createAudioPatch(sourceAddress, usage, gainInMillibels);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.media.CarAudioPatchHandle.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void releaseAudioPatch(android.car.media.CarAudioPatchHandle patch) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((patch!=null)) {
            _data.writeInt(1);
            patch.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseAudioPatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseAudioPatch(patch);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getVolumeGroupCount(int zoneId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVolumeGroupCount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVolumeGroupCount(zoneId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getVolumeGroupIdForUsage(int zoneId, int usage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(usage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVolumeGroupIdForUsage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVolumeGroupIdForUsage(zoneId, usage);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int[] getUsagesForVolumeGroupId(int zoneId, int groupId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(groupId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUsagesForVolumeGroupId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUsagesForVolumeGroupId(zoneId, groupId);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int[] getAudioZoneIds() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAudioZoneIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAudioZoneIds();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getZoneIdForUid(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getZoneIdForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getZoneIdForUid(uid);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean setZoneIdForUid(int zoneId, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(zoneId);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setZoneIdForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setZoneIdForUid(zoneId, uid);
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
      @Override public boolean clearZoneIdForUid(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearZoneIdForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearZoneIdForUid(uid);
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
      @Override public int getZoneIdForDisplayPortId(byte displayPortId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByte(displayPortId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getZoneIdForDisplayPortId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getZoneIdForDisplayPortId(displayPortId);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * IBinder is ICarVolumeCallback but passed as IBinder due to aidl hidden.
           */
      @Override public void registerVolumeCallback(android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerVolumeCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerVolumeCallback(binder);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterVolumeCallback(android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterVolumeCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterVolumeCallback(binder);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.media.ICarAudio sDefaultImpl;
    }
    static final int TRANSACTION_isDynamicRoutingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setGroupVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getGroupMaxVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getGroupMinVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getGroupVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setFadeTowardFront = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setBalanceTowardRight = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getExternalSources = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_createAudioPatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_releaseAudioPatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getVolumeGroupCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getVolumeGroupIdForUsage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getUsagesForVolumeGroupId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getAudioZoneIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getZoneIdForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setZoneIdForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_clearZoneIdForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getZoneIdForDisplayPortId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_registerVolumeCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_unregisterVolumeCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    public static boolean setDefaultImpl(android.car.media.ICarAudio impl) {
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
    public static android.car.media.ICarAudio getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isDynamicRoutingEnabled() throws android.os.RemoteException;
  public void setGroupVolume(int zoneId, int groupId, int index, int flags) throws android.os.RemoteException;
  public int getGroupMaxVolume(int zoneId, int groupId) throws android.os.RemoteException;
  public int getGroupMinVolume(int zoneId, int groupId) throws android.os.RemoteException;
  public int getGroupVolume(int zoneId, int groupId) throws android.os.RemoteException;
  public void setFadeTowardFront(float value) throws android.os.RemoteException;
  public void setBalanceTowardRight(float value) throws android.os.RemoteException;
  public java.lang.String[] getExternalSources() throws android.os.RemoteException;
  public android.car.media.CarAudioPatchHandle createAudioPatch(java.lang.String sourceAddress, int usage, int gainInMillibels) throws android.os.RemoteException;
  public void releaseAudioPatch(android.car.media.CarAudioPatchHandle patch) throws android.os.RemoteException;
  public int getVolumeGroupCount(int zoneId) throws android.os.RemoteException;
  public int getVolumeGroupIdForUsage(int zoneId, int usage) throws android.os.RemoteException;
  public int[] getUsagesForVolumeGroupId(int zoneId, int groupId) throws android.os.RemoteException;
  public int[] getAudioZoneIds() throws android.os.RemoteException;
  public int getZoneIdForUid(int uid) throws android.os.RemoteException;
  public boolean setZoneIdForUid(int zoneId, int uid) throws android.os.RemoteException;
  public boolean clearZoneIdForUid(int uid) throws android.os.RemoteException;
  public int getZoneIdForDisplayPortId(byte displayPortId) throws android.os.RemoteException;
  /**
       * IBinder is ICarVolumeCallback but passed as IBinder due to aidl hidden.
       */
  public void registerVolumeCallback(android.os.IBinder binder) throws android.os.RemoteException;
  public void unregisterVolumeCallback(android.os.IBinder binder) throws android.os.RemoteException;
}
