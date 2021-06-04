/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.timezone;
/**
  * Interface to the TimeZone Rules Manager Service.
  *
  * <p>This interface is only intended for system apps to call. They should use the
  * {@link android.app.timezone.RulesManager} class rather than going through this
  * Binder interface directly. See {@link android.app.timezone.RulesManager} for more complete
  * documentation.
  *
  * {@hide}
  */
public interface IRulesManager extends android.os.IInterface
{
  /** Default implementation for IRulesManager. */
  public static class Default implements android.app.timezone.IRulesManager
  {
    /**
         * Returns information about the current time zone rules state such as the IANA version of
         * the system and any currently installed distro. This method is intended to allow clients to
         * determine if the current state can be improved; for example by passing the information to a
         * server that may provide a new distro for download.
         */
    @Override public android.app.timezone.RulesState getRulesState() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Requests installation of the supplied distro. The distro must have been checked for integrity
         * by the caller or have been received via a trusted mechanism.
         *
         * @param distroFileDescriptor the file descriptor for the distro
         * @param checkToken an optional token provided if the install was triggered in response to a
         *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
         * @param callback the {@link ICallback} to receive callbacks related to the
         *     installation
         * @return zero if the installation will be attempted; nonzero on error
         */
    @Override public int requestInstall(android.os.ParcelFileDescriptor distroFileDescriptor, byte[] checkToken, android.app.timezone.ICallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Requests uninstallation of the currently installed distro (leaving the device with no
         * distro installed).
         *
         * @param checkToken an optional token provided if the uninstall was triggered in response to a
         *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
         * @param callback the {@link ICallback} to receive callbacks related to the
         *     uninstall
         * @return zero if the uninstallation will be attempted; nonzero on error
         */
    @Override public int requestUninstall(byte[] checkToken, android.app.timezone.ICallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Requests the system does not modify the currently installed time zone distro, if any. This
         * method records the fact that a time zone check operation triggered by the system is now
         * complete and there was nothing to do. The token passed should be the one presented when the
         * check was triggered.
         *
         * <p>Note: Passing {@code success == false} may result in more checks being triggered. Clients
         * should be careful not to pass false if the failure is unlikely to resolve by itself.
         *
         * @param checkToken an optional token provided if the install was triggered in response to a
         *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
         * @param success true if the check was successful, false if it was not successful but may
         *     succeed if it is retried
         */
    @Override public void requestNothing(byte[] token, boolean success) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.timezone.IRulesManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.timezone.IRulesManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.timezone.IRulesManager interface,
     * generating a proxy if needed.
     */
    public static android.app.timezone.IRulesManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.timezone.IRulesManager))) {
        return ((android.app.timezone.IRulesManager)iin);
      }
      return new android.app.timezone.IRulesManager.Stub.Proxy(obj);
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
        case TRANSACTION_getRulesState:
        {
          data.enforceInterface(descriptor);
          android.app.timezone.RulesState _result = this.getRulesState();
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
        case TRANSACTION_requestInstall:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          byte[] _arg1;
          _arg1 = data.createByteArray();
          android.app.timezone.ICallback _arg2;
          _arg2 = android.app.timezone.ICallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.requestInstall(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestUninstall:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          android.app.timezone.ICallback _arg1;
          _arg1 = android.app.timezone.ICallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.requestUninstall(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_requestNothing:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.requestNothing(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.timezone.IRulesManager
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
           * Returns information about the current time zone rules state such as the IANA version of
           * the system and any currently installed distro. This method is intended to allow clients to
           * determine if the current state can be improved; for example by passing the information to a
           * server that may provide a new distro for download.
           */
      @Override public android.app.timezone.RulesState getRulesState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.timezone.RulesState _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRulesState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRulesState();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.timezone.RulesState.CREATOR.createFromParcel(_reply);
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
      /**
           * Requests installation of the supplied distro. The distro must have been checked for integrity
           * by the caller or have been received via a trusted mechanism.
           *
           * @param distroFileDescriptor the file descriptor for the distro
           * @param checkToken an optional token provided if the install was triggered in response to a
           *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
           * @param callback the {@link ICallback} to receive callbacks related to the
           *     installation
           * @return zero if the installation will be attempted; nonzero on error
           */
      @Override public int requestInstall(android.os.ParcelFileDescriptor distroFileDescriptor, byte[] checkToken, android.app.timezone.ICallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((distroFileDescriptor!=null)) {
            _data.writeInt(1);
            distroFileDescriptor.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeByteArray(checkToken);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestInstall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestInstall(distroFileDescriptor, checkToken, callback);
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
      /**
           * Requests uninstallation of the currently installed distro (leaving the device with no
           * distro installed).
           *
           * @param checkToken an optional token provided if the uninstall was triggered in response to a
           *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
           * @param callback the {@link ICallback} to receive callbacks related to the
           *     uninstall
           * @return zero if the uninstallation will be attempted; nonzero on error
           */
      @Override public int requestUninstall(byte[] checkToken, android.app.timezone.ICallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(checkToken);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestUninstall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestUninstall(checkToken, callback);
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
      /**
           * Requests the system does not modify the currently installed time zone distro, if any. This
           * method records the fact that a time zone check operation triggered by the system is now
           * complete and there was nothing to do. The token passed should be the one presented when the
           * check was triggered.
           *
           * <p>Note: Passing {@code success == false} may result in more checks being triggered. Clients
           * should be careful not to pass false if the failure is unlikely to resolve by itself.
           *
           * @param checkToken an optional token provided if the install was triggered in response to a
           *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
           * @param success true if the check was successful, false if it was not successful but may
           *     succeed if it is retried
           */
      @Override public void requestNothing(byte[] token, boolean success) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(token);
          _data.writeInt(((success)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestNothing, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestNothing(token, success);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.timezone.IRulesManager sDefaultImpl;
    }
    static final int TRANSACTION_getRulesState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_requestInstall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_requestUninstall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_requestNothing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.app.timezone.IRulesManager impl) {
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
    public static android.app.timezone.IRulesManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns information about the current time zone rules state such as the IANA version of
       * the system and any currently installed distro. This method is intended to allow clients to
       * determine if the current state can be improved; for example by passing the information to a
       * server that may provide a new distro for download.
       */
  public android.app.timezone.RulesState getRulesState() throws android.os.RemoteException;
  /**
       * Requests installation of the supplied distro. The distro must have been checked for integrity
       * by the caller or have been received via a trusted mechanism.
       *
       * @param distroFileDescriptor the file descriptor for the distro
       * @param checkToken an optional token provided if the install was triggered in response to a
       *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
       * @param callback the {@link ICallback} to receive callbacks related to the
       *     installation
       * @return zero if the installation will be attempted; nonzero on error
       */
  public int requestInstall(android.os.ParcelFileDescriptor distroFileDescriptor, byte[] checkToken, android.app.timezone.ICallback callback) throws android.os.RemoteException;
  /**
       * Requests uninstallation of the currently installed distro (leaving the device with no
       * distro installed).
       *
       * @param checkToken an optional token provided if the uninstall was triggered in response to a
       *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
       * @param callback the {@link ICallback} to receive callbacks related to the
       *     uninstall
       * @return zero if the uninstallation will be attempted; nonzero on error
       */
  public int requestUninstall(byte[] checkToken, android.app.timezone.ICallback callback) throws android.os.RemoteException;
  /**
       * Requests the system does not modify the currently installed time zone distro, if any. This
       * method records the fact that a time zone check operation triggered by the system is now
       * complete and there was nothing to do. The token passed should be the one presented when the
       * check was triggered.
       *
       * <p>Note: Passing {@code success == false} may result in more checks being triggered. Clients
       * should be careful not to pass false if the failure is unlikely to resolve by itself.
       *
       * @param checkToken an optional token provided if the install was triggered in response to a
       *     {@link RulesUpdaterContract#ACTION_TRIGGER_RULES_UPDATE_CHECK} intent
       * @param success true if the check was successful, false if it was not successful but may
       *     succeed if it is retried
       */
  public void requestNothing(byte[] token, boolean success) throws android.os.RemoteException;
}
