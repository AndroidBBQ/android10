/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * See ImsRegistration for more information.
 *
 * {@hide}
 */
public interface IImsRegistration extends android.os.IInterface
{
  /** Default implementation for IImsRegistration. */
  public static class Default implements android.telephony.ims.aidl.IImsRegistration
  {
    @Override public int getRegistrationTechnology() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void addRegistrationCallback(android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
    {
    }
    @Override public void removeRegistrationCallback(android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsRegistration
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsRegistration";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsRegistration interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsRegistration asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsRegistration))) {
        return ((android.telephony.ims.aidl.IImsRegistration)iin);
      }
      return new android.telephony.ims.aidl.IImsRegistration.Stub.Proxy(obj);
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
        case TRANSACTION_getRegistrationTechnology:
        {
          data.enforceInterface(descriptor);
          int _result = this.getRegistrationTechnology();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addRegistrationCallback:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsRegistrationCallback _arg0;
          _arg0 = android.telephony.ims.aidl.IImsRegistrationCallback.Stub.asInterface(data.readStrongBinder());
          this.addRegistrationCallback(_arg0);
          return true;
        }
        case TRANSACTION_removeRegistrationCallback:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.aidl.IImsRegistrationCallback _arg0;
          _arg0 = android.telephony.ims.aidl.IImsRegistrationCallback.Stub.asInterface(data.readStrongBinder());
          this.removeRegistrationCallback(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsRegistration
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
      @Override public int getRegistrationTechnology() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRegistrationTechnology, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRegistrationTechnology();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void addRegistrationCallback(android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addRegistrationCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addRegistrationCallback(c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeRegistrationCallback(android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRegistrationCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeRegistrationCallback(c);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsRegistration sDefaultImpl;
    }
    static final int TRANSACTION_getRegistrationTechnology = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addRegistrationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeRegistrationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsRegistration impl) {
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
    public static android.telephony.ims.aidl.IImsRegistration getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int getRegistrationTechnology() throws android.os.RemoteException;
  public void addRegistrationCallback(android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException;
  public void removeRegistrationCallback(android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException;
}
