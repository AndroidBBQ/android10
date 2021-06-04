/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.inputmethod;
public interface IMultiClientInputMethodSession extends android.os.IInterface
{
  /** Default implementation for IMultiClientInputMethodSession. */
  public static class Default implements com.android.internal.inputmethod.IMultiClientInputMethodSession
  {
    @Override public void startInputOrWindowGainedFocus(com.android.internal.view.IInputContext inputContext, int missingMethods, android.view.inputmethod.EditorInfo attribute, int controlFlags, int softInputMode, int targetWindowHandle) throws android.os.RemoteException
    {
    }
    @Override public void showSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
    }
    @Override public void hideSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.inputmethod.IMultiClientInputMethodSession
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.inputmethod.IMultiClientInputMethodSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.inputmethod.IMultiClientInputMethodSession interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.inputmethod.IMultiClientInputMethodSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.inputmethod.IMultiClientInputMethodSession))) {
        return ((com.android.internal.inputmethod.IMultiClientInputMethodSession)iin);
      }
      return new com.android.internal.inputmethod.IMultiClientInputMethodSession.Stub.Proxy(obj);
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
        case TRANSACTION_startInputOrWindowGainedFocus:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputContext _arg0;
          _arg0 = com.android.internal.view.IInputContext.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.view.inputmethod.EditorInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.inputmethod.EditorInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          this.startInputOrWindowGainedFocus(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_showSoftInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ResultReceiver _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.showSoftInput(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_hideSoftInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.ResultReceiver _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.hideSoftInput(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.inputmethod.IMultiClientInputMethodSession
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
      @Override public void startInputOrWindowGainedFocus(com.android.internal.view.IInputContext inputContext, int missingMethods, android.view.inputmethod.EditorInfo attribute, int controlFlags, int softInputMode, int targetWindowHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((inputContext!=null))?(inputContext.asBinder()):(null)));
          _data.writeInt(missingMethods);
          if ((attribute!=null)) {
            _data.writeInt(1);
            attribute.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(controlFlags);
          _data.writeInt(softInputMode);
          _data.writeInt(targetWindowHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startInputOrWindowGainedFocus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startInputOrWindowGainedFocus(inputContext, missingMethods, attribute, controlFlags, softInputMode, targetWindowHandle);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          if ((resultReceiver!=null)) {
            _data.writeInt(1);
            resultReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_showSoftInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showSoftInput(flags, resultReceiver);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hideSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          if ((resultReceiver!=null)) {
            _data.writeInt(1);
            resultReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideSoftInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideSoftInput(flags, resultReceiver);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.inputmethod.IMultiClientInputMethodSession sDefaultImpl;
    }
    static final int TRANSACTION_startInputOrWindowGainedFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_showSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_hideSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.inputmethod.IMultiClientInputMethodSession impl) {
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
    public static com.android.internal.inputmethod.IMultiClientInputMethodSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void startInputOrWindowGainedFocus(com.android.internal.view.IInputContext inputContext, int missingMethods, android.view.inputmethod.EditorInfo attribute, int controlFlags, int softInputMode, int targetWindowHandle) throws android.os.RemoteException;
  public void showSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  public void hideSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
}
