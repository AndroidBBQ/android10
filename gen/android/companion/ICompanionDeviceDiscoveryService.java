/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.companion;
/** @hide */
public interface ICompanionDeviceDiscoveryService extends android.os.IInterface
{
  /** Default implementation for ICompanionDeviceDiscoveryService. */
  public static class Default implements android.companion.ICompanionDeviceDiscoveryService
  {
    @Override public void startDiscovery(android.companion.AssociationRequest request, java.lang.String callingPackage, android.companion.IFindDeviceCallback findCallback, android.companion.ICompanionDeviceDiscoveryServiceCallback serviceCallback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.companion.ICompanionDeviceDiscoveryService
  {
    private static final java.lang.String DESCRIPTOR = "android.companion.ICompanionDeviceDiscoveryService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.companion.ICompanionDeviceDiscoveryService interface,
     * generating a proxy if needed.
     */
    public static android.companion.ICompanionDeviceDiscoveryService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.companion.ICompanionDeviceDiscoveryService))) {
        return ((android.companion.ICompanionDeviceDiscoveryService)iin);
      }
      return new android.companion.ICompanionDeviceDiscoveryService.Stub.Proxy(obj);
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
        case TRANSACTION_startDiscovery:
        {
          data.enforceInterface(descriptor);
          android.companion.AssociationRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.companion.AssociationRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.companion.IFindDeviceCallback _arg2;
          _arg2 = android.companion.IFindDeviceCallback.Stub.asInterface(data.readStrongBinder());
          android.companion.ICompanionDeviceDiscoveryServiceCallback _arg3;
          _arg3 = android.companion.ICompanionDeviceDiscoveryServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.startDiscovery(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.companion.ICompanionDeviceDiscoveryService
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
      @Override public void startDiscovery(android.companion.AssociationRequest request, java.lang.String callingPackage, android.companion.IFindDeviceCallback findCallback, android.companion.ICompanionDeviceDiscoveryServiceCallback serviceCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((findCallback!=null))?(findCallback.asBinder()):(null)));
          _data.writeStrongBinder((((serviceCallback!=null))?(serviceCallback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startDiscovery, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startDiscovery(request, callingPackage, findCallback, serviceCallback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.companion.ICompanionDeviceDiscoveryService sDefaultImpl;
    }
    static final int TRANSACTION_startDiscovery = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.companion.ICompanionDeviceDiscoveryService impl) {
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
    public static android.companion.ICompanionDeviceDiscoveryService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void startDiscovery(android.companion.AssociationRequest request, java.lang.String callingPackage, android.companion.IFindDeviceCallback findCallback, android.companion.ICompanionDeviceDiscoveryServiceCallback serviceCallback) throws android.os.RemoteException;
}
