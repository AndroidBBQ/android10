/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.autofill;
/**
 * Mediator between apps being auto-filled and auto-fill service implementations.
 *
 * {@hide}
 */
public interface IAutoFillManager extends android.os.IInterface
{
  /** Default implementation for IAutoFillManager. */
  public static class Default implements android.view.autofill.IAutoFillManager
  {
    // Returns flags: FLAG_ADD_CLIENT_ENABLED | FLAG_ADD_CLIENT_DEBUG | FLAG_ADD_CLIENT_VERBOSE

    @Override public void addClient(android.view.autofill.IAutoFillManagerClient client, android.content.ComponentName componentName, int userId, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void removeClient(android.view.autofill.IAutoFillManagerClient client, int userId) throws android.os.RemoteException
    {
    }
    @Override public void startSession(android.os.IBinder activityToken, android.os.IBinder appCallback, android.view.autofill.AutofillId autoFillId, android.graphics.Rect bounds, android.view.autofill.AutofillValue value, int userId, boolean hasCallback, int flags, android.content.ComponentName componentName, boolean compatMode, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void getFillEventHistory(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void restoreSession(int sessionId, android.os.IBinder activityToken, android.os.IBinder appCallback, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void updateSession(int sessionId, android.view.autofill.AutofillId id, android.graphics.Rect bounds, android.view.autofill.AutofillValue value, int action, int flags, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setAutofillFailure(int sessionId, java.util.List<android.view.autofill.AutofillId> ids, int userId) throws android.os.RemoteException
    {
    }
    @Override public void finishSession(int sessionId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void cancelSession(int sessionId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setAuthenticationResult(android.os.Bundle data, int sessionId, int authenticationId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void setHasCallback(int sessionId, int userId, boolean hasIt) throws android.os.RemoteException
    {
    }
    @Override public void disableOwnedAutofillServices(int userId) throws android.os.RemoteException
    {
    }
    @Override public void isServiceSupported(int userId, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void isServiceEnabled(int userId, java.lang.String packageName, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void onPendingSaveUi(int operation, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void getUserData(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void getUserDataId(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void setUserData(android.service.autofill.UserData userData) throws android.os.RemoteException
    {
    }
    @Override public void isFieldClassificationEnabled(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void getAutofillServiceComponentName(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void getAvailableFieldClassificationAlgorithms(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void getDefaultFieldClassificationAlgorithm(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override public void setAugmentedAutofillWhitelist(java.util.List<java.lang.String> packages, java.util.List<android.content.ComponentName> activities, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.autofill.IAutoFillManager
  {
    private static final java.lang.String DESCRIPTOR = "android.view.autofill.IAutoFillManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.autofill.IAutoFillManager interface,
     * generating a proxy if needed.
     */
    public static android.view.autofill.IAutoFillManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.autofill.IAutoFillManager))) {
        return ((android.view.autofill.IAutoFillManager)iin);
      }
      return new android.view.autofill.IAutoFillManager.Stub.Proxy(obj);
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
        case TRANSACTION_addClient:
        {
          data.enforceInterface(descriptor);
          android.view.autofill.IAutoFillManagerClient _arg0;
          _arg0 = android.view.autofill.IAutoFillManagerClient.Stub.asInterface(data.readStrongBinder());
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.os.IResultReceiver _arg3;
          _arg3 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.addClient(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_removeClient:
        {
          data.enforceInterface(descriptor);
          android.view.autofill.IAutoFillManagerClient _arg0;
          _arg0 = android.view.autofill.IAutoFillManagerClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.removeClient(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_startSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.view.autofill.AutofillId _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.autofill.AutofillId.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.view.autofill.AutofillValue _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.view.autofill.AutofillValue.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          int _arg5;
          _arg5 = data.readInt();
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          int _arg7;
          _arg7 = data.readInt();
          android.content.ComponentName _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          boolean _arg9;
          _arg9 = (0!=data.readInt());
          com.android.internal.os.IResultReceiver _arg10;
          _arg10 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.startSession(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10);
          return true;
        }
        case TRANSACTION_getFillEventHistory:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getFillEventHistory(_arg0);
          return true;
        }
        case TRANSACTION_restoreSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          com.android.internal.os.IResultReceiver _arg3;
          _arg3 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.restoreSession(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_updateSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.autofill.AutofillId _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.autofill.AutofillId.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.graphics.Rect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.view.autofill.AutofillValue _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.view.autofill.AutofillValue.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          this.updateSession(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        case TRANSACTION_setAutofillFailure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.view.autofill.AutofillId> _arg1;
          _arg1 = data.createTypedArrayList(android.view.autofill.AutofillId.CREATOR);
          int _arg2;
          _arg2 = data.readInt();
          this.setAutofillFailure(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_finishSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.finishSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_cancelSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.cancelSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setAuthenticationResult:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.setAuthenticationResult(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_setHasCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setHasCallback(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_disableOwnedAutofillServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableOwnedAutofillServices(_arg0);
          return true;
        }
        case TRANSACTION_isServiceSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.os.IResultReceiver _arg1;
          _arg1 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.isServiceSupported(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_isServiceEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.os.IResultReceiver _arg2;
          _arg2 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.isServiceEnabled(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onPendingSaveUi:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.onPendingSaveUi(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getUserData:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getUserData(_arg0);
          return true;
        }
        case TRANSACTION_getUserDataId:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getUserDataId(_arg0);
          return true;
        }
        case TRANSACTION_setUserData:
        {
          data.enforceInterface(descriptor);
          android.service.autofill.UserData _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.autofill.UserData.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setUserData(_arg0);
          return true;
        }
        case TRANSACTION_isFieldClassificationEnabled:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.isFieldClassificationEnabled(_arg0);
          return true;
        }
        case TRANSACTION_getAutofillServiceComponentName:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getAutofillServiceComponentName(_arg0);
          return true;
        }
        case TRANSACTION_getAvailableFieldClassificationAlgorithms:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getAvailableFieldClassificationAlgorithms(_arg0);
          return true;
        }
        case TRANSACTION_getDefaultFieldClassificationAlgorithm:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.getDefaultFieldClassificationAlgorithm(_arg0);
          return true;
        }
        case TRANSACTION_setAugmentedAutofillWhitelist:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          java.util.List<android.content.ComponentName> _arg1;
          _arg1 = data.createTypedArrayList(android.content.ComponentName.CREATOR);
          com.android.internal.os.IResultReceiver _arg2;
          _arg2 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          this.setAugmentedAutofillWhitelist(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.autofill.IAutoFillManager
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
      // Returns flags: FLAG_ADD_CLIENT_ENABLED | FLAG_ADD_CLIENT_DEBUG | FLAG_ADD_CLIENT_VERBOSE

      @Override public void addClient(android.view.autofill.IAutoFillManagerClient client, android.content.ComponentName componentName, int userId, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addClient(client, componentName, userId, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeClient(android.view.autofill.IAutoFillManagerClient client, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeClient(client, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startSession(android.os.IBinder activityToken, android.os.IBinder appCallback, android.view.autofill.AutofillId autoFillId, android.graphics.Rect bounds, android.view.autofill.AutofillValue value, int userId, boolean hasCallback, int flags, android.content.ComponentName componentName, boolean compatMode, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activityToken);
          _data.writeStrongBinder(appCallback);
          if ((autoFillId!=null)) {
            _data.writeInt(1);
            autoFillId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((value!=null)) {
            _data.writeInt(1);
            value.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          _data.writeInt(((hasCallback)?(1):(0)));
          _data.writeInt(flags);
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((compatMode)?(1):(0)));
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startSession(activityToken, appCallback, autoFillId, bounds, value, userId, hasCallback, flags, componentName, compatMode, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getFillEventHistory(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFillEventHistory, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getFillEventHistory(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void restoreSession(int sessionId, android.os.IBinder activityToken, android.os.IBinder appCallback, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeStrongBinder(activityToken);
          _data.writeStrongBinder(appCallback);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreSession(sessionId, activityToken, appCallback, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateSession(int sessionId, android.view.autofill.AutofillId id, android.graphics.Rect bounds, android.view.autofill.AutofillValue value, int action, int flags, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          if ((id!=null)) {
            _data.writeInt(1);
            id.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((value!=null)) {
            _data.writeInt(1);
            value.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(action);
          _data.writeInt(flags);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateSession(sessionId, id, bounds, value, action, flags, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAutofillFailure(int sessionId, java.util.List<android.view.autofill.AutofillId> ids, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeTypedList(ids);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAutofillFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAutofillFailure(sessionId, ids, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void finishSession(int sessionId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSession(sessionId, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelSession(int sessionId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSession(sessionId, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAuthenticationResult(android.os.Bundle data, int sessionId, int authenticationId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sessionId);
          _data.writeInt(authenticationId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAuthenticationResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAuthenticationResult(data, sessionId, authenticationId, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setHasCallback(int sessionId, int userId, boolean hasIt) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeInt(userId);
          _data.writeInt(((hasIt)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHasCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setHasCallback(sessionId, userId, hasIt);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disableOwnedAutofillServices(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableOwnedAutofillServices, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableOwnedAutofillServices(userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void isServiceSupported(int userId, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isServiceSupported, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isServiceSupported(userId, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void isServiceEnabled(int userId, java.lang.String packageName, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isServiceEnabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isServiceEnabled(userId, packageName, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPendingSaveUi(int operation, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(operation);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPendingSaveUi, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPendingSaveUi(operation, token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getUserData(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUserData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getUserData(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getUserDataId(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUserDataId, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getUserDataId(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setUserData(android.service.autofill.UserData userData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((userData!=null)) {
            _data.writeInt(1);
            userData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserData(userData);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void isFieldClassificationEnabled(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isFieldClassificationEnabled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().isFieldClassificationEnabled(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getAutofillServiceComponentName(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAutofillServiceComponentName, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAutofillServiceComponentName(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getAvailableFieldClassificationAlgorithms(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableFieldClassificationAlgorithms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAvailableFieldClassificationAlgorithms(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getDefaultFieldClassificationAlgorithm(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultFieldClassificationAlgorithm, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getDefaultFieldClassificationAlgorithm(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAugmentedAutofillWhitelist(java.util.List<java.lang.String> packages, java.util.List<android.content.ComponentName> activities, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(packages);
          _data.writeTypedList(activities);
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAugmentedAutofillWhitelist, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAugmentedAutofillWhitelist(packages, activities, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.autofill.IAutoFillManager sDefaultImpl;
    }
    static final int TRANSACTION_addClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getFillEventHistory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_restoreSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_updateSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setAutofillFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_finishSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_cancelSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setAuthenticationResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setHasCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_disableOwnedAutofillServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_isServiceSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_isServiceEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onPendingSaveUi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getUserData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getUserDataId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setUserData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_isFieldClassificationEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getAutofillServiceComponentName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getAvailableFieldClassificationAlgorithms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getDefaultFieldClassificationAlgorithm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setAugmentedAutofillWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    public static boolean setDefaultImpl(android.view.autofill.IAutoFillManager impl) {
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
    public static android.view.autofill.IAutoFillManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Returns flags: FLAG_ADD_CLIENT_ENABLED | FLAG_ADD_CLIENT_DEBUG | FLAG_ADD_CLIENT_VERBOSE

  public void addClient(android.view.autofill.IAutoFillManagerClient client, android.content.ComponentName componentName, int userId, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void removeClient(android.view.autofill.IAutoFillManagerClient client, int userId) throws android.os.RemoteException;
  public void startSession(android.os.IBinder activityToken, android.os.IBinder appCallback, android.view.autofill.AutofillId autoFillId, android.graphics.Rect bounds, android.view.autofill.AutofillValue value, int userId, boolean hasCallback, int flags, android.content.ComponentName componentName, boolean compatMode, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void getFillEventHistory(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void restoreSession(int sessionId, android.os.IBinder activityToken, android.os.IBinder appCallback, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void updateSession(int sessionId, android.view.autofill.AutofillId id, android.graphics.Rect bounds, android.view.autofill.AutofillValue value, int action, int flags, int userId) throws android.os.RemoteException;
  public void setAutofillFailure(int sessionId, java.util.List<android.view.autofill.AutofillId> ids, int userId) throws android.os.RemoteException;
  public void finishSession(int sessionId, int userId) throws android.os.RemoteException;
  public void cancelSession(int sessionId, int userId) throws android.os.RemoteException;
  public void setAuthenticationResult(android.os.Bundle data, int sessionId, int authenticationId, int userId) throws android.os.RemoteException;
  public void setHasCallback(int sessionId, int userId, boolean hasIt) throws android.os.RemoteException;
  public void disableOwnedAutofillServices(int userId) throws android.os.RemoteException;
  public void isServiceSupported(int userId, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void isServiceEnabled(int userId, java.lang.String packageName, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void onPendingSaveUi(int operation, android.os.IBinder token) throws android.os.RemoteException;
  public void getUserData(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void getUserDataId(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void setUserData(android.service.autofill.UserData userData) throws android.os.RemoteException;
  public void isFieldClassificationEnabled(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void getAutofillServiceComponentName(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void getAvailableFieldClassificationAlgorithms(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void getDefaultFieldClassificationAlgorithm(com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
  public void setAugmentedAutofillWhitelist(java.util.List<java.lang.String> packages, java.util.List<android.content.ComponentName> activities, com.android.internal.os.IResultReceiver result) throws android.os.RemoteException;
}
