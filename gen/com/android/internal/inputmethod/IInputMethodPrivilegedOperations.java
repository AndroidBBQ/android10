/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.inputmethod;
/**
 * Defines priviledged operations that only the current IME is allowed to call.
 * Actual operations are implemented and handled by InputMethodManagerService.
 */
public interface IInputMethodPrivilegedOperations extends android.os.IInterface
{
  /** Default implementation for IInputMethodPrivilegedOperations. */
  public static class Default implements com.android.internal.inputmethod.IInputMethodPrivilegedOperations
  {
    @Override public void setImeWindowStatus(int vis, int backDisposition) throws android.os.RemoteException
    {
    }
    @Override public void reportStartInput(android.os.IBinder startInputToken) throws android.os.RemoteException
    {
    }
    @Override public com.android.internal.inputmethod.IInputContentUriToken createInputContentUriToken(android.net.Uri contentUri, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void reportFullscreenMode(boolean fullscreen) throws android.os.RemoteException
    {
    }
    @Override public void setInputMethod(java.lang.String id) throws android.os.RemoteException
    {
    }
    @Override public void setInputMethodAndSubtype(java.lang.String id, android.view.inputmethod.InputMethodSubtype subtype) throws android.os.RemoteException
    {
    }
    @Override public void hideMySoftInput(int flags) throws android.os.RemoteException
    {
    }
    @Override public void showMySoftInput(int flags) throws android.os.RemoteException
    {
    }
    @Override public void updateStatusIcon(java.lang.String packageName, int iconId) throws android.os.RemoteException
    {
    }
    @Override public boolean switchToPreviousInputMethod() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean switchToNextInputMethod(boolean onlyCurrentIme) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean shouldOfferSwitchingToNextInputMethod() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void notifyUserAction() throws android.os.RemoteException
    {
    }
    @Override public void reportPreRendered(android.view.inputmethod.EditorInfo info) throws android.os.RemoteException
    {
    }
    @Override public void applyImeVisibility(boolean setVisible) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.inputmethod.IInputMethodPrivilegedOperations
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.inputmethod.IInputMethodPrivilegedOperations";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.inputmethod.IInputMethodPrivilegedOperations interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.inputmethod.IInputMethodPrivilegedOperations asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.inputmethod.IInputMethodPrivilegedOperations))) {
        return ((com.android.internal.inputmethod.IInputMethodPrivilegedOperations)iin);
      }
      return new com.android.internal.inputmethod.IInputMethodPrivilegedOperations.Stub.Proxy(obj);
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
        case TRANSACTION_setImeWindowStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setImeWindowStatus(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportStartInput:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.reportStartInput(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createInputContentUriToken:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.inputmethod.IInputContentUriToken _result = this.createInputContentUriToken(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_reportFullscreenMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.reportFullscreenMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInputMethod:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setInputMethod(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInputMethodAndSubtype:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.view.inputmethod.InputMethodSubtype _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.inputmethod.InputMethodSubtype.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setInputMethodAndSubtype(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hideMySoftInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.hideMySoftInput(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showMySoftInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.showMySoftInput(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateStatusIcon:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.updateStatusIcon(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_switchToPreviousInputMethod:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.switchToPreviousInputMethod();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_switchToNextInputMethod:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.switchToNextInputMethod(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_shouldOfferSwitchingToNextInputMethod:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.shouldOfferSwitchingToNextInputMethod();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_notifyUserAction:
        {
          data.enforceInterface(descriptor);
          this.notifyUserAction();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportPreRendered:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.EditorInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.EditorInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.reportPreRendered(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_applyImeVisibility:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.applyImeVisibility(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.inputmethod.IInputMethodPrivilegedOperations
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
      @Override public void setImeWindowStatus(int vis, int backDisposition) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(vis);
          _data.writeInt(backDisposition);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImeWindowStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setImeWindowStatus(vis, backDisposition);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportStartInput(android.os.IBinder startInputToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(startInputToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportStartInput, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportStartInput(startInputToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public com.android.internal.inputmethod.IInputContentUriToken createInputContentUriToken(android.net.Uri contentUri, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.inputmethod.IInputContentUriToken _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((contentUri!=null)) {
            _data.writeInt(1);
            contentUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createInputContentUriToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createInputContentUriToken(contentUri, packageName);
          }
          _reply.readException();
          _result = com.android.internal.inputmethod.IInputContentUriToken.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void reportFullscreenMode(boolean fullscreen) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((fullscreen)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportFullscreenMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportFullscreenMode(fullscreen);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setInputMethod(java.lang.String id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInputMethod, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInputMethod(id);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setInputMethodAndSubtype(java.lang.String id, android.view.inputmethod.InputMethodSubtype subtype) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          if ((subtype!=null)) {
            _data.writeInt(1);
            subtype.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInputMethodAndSubtype, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInputMethodAndSubtype(id, subtype);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void hideMySoftInput(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideMySoftInput, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideMySoftInput(flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void showMySoftInput(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showMySoftInput, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showMySoftInput(flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateStatusIcon(java.lang.String packageName, int iconId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(iconId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateStatusIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateStatusIcon(packageName, iconId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean switchToPreviousInputMethod() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchToPreviousInputMethod, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().switchToPreviousInputMethod();
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
      @Override public boolean switchToNextInputMethod(boolean onlyCurrentIme) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((onlyCurrentIme)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchToNextInputMethod, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().switchToNextInputMethod(onlyCurrentIme);
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
      @Override public boolean shouldOfferSwitchingToNextInputMethod() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shouldOfferSwitchingToNextInputMethod, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().shouldOfferSwitchingToNextInputMethod();
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
      @Override public void notifyUserAction() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyUserAction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyUserAction();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportPreRendered(android.view.inputmethod.EditorInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportPreRendered, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportPreRendered(info);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void applyImeVisibility(boolean setVisible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((setVisible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyImeVisibility, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyImeVisibility(setVisible);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.inputmethod.IInputMethodPrivilegedOperations sDefaultImpl;
    }
    static final int TRANSACTION_setImeWindowStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_reportStartInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_createInputContentUriToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_reportFullscreenMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setInputMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setInputMethodAndSubtype = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_hideMySoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_showMySoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_updateStatusIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_switchToPreviousInputMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_switchToNextInputMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_shouldOfferSwitchingToNextInputMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_notifyUserAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_reportPreRendered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_applyImeVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    public static boolean setDefaultImpl(com.android.internal.inputmethod.IInputMethodPrivilegedOperations impl) {
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
    public static com.android.internal.inputmethod.IInputMethodPrivilegedOperations getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setImeWindowStatus(int vis, int backDisposition) throws android.os.RemoteException;
  public void reportStartInput(android.os.IBinder startInputToken) throws android.os.RemoteException;
  public com.android.internal.inputmethod.IInputContentUriToken createInputContentUriToken(android.net.Uri contentUri, java.lang.String packageName) throws android.os.RemoteException;
  public void reportFullscreenMode(boolean fullscreen) throws android.os.RemoteException;
  public void setInputMethod(java.lang.String id) throws android.os.RemoteException;
  public void setInputMethodAndSubtype(java.lang.String id, android.view.inputmethod.InputMethodSubtype subtype) throws android.os.RemoteException;
  public void hideMySoftInput(int flags) throws android.os.RemoteException;
  public void showMySoftInput(int flags) throws android.os.RemoteException;
  public void updateStatusIcon(java.lang.String packageName, int iconId) throws android.os.RemoteException;
  public boolean switchToPreviousInputMethod() throws android.os.RemoteException;
  public boolean switchToNextInputMethod(boolean onlyCurrentIme) throws android.os.RemoteException;
  public boolean shouldOfferSwitchingToNextInputMethod() throws android.os.RemoteException;
  public void notifyUserAction() throws android.os.RemoteException;
  public void reportPreRendered(android.view.inputmethod.EditorInfo info) throws android.os.RemoteException;
  public void applyImeVisibility(boolean setVisible) throws android.os.RemoteException;
}
