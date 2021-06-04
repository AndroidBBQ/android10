/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Top-level interface to an input method component (implemented in a
 * Service).
 * {@hide}
 */
public interface IInputMethod extends android.os.IInterface
{
  /** Default implementation for IInputMethod. */
  public static class Default implements com.android.internal.view.IInputMethod
  {
    @Override public void initializeInternal(android.os.IBinder token, int displayId, com.android.internal.inputmethod.IInputMethodPrivilegedOperations privOps) throws android.os.RemoteException
    {
    }
    @Override public void bindInput(android.view.inputmethod.InputBinding binding) throws android.os.RemoteException
    {
    }
    @Override public void unbindInput() throws android.os.RemoteException
    {
    }
    @Override public void startInput(android.os.IBinder startInputToken, com.android.internal.view.IInputContext inputContext, int missingMethods, android.view.inputmethod.EditorInfo attribute, boolean restarting, boolean preRenderImeViews) throws android.os.RemoteException
    {
    }
    @Override public void createSession(android.view.InputChannel channel, com.android.internal.view.IInputSessionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setSessionEnabled(com.android.internal.view.IInputMethodSession session, boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void revokeSession(com.android.internal.view.IInputMethodSession session) throws android.os.RemoteException
    {
    }
    @Override public void showSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
    }
    @Override public void hideSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
    }
    @Override public void changeInputMethodSubtype(android.view.inputmethod.InputMethodSubtype subtype) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IInputMethod
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IInputMethod";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IInputMethod interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IInputMethod asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IInputMethod))) {
        return ((com.android.internal.view.IInputMethod)iin);
      }
      return new com.android.internal.view.IInputMethod.Stub.Proxy(obj);
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
        case TRANSACTION_initializeInternal:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          com.android.internal.inputmethod.IInputMethodPrivilegedOperations _arg2;
          _arg2 = com.android.internal.inputmethod.IInputMethodPrivilegedOperations.Stub.asInterface(data.readStrongBinder());
          this.initializeInternal(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_bindInput:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.InputBinding _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.InputBinding.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.bindInput(_arg0);
          return true;
        }
        case TRANSACTION_unbindInput:
        {
          data.enforceInterface(descriptor);
          this.unbindInput();
          return true;
        }
        case TRANSACTION_startInput:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          com.android.internal.view.IInputContext _arg1;
          _arg1 = com.android.internal.view.IInputContext.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          android.view.inputmethod.EditorInfo _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.view.inputmethod.EditorInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          this.startInput(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_createSession:
        {
          data.enforceInterface(descriptor);
          android.view.InputChannel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InputChannel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          com.android.internal.view.IInputSessionCallback _arg1;
          _arg1 = com.android.internal.view.IInputSessionCallback.Stub.asInterface(data.readStrongBinder());
          this.createSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setSessionEnabled:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodSession _arg0;
          _arg0 = com.android.internal.view.IInputMethodSession.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setSessionEnabled(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_revokeSession:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodSession _arg0;
          _arg0 = com.android.internal.view.IInputMethodSession.Stub.asInterface(data.readStrongBinder());
          this.revokeSession(_arg0);
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
        case TRANSACTION_changeInputMethodSubtype:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.InputMethodSubtype _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.InputMethodSubtype.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.changeInputMethodSubtype(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IInputMethod
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
      @Override public void initializeInternal(android.os.IBinder token, int displayId, com.android.internal.inputmethod.IInputMethodPrivilegedOperations privOps) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(displayId);
          _data.writeStrongBinder((((privOps!=null))?(privOps.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_initializeInternal, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().initializeInternal(token, displayId, privOps);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void bindInput(android.view.inputmethod.InputBinding binding) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((binding!=null)) {
            _data.writeInt(1);
            binding.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_bindInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().bindInput(binding);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unbindInput() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unbindInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unbindInput();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startInput(android.os.IBinder startInputToken, com.android.internal.view.IInputContext inputContext, int missingMethods, android.view.inputmethod.EditorInfo attribute, boolean restarting, boolean preRenderImeViews) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(startInputToken);
          _data.writeStrongBinder((((inputContext!=null))?(inputContext.asBinder()):(null)));
          _data.writeInt(missingMethods);
          if ((attribute!=null)) {
            _data.writeInt(1);
            attribute.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((restarting)?(1):(0)));
          _data.writeInt(((preRenderImeViews)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startInput(startInputToken, inputContext, missingMethods, attribute, restarting, preRenderImeViews);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createSession(android.view.InputChannel channel, com.android.internal.view.IInputSessionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((channel!=null)) {
            _data.writeInt(1);
            channel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createSession(channel, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setSessionEnabled(com.android.internal.view.IInputMethodSession session, boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSessionEnabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSessionEnabled(session, enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void revokeSession(com.android.internal.view.IInputMethodSession session) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeSession(session);
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
      @Override public void changeInputMethodSubtype(android.view.inputmethod.InputMethodSubtype subtype) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((subtype!=null)) {
            _data.writeInt(1);
            subtype.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_changeInputMethodSubtype, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().changeInputMethodSubtype(subtype);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.view.IInputMethod sDefaultImpl;
    }
    static final int TRANSACTION_initializeInternal = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_bindInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unbindInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_startInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_createSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setSessionEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_revokeSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_showSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_hideSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_changeInputMethodSubtype = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(com.android.internal.view.IInputMethod impl) {
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
    public static com.android.internal.view.IInputMethod getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void initializeInternal(android.os.IBinder token, int displayId, com.android.internal.inputmethod.IInputMethodPrivilegedOperations privOps) throws android.os.RemoteException;
  public void bindInput(android.view.inputmethod.InputBinding binding) throws android.os.RemoteException;
  public void unbindInput() throws android.os.RemoteException;
  public void startInput(android.os.IBinder startInputToken, com.android.internal.view.IInputContext inputContext, int missingMethods, android.view.inputmethod.EditorInfo attribute, boolean restarting, boolean preRenderImeViews) throws android.os.RemoteException;
  public void createSession(android.view.InputChannel channel, com.android.internal.view.IInputSessionCallback callback) throws android.os.RemoteException;
  public void setSessionEnabled(com.android.internal.view.IInputMethodSession session, boolean enabled) throws android.os.RemoteException;
  public void revokeSession(com.android.internal.view.IInputMethodSession session) throws android.os.RemoteException;
  public void showSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  public void hideSoftInput(int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  public void changeInputMethodSubtype(android.view.inputmethod.InputMethodSubtype subtype) throws android.os.RemoteException;
}
