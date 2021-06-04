/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.voice;
/**
 * @hide
 */
public interface IVoiceInteractionSession extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractionSession. */
  public static class Default implements android.service.voice.IVoiceInteractionSession
  {
    @Override public void show(android.os.Bundle sessionArgs, int flags, com.android.internal.app.IVoiceInteractionSessionShowCallback showCallback) throws android.os.RemoteException
    {
    }
    @Override public void hide() throws android.os.RemoteException
    {
    }
    @Override public void handleAssist(int taskId, android.os.IBinder activityId, android.os.Bundle assistData, android.app.assist.AssistStructure structure, android.app.assist.AssistContent content, int index, int count) throws android.os.RemoteException
    {
    }
    @Override public void handleScreenshot(android.graphics.Bitmap screenshot) throws android.os.RemoteException
    {
    }
    @Override public void taskStarted(android.content.Intent intent, int taskId) throws android.os.RemoteException
    {
    }
    @Override public void taskFinished(android.content.Intent intent, int taskId) throws android.os.RemoteException
    {
    }
    @Override public void closeSystemDialogs() throws android.os.RemoteException
    {
    }
    @Override public void onLockscreenShown() throws android.os.RemoteException
    {
    }
    @Override public void destroy() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.voice.IVoiceInteractionSession
  {
    private static final java.lang.String DESCRIPTOR = "android.service.voice.IVoiceInteractionSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.voice.IVoiceInteractionSession interface,
     * generating a proxy if needed.
     */
    public static android.service.voice.IVoiceInteractionSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.voice.IVoiceInteractionSession))) {
        return ((android.service.voice.IVoiceInteractionSession)iin);
      }
      return new android.service.voice.IVoiceInteractionSession.Stub.Proxy(obj);
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
        case TRANSACTION_show:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          com.android.internal.app.IVoiceInteractionSessionShowCallback _arg2;
          _arg2 = com.android.internal.app.IVoiceInteractionSessionShowCallback.Stub.asInterface(data.readStrongBinder());
          this.show(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_hide:
        {
          data.enforceInterface(descriptor);
          this.hide();
          return true;
        }
        case TRANSACTION_handleAssist:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.app.assist.AssistStructure _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.assist.AssistStructure.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.app.assist.AssistContent _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.app.assist.AssistContent.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          this.handleAssist(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        case TRANSACTION_handleScreenshot:
        {
          data.enforceInterface(descriptor);
          android.graphics.Bitmap _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Bitmap.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.handleScreenshot(_arg0);
          return true;
        }
        case TRANSACTION_taskStarted:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.taskStarted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_taskFinished:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.taskFinished(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_closeSystemDialogs:
        {
          data.enforceInterface(descriptor);
          this.closeSystemDialogs();
          return true;
        }
        case TRANSACTION_onLockscreenShown:
        {
          data.enforceInterface(descriptor);
          this.onLockscreenShown();
          return true;
        }
        case TRANSACTION_destroy:
        {
          data.enforceInterface(descriptor);
          this.destroy();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.voice.IVoiceInteractionSession
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
      @Override public void show(android.os.Bundle sessionArgs, int flags, com.android.internal.app.IVoiceInteractionSessionShowCallback showCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionArgs!=null)) {
            _data.writeInt(1);
            sessionArgs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          _data.writeStrongBinder((((showCallback!=null))?(showCallback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_show, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().show(sessionArgs, flags, showCallback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hide() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hide, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hide();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handleAssist(int taskId, android.os.IBinder activityId, android.os.Bundle assistData, android.app.assist.AssistStructure structure, android.app.assist.AssistContent content, int index, int count) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeStrongBinder(activityId);
          if ((assistData!=null)) {
            _data.writeInt(1);
            assistData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((structure!=null)) {
            _data.writeInt(1);
            structure.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((content!=null)) {
            _data.writeInt(1);
            content.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(index);
          _data.writeInt(count);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleAssist, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleAssist(taskId, activityId, assistData, structure, content, index, count);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handleScreenshot(android.graphics.Bitmap screenshot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((screenshot!=null)) {
            _data.writeInt(1);
            screenshot.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleScreenshot, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleScreenshot(screenshot);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void taskStarted(android.content.Intent intent, int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_taskStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().taskStarted(intent, taskId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void taskFinished(android.content.Intent intent, int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_taskFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().taskFinished(intent, taskId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void closeSystemDialogs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSystemDialogs, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSystemDialogs();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLockscreenShown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLockscreenShown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLockscreenShown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void destroy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroy();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.voice.IVoiceInteractionSession sDefaultImpl;
    }
    static final int TRANSACTION_show = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_hide = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_handleAssist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_handleScreenshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_taskStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_taskFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_closeSystemDialogs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onLockscreenShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_destroy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.service.voice.IVoiceInteractionSession impl) {
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
    public static android.service.voice.IVoiceInteractionSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void show(android.os.Bundle sessionArgs, int flags, com.android.internal.app.IVoiceInteractionSessionShowCallback showCallback) throws android.os.RemoteException;
  public void hide() throws android.os.RemoteException;
  public void handleAssist(int taskId, android.os.IBinder activityId, android.os.Bundle assistData, android.app.assist.AssistStructure structure, android.app.assist.AssistContent content, int index, int count) throws android.os.RemoteException;
  public void handleScreenshot(android.graphics.Bitmap screenshot) throws android.os.RemoteException;
  public void taskStarted(android.content.Intent intent, int taskId) throws android.os.RemoteException;
  public void taskFinished(android.content.Intent intent, int taskId) throws android.os.RemoteException;
  public void closeSystemDialogs() throws android.os.RemoteException;
  public void onLockscreenShown() throws android.os.RemoteException;
  public void destroy() throws android.os.RemoteException;
}
