/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.slice;
/** @hide */
public interface ISliceListener extends android.os.IInterface
{
  /** Default implementation for ISliceListener. */
  public static class Default implements android.app.slice.ISliceListener
  {
    @Override public void onSliceUpdated(android.app.slice.Slice s) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.slice.ISliceListener
  {
    private static final java.lang.String DESCRIPTOR = "android.app.slice.ISliceListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.slice.ISliceListener interface,
     * generating a proxy if needed.
     */
    public static android.app.slice.ISliceListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.slice.ISliceListener))) {
        return ((android.app.slice.ISliceListener)iin);
      }
      return new android.app.slice.ISliceListener.Stub.Proxy(obj);
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
        case TRANSACTION_onSliceUpdated:
        {
          data.enforceInterface(descriptor);
          android.app.slice.Slice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.slice.Slice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSliceUpdated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.slice.ISliceListener
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
      @Override public void onSliceUpdated(android.app.slice.Slice s) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((s!=null)) {
            _data.writeInt(1);
            s.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSliceUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSliceUpdated(s);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.slice.ISliceListener sDefaultImpl;
    }
    static final int TRANSACTION_onSliceUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.slice.ISliceListener impl) {
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
    public static android.app.slice.ISliceListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSliceUpdated(android.app.slice.Slice s) throws android.os.RemoteException;
}
