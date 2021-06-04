/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.audiopolicy;
/**
 * @hide
 */
public interface IAudioPolicyCallback extends android.os.IInterface
{
  /** Default implementation for IAudioPolicyCallback. */
  public static class Default implements android.media.audiopolicy.IAudioPolicyCallback
  {
    // callbacks for audio focus listening

    @Override public void notifyAudioFocusGrant(android.media.AudioFocusInfo afi, int requestResult) throws android.os.RemoteException
    {
    }
    @Override public void notifyAudioFocusLoss(android.media.AudioFocusInfo afi, boolean wasNotified) throws android.os.RemoteException
    {
    }
    // callback for audio focus policy

    @Override public void notifyAudioFocusRequest(android.media.AudioFocusInfo afi, int requestResult) throws android.os.RemoteException
    {
    }
    @Override public void notifyAudioFocusAbandon(android.media.AudioFocusInfo afi) throws android.os.RemoteException
    {
    }
    // callback for mix activity status update

    @Override public void notifyMixStateUpdate(java.lang.String regId, int state) throws android.os.RemoteException
    {
    }
    // callback for volume events

    @Override public void notifyVolumeAdjust(int adjustment) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.audiopolicy.IAudioPolicyCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.media.audiopolicy.IAudioPolicyCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.audiopolicy.IAudioPolicyCallback interface,
     * generating a proxy if needed.
     */
    public static android.media.audiopolicy.IAudioPolicyCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.audiopolicy.IAudioPolicyCallback))) {
        return ((android.media.audiopolicy.IAudioPolicyCallback)iin);
      }
      return new android.media.audiopolicy.IAudioPolicyCallback.Stub.Proxy(obj);
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
        case TRANSACTION_notifyAudioFocusGrant:
        {
          data.enforceInterface(descriptor);
          android.media.AudioFocusInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioFocusInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.notifyAudioFocusGrant(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyAudioFocusLoss:
        {
          data.enforceInterface(descriptor);
          android.media.AudioFocusInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioFocusInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.notifyAudioFocusLoss(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyAudioFocusRequest:
        {
          data.enforceInterface(descriptor);
          android.media.AudioFocusInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioFocusInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.notifyAudioFocusRequest(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyAudioFocusAbandon:
        {
          data.enforceInterface(descriptor);
          android.media.AudioFocusInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.AudioFocusInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyAudioFocusAbandon(_arg0);
          return true;
        }
        case TRANSACTION_notifyMixStateUpdate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.notifyMixStateUpdate(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyVolumeAdjust:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyVolumeAdjust(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.audiopolicy.IAudioPolicyCallback
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
      // callbacks for audio focus listening

      @Override public void notifyAudioFocusGrant(android.media.AudioFocusInfo afi, int requestResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((afi!=null)) {
            _data.writeInt(1);
            afi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(requestResult);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAudioFocusGrant, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAudioFocusGrant(afi, requestResult);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyAudioFocusLoss(android.media.AudioFocusInfo afi, boolean wasNotified) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((afi!=null)) {
            _data.writeInt(1);
            afi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((wasNotified)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAudioFocusLoss, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAudioFocusLoss(afi, wasNotified);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // callback for audio focus policy

      @Override public void notifyAudioFocusRequest(android.media.AudioFocusInfo afi, int requestResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((afi!=null)) {
            _data.writeInt(1);
            afi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(requestResult);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAudioFocusRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAudioFocusRequest(afi, requestResult);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyAudioFocusAbandon(android.media.AudioFocusInfo afi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((afi!=null)) {
            _data.writeInt(1);
            afi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAudioFocusAbandon, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAudioFocusAbandon(afi);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // callback for mix activity status update

      @Override public void notifyMixStateUpdate(java.lang.String regId, int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(regId);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyMixStateUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyMixStateUpdate(regId, state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // callback for volume events

      @Override public void notifyVolumeAdjust(int adjustment) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(adjustment);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyVolumeAdjust, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyVolumeAdjust(adjustment);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.audiopolicy.IAudioPolicyCallback sDefaultImpl;
    }
    static final int TRANSACTION_notifyAudioFocusGrant = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_notifyAudioFocusLoss = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyAudioFocusRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_notifyAudioFocusAbandon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_notifyMixStateUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_notifyVolumeAdjust = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.media.audiopolicy.IAudioPolicyCallback impl) {
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
    public static android.media.audiopolicy.IAudioPolicyCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // callbacks for audio focus listening

  public void notifyAudioFocusGrant(android.media.AudioFocusInfo afi, int requestResult) throws android.os.RemoteException;
  public void notifyAudioFocusLoss(android.media.AudioFocusInfo afi, boolean wasNotified) throws android.os.RemoteException;
  // callback for audio focus policy

  public void notifyAudioFocusRequest(android.media.AudioFocusInfo afi, int requestResult) throws android.os.RemoteException;
  public void notifyAudioFocusAbandon(android.media.AudioFocusInfo afi) throws android.os.RemoteException;
  // callback for mix activity status update

  public void notifyMixStateUpdate(java.lang.String regId, int state) throws android.os.RemoteException;
  // callback for volume events

  public void notifyVolumeAdjust(int adjustment) throws android.os.RemoteException;
}
