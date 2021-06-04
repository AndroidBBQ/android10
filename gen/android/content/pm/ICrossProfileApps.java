/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.pm;
/**
 * @hide
 */
public interface ICrossProfileApps extends android.os.IInterface
{
  /** Default implementation for ICrossProfileApps. */
  public static class Default implements android.content.pm.ICrossProfileApps
  {
    @Override public void startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, int userId, boolean launchMainActivity) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.os.UserHandle> getTargetUserProfiles(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.pm.ICrossProfileApps
  {
    private static final java.lang.String DESCRIPTOR = "android.content.pm.ICrossProfileApps";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.pm.ICrossProfileApps interface,
     * generating a proxy if needed.
     */
    public static android.content.pm.ICrossProfileApps asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.pm.ICrossProfileApps))) {
        return ((android.content.pm.ICrossProfileApps)iin);
      }
      return new android.content.pm.ICrossProfileApps.Stub.Proxy(obj);
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
        case TRANSACTION_startActivityAsUser:
        {
          data.enforceInterface(descriptor);
          android.app.IApplicationThread _arg0;
          _arg0 = android.app.IApplicationThread.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.startActivityAsUser(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTargetUserProfiles:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.os.UserHandle> _result = this.getTargetUserProfiles(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.pm.ICrossProfileApps
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
      @Override public void startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, int userId, boolean launchMainActivity) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((caller!=null))?(caller.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((component!=null)) {
            _data.writeInt(1);
            component.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          _data.writeInt(((launchMainActivity)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startActivityAsUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startActivityAsUser(caller, callingPackage, component, userId, launchMainActivity);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.os.UserHandle> getTargetUserProfiles(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.os.UserHandle> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTargetUserProfiles, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTargetUserProfiles(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.os.UserHandle.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.content.pm.ICrossProfileApps sDefaultImpl;
    }
    static final int TRANSACTION_startActivityAsUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getTargetUserProfiles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.content.pm.ICrossProfileApps impl) {
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
    public static android.content.pm.ICrossProfileApps getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void startActivityAsUser(android.app.IApplicationThread caller, java.lang.String callingPackage, android.content.ComponentName component, int userId, boolean launchMainActivity) throws android.os.RemoteException;
  public java.util.List<android.os.UserHandle> getTargetUserProfiles(java.lang.String callingPackage) throws android.os.RemoteException;
}
