/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.lowpan;
/** {@hide} */
public interface ILowpanManagerListener extends android.os.IInterface
{
  /** Default implementation for ILowpanManagerListener. */
  public static class Default implements android.net.lowpan.ILowpanManagerListener
  {
    @Override public void onInterfaceAdded(android.net.lowpan.ILowpanInterface lowpanInterface) throws android.os.RemoteException
    {
    }
    @Override public void onInterfaceRemoved(android.net.lowpan.ILowpanInterface lowpanInterface) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.lowpan.ILowpanManagerListener
  {
    private static final java.lang.String DESCRIPTOR = "android.net.lowpan.ILowpanManagerListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.lowpan.ILowpanManagerListener interface,
     * generating a proxy if needed.
     */
    public static android.net.lowpan.ILowpanManagerListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.lowpan.ILowpanManagerListener))) {
        return ((android.net.lowpan.ILowpanManagerListener)iin);
      }
      return new android.net.lowpan.ILowpanManagerListener.Stub.Proxy(obj);
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
        case TRANSACTION_onInterfaceAdded:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanInterface _arg0;
          _arg0 = android.net.lowpan.ILowpanInterface.Stub.asInterface(data.readStrongBinder());
          this.onInterfaceAdded(_arg0);
          return true;
        }
        case TRANSACTION_onInterfaceRemoved:
        {
          data.enforceInterface(descriptor);
          android.net.lowpan.ILowpanInterface _arg0;
          _arg0 = android.net.lowpan.ILowpanInterface.Stub.asInterface(data.readStrongBinder());
          this.onInterfaceRemoved(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.lowpan.ILowpanManagerListener
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
      @Override public void onInterfaceAdded(android.net.lowpan.ILowpanInterface lowpanInterface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((lowpanInterface!=null))?(lowpanInterface.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInterfaceAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInterfaceAdded(lowpanInterface);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onInterfaceRemoved(android.net.lowpan.ILowpanInterface lowpanInterface) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((lowpanInterface!=null))?(lowpanInterface.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInterfaceRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInterfaceRemoved(lowpanInterface);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.lowpan.ILowpanManagerListener sDefaultImpl;
    }
    static final int TRANSACTION_onInterfaceAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onInterfaceRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.lowpan.ILowpanManagerListener impl) {
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
    public static android.net.lowpan.ILowpanManagerListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onInterfaceAdded(android.net.lowpan.ILowpanInterface lowpanInterface) throws android.os.RemoteException;
  public void onInterfaceRemoved(android.net.lowpan.ILowpanInterface lowpanInterface) throws android.os.RemoteException;
}
