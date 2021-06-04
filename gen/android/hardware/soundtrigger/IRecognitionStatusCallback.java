/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.soundtrigger;
/**
 * @hide
 */
public interface IRecognitionStatusCallback extends android.os.IInterface
{
  /** Default implementation for IRecognitionStatusCallback. */
  public static class Default implements android.hardware.soundtrigger.IRecognitionStatusCallback
  {
    /**
         * Called when the keyphrase is spoken.
         *
         * @param recognitionEvent Object containing data relating to the
         *                         keyphrase recognition event such as keyphrase
         *                         extras.
         */
    @Override public void onKeyphraseDetected(android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionEvent recognitionEvent) throws android.os.RemoteException
    {
    }
    /**
         * Called when a generic sound trigger event is witnessed.
         *
         * @param recognitionEvent Object containing data relating to the
         *                         recognition event such as trigger audio data (if
         *                         requested).
         */
    @Override public void onGenericSoundTriggerDetected(android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent recognitionEvent) throws android.os.RemoteException
    {
    }
    /**
         * Called when the detection fails due to an error.
         *
         * @param status The error code that was seen.
         */
    @Override public void onError(int status) throws android.os.RemoteException
    {
    }
    /**
         * Called when the recognition is paused temporarily for some reason.
         */
    @Override public void onRecognitionPaused() throws android.os.RemoteException
    {
    }
    /**
         * Called when the recognition is resumed after it was temporarily paused.
         */
    @Override public void onRecognitionResumed() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.soundtrigger.IRecognitionStatusCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.soundtrigger.IRecognitionStatusCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.soundtrigger.IRecognitionStatusCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.soundtrigger.IRecognitionStatusCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.soundtrigger.IRecognitionStatusCallback))) {
        return ((android.hardware.soundtrigger.IRecognitionStatusCallback)iin);
      }
      return new android.hardware.soundtrigger.IRecognitionStatusCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onKeyphraseDetected:
        {
          data.enforceInterface(descriptor);
          android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onKeyphraseDetected(_arg0);
          return true;
        }
        case TRANSACTION_onGenericSoundTriggerDetected:
        {
          data.enforceInterface(descriptor);
          android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onGenericSoundTriggerDetected(_arg0);
          return true;
        }
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onError(_arg0);
          return true;
        }
        case TRANSACTION_onRecognitionPaused:
        {
          data.enforceInterface(descriptor);
          this.onRecognitionPaused();
          return true;
        }
        case TRANSACTION_onRecognitionResumed:
        {
          data.enforceInterface(descriptor);
          this.onRecognitionResumed();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.soundtrigger.IRecognitionStatusCallback
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
           * Called when the keyphrase is spoken.
           *
           * @param recognitionEvent Object containing data relating to the
           *                         keyphrase recognition event such as keyphrase
           *                         extras.
           */
      @Override public void onKeyphraseDetected(android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionEvent recognitionEvent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((recognitionEvent!=null)) {
            _data.writeInt(1);
            recognitionEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onKeyphraseDetected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onKeyphraseDetected(recognitionEvent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when a generic sound trigger event is witnessed.
           *
           * @param recognitionEvent Object containing data relating to the
           *                         recognition event such as trigger audio data (if
           *                         requested).
           */
      @Override public void onGenericSoundTriggerDetected(android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent recognitionEvent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((recognitionEvent!=null)) {
            _data.writeInt(1);
            recognitionEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGenericSoundTriggerDetected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGenericSoundTriggerDetected(recognitionEvent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the detection fails due to an error.
           *
           * @param status The error code that was seen.
           */
      @Override public void onError(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the recognition is paused temporarily for some reason.
           */
      @Override public void onRecognitionPaused() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRecognitionPaused, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRecognitionPaused();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the recognition is resumed after it was temporarily paused.
           */
      @Override public void onRecognitionResumed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRecognitionResumed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRecognitionResumed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.soundtrigger.IRecognitionStatusCallback sDefaultImpl;
    }
    static final int TRANSACTION_onKeyphraseDetected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGenericSoundTriggerDetected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onRecognitionPaused = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onRecognitionResumed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.hardware.soundtrigger.IRecognitionStatusCallback impl) {
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
    public static android.hardware.soundtrigger.IRecognitionStatusCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the keyphrase is spoken.
       *
       * @param recognitionEvent Object containing data relating to the
       *                         keyphrase recognition event such as keyphrase
       *                         extras.
       */
  public void onKeyphraseDetected(android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionEvent recognitionEvent) throws android.os.RemoteException;
  /**
       * Called when a generic sound trigger event is witnessed.
       *
       * @param recognitionEvent Object containing data relating to the
       *                         recognition event such as trigger audio data (if
       *                         requested).
       */
  public void onGenericSoundTriggerDetected(android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent recognitionEvent) throws android.os.RemoteException;
  /**
       * Called when the detection fails due to an error.
       *
       * @param status The error code that was seen.
       */
  public void onError(int status) throws android.os.RemoteException;
  /**
       * Called when the recognition is paused temporarily for some reason.
       */
  public void onRecognitionPaused() throws android.os.RemoteException;
  /**
       * Called when the recognition is resumed after it was temporarily paused.
       */
  public void onRecognitionResumed() throws android.os.RemoteException;
}
