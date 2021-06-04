/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal.uce.uceservice;
/** IUceListener
 * {@hide}  */
public interface IUceListener extends android.os.IInterface
{
  /** Default implementation for IUceListener. */
  public static class Default implements com.android.ims.internal.uce.uceservice.IUceListener
  {
    /**
         * Get UCE Status
         * @param serviceStatusValue defined in ImsUceManager
         * @hide
         */
    @Override public void setStatus(int serviceStatusValue) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.uce.uceservice.IUceListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.uce.uceservice.IUceListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.uce.uceservice.IUceListener interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.uce.uceservice.IUceListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.uce.uceservice.IUceListener))) {
        return ((com.android.ims.internal.uce.uceservice.IUceListener)iin);
      }
      return new com.android.ims.internal.uce.uceservice.IUceListener.Stub.Proxy(obj);
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
        case TRANSACTION_setStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setStatus(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.uce.uceservice.IUceListener
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
      /**
           * Get UCE Status
           * @param serviceStatusValue defined in ImsUceManager
           * @hide
           */
      @Override public void setStatus(int serviceStatusValue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(serviceStatusValue);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStatus(serviceStatusValue);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.ims.internal.uce.uceservice.IUceListener sDefaultImpl;
    }
    static final int TRANSACTION_setStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.ims.internal.uce.uceservice.IUceListener impl) {
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
    public static com.android.ims.internal.uce.uceservice.IUceListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Get UCE Status
       * @param serviceStatusValue defined in ImsUceManager
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceListener.aidl:28:1:28:25")
  public void setStatus(int serviceStatusValue) throws android.os.RemoteException;
}
