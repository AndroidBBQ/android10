/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.cluster.renderer;
/**
 * Helper binder API for InstrumentClusterRenderingService. This contains binder calls to car
 * service.
 *
 * @hide
 */
public interface IInstrumentClusterHelper extends android.os.IInterface
{
  /** Default implementation for IInstrumentClusterHelper. */
  public static class Default implements android.car.cluster.renderer.IInstrumentClusterHelper
  {
    /**
         * Start an activity to specified display / user. The activity is considered as
         * in fixed mode for the display and will be re-launched if the activity crashes, the package
         * is updated or goes to background for whatever reason.
         * Only one activity can exist in fixed mode for the target display and calling this multiple
         * times with different {@code Intent} will lead into making all previous activities into
         * non-fixed normal state (= will not be re-launched.)
         *
         * Do not change binder transaction number.
         */
    @Override public boolean startFixedActivityModeForDisplayAndUser(android.content.Intent intent, android.os.Bundle activityOptionsBundle, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * The activity lauched on the display is no longer in fixed mode. Re-launching or finishing
         * should not trigger re-launfhing any more. Note that Activity for non-current user will
         * be auto-stopped and there is no need to call this for user swiching. Note that this does not
         * stop the activity but it will not be re-launched any more.
         *
         * Do not change binder transaction number.
         */
    @Override public void stopFixedActivityMode(int displayId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.cluster.renderer.IInstrumentClusterHelper
  {
    private static final java.lang.String DESCRIPTOR = "android.car.cluster.renderer.IInstrumentClusterHelper";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.cluster.renderer.IInstrumentClusterHelper interface,
     * generating a proxy if needed.
     */
    public static android.car.cluster.renderer.IInstrumentClusterHelper asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.cluster.renderer.IInstrumentClusterHelper))) {
        return ((android.car.cluster.renderer.IInstrumentClusterHelper)iin);
      }
      return new android.car.cluster.renderer.IInstrumentClusterHelper.Stub.Proxy(obj);
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
        case TRANSACTION_startFixedActivityModeForDisplayAndUser:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.startFixedActivityModeForDisplayAndUser(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopFixedActivityMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.stopFixedActivityMode(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.cluster.renderer.IInstrumentClusterHelper
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
           * Start an activity to specified display / user. The activity is considered as
           * in fixed mode for the display and will be re-launched if the activity crashes, the package
           * is updated or goes to background for whatever reason.
           * Only one activity can exist in fixed mode for the target display and calling this multiple
           * times with different {@code Intent} will lead into making all previous activities into
           * non-fixed normal state (= will not be re-launched.)
           *
           * Do not change binder transaction number.
           */
      @Override public boolean startFixedActivityModeForDisplayAndUser(android.content.Intent intent, android.os.Bundle activityOptionsBundle, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((activityOptionsBundle!=null)) {
            _data.writeInt(1);
            activityOptionsBundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startFixedActivityModeForDisplayAndUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startFixedActivityModeForDisplayAndUser(intent, activityOptionsBundle, userId);
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
      /**
           * The activity lauched on the display is no longer in fixed mode. Re-launching or finishing
           * should not trigger re-launfhing any more. Note that Activity for non-current user will
           * be auto-stopped and there is no need to call this for user swiching. Note that this does not
           * stop the activity but it will not be re-launched any more.
           *
           * Do not change binder transaction number.
           */
      @Override public void stopFixedActivityMode(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopFixedActivityMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopFixedActivityMode(displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.cluster.renderer.IInstrumentClusterHelper sDefaultImpl;
    }
    static final int TRANSACTION_startFixedActivityModeForDisplayAndUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopFixedActivityMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.cluster.renderer.IInstrumentClusterHelper impl) {
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
    public static android.car.cluster.renderer.IInstrumentClusterHelper getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Start an activity to specified display / user. The activity is considered as
       * in fixed mode for the display and will be re-launched if the activity crashes, the package
       * is updated or goes to background for whatever reason.
       * Only one activity can exist in fixed mode for the target display and calling this multiple
       * times with different {@code Intent} will lead into making all previous activities into
       * non-fixed normal state (= will not be re-launched.)
       *
       * Do not change binder transaction number.
       */
  public boolean startFixedActivityModeForDisplayAndUser(android.content.Intent intent, android.os.Bundle activityOptionsBundle, int userId) throws android.os.RemoteException;
  /**
       * The activity lauched on the display is no longer in fixed mode. Re-launching or finishing
       * should not trigger re-launfhing any more. Note that Activity for non-current user will
       * be auto-stopped and there is no need to call this for user swiching. Note that this does not
       * stop the activity but it will not be re-launched any more.
       *
       * Do not change binder transaction number.
       */
  public void stopFixedActivityMode(int displayId) throws android.os.RemoteException;
}
