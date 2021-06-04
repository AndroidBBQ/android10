/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * {@hide}
 */
public interface IDisplayFoldListener extends android.os.IInterface
{
  /** Default implementation for IDisplayFoldListener. */
  public static class Default implements android.view.IDisplayFoldListener
  {
    /** Called when the foldedness of a display changes */
    @Override public void onDisplayFoldChanged(int displayId, boolean folded) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IDisplayFoldListener
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IDisplayFoldListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IDisplayFoldListener interface,
     * generating a proxy if needed.
     */
    public static android.view.IDisplayFoldListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IDisplayFoldListener))) {
        return ((android.view.IDisplayFoldListener)iin);
      }
      return new android.view.IDisplayFoldListener.Stub.Proxy(obj);
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
        case TRANSACTION_onDisplayFoldChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onDisplayFoldChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IDisplayFoldListener
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
      /** Called when the foldedness of a display changes */
      @Override public void onDisplayFoldChanged(int displayId, boolean folded) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((folded)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDisplayFoldChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDisplayFoldChanged(displayId, folded);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IDisplayFoldListener sDefaultImpl;
    }
    static final int TRANSACTION_onDisplayFoldChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.IDisplayFoldListener impl) {
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
    public static android.view.IDisplayFoldListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Called when the foldedness of a display changes */
  public void onDisplayFoldChanged(int displayId, boolean folded) throws android.os.RemoteException;
}
