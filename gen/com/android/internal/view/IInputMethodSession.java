/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Sub-interface of IInputMethod which is safe to give to client applications.
 * {@hide}
 */
public interface IInputMethodSession extends android.os.IInterface
{
  /** Default implementation for IInputMethodSession. */
  public static class Default implements com.android.internal.view.IInputMethodSession
  {
    @Override public void updateExtractedText(int token, android.view.inputmethod.ExtractedText text) throws android.os.RemoteException
    {
    }
    @Override public void updateSelection(int oldSelStart, int oldSelEnd, int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) throws android.os.RemoteException
    {
    }
    @Override public void viewClicked(boolean focusChanged) throws android.os.RemoteException
    {
    }
    @Override public void updateCursor(android.graphics.Rect newCursor) throws android.os.RemoteException
    {
    }
    @Override public void displayCompletions(android.view.inputmethod.CompletionInfo[] completions) throws android.os.RemoteException
    {
    }
    @Override public void appPrivateCommand(java.lang.String action, android.os.Bundle data) throws android.os.RemoteException
    {
    }
    @Override public void toggleSoftInput(int showFlags, int hideFlags) throws android.os.RemoteException
    {
    }
    @Override public void finishSession() throws android.os.RemoteException
    {
    }
    @Override public void updateCursorAnchorInfo(android.view.inputmethod.CursorAnchorInfo cursorAnchorInfo) throws android.os.RemoteException
    {
    }
    @Override public void notifyImeHidden() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IInputMethodSession
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IInputMethodSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IInputMethodSession interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IInputMethodSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IInputMethodSession))) {
        return ((com.android.internal.view.IInputMethodSession)iin);
      }
      return new com.android.internal.view.IInputMethodSession.Stub.Proxy(obj);
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
        case TRANSACTION_updateExtractedText:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.inputmethod.ExtractedText _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.inputmethod.ExtractedText.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.updateExtractedText(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_updateSelection:
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
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          this.updateSelection(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_viewClicked:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.viewClicked(_arg0);
          return true;
        }
        case TRANSACTION_updateCursor:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.updateCursor(_arg0);
          return true;
        }
        case TRANSACTION_displayCompletions:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.CompletionInfo[] _arg0;
          _arg0 = data.createTypedArray(android.view.inputmethod.CompletionInfo.CREATOR);
          this.displayCompletions(_arg0);
          return true;
        }
        case TRANSACTION_appPrivateCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.appPrivateCommand(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_toggleSoftInput:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.toggleSoftInput(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_finishSession:
        {
          data.enforceInterface(descriptor);
          this.finishSession();
          return true;
        }
        case TRANSACTION_updateCursorAnchorInfo:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.CursorAnchorInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.CursorAnchorInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.updateCursorAnchorInfo(_arg0);
          return true;
        }
        case TRANSACTION_notifyImeHidden:
        {
          data.enforceInterface(descriptor);
          this.notifyImeHidden();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IInputMethodSession
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
      @Override public void updateExtractedText(int token, android.view.inputmethod.ExtractedText text) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          if ((text!=null)) {
            _data.writeInt(1);
            text.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateExtractedText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateExtractedText(token, text);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateSelection(int oldSelStart, int oldSelEnd, int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(oldSelStart);
          _data.writeInt(oldSelEnd);
          _data.writeInt(newSelStart);
          _data.writeInt(newSelEnd);
          _data.writeInt(candidatesStart);
          _data.writeInt(candidatesEnd);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateSelection, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateSelection(oldSelStart, oldSelEnd, newSelStart, newSelEnd, candidatesStart, candidatesEnd);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void viewClicked(boolean focusChanged) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((focusChanged)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_viewClicked, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().viewClicked(focusChanged);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateCursor(android.graphics.Rect newCursor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((newCursor!=null)) {
            _data.writeInt(1);
            newCursor.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCursor, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateCursor(newCursor);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void displayCompletions(android.view.inputmethod.CompletionInfo[] completions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(completions, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_displayCompletions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().displayCompletions(completions);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void appPrivateCommand(java.lang.String action, android.os.Bundle data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(action);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_appPrivateCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().appPrivateCommand(action, data);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void toggleSoftInput(int showFlags, int hideFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(showFlags);
          _data.writeInt(hideFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleSoftInput, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleSoftInput(showFlags, hideFlags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void finishSession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSession();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateCursorAnchorInfo(android.view.inputmethod.CursorAnchorInfo cursorAnchorInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((cursorAnchorInfo!=null)) {
            _data.writeInt(1);
            cursorAnchorInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCursorAnchorInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateCursorAnchorInfo(cursorAnchorInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyImeHidden() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyImeHidden, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyImeHidden();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.view.IInputMethodSession sDefaultImpl;
    }
    static final int TRANSACTION_updateExtractedText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_updateSelection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_viewClicked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_updateCursor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_displayCompletions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_appPrivateCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_toggleSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_finishSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_updateCursorAnchorInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_notifyImeHidden = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(com.android.internal.view.IInputMethodSession impl) {
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
    public static com.android.internal.view.IInputMethodSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void updateExtractedText(int token, android.view.inputmethod.ExtractedText text) throws android.os.RemoteException;
  public void updateSelection(int oldSelStart, int oldSelEnd, int newSelStart, int newSelEnd, int candidatesStart, int candidatesEnd) throws android.os.RemoteException;
  public void viewClicked(boolean focusChanged) throws android.os.RemoteException;
  public void updateCursor(android.graphics.Rect newCursor) throws android.os.RemoteException;
  public void displayCompletions(android.view.inputmethod.CompletionInfo[] completions) throws android.os.RemoteException;
  public void appPrivateCommand(java.lang.String action, android.os.Bundle data) throws android.os.RemoteException;
  public void toggleSoftInput(int showFlags, int hideFlags) throws android.os.RemoteException;
  public void finishSession() throws android.os.RemoteException;
  public void updateCursorAnchorInfo(android.view.inputmethod.CursorAnchorInfo cursorAnchorInfo) throws android.os.RemoteException;
  public void notifyImeHidden() throws android.os.RemoteException;
}
