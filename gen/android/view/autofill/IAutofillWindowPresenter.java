/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.autofill;
/**
 * This is a handle to the FillUi for controlling
 * when its window should be shown and hidden.
 *
 * {@hide}
 */
public interface IAutofillWindowPresenter extends android.os.IInterface
{
  /** Default implementation for IAutofillWindowPresenter. */
  public static class Default implements android.view.autofill.IAutofillWindowPresenter
  {
    @Override public void show(android.view.WindowManager.LayoutParams p, android.graphics.Rect transitionEpicenter, boolean fitsSystemWindows, int layoutDirection) throws android.os.RemoteException
    {
    }
    @Override public void hide(android.graphics.Rect transitionEpicenter) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.autofill.IAutofillWindowPresenter
  {
    private static final java.lang.String DESCRIPTOR = "android.view.autofill.IAutofillWindowPresenter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.autofill.IAutofillWindowPresenter interface,
     * generating a proxy if needed.
     */
    public static android.view.autofill.IAutofillWindowPresenter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.autofill.IAutofillWindowPresenter))) {
        return ((android.view.autofill.IAutofillWindowPresenter)iin);
      }
      return new android.view.autofill.IAutofillWindowPresenter.Stub.Proxy(obj);
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
          android.view.WindowManager.LayoutParams _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.WindowManager.LayoutParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _arg3;
          _arg3 = data.readInt();
          this.show(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_hide:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.hide(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.autofill.IAutofillWindowPresenter
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
      @Override public void show(android.view.WindowManager.LayoutParams p, android.graphics.Rect transitionEpicenter, boolean fitsSystemWindows, int layoutDirection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((p!=null)) {
            _data.writeInt(1);
            p.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((transitionEpicenter!=null)) {
            _data.writeInt(1);
            transitionEpicenter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((fitsSystemWindows)?(1):(0)));
          _data.writeInt(layoutDirection);
          boolean _status = mRemote.transact(Stub.TRANSACTION_show, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().show(p, transitionEpicenter, fitsSystemWindows, layoutDirection);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hide(android.graphics.Rect transitionEpicenter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((transitionEpicenter!=null)) {
            _data.writeInt(1);
            transitionEpicenter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hide, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hide(transitionEpicenter);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.autofill.IAutofillWindowPresenter sDefaultImpl;
    }
    static final int TRANSACTION_show = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_hide = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.view.autofill.IAutofillWindowPresenter impl) {
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
    public static android.view.autofill.IAutofillWindowPresenter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void show(android.view.WindowManager.LayoutParams p, android.graphics.Rect transitionEpicenter, boolean fitsSystemWindows, int layoutDirection) throws android.os.RemoteException;
  public void hide(android.graphics.Rect transitionEpicenter) throws android.os.RemoteException;
}
