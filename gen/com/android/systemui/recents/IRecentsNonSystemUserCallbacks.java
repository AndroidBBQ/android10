/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.systemui.recents;
/**
 * Due to the fact that RecentsActivity is per-user, we need to establish an
 * interface (this) for the system user to callback to the secondary users in
 * response to UI events coming in from the system user's SystemUI.
 */
public interface IRecentsNonSystemUserCallbacks extends android.os.IInterface
{
  /** Default implementation for IRecentsNonSystemUserCallbacks. */
  public static class Default implements com.android.systemui.recents.IRecentsNonSystemUserCallbacks
  {
    @Override public void preloadRecents() throws android.os.RemoteException
    {
    }
    @Override public void cancelPreloadingRecents() throws android.os.RemoteException
    {
    }
    @Override public void showRecents(boolean triggeredFromAltTab, boolean draggingInRecents, boolean animate, int recentsGrowTarget) throws android.os.RemoteException
    {
    }
    @Override public void hideRecents(boolean triggeredFromAltTab, boolean triggeredFromHomeKey) throws android.os.RemoteException
    {
    }
    @Override public void toggleRecents(int recentsGrowTarget) throws android.os.RemoteException
    {
    }
    @Override public void onConfigurationChanged() throws android.os.RemoteException
    {
    }
    @Override public void splitPrimaryTask(int topTaskId, int stackCreateMode, android.graphics.Rect initialBounds) throws android.os.RemoteException
    {
    }
    @Override public void onDraggingInRecents(float distanceFromTop) throws android.os.RemoteException
    {
    }
    @Override public void onDraggingInRecentsEnded(float velocity) throws android.os.RemoteException
    {
    }
    @Override public void showCurrentUserToast(int msgResId, int msgLength) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.systemui.recents.IRecentsNonSystemUserCallbacks
  {
    private static final java.lang.String DESCRIPTOR = "com.android.systemui.recents.IRecentsNonSystemUserCallbacks";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.systemui.recents.IRecentsNonSystemUserCallbacks interface,
     * generating a proxy if needed.
     */
    public static com.android.systemui.recents.IRecentsNonSystemUserCallbacks asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.systemui.recents.IRecentsNonSystemUserCallbacks))) {
        return ((com.android.systemui.recents.IRecentsNonSystemUserCallbacks)iin);
      }
      return new com.android.systemui.recents.IRecentsNonSystemUserCallbacks.Stub.Proxy(obj);
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
        case TRANSACTION_preloadRecents:
        {
          data.enforceInterface(descriptor);
          this.preloadRecents();
          return true;
        }
        case TRANSACTION_cancelPreloadingRecents:
        {
          data.enforceInterface(descriptor);
          this.cancelPreloadingRecents();
          return true;
        }
        case TRANSACTION_showRecents:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.showRecents(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_hideRecents:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.hideRecents(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_toggleRecents:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.toggleRecents(_arg0);
          return true;
        }
        case TRANSACTION_onConfigurationChanged:
        {
          data.enforceInterface(descriptor);
          this.onConfigurationChanged();
          return true;
        }
        case TRANSACTION_splitPrimaryTask:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.Rect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.splitPrimaryTask(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onDraggingInRecents:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.onDraggingInRecents(_arg0);
          return true;
        }
        case TRANSACTION_onDraggingInRecentsEnded:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          this.onDraggingInRecentsEnded(_arg0);
          return true;
        }
        case TRANSACTION_showCurrentUserToast:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.showCurrentUserToast(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.systemui.recents.IRecentsNonSystemUserCallbacks
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
      @Override public void preloadRecents() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_preloadRecents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().preloadRecents();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelPreloadingRecents() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelPreloadingRecents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelPreloadingRecents();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showRecents(boolean triggeredFromAltTab, boolean draggingInRecents, boolean animate, int recentsGrowTarget) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((triggeredFromAltTab)?(1):(0)));
          _data.writeInt(((draggingInRecents)?(1):(0)));
          _data.writeInt(((animate)?(1):(0)));
          _data.writeInt(recentsGrowTarget);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showRecents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showRecents(triggeredFromAltTab, draggingInRecents, animate, recentsGrowTarget);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hideRecents(boolean triggeredFromAltTab, boolean triggeredFromHomeKey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((triggeredFromAltTab)?(1):(0)));
          _data.writeInt(((triggeredFromHomeKey)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideRecents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideRecents(triggeredFromAltTab, triggeredFromHomeKey);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void toggleRecents(int recentsGrowTarget) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(recentsGrowTarget);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleRecents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleRecents(recentsGrowTarget);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConfigurationChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConfigurationChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConfigurationChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void splitPrimaryTask(int topTaskId, int stackCreateMode, android.graphics.Rect initialBounds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(topTaskId);
          _data.writeInt(stackCreateMode);
          if ((initialBounds!=null)) {
            _data.writeInt(1);
            initialBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_splitPrimaryTask, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().splitPrimaryTask(topTaskId, stackCreateMode, initialBounds);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDraggingInRecents(float distanceFromTop) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(distanceFromTop);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDraggingInRecents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDraggingInRecents(distanceFromTop);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDraggingInRecentsEnded(float velocity) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(velocity);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDraggingInRecentsEnded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDraggingInRecentsEnded(velocity);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showCurrentUserToast(int msgResId, int msgLength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(msgResId);
          _data.writeInt(msgLength);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showCurrentUserToast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showCurrentUserToast(msgResId, msgLength);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.systemui.recents.IRecentsNonSystemUserCallbacks sDefaultImpl;
    }
    static final int TRANSACTION_preloadRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_cancelPreloadingRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_showRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_hideRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_toggleRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onConfigurationChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_splitPrimaryTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onDraggingInRecents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onDraggingInRecentsEnded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_showCurrentUserToast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(com.android.systemui.recents.IRecentsNonSystemUserCallbacks impl) {
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
    public static com.android.systemui.recents.IRecentsNonSystemUserCallbacks getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void preloadRecents() throws android.os.RemoteException;
  public void cancelPreloadingRecents() throws android.os.RemoteException;
  public void showRecents(boolean triggeredFromAltTab, boolean draggingInRecents, boolean animate, int recentsGrowTarget) throws android.os.RemoteException;
  public void hideRecents(boolean triggeredFromAltTab, boolean triggeredFromHomeKey) throws android.os.RemoteException;
  public void toggleRecents(int recentsGrowTarget) throws android.os.RemoteException;
  public void onConfigurationChanged() throws android.os.RemoteException;
  public void splitPrimaryTask(int topTaskId, int stackCreateMode, android.graphics.Rect initialBounds) throws android.os.RemoteException;
  public void onDraggingInRecents(float distanceFromTop) throws android.os.RemoteException;
  public void onDraggingInRecentsEnded(float velocity) throws android.os.RemoteException;
  public void showCurrentUserToast(int msgResId, int msgLength) throws android.os.RemoteException;
}
