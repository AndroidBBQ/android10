/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Interface from an input method to the application, allowing it to perform
 * edits on the current input field and other interactions with the application.
 * {@hide}
 */
public interface IInputContext extends android.os.IInterface
{
  /** Default implementation for IInputContext. */
  public static class Default implements com.android.internal.view.IInputContext
  {
    @Override public void getTextBeforeCursor(int length, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getTextAfterCursor(int length, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getCursorCapsMode(int reqModes, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getExtractedText(android.view.inputmethod.ExtractedTextRequest request, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void deleteSurroundingText(int beforeLength, int afterLength) throws android.os.RemoteException
    {
    }
    @Override public void deleteSurroundingTextInCodePoints(int beforeLength, int afterLength) throws android.os.RemoteException
    {
    }
    @Override public void setComposingText(java.lang.CharSequence text, int newCursorPosition) throws android.os.RemoteException
    {
    }
    @Override public void finishComposingText() throws android.os.RemoteException
    {
    }
    @Override public void commitText(java.lang.CharSequence text, int newCursorPosition) throws android.os.RemoteException
    {
    }
    @Override public void commitCompletion(android.view.inputmethod.CompletionInfo completion) throws android.os.RemoteException
    {
    }
    @Override public void commitCorrection(android.view.inputmethod.CorrectionInfo correction) throws android.os.RemoteException
    {
    }
    @Override public void setSelection(int start, int end) throws android.os.RemoteException
    {
    }
    @Override public void performEditorAction(int actionCode) throws android.os.RemoteException
    {
    }
    @Override public void performContextMenuAction(int id) throws android.os.RemoteException
    {
    }
    @Override public void beginBatchEdit() throws android.os.RemoteException
    {
    }
    @Override public void endBatchEdit() throws android.os.RemoteException
    {
    }
    @Override public void sendKeyEvent(android.view.KeyEvent event) throws android.os.RemoteException
    {
    }
    @Override public void clearMetaKeyStates(int states) throws android.os.RemoteException
    {
    }
    @Override public void performPrivateCommand(java.lang.String action, android.os.Bundle data) throws android.os.RemoteException
    {
    }
    @Override public void setComposingRegion(int start, int end) throws android.os.RemoteException
    {
    }
    @Override public void getSelectedText(int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void requestUpdateCursorAnchorInfo(int cursorUpdateMode, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void commitContent(android.view.inputmethod.InputContentInfo inputContentInfo, int flags, android.os.Bundle opts, int sec, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IInputContext
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IInputContext";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IInputContext interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IInputContext asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IInputContext))) {
        return ((com.android.internal.view.IInputContext)iin);
      }
      return new com.android.internal.view.IInputContext.Stub.Proxy(obj);
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
        case TRANSACTION_getTextBeforeCursor:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg3;
          _arg3 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.getTextBeforeCursor(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getTextAfterCursor:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg3;
          _arg3 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.getTextAfterCursor(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getCursorCapsMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg2;
          _arg2 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.getCursorCapsMode(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getExtractedText:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.ExtractedTextRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.ExtractedTextRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg3;
          _arg3 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.getExtractedText(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_deleteSurroundingText:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.deleteSurroundingText(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_deleteSurroundingTextInCodePoints:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.deleteSurroundingTextInCodePoints(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setComposingText:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.setComposingText(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_finishComposingText:
        {
          data.enforceInterface(descriptor);
          this.finishComposingText();
          return true;
        }
        case TRANSACTION_commitText:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.commitText(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_commitCompletion:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.CompletionInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.CompletionInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.commitCompletion(_arg0);
          return true;
        }
        case TRANSACTION_commitCorrection:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.CorrectionInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.CorrectionInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.commitCorrection(_arg0);
          return true;
        }
        case TRANSACTION_setSelection:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setSelection(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_performEditorAction:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.performEditorAction(_arg0);
          return true;
        }
        case TRANSACTION_performContextMenuAction:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.performContextMenuAction(_arg0);
          return true;
        }
        case TRANSACTION_beginBatchEdit:
        {
          data.enforceInterface(descriptor);
          this.beginBatchEdit();
          return true;
        }
        case TRANSACTION_endBatchEdit:
        {
          data.enforceInterface(descriptor);
          this.endBatchEdit();
          return true;
        }
        case TRANSACTION_sendKeyEvent:
        {
          data.enforceInterface(descriptor);
          android.view.KeyEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.sendKeyEvent(_arg0);
          return true;
        }
        case TRANSACTION_clearMetaKeyStates:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.clearMetaKeyStates(_arg0);
          return true;
        }
        case TRANSACTION_performPrivateCommand:
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
          this.performPrivateCommand(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setComposingRegion:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setComposingRegion(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getSelectedText:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg2;
          _arg2 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.getSelectedText(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_requestUpdateCursorAnchorInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg2;
          _arg2 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.requestUpdateCursorAnchorInfo(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_commitContent:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.InputContentInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.inputmethod.InputContentInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          com.android.internal.view.IInputContextCallback _arg4;
          _arg4 = com.android.internal.view.IInputContextCallback.Stub.asInterface(data.readStrongBinder());
          this.commitContent(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IInputContext
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
      @Override public void getTextBeforeCursor(int length, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(length);
          _data.writeInt(flags);
          _data.writeInt(seq);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTextBeforeCursor, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getTextBeforeCursor(length, flags, seq, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getTextAfterCursor(int length, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(length);
          _data.writeInt(flags);
          _data.writeInt(seq);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTextAfterCursor, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getTextAfterCursor(length, flags, seq, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getCursorCapsMode(int reqModes, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reqModes);
          _data.writeInt(seq);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCursorCapsMode, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getCursorCapsMode(reqModes, seq, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getExtractedText(android.view.inputmethod.ExtractedTextRequest request, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          _data.writeInt(seq);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getExtractedText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getExtractedText(request, flags, seq, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deleteSurroundingText(int beforeLength, int afterLength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(beforeLength);
          _data.writeInt(afterLength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteSurroundingText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteSurroundingText(beforeLength, afterLength);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deleteSurroundingTextInCodePoints(int beforeLength, int afterLength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(beforeLength);
          _data.writeInt(afterLength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteSurroundingTextInCodePoints, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteSurroundingTextInCodePoints(beforeLength, afterLength);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setComposingText(java.lang.CharSequence text, int newCursorPosition) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (text!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(text, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(newCursorPosition);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setComposingText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setComposingText(text, newCursorPosition);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void finishComposingText() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishComposingText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishComposingText();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void commitText(java.lang.CharSequence text, int newCursorPosition) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (text!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(text, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(newCursorPosition);
          boolean _status = mRemote.transact(Stub.TRANSACTION_commitText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().commitText(text, newCursorPosition);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void commitCompletion(android.view.inputmethod.CompletionInfo completion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((completion!=null)) {
            _data.writeInt(1);
            completion.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_commitCompletion, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().commitCompletion(completion);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void commitCorrection(android.view.inputmethod.CorrectionInfo correction) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((correction!=null)) {
            _data.writeInt(1);
            correction.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_commitCorrection, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().commitCorrection(correction);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setSelection(int start, int end) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(start);
          _data.writeInt(end);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSelection, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSelection(start, end);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void performEditorAction(int actionCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(actionCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performEditorAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performEditorAction(actionCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void performContextMenuAction(int id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performContextMenuAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performContextMenuAction(id);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void beginBatchEdit() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_beginBatchEdit, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().beginBatchEdit();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void endBatchEdit() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_endBatchEdit, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().endBatchEdit();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendKeyEvent(android.view.KeyEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendKeyEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendKeyEvent(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void clearMetaKeyStates(int states) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(states);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearMetaKeyStates, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearMetaKeyStates(states);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void performPrivateCommand(java.lang.String action, android.os.Bundle data) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_performPrivateCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performPrivateCommand(action, data);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setComposingRegion(int start, int end) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(start);
          _data.writeInt(end);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setComposingRegion, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setComposingRegion(start, end);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getSelectedText(int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          _data.writeInt(seq);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSelectedText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getSelectedText(flags, seq, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestUpdateCursorAnchorInfo(int cursorUpdateMode, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cursorUpdateMode);
          _data.writeInt(seq);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestUpdateCursorAnchorInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestUpdateCursorAnchorInfo(cursorUpdateMode, seq, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void commitContent(android.view.inputmethod.InputContentInfo inputContentInfo, int flags, android.os.Bundle opts, int sec, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((inputContentInfo!=null)) {
            _data.writeInt(1);
            inputContentInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          if ((opts!=null)) {
            _data.writeInt(1);
            opts.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sec);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_commitContent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().commitContent(inputContentInfo, flags, opts, sec, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.view.IInputContext sDefaultImpl;
    }
    static final int TRANSACTION_getTextBeforeCursor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getTextAfterCursor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCursorCapsMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getExtractedText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_deleteSurroundingText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_deleteSurroundingTextInCodePoints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setComposingText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_finishComposingText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_commitText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_commitCompletion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_commitCorrection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setSelection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_performEditorAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_performContextMenuAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_beginBatchEdit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_endBatchEdit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_sendKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_clearMetaKeyStates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_performPrivateCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setComposingRegion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getSelectedText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_requestUpdateCursorAnchorInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_commitContent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    public static boolean setDefaultImpl(com.android.internal.view.IInputContext impl) {
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
    public static com.android.internal.view.IInputContext getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void getTextBeforeCursor(int length, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
  public void getTextAfterCursor(int length, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
  public void getCursorCapsMode(int reqModes, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
  public void getExtractedText(android.view.inputmethod.ExtractedTextRequest request, int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
  public void deleteSurroundingText(int beforeLength, int afterLength) throws android.os.RemoteException;
  public void deleteSurroundingTextInCodePoints(int beforeLength, int afterLength) throws android.os.RemoteException;
  public void setComposingText(java.lang.CharSequence text, int newCursorPosition) throws android.os.RemoteException;
  public void finishComposingText() throws android.os.RemoteException;
  public void commitText(java.lang.CharSequence text, int newCursorPosition) throws android.os.RemoteException;
  public void commitCompletion(android.view.inputmethod.CompletionInfo completion) throws android.os.RemoteException;
  public void commitCorrection(android.view.inputmethod.CorrectionInfo correction) throws android.os.RemoteException;
  public void setSelection(int start, int end) throws android.os.RemoteException;
  public void performEditorAction(int actionCode) throws android.os.RemoteException;
  public void performContextMenuAction(int id) throws android.os.RemoteException;
  public void beginBatchEdit() throws android.os.RemoteException;
  public void endBatchEdit() throws android.os.RemoteException;
  public void sendKeyEvent(android.view.KeyEvent event) throws android.os.RemoteException;
  public void clearMetaKeyStates(int states) throws android.os.RemoteException;
  public void performPrivateCommand(java.lang.String action, android.os.Bundle data) throws android.os.RemoteException;
  public void setComposingRegion(int start, int end) throws android.os.RemoteException;
  public void getSelectedText(int flags, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
  public void requestUpdateCursorAnchorInfo(int cursorUpdateMode, int seq, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
  public void commitContent(android.view.inputmethod.InputContentInfo inputContentInfo, int flags, android.os.Bundle opts, int sec, com.android.internal.view.IInputContextCallback callback) throws android.os.RemoteException;
}
