/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Public interface to the global input method manager, used by all client
 * applications.
 */
public interface IInputMethodManager extends android.os.IInterface
{
  /** Default implementation for IInputMethodManager. */
  public static class Default implements com.android.internal.view.IInputMethodManager
  {
    @Override public void addClient(com.android.internal.view.IInputMethodClient client, com.android.internal.view.IInputContext inputContext, int untrustedDisplayId) throws android.os.RemoteException
    {
    }
    // TODO: Use ParceledListSlice instead

    @Override public java.util.List<android.view.inputmethod.InputMethodInfo> getInputMethodList(int userId) throws android.os.RemoteException
    {
      return null;
    }
    // TODO: Use ParceledListSlice instead

    @Override public java.util.List<android.view.inputmethod.InputMethodInfo> getEnabledInputMethodList(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.view.inputmethod.InputMethodSubtype> getEnabledInputMethodSubtypeList(java.lang.String imiId, boolean allowsImplicitlySelectedSubtypes) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.view.inputmethod.InputMethodSubtype getLastInputMethodSubtype() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean showSoftInput(com.android.internal.view.IInputMethodClient client, int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean hideSoftInput(com.android.internal.view.IInputMethodClient client, int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
    {
      return false;
    }
    // If windowToken is null, this just does startInput().  Otherwise this reports that a window
    // has gained focus, and if 'attribute' is non-null then also does startInput.
    // @NonNull

    @Override public com.android.internal.view.InputBindResult startInputOrWindowGainedFocus(int startInputReason, com.android.internal.view.IInputMethodClient client, android.os.IBinder windowToken, int startInputFlags, int softInputMode, int windowFlags, android.view.inputmethod.EditorInfo attribute, com.android.internal.view.IInputContext inputContext, int missingMethodFlags, int unverifiedTargetSdkVersion) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void showInputMethodPickerFromClient(com.android.internal.view.IInputMethodClient client, int auxiliarySubtypeMode) throws android.os.RemoteException
    {
    }
    @Override public void showInputMethodPickerFromSystem(com.android.internal.view.IInputMethodClient client, int auxiliarySubtypeMode, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void showInputMethodAndSubtypeEnablerFromClient(com.android.internal.view.IInputMethodClient client, java.lang.String topId) throws android.os.RemoteException
    {
    }
    @Override public boolean isInputMethodPickerShownForTest() throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.view.inputmethod.InputMethodSubtype getCurrentInputMethodSubtype() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setAdditionalInputMethodSubtypes(java.lang.String id, android.view.inputmethod.InputMethodSubtype[] subtypes) throws android.os.RemoteException
    {
    }
    // This is kept due to @UnsupportedAppUsage.
    // TODO(Bug 113914148): Consider removing this.

    @Override public int getInputMethodWindowVisibleHeight() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void reportActivityView(com.android.internal.view.IInputMethodClient parentClient, int childDisplayId, float[] matrixValues) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IInputMethodManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IInputMethodManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IInputMethodManager interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IInputMethodManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IInputMethodManager))) {
        return ((com.android.internal.view.IInputMethodManager)iin);
      }
      return new com.android.internal.view.IInputMethodManager.Stub.Proxy(obj);
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
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          com.android.internal.view.IInputContext _arg1;
          _arg1 = com.android.internal.view.IInputContext.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.addClient(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInputMethodList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.view.inputmethod.InputMethodInfo> _result = this.getInputMethodList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getEnabledInputMethodList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.view.inputmethod.InputMethodInfo> _result = this.getEnabledInputMethodList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getEnabledInputMethodSubtypeList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.util.List<android.view.inputmethod.InputMethodSubtype> _result = this.getEnabledInputMethodSubtypeList(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getLastInputMethodSubtype:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.InputMethodSubtype _result = this.getLastInputMethodSubtype();
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
        case TRANSACTION_showSoftInput:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.os.ResultReceiver _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.showSoftInput(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hideSoftInput:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.os.ResultReceiver _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.hideSoftInput(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startInputOrWindowGainedFocus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.view.IInputMethodClient _arg1;
          _arg1 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          android.view.inputmethod.EditorInfo _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.view.inputmethod.EditorInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          com.android.internal.view.IInputContext _arg7;
          _arg7 = com.android.internal.view.IInputContext.Stub.asInterface(data.readStrongBinder());
          int _arg8;
          _arg8 = data.readInt();
          int _arg9;
          _arg9 = data.readInt();
          com.android.internal.view.InputBindResult _result = this.startInputOrWindowGainedFocus(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
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
        case TRANSACTION_showInputMethodPickerFromClient:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.showInputMethodPickerFromClient(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showInputMethodPickerFromSystem:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.showInputMethodPickerFromSystem(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_showInputMethodAndSubtypeEnablerFromClient:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.showInputMethodAndSubtypeEnablerFromClient(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isInputMethodPickerShownForTest:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isInputMethodPickerShownForTest();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentInputMethodSubtype:
        {
          data.enforceInterface(descriptor);
          android.view.inputmethod.InputMethodSubtype _result = this.getCurrentInputMethodSubtype();
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
        case TRANSACTION_setAdditionalInputMethodSubtypes:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.view.inputmethod.InputMethodSubtype[] _arg1;
          _arg1 = data.createTypedArray(android.view.inputmethod.InputMethodSubtype.CREATOR);
          this.setAdditionalInputMethodSubtypes(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInputMethodWindowVisibleHeight:
        {
          data.enforceInterface(descriptor);
          int _result = this.getInputMethodWindowVisibleHeight();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_reportActivityView:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodClient _arg0;
          _arg0 = com.android.internal.view.IInputMethodClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          float[] _arg2;
          _arg2 = data.createFloatArray();
          this.reportActivityView(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IInputMethodManager
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
      @Override public void addClient(com.android.internal.view.IInputMethodClient client, com.android.internal.view.IInputContext inputContext, int untrustedDisplayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeStrongBinder((((inputContext!=null))?(inputContext.asBinder()):(null)));
          _data.writeInt(untrustedDisplayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addClient(client, inputContext, untrustedDisplayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // TODO: Use ParceledListSlice instead

      @Override public java.util.List<android.view.inputmethod.InputMethodInfo> getInputMethodList(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.view.inputmethod.InputMethodInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInputMethodList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInputMethodList(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.view.inputmethod.InputMethodInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // TODO: Use ParceledListSlice instead

      @Override public java.util.List<android.view.inputmethod.InputMethodInfo> getEnabledInputMethodList(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.view.inputmethod.InputMethodInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnabledInputMethodList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnabledInputMethodList(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.view.inputmethod.InputMethodInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.view.inputmethod.InputMethodSubtype> getEnabledInputMethodSubtypeList(java.lang.String imiId, boolean allowsImplicitlySelectedSubtypes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.view.inputmethod.InputMethodSubtype> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(imiId);
          _data.writeInt(((allowsImplicitlySelectedSubtypes)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnabledInputMethodSubtypeList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnabledInputMethodSubtypeList(imiId, allowsImplicitlySelectedSubtypes);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.view.inputmethod.InputMethodSubtype.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.view.inputmethod.InputMethodSubtype getLastInputMethodSubtype() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.inputmethod.InputMethodSubtype _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastInputMethodSubtype, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastInputMethodSubtype();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.inputmethod.InputMethodSubtype.CREATOR.createFromParcel(_reply);
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
      @Override public boolean showSoftInput(com.android.internal.view.IInputMethodClient client, int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(flags);
          if ((resultReceiver!=null)) {
            _data.writeInt(1);
            resultReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_showSoftInput, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().showSoftInput(client, flags, resultReceiver);
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
      @Override public boolean hideSoftInput(com.android.internal.view.IInputMethodClient client, int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(flags);
          if ((resultReceiver!=null)) {
            _data.writeInt(1);
            resultReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideSoftInput, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hideSoftInput(client, flags, resultReceiver);
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
      // If windowToken is null, this just does startInput().  Otherwise this reports that a window
      // has gained focus, and if 'attribute' is non-null then also does startInput.
      // @NonNull

      @Override public com.android.internal.view.InputBindResult startInputOrWindowGainedFocus(int startInputReason, com.android.internal.view.IInputMethodClient client, android.os.IBinder windowToken, int startInputFlags, int softInputMode, int windowFlags, android.view.inputmethod.EditorInfo attribute, com.android.internal.view.IInputContext inputContext, int missingMethodFlags, int unverifiedTargetSdkVersion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.view.InputBindResult _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(startInputReason);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeStrongBinder(windowToken);
          _data.writeInt(startInputFlags);
          _data.writeInt(softInputMode);
          _data.writeInt(windowFlags);
          if ((attribute!=null)) {
            _data.writeInt(1);
            attribute.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((inputContext!=null))?(inputContext.asBinder()):(null)));
          _data.writeInt(missingMethodFlags);
          _data.writeInt(unverifiedTargetSdkVersion);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startInputOrWindowGainedFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startInputOrWindowGainedFocus(startInputReason, client, windowToken, startInputFlags, softInputMode, windowFlags, attribute, inputContext, missingMethodFlags, unverifiedTargetSdkVersion);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.view.InputBindResult.CREATOR.createFromParcel(_reply);
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
      @Override public void showInputMethodPickerFromClient(com.android.internal.view.IInputMethodClient client, int auxiliarySubtypeMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(auxiliarySubtypeMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showInputMethodPickerFromClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showInputMethodPickerFromClient(client, auxiliarySubtypeMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void showInputMethodPickerFromSystem(com.android.internal.view.IInputMethodClient client, int auxiliarySubtypeMode, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(auxiliarySubtypeMode);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showInputMethodPickerFromSystem, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showInputMethodPickerFromSystem(client, auxiliarySubtypeMode, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void showInputMethodAndSubtypeEnablerFromClient(com.android.internal.view.IInputMethodClient client, java.lang.String topId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeString(topId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showInputMethodAndSubtypeEnablerFromClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showInputMethodAndSubtypeEnablerFromClient(client, topId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isInputMethodPickerShownForTest() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInputMethodPickerShownForTest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInputMethodPickerShownForTest();
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
      @Override public android.view.inputmethod.InputMethodSubtype getCurrentInputMethodSubtype() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.inputmethod.InputMethodSubtype _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentInputMethodSubtype, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentInputMethodSubtype();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.inputmethod.InputMethodSubtype.CREATOR.createFromParcel(_reply);
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
      @Override public void setAdditionalInputMethodSubtypes(java.lang.String id, android.view.inputmethod.InputMethodSubtype[] subtypes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(id);
          _data.writeTypedArray(subtypes, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAdditionalInputMethodSubtypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAdditionalInputMethodSubtypes(id, subtypes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // This is kept due to @UnsupportedAppUsage.
      // TODO(Bug 113914148): Consider removing this.

      @Override public int getInputMethodWindowVisibleHeight() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInputMethodWindowVisibleHeight, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInputMethodWindowVisibleHeight();
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
      @Override public void reportActivityView(com.android.internal.view.IInputMethodClient parentClient, int childDisplayId, float[] matrixValues) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((parentClient!=null))?(parentClient.asBinder()):(null)));
          _data.writeInt(childDisplayId);
          _data.writeFloatArray(matrixValues);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportActivityView, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportActivityView(parentClient, childDisplayId, matrixValues);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.view.IInputMethodManager sDefaultImpl;
    }
    static final int TRANSACTION_addClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getInputMethodList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getEnabledInputMethodList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getEnabledInputMethodSubtypeList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getLastInputMethodSubtype = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_showSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_hideSoftInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_startInputOrWindowGainedFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_showInputMethodPickerFromClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_showInputMethodPickerFromSystem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_showInputMethodAndSubtypeEnablerFromClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_isInputMethodPickerShownForTest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getCurrentInputMethodSubtype = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setAdditionalInputMethodSubtypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getInputMethodWindowVisibleHeight = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_reportActivityView = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    public static boolean setDefaultImpl(com.android.internal.view.IInputMethodManager impl) {
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
    public static com.android.internal.view.IInputMethodManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void addClient(com.android.internal.view.IInputMethodClient client, com.android.internal.view.IInputContext inputContext, int untrustedDisplayId) throws android.os.RemoteException;
  // TODO: Use ParceledListSlice instead

  public java.util.List<android.view.inputmethod.InputMethodInfo> getInputMethodList(int userId) throws android.os.RemoteException;
  // TODO: Use ParceledListSlice instead

  public java.util.List<android.view.inputmethod.InputMethodInfo> getEnabledInputMethodList(int userId) throws android.os.RemoteException;
  public java.util.List<android.view.inputmethod.InputMethodSubtype> getEnabledInputMethodSubtypeList(java.lang.String imiId, boolean allowsImplicitlySelectedSubtypes) throws android.os.RemoteException;
  public android.view.inputmethod.InputMethodSubtype getLastInputMethodSubtype() throws android.os.RemoteException;
  public boolean showSoftInput(com.android.internal.view.IInputMethodClient client, int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  public boolean hideSoftInput(com.android.internal.view.IInputMethodClient client, int flags, android.os.ResultReceiver resultReceiver) throws android.os.RemoteException;
  // If windowToken is null, this just does startInput().  Otherwise this reports that a window
  // has gained focus, and if 'attribute' is non-null then also does startInput.
  // @NonNull

  public com.android.internal.view.InputBindResult startInputOrWindowGainedFocus(int startInputReason, com.android.internal.view.IInputMethodClient client, android.os.IBinder windowToken, int startInputFlags, int softInputMode, int windowFlags, android.view.inputmethod.EditorInfo attribute, com.android.internal.view.IInputContext inputContext, int missingMethodFlags, int unverifiedTargetSdkVersion) throws android.os.RemoteException;
  public void showInputMethodPickerFromClient(com.android.internal.view.IInputMethodClient client, int auxiliarySubtypeMode) throws android.os.RemoteException;
  public void showInputMethodPickerFromSystem(com.android.internal.view.IInputMethodClient client, int auxiliarySubtypeMode, int displayId) throws android.os.RemoteException;
  public void showInputMethodAndSubtypeEnablerFromClient(com.android.internal.view.IInputMethodClient client, java.lang.String topId) throws android.os.RemoteException;
  public boolean isInputMethodPickerShownForTest() throws android.os.RemoteException;
  public android.view.inputmethod.InputMethodSubtype getCurrentInputMethodSubtype() throws android.os.RemoteException;
  public void setAdditionalInputMethodSubtypes(java.lang.String id, android.view.inputmethod.InputMethodSubtype[] subtypes) throws android.os.RemoteException;
  // This is kept due to @UnsupportedAppUsage.
  // TODO(Bug 113914148): Consider removing this.

  public int getInputMethodWindowVisibleHeight() throws android.os.RemoteException;
  public void reportActivityView(com.android.internal.view.IInputMethodClient parentClient, int childDisplayId, float[] matrixValues) throws android.os.RemoteException;
}
