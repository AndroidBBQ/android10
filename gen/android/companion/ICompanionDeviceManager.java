/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.companion;
/**
 * Interface for communication with the core companion device manager service.
 *
 * @hide
 */
public interface ICompanionDeviceManager extends android.os.IInterface
{
  /** Default implementation for ICompanionDeviceManager. */
  public static class Default implements android.companion.ICompanionDeviceManager
  {
    @Override public void associate(android.companion.AssociationRequest request, android.companion.IFindDeviceCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void stopScan(android.companion.AssociationRequest request, android.companion.IFindDeviceCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<java.lang.String> getAssociations(java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void disassociate(java.lang.String deviceMacAddress, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public boolean hasNotificationAccess(android.content.ComponentName component) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.app.PendingIntent requestNotificationAccess(android.content.ComponentName component) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.companion.ICompanionDeviceManager
  {
    private static final java.lang.String DESCRIPTOR = "android.companion.ICompanionDeviceManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.companion.ICompanionDeviceManager interface,
     * generating a proxy if needed.
     */
    public static android.companion.ICompanionDeviceManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.companion.ICompanionDeviceManager))) {
        return ((android.companion.ICompanionDeviceManager)iin);
      }
      return new android.companion.ICompanionDeviceManager.Stub.Proxy(obj);
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
        case TRANSACTION_associate:
        {
          data.enforceInterface(descriptor);
          android.companion.AssociationRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.companion.AssociationRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.companion.IFindDeviceCallback _arg1;
          _arg1 = android.companion.IFindDeviceCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.associate(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopScan:
        {
          data.enforceInterface(descriptor);
          android.companion.AssociationRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.companion.AssociationRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.companion.IFindDeviceCallback _arg1;
          _arg1 = android.companion.IFindDeviceCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.stopScan(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAssociations:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<java.lang.String> _result = this.getAssociations(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_disassociate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.disassociate(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasNotificationAccess:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.hasNotificationAccess(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestNotificationAccess:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.PendingIntent _result = this.requestNotificationAccess(_arg0);
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
    private static class Proxy implements android.companion.ICompanionDeviceManager
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
      @Override public void associate(android.companion.AssociationRequest request, android.companion.IFindDeviceCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
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
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_associate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().associate(request, callback, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopScan(android.companion.AssociationRequest request, android.companion.IFindDeviceCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
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
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopScan(request, callback, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<java.lang.String> getAssociations(java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAssociations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAssociations(callingPackage, userId);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void disassociate(java.lang.String deviceMacAddress, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceMacAddress);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disassociate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disassociate(deviceMacAddress, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean hasNotificationAccess(android.content.ComponentName component) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasNotificationAccess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasNotificationAccess(component);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.app.PendingIntent requestNotificationAccess(android.content.ComponentName component) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.PendingIntent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestNotificationAccess, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestNotificationAccess(component);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.PendingIntent.CREATOR.createFromParcel(_reply);
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
      public static android.companion.ICompanionDeviceManager sDefaultImpl;
    }
    static final int TRANSACTION_associate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getAssociations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_disassociate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_hasNotificationAccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_requestNotificationAccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.companion.ICompanionDeviceManager impl) {
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
    public static android.companion.ICompanionDeviceManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void associate(android.companion.AssociationRequest request, android.companion.IFindDeviceCallback callback, java.lang.String callingPackage) throws android.os.RemoteException;
  public void stopScan(android.companion.AssociationRequest request, android.companion.IFindDeviceCallback callback, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.util.List<java.lang.String> getAssociations(java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  public void disassociate(java.lang.String deviceMacAddress, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean hasNotificationAccess(android.content.ComponentName component) throws android.os.RemoteException;
  public android.app.PendingIntent requestNotificationAccess(android.content.ComponentName component) throws android.os.RemoteException;
}
