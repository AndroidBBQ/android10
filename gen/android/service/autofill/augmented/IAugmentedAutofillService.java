/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.autofill.augmented;
/**
 * Interface from the system to an Augmented Autofill service.
 *
 * @hide
 */
public interface IAugmentedAutofillService extends android.os.IInterface
{
  /** Default implementation for IAugmentedAutofillService. */
  public static class Default implements android.service.autofill.augmented.IAugmentedAutofillService
  {
    @Override public void onConnected(boolean debug, boolean verbose) throws android.os.RemoteException
    {
    }
    @Override public void onDisconnected() throws android.os.RemoteException
    {
    }
    @Override public void onFillRequest(int sessionId, android.os.IBinder autofillManagerClient, int taskId, android.content.ComponentName activityComponent, android.view.autofill.AutofillId focusedId, android.view.autofill.AutofillValue focusedValue, long requestTime, android.service.autofill.augmented.IFillCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onDestroyAllFillWindowsRequest() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.autofill.augmented.IAugmentedAutofillService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.autofill.augmented.IAugmentedAutofillService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.autofill.augmented.IAugmentedAutofillService interface,
     * generating a proxy if needed.
     */
    public static android.service.autofill.augmented.IAugmentedAutofillService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.autofill.augmented.IAugmentedAutofillService))) {
        return ((android.service.autofill.augmented.IAugmentedAutofillService)iin);
      }
      return new android.service.autofill.augmented.IAugmentedAutofillService.Stub.Proxy(obj);
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
        case TRANSACTION_onConnected:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onConnected(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDisconnected:
        {
          data.enforceInterface(descriptor);
          this.onDisconnected();
          return true;
        }
        case TRANSACTION_onFillRequest:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          android.content.ComponentName _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.view.autofill.AutofillId _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.view.autofill.AutofillId.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.view.autofill.AutofillValue _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.view.autofill.AutofillValue.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          long _arg6;
          _arg6 = data.readLong();
          android.service.autofill.augmented.IFillCallback _arg7;
          _arg7 = android.service.autofill.augmented.IFillCallback.Stub.asInterface(data.readStrongBinder());
          this.onFillRequest(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_onDestroyAllFillWindowsRequest:
        {
          data.enforceInterface(descriptor);
          this.onDestroyAllFillWindowsRequest();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.autofill.augmented.IAugmentedAutofillService
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
      @Override public void onConnected(boolean debug, boolean verbose) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((debug)?(1):(0)));
          _data.writeInt(((verbose)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnected(debug, verbose);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDisconnected() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDisconnected();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFillRequest(int sessionId, android.os.IBinder autofillManagerClient, int taskId, android.content.ComponentName activityComponent, android.view.autofill.AutofillId focusedId, android.view.autofill.AutofillValue focusedValue, long requestTime, android.service.autofill.augmented.IFillCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeStrongBinder(autofillManagerClient);
          _data.writeInt(taskId);
          if ((activityComponent!=null)) {
            _data.writeInt(1);
            activityComponent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((focusedId!=null)) {
            _data.writeInt(1);
            focusedId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((focusedValue!=null)) {
            _data.writeInt(1);
            focusedValue.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(requestTime);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFillRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFillRequest(sessionId, autofillManagerClient, taskId, activityComponent, focusedId, focusedValue, requestTime, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDestroyAllFillWindowsRequest() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDestroyAllFillWindowsRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDestroyAllFillWindowsRequest();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.autofill.augmented.IAugmentedAutofillService sDefaultImpl;
    }
    static final int TRANSACTION_onConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onFillRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onDestroyAllFillWindowsRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.service.autofill.augmented.IAugmentedAutofillService impl) {
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
    public static android.service.autofill.augmented.IAugmentedAutofillService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onConnected(boolean debug, boolean verbose) throws android.os.RemoteException;
  public void onDisconnected() throws android.os.RemoteException;
  public void onFillRequest(int sessionId, android.os.IBinder autofillManagerClient, int taskId, android.content.ComponentName activityComponent, android.view.autofill.AutofillId focusedId, android.view.autofill.AutofillValue focusedValue, long requestTime, android.service.autofill.augmented.IFillCallback callback) throws android.os.RemoteException;
  public void onDestroyAllFillWindowsRequest() throws android.os.RemoteException;
}
