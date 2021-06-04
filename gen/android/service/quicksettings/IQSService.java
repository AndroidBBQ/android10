/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.quicksettings;
/**
 * @hide
 */
public interface IQSService extends android.os.IInterface
{
  /** Default implementation for IQSService. */
  public static class Default implements android.service.quicksettings.IQSService
  {
    @Override public android.service.quicksettings.Tile getTile(android.os.IBinder tile) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void updateQsTile(android.service.quicksettings.Tile tile, android.os.IBinder service) throws android.os.RemoteException
    {
    }
    @Override public void updateStatusIcon(android.os.IBinder tile, android.graphics.drawable.Icon icon, java.lang.String contentDescription) throws android.os.RemoteException
    {
    }
    @Override public void onShowDialog(android.os.IBinder tile) throws android.os.RemoteException
    {
    }
    @Override public void onStartActivity(android.os.IBinder tile) throws android.os.RemoteException
    {
    }
    @Override public boolean isLocked() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isSecure() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void startUnlockAndRun(android.os.IBinder tile) throws android.os.RemoteException
    {
    }
    @Override public void onDialogHidden(android.os.IBinder tile) throws android.os.RemoteException
    {
    }
    @Override public void onStartSuccessful(android.os.IBinder tile) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.quicksettings.IQSService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.quicksettings.IQSService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.quicksettings.IQSService interface,
     * generating a proxy if needed.
     */
    public static android.service.quicksettings.IQSService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.quicksettings.IQSService))) {
        return ((android.service.quicksettings.IQSService)iin);
      }
      return new android.service.quicksettings.IQSService.Stub.Proxy(obj);
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
        case TRANSACTION_getTile:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.service.quicksettings.Tile _result = this.getTile(_arg0);
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
        case TRANSACTION_updateQsTile:
        {
          data.enforceInterface(descriptor);
          android.service.quicksettings.Tile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.quicksettings.Tile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.updateQsTile(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateStatusIcon:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.graphics.drawable.Icon _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.drawable.Icon.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.updateStatusIcon(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onShowDialog:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.onShowDialog(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onStartActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.onStartActivity(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isLocked:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isLocked();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isSecure:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSecure();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startUnlockAndRun:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.startUnlockAndRun(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onDialogHidden:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.onDialogHidden(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onStartSuccessful:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.onStartSuccessful(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.quicksettings.IQSService
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
      @Override public android.service.quicksettings.Tile getTile(android.os.IBinder tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.service.quicksettings.Tile _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTile(tile);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.service.quicksettings.Tile.CREATOR.createFromParcel(_reply);
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
      @Override public void updateQsTile(android.service.quicksettings.Tile tile, android.os.IBinder service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((tile!=null)) {
            _data.writeInt(1);
            tile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(service);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateQsTile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateQsTile(tile, service);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updateStatusIcon(android.os.IBinder tile, android.graphics.drawable.Icon icon, java.lang.String contentDescription) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          if ((icon!=null)) {
            _data.writeInt(1);
            icon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(contentDescription);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateStatusIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateStatusIcon(tile, icon, contentDescription);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onShowDialog(android.os.IBinder tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShowDialog, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShowDialog(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onStartActivity(android.os.IBinder tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartActivity(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isLocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isLocked();
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
      @Override public boolean isSecure() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSecure, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSecure();
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
      @Override public void startUnlockAndRun(android.os.IBinder tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startUnlockAndRun, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startUnlockAndRun(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onDialogHidden(android.os.IBinder tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDialogHidden, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDialogHidden(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onStartSuccessful(android.os.IBinder tile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(tile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartSuccessful, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartSuccessful(tile);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.service.quicksettings.IQSService sDefaultImpl;
    }
    static final int TRANSACTION_getTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_updateQsTile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_updateStatusIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onShowDialog = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onStartActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_isSecure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_startUnlockAndRun = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onDialogHidden = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onStartSuccessful = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.service.quicksettings.IQSService impl) {
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
    public static android.service.quicksettings.IQSService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.service.quicksettings.Tile getTile(android.os.IBinder tile) throws android.os.RemoteException;
  public void updateQsTile(android.service.quicksettings.Tile tile, android.os.IBinder service) throws android.os.RemoteException;
  public void updateStatusIcon(android.os.IBinder tile, android.graphics.drawable.Icon icon, java.lang.String contentDescription) throws android.os.RemoteException;
  public void onShowDialog(android.os.IBinder tile) throws android.os.RemoteException;
  public void onStartActivity(android.os.IBinder tile) throws android.os.RemoteException;
  public boolean isLocked() throws android.os.RemoteException;
  public boolean isSecure() throws android.os.RemoteException;
  public void startUnlockAndRun(android.os.IBinder tile) throws android.os.RemoteException;
  public void onDialogHidden(android.os.IBinder tile) throws android.os.RemoteException;
  public void onStartSuccessful(android.os.IBinder tile) throws android.os.RemoteException;
}
