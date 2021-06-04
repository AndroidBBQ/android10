/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.speech;
/**
 *  Listener for speech recognition events, used with RecognitionService.
 *  This gives you both the final recognition results, as well as various
 *  intermediate events that can be used to show visual feedback to the user.
 *  {@hide}
 */
public interface IRecognitionListener extends android.os.IInterface
{
  /** Default implementation for IRecognitionListener. */
  public static class Default implements android.speech.IRecognitionListener
  {
    /**
         * Called when the endpointer is ready for the user to start speaking.
         *
         * @param params parameters set by the recognition service. Reserved for future use.
         */
    @Override public void onReadyForSpeech(android.os.Bundle params) throws android.os.RemoteException
    {
    }
    /**
         * The user has started to speak.
         */
    @Override public void onBeginningOfSpeech() throws android.os.RemoteException
    {
    }
    /**
         * The sound level in the audio stream has changed.
         *
         * @param rmsdB the new RMS dB value
         */
    @Override public void onRmsChanged(float rmsdB) throws android.os.RemoteException
    {
    }
    /**
         * More sound has been received.
         *
         * @param buffer the byte buffer containing a sequence of 16-bit shorts.
         */
    @Override public void onBufferReceived(byte[] buffer) throws android.os.RemoteException
    {
    }
    /**
         * Called after the user stops speaking.
         */
    @Override public void onEndOfSpeech() throws android.os.RemoteException
    {
    }
    /**
         * A network or recognition error occurred.
         *
         * @param error code is defined in {@link SpeechRecognizer}
         */
    @Override public void onError(int error) throws android.os.RemoteException
    {
    }
    /**
         * Called when recognition results are ready.
         *
         * @param results a Bundle containing the most likely results (N-best list).
         */
    @Override public void onResults(android.os.Bundle results) throws android.os.RemoteException
    {
    }
    /**
         * Called when recognition partial results are ready.
         *
         * @param results a Bundle containing the current most likely result.
         */
    @Override public void onPartialResults(android.os.Bundle results) throws android.os.RemoteException
    {
    }
    /**
         * Reserved for adding future events.
         *
         * @param eventType the type of the occurred event
         * @param params a Bundle containing the passed parameters
         */
    @Override public void onEvent(int eventType, android.os.Bundle params) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.speech.IRecognitionListener
  {
    private static final java.lang.String DESCRIPTOR = "android.speech.IRecognitionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.speech.IRecognitionListener interface,
     * generating a proxy if needed.
     */
    public static android.speech.IRecognitionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.speech.IRecognitionListener))) {
        return ((android.speech.IRecognitionListener)iin);
      }
      return new android.speech.IRecognitionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onReadyForSpeech:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onReadyForSpeech(_arg0);
          return true;
        }
        case TRANSACTION_onBeginningOfSpeech:
        {
          data.enforceInterface(descriptor);
          this.onBeginningOfSpeech();
          return true;
        }
        case TRANSACTION_onRmsChanged:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.onRmsChanged(_arg0);
          return true;
        }
        case TRANSACTION_onBufferReceived:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.onBufferReceived(_arg0);
          return true;
        }
        case TRANSACTION_onEndOfSpeech:
        {
          data.enforceInterface(descriptor);
          this.onEndOfSpeech();
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
        case TRANSACTION_onResults:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onResults(_arg0);
          return true;
        }
        case TRANSACTION_onPartialResults:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPartialResults(_arg0);
          return true;
        }
        case TRANSACTION_onEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onEvent(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.speech.IRecognitionListener
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
           * Called when the endpointer is ready for the user to start speaking.
           *
           * @param params parameters set by the recognition service. Reserved for future use.
           */
      @Override public void onReadyForSpeech(android.os.Bundle params) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReadyForSpeech, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReadyForSpeech(params);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * The user has started to speak.
           */
      @Override public void onBeginningOfSpeech() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBeginningOfSpeech, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBeginningOfSpeech();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * The sound level in the audio stream has changed.
           *
           * @param rmsdB the new RMS dB value
           */
      @Override public void onRmsChanged(float rmsdB) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(rmsdB);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRmsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRmsChanged(rmsdB);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * More sound has been received.
           *
           * @param buffer the byte buffer containing a sequence of 16-bit shorts.
           */
      @Override public void onBufferReceived(byte[] buffer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(buffer);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBufferReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBufferReceived(buffer);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called after the user stops speaking.
           */
      @Override public void onEndOfSpeech() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEndOfSpeech, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEndOfSpeech();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * A network or recognition error occurred.
           *
           * @param error code is defined in {@link SpeechRecognizer}
           */
      @Override public void onError(int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when recognition results are ready.
           *
           * @param results a Bundle containing the most likely results (N-best list).
           */
      @Override public void onResults(android.os.Bundle results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((results!=null)) {
            _data.writeInt(1);
            results.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onResults, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onResults(results);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when recognition partial results are ready.
           *
           * @param results a Bundle containing the current most likely result.
           */
      @Override public void onPartialResults(android.os.Bundle results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((results!=null)) {
            _data.writeInt(1);
            results.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPartialResults, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPartialResults(results);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Reserved for adding future events.
           *
           * @param eventType the type of the occurred event
           * @param params a Bundle containing the passed parameters
           */
      @Override public void onEvent(int eventType, android.os.Bundle params) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(eventType);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEvent(eventType, params);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.speech.IRecognitionListener sDefaultImpl;
    }
    static final int TRANSACTION_onReadyForSpeech = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onBeginningOfSpeech = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onRmsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onBufferReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onEndOfSpeech = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onPartialResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.speech.IRecognitionListener impl) {
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
    public static android.speech.IRecognitionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the endpointer is ready for the user to start speaking.
       *
       * @param params parameters set by the recognition service. Reserved for future use.
       */
  public void onReadyForSpeech(android.os.Bundle params) throws android.os.RemoteException;
  /**
       * The user has started to speak.
       */
  public void onBeginningOfSpeech() throws android.os.RemoteException;
  /**
       * The sound level in the audio stream has changed.
       *
       * @param rmsdB the new RMS dB value
       */
  public void onRmsChanged(float rmsdB) throws android.os.RemoteException;
  /**
       * More sound has been received.
       *
       * @param buffer the byte buffer containing a sequence of 16-bit shorts.
       */
  public void onBufferReceived(byte[] buffer) throws android.os.RemoteException;
  /**
       * Called after the user stops speaking.
       */
  public void onEndOfSpeech() throws android.os.RemoteException;
  /**
       * A network or recognition error occurred.
       *
       * @param error code is defined in {@link SpeechRecognizer}
       */
  public void onError(int error) throws android.os.RemoteException;
  /**
       * Called when recognition results are ready.
       *
       * @param results a Bundle containing the most likely results (N-best list).
       */
  public void onResults(android.os.Bundle results) throws android.os.RemoteException;
  /**
       * Called when recognition partial results are ready.
       *
       * @param results a Bundle containing the current most likely result.
       */
  public void onPartialResults(android.os.Bundle results) throws android.os.RemoteException;
  /**
       * Reserved for adding future events.
       *
       * @param eventType the type of the occurred event
       * @param params a Bundle containing the passed parameters
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/speech/IRecognitionListener.aidl:86:1:86:25")
  public void onEvent(int eventType, android.os.Bundle params) throws android.os.RemoteException;
}
