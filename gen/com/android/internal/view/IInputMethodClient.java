/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Interface a client of the IInputMethodManager implements, to identify
 * itself and receive information about changes to the global manager state.
 */
public interface IInputMethodClient extends android.os.IInterface
{
  /** Default implementation for IInputMethodClient. */
  public static class Default implements com.android.internal.view.IInputMethodClient
  {
    @Override public void onBindMethod(com.android.internal.view.InputBindResult res) throws android.os.RemoteException
    {
    }
    @Override public void onUnbindMethod(int sequence, int unbindReason) throws android.os.RemoteException
    {
    }
    @Override public void setActive(boolean active, boolean fullscreen) throws android.os.RemoteException
    {
    }
    @Override public void reportFullscreenMode(boolean fullscreen) throws android.os.RemoteException
    {
    }
    @Override public void reportPreRendered(android.view.inputmethod.EditorInfo info) throws android.os.RemoteException
    {
    }
    @Override public void applyImeVisibility(boolean setVisible) throws android.os.RemoteException
    {
    }
    @Override public void updateActivityViewToScreenMatrix(int bindSequence, float[] matrixValues) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IInputMethodClient
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IInputMethodClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IInputMethodClient interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IInputMethodClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IInputMethodClient))) {
        return ((com.android.internal.view.IInputMethodClient)iin);
      }
      return new com.android.internal.view.IInputMethodClient.Stub.Proxy(obj);
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
        case TRANSACTION_onBindMethod:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.InputBindResult _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.internal.view.InputBindResult.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onBindMethod(_arg0);
          return true;
        }
        case TRANSACTION_onUnbindMethod:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onUnbindMethod(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setActive:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setActive(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_reportFullscreenMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.reportFullscreenMode(_arg0);
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
          return true;
        }
        case TRANSACTION_applyImeVisibility:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.applyImeVisibility(_arg0);
          return true;
        }
        case TRANSACTION_updateActivityViewToScreenMatrix:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float[] _arg1;
          _arg1 = data.createFloatArray();
          this.updateActivityViewToScreenMatrix(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IInputMethodClient
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
      @Override public void onBindMethod(com.android.internal.view.InputBindResult res) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((res!=null)) {
            _data.writeInt(1);
            res.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBindMethod, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBindMethod(res);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUnbindMethod(int sequence, int unbindReason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          _data.writeInt(unbindReason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUnbindMethod, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUnbindMethod(sequence, unbindReason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setActive(boolean active, boolean fullscreen) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          _data.writeInt(((fullscreen)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActive, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActive(active, fullscreen);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void reportFullscreenMode(boolean fullscreen) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((fullscreen)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportFullscreenMode, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportFullscreenMode(fullscreen);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void reportPreRendered(android.view.inputmethod.EditorInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportPreRendered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportPreRendered(info);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void applyImeVisibility(boolean setVisible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((setVisible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyImeVisibility, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyImeVisibility(setVisible);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateActivityViewToScreenMatrix(int bindSequence, float[] matrixValues) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(bindSequence);
          _data.writeFloatArray(matrixValues);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateActivityViewToScreenMatrix, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateActivityViewToScreenMatrix(bindSequence, matrixValues);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.view.IInputMethodClient sDefaultImpl;
    }
    static final int TRANSACTION_onBindMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onUnbindMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_reportFullscreenMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_reportPreRendered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_applyImeVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_updateActivityViewToScreenMatrix = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(com.android.internal.view.IInputMethodClient impl) {
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
    public static com.android.internal.view.IInputMethodClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onBindMethod(com.android.internal.view.InputBindResult res) throws android.os.RemoteException;
  public void onUnbindMethod(int sequence, int unbindReason) throws android.os.RemoteException;
  public void setActive(boolean active, boolean fullscreen) throws android.os.RemoteException;
  public void reportFullscreenMode(boolean fullscreen) throws android.os.RemoteException;
  public void reportPreRendered(android.view.inputmethod.EditorInfo info) throws android.os.RemoteException;
  public void applyImeVisibility(boolean setVisible) throws android.os.RemoteException;
  public void updateActivityViewToScreenMatrix(int bindSequence, float[] matrixValues) throws android.os.RemoteException;
}
