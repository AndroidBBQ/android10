/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.autofill;
/**
 * Object running in the application process and responsible to provide the functionalities
 * required by an Augmented Autofill service.
 *
 * @hide
 */
public interface IAugmentedAutofillManagerClient extends android.os.IInterface
{
  /** Default implementation for IAugmentedAutofillManagerClient. */
  public static class Default implements android.view.autofill.IAugmentedAutofillManagerClient
  {
    /**
          * Gets the coordinates of the input field view.
          */
    @Override public android.graphics.Rect getViewCoordinates(android.view.autofill.AutofillId id) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Autofills the activity with the contents of the values.
         */
    @Override public void autofill(int sessionId, java.util.List<android.view.autofill.AutofillId> ids, java.util.List<android.view.autofill.AutofillValue> values) throws android.os.RemoteException
    {
    }
    /**
          * Requests showing the fill UI.
          */
    @Override public void requestShowFillUi(int sessionId, android.view.autofill.AutofillId id, int width, int height, android.graphics.Rect anchorBounds, android.view.autofill.IAutofillWindowPresenter presenter) throws android.os.RemoteException
    {
    }
    /**
          * Requests hiding the fill UI.
          */
    @Override public void requestHideFillUi(int sessionId, android.view.autofill.AutofillId id) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.autofill.IAugmentedAutofillManagerClient
  {
    private static final java.lang.String DESCRIPTOR = "android.view.autofill.IAugmentedAutofillManagerClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.autofill.IAugmentedAutofillManagerClient interface,
     * generating a proxy if needed.
     */
    public static android.view.autofill.IAugmentedAutofillManagerClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.autofill.IAugmentedAutofillManagerClient))) {
        return ((android.view.autofill.IAugmentedAutofillManagerClient)iin);
      }
      return new android.view.autofill.IAugmentedAutofillManagerClient.Stub.Proxy(obj);
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
        case TRANSACTION_getViewCoordinates:
        {
          data.enforceInterface(descriptor);
          android.view.autofill.AutofillId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.autofill.AutofillId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.graphics.Rect _result = this.getViewCoordinates(_arg0);
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
        case TRANSACTION_autofill:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.view.autofill.AutofillId> _arg1;
          _arg1 = data.createTypedArrayList(android.view.autofill.AutofillId.CREATOR);
          java.util.List<android.view.autofill.AutofillValue> _arg2;
          _arg2 = data.createTypedArrayList(android.view.autofill.AutofillValue.CREATOR);
          this.autofill(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestShowFillUi:
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
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.graphics.Rect _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.view.autofill.IAutofillWindowPresenter _arg5;
          _arg5 = android.view.autofill.IAutofillWindowPresenter.Stub.asInterface(data.readStrongBinder());
          this.requestShowFillUi(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestHideFillUi:
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
          this.requestHideFillUi(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.autofill.IAugmentedAutofillManagerClient
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
            * Gets the coordinates of the input field view.
            */
      @Override public android.graphics.Rect getViewCoordinates(android.view.autofill.AutofillId id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Rect _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((id!=null)) {
            _data.writeInt(1);
            id.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getViewCoordinates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getViewCoordinates(id);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Rect.CREATOR.createFromParcel(_reply);
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
           * Autofills the activity with the contents of the values.
           */
      @Override public void autofill(int sessionId, java.util.List<android.view.autofill.AutofillId> ids, java.util.List<android.view.autofill.AutofillValue> values) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          _data.writeTypedList(ids);
          _data.writeTypedList(values);
          boolean _status = mRemote.transact(Stub.TRANSACTION_autofill, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().autofill(sessionId, ids, values);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
            * Requests showing the fill UI.
            */
      @Override public void requestShowFillUi(int sessionId, android.view.autofill.AutofillId id, int width, int height, android.graphics.Rect anchorBounds, android.view.autofill.IAutofillWindowPresenter presenter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
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
          _data.writeInt(width);
          _data.writeInt(height);
          if ((anchorBounds!=null)) {
            _data.writeInt(1);
            anchorBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((presenter!=null))?(presenter.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestShowFillUi, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestShowFillUi(sessionId, id, width, height, anchorBounds, presenter);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
            * Requests hiding the fill UI.
            */
      @Override public void requestHideFillUi(int sessionId, android.view.autofill.AutofillId id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestHideFillUi, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestHideFillUi(sessionId, id);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.view.autofill.IAugmentedAutofillManagerClient sDefaultImpl;
    }
    static final int TRANSACTION_getViewCoordinates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_autofill = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_requestShowFillUi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_requestHideFillUi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.view.autofill.IAugmentedAutofillManagerClient impl) {
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
    public static android.view.autofill.IAugmentedAutofillManagerClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
        * Gets the coordinates of the input field view.
        */
  public android.graphics.Rect getViewCoordinates(android.view.autofill.AutofillId id) throws android.os.RemoteException;
  /**
       * Autofills the activity with the contents of the values.
       */
  public void autofill(int sessionId, java.util.List<android.view.autofill.AutofillId> ids, java.util.List<android.view.autofill.AutofillValue> values) throws android.os.RemoteException;
  /**
        * Requests showing the fill UI.
        */
  public void requestShowFillUi(int sessionId, android.view.autofill.AutofillId id, int width, int height, android.graphics.Rect anchorBounds, android.view.autofill.IAutofillWindowPresenter presenter) throws android.os.RemoteException;
  /**
        * Requests hiding the fill UI.
        */
  public void requestHideFillUi(int sessionId, android.view.autofill.AutofillId id) throws android.os.RemoteException;
}
