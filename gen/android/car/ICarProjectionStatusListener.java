/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/**
 * Listener interface to notify interested parties of projection status change.
 *
 * @hide
 */
public interface ICarProjectionStatusListener extends android.os.IInterface
{
  /** Default implementation for ICarProjectionStatusListener. */
  public static class Default implements android.car.ICarProjectionStatusListener
  {
    @Override public void onProjectionStatusChanged(int projectionState, java.lang.String activeProjectionPackageName, java.util.List<android.car.projection.ProjectionStatus> details) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.ICarProjectionStatusListener
  {
    private static final java.lang.String DESCRIPTOR = "android.car.ICarProjectionStatusListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.ICarProjectionStatusListener interface,
     * generating a proxy if needed.
     */
    public static android.car.ICarProjectionStatusListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.ICarProjectionStatusListener))) {
        return ((android.car.ICarProjectionStatusListener)iin);
      }
      return new android.car.ICarProjectionStatusListener.Stub.Proxy(obj);
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
        case TRANSACTION_onProjectionStatusChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.car.projection.ProjectionStatus> _arg2;
          _arg2 = data.createTypedArrayList(android.car.projection.ProjectionStatus.CREATOR);
          this.onProjectionStatusChanged(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.ICarProjectionStatusListener
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
      @Override public void onProjectionStatusChanged(int projectionState, java.lang.String activeProjectionPackageName, java.util.List<android.car.projection.ProjectionStatus> details) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(projectionState);
          _data.writeString(activeProjectionPackageName);
          _data.writeTypedList(details);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProjectionStatusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProjectionStatusChanged(projectionState, activeProjectionPackageName, details);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.ICarProjectionStatusListener sDefaultImpl;
    }
    static final int TRANSACTION_onProjectionStatusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.car.ICarProjectionStatusListener impl) {
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
    public static android.car.ICarProjectionStatusListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onProjectionStatusChanged(int projectionState, java.lang.String activeProjectionPackageName, java.util.List<android.car.projection.ProjectionStatus> details) throws android.os.RemoteException;
}
