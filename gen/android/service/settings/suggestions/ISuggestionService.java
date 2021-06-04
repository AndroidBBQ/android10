/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.settings.suggestions;
/** @hide */
public interface ISuggestionService extends android.os.IInterface
{
  /** Default implementation for ISuggestionService. */
  public static class Default implements android.service.settings.suggestions.ISuggestionService
  {
    /**
         * Return all available suggestions.
         */
    @Override public java.util.List<android.service.settings.suggestions.Suggestion> getSuggestions() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Dismiss a suggestion. The suggestion will not be included in future {@link #getSuggestions)
         * calls.
         */
    @Override public void dismissSuggestion(android.service.settings.suggestions.Suggestion suggestion) throws android.os.RemoteException
    {
    }
    /**
         * This is the opposite signal to {@link #dismissSuggestion}, indicating a suggestion has been
         * launched.
         */
    @Override public void launchSuggestion(android.service.settings.suggestions.Suggestion suggestion) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.settings.suggestions.ISuggestionService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.settings.suggestions.ISuggestionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.settings.suggestions.ISuggestionService interface,
     * generating a proxy if needed.
     */
    public static android.service.settings.suggestions.ISuggestionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.settings.suggestions.ISuggestionService))) {
        return ((android.service.settings.suggestions.ISuggestionService)iin);
      }
      return new android.service.settings.suggestions.ISuggestionService.Stub.Proxy(obj);
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
        case TRANSACTION_getSuggestions:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.service.settings.suggestions.Suggestion> _result = this.getSuggestions();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_dismissSuggestion:
        {
          data.enforceInterface(descriptor);
          android.service.settings.suggestions.Suggestion _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.settings.suggestions.Suggestion.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.dismissSuggestion(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_launchSuggestion:
        {
          data.enforceInterface(descriptor);
          android.service.settings.suggestions.Suggestion _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.settings.suggestions.Suggestion.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.launchSuggestion(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.settings.suggestions.ISuggestionService
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
           * Return all available suggestions.
           */
      @Override public java.util.List<android.service.settings.suggestions.Suggestion> getSuggestions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.service.settings.suggestions.Suggestion> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSuggestions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSuggestions();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.service.settings.suggestions.Suggestion.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Dismiss a suggestion. The suggestion will not be included in future {@link #getSuggestions)
           * calls.
           */
      @Override public void dismissSuggestion(android.service.settings.suggestions.Suggestion suggestion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((suggestion!=null)) {
            _data.writeInt(1);
            suggestion.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dismissSuggestion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dismissSuggestion(suggestion);
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
           * This is the opposite signal to {@link #dismissSuggestion}, indicating a suggestion has been
           * launched.
           */
      @Override public void launchSuggestion(android.service.settings.suggestions.Suggestion suggestion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((suggestion!=null)) {
            _data.writeInt(1);
            suggestion.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchSuggestion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().launchSuggestion(suggestion);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.service.settings.suggestions.ISuggestionService sDefaultImpl;
    }
    static final int TRANSACTION_getSuggestions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_dismissSuggestion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_launchSuggestion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.service.settings.suggestions.ISuggestionService impl) {
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
    public static android.service.settings.suggestions.ISuggestionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Return all available suggestions.
       */
  public java.util.List<android.service.settings.suggestions.Suggestion> getSuggestions() throws android.os.RemoteException;
  /**
       * Dismiss a suggestion. The suggestion will not be included in future {@link #getSuggestions)
       * calls.
       */
  public void dismissSuggestion(android.service.settings.suggestions.Suggestion suggestion) throws android.os.RemoteException;
  /**
       * This is the opposite signal to {@link #dismissSuggestion}, indicating a suggestion has been
       * launched.
       */
  public void launchSuggestion(android.service.settings.suggestions.Suggestion suggestion) throws android.os.RemoteException;
}
