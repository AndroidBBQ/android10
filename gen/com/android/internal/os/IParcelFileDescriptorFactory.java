/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.os;
/** {@hide} */
public interface IParcelFileDescriptorFactory extends android.os.IInterface
{
  /** Default implementation for IParcelFileDescriptorFactory. */
  public static class Default implements com.android.internal.os.IParcelFileDescriptorFactory
  {
    // NOTE: implementors should carefully sanitize the incoming name
    // using something like FileUtils.isValidExtFilename()

    @Override public android.os.ParcelFileDescriptor open(java.lang.String name, int mode) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.os.IParcelFileDescriptorFactory
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.os.IParcelFileDescriptorFactory";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.os.IParcelFileDescriptorFactory interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.os.IParcelFileDescriptorFactory asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.os.IParcelFileDescriptorFactory))) {
        return ((com.android.internal.os.IParcelFileDescriptorFactory)iin);
      }
      return new com.android.internal.os.IParcelFileDescriptorFactory.Stub.Proxy(obj);
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
        case TRANSACTION_open:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.ParcelFileDescriptor _result = this.open(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.os.IParcelFileDescriptorFactory
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
      // NOTE: implementors should carefully sanitize the incoming name
      // using something like FileUtils.isValidExtFilename()

      @Override public android.os.ParcelFileDescriptor open(java.lang.String name, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_open, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().open(name, mode);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.internal.os.IParcelFileDescriptorFactory sDefaultImpl;
    }
    static final int TRANSACTION_open = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.os.IParcelFileDescriptorFactory impl) {
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
    public static com.android.internal.os.IParcelFileDescriptorFactory getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // NOTE: implementors should carefully sanitize the incoming name
  // using something like FileUtils.isValidExtFilename()

  public android.os.ParcelFileDescriptor open(java.lang.String name, int mode) throws android.os.RemoteException;
}
