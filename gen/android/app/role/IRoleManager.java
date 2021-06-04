/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.role;
/**
 * @hide
 */
public interface IRoleManager extends android.os.IInterface
{
  /** Default implementation for IRoleManager. */
  public static class Default implements android.app.role.IRoleManager
  {
    @Override public boolean isRoleAvailable(java.lang.String roleName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isRoleHeld(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<java.lang.String> getRoleHoldersAsUser(java.lang.String roleName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addRoleHolderAsUser(java.lang.String roleName, java.lang.String packageName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void removeRoleHolderAsUser(java.lang.String roleName, java.lang.String packageName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void clearRoleHoldersAsUser(java.lang.String roleName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void addOnRoleHoldersChangedListenerAsUser(android.app.role.IOnRoleHoldersChangedListener listener, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removeOnRoleHoldersChangedListenerAsUser(android.app.role.IOnRoleHoldersChangedListener listener, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setRoleNamesFromController(java.util.List<java.lang.String> roleNames) throws android.os.RemoteException
    {
    }
    @Override public boolean addRoleHolderFromController(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removeRoleHolderFromController(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<java.lang.String> getHeldRolesFromController(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getDefaultSmsPackage(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get filtered SMS messages for financial app.
         */
    @Override public void getSmsMessagesForFinancialApp(java.lang.String callingPkg, android.os.Bundle params, android.telephony.IFinancialSmsCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.role.IRoleManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.role.IRoleManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.role.IRoleManager interface,
     * generating a proxy if needed.
     */
    public static android.app.role.IRoleManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.role.IRoleManager))) {
        return ((android.app.role.IRoleManager)iin);
      }
      return new android.app.role.IRoleManager.Stub.Proxy(obj);
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
        case TRANSACTION_isRoleAvailable:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isRoleAvailable(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isRoleHeld:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isRoleHeld(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getRoleHoldersAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<java.lang.String> _result = this.getRoleHoldersAsUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_addRoleHolderAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.os.RemoteCallback _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.addRoleHolderAsUser(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeRoleHolderAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.os.RemoteCallback _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.removeRoleHolderAsUser(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearRoleHoldersAsUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.os.RemoteCallback _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.clearRoleHoldersAsUser(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addOnRoleHoldersChangedListenerAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.role.IOnRoleHoldersChangedListener _arg0;
          _arg0 = android.app.role.IOnRoleHoldersChangedListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.addOnRoleHoldersChangedListenerAsUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeOnRoleHoldersChangedListenerAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.role.IOnRoleHoldersChangedListener _arg0;
          _arg0 = android.app.role.IOnRoleHoldersChangedListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.removeOnRoleHoldersChangedListenerAsUser(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRoleNamesFromController:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          this.setRoleNamesFromController(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addRoleHolderFromController:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.addRoleHolderFromController(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeRoleHolderFromController:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.removeRoleHolderFromController(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getHeldRolesFromController:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<java.lang.String> _result = this.getHeldRolesFromController(_arg0);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_getDefaultSmsPackage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getDefaultSmsPackage(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getSmsMessagesForFinancialApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.telephony.IFinancialSmsCallback _arg2;
          _arg2 = android.telephony.IFinancialSmsCallback.Stub.asInterface(data.readStrongBinder());
          this.getSmsMessagesForFinancialApp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.role.IRoleManager
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
      @Override public boolean isRoleAvailable(java.lang.String roleName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRoleAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRoleAvailable(roleName);
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
      @Override public boolean isRoleHeld(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRoleHeld, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRoleHeld(roleName, packageName);
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
      @Override public java.util.List<java.lang.String> getRoleHoldersAsUser(java.lang.String roleName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRoleHoldersAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRoleHoldersAsUser(roleName, userId);
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
      @Override public void addRoleHolderAsUser(java.lang.String roleName, java.lang.String packageName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          _data.writeInt(flags);
          _data.writeInt(userId);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addRoleHolderAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addRoleHolderAsUser(roleName, packageName, flags, userId, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeRoleHolderAsUser(java.lang.String roleName, java.lang.String packageName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          _data.writeInt(flags);
          _data.writeInt(userId);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRoleHolderAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeRoleHolderAsUser(roleName, packageName, flags, userId, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearRoleHoldersAsUser(java.lang.String roleName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeInt(flags);
          _data.writeInt(userId);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearRoleHoldersAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearRoleHoldersAsUser(roleName, flags, userId, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addOnRoleHoldersChangedListenerAsUser(android.app.role.IOnRoleHoldersChangedListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOnRoleHoldersChangedListenerAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOnRoleHoldersChangedListenerAsUser(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeOnRoleHoldersChangedListenerAsUser(android.app.role.IOnRoleHoldersChangedListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeOnRoleHoldersChangedListenerAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeOnRoleHoldersChangedListenerAsUser(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setRoleNamesFromController(java.util.List<java.lang.String> roleNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(roleNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRoleNamesFromController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRoleNamesFromController(roleNames);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean addRoleHolderFromController(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addRoleHolderFromController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addRoleHolderFromController(roleName, packageName);
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
      @Override public boolean removeRoleHolderFromController(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(roleName);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeRoleHolderFromController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeRoleHolderFromController(roleName, packageName);
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
      @Override public java.util.List<java.lang.String> getHeldRolesFromController(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getHeldRolesFromController, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getHeldRolesFromController(packageName);
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
      @Override public java.lang.String getDefaultSmsPackage(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultSmsPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultSmsPackage(userId);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get filtered SMS messages for financial app.
           */
      @Override public void getSmsMessagesForFinancialApp(java.lang.String callingPkg, android.os.Bundle params, android.telephony.IFinancialSmsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPkg);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSmsMessagesForFinancialApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getSmsMessagesForFinancialApp(callingPkg, params, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.role.IRoleManager sDefaultImpl;
    }
    static final int TRANSACTION_isRoleAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isRoleHeld = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getRoleHoldersAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_addRoleHolderAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_removeRoleHolderAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_clearRoleHoldersAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_addOnRoleHoldersChangedListenerAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_removeOnRoleHoldersChangedListenerAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setRoleNamesFromController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_addRoleHolderFromController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_removeRoleHolderFromController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getHeldRolesFromController = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getDefaultSmsPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getSmsMessagesForFinancialApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    public static boolean setDefaultImpl(android.app.role.IRoleManager impl) {
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
    public static android.app.role.IRoleManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isRoleAvailable(java.lang.String roleName) throws android.os.RemoteException;
  public boolean isRoleHeld(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException;
  public java.util.List<java.lang.String> getRoleHoldersAsUser(java.lang.String roleName, int userId) throws android.os.RemoteException;
  public void addRoleHolderAsUser(java.lang.String roleName, java.lang.String packageName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void removeRoleHolderAsUser(java.lang.String roleName, java.lang.String packageName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void clearRoleHoldersAsUser(java.lang.String roleName, int flags, int userId, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void addOnRoleHoldersChangedListenerAsUser(android.app.role.IOnRoleHoldersChangedListener listener, int userId) throws android.os.RemoteException;
  public void removeOnRoleHoldersChangedListenerAsUser(android.app.role.IOnRoleHoldersChangedListener listener, int userId) throws android.os.RemoteException;
  public void setRoleNamesFromController(java.util.List<java.lang.String> roleNames) throws android.os.RemoteException;
  public boolean addRoleHolderFromController(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException;
  public boolean removeRoleHolderFromController(java.lang.String roleName, java.lang.String packageName) throws android.os.RemoteException;
  public java.util.List<java.lang.String> getHeldRolesFromController(java.lang.String packageName) throws android.os.RemoteException;
  public java.lang.String getDefaultSmsPackage(int userId) throws android.os.RemoteException;
  /**
       * Get filtered SMS messages for financial app.
       */
  public void getSmsMessagesForFinancialApp(java.lang.String callingPkg, android.os.Bundle params, android.telephony.IFinancialSmsCallback callback) throws android.os.RemoteException;
}
