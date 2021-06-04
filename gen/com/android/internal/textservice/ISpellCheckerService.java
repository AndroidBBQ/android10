/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.textservice;
/**
 * IPC channels from TextServicesManagerService to SpellCheckerService.
 * @hide
 */
public interface ISpellCheckerService extends android.os.IInterface
{
  /** Default implementation for ISpellCheckerService. */
  public static class Default implements com.android.internal.textservice.ISpellCheckerService
  {
    /**
         * Called from the system when an application is requesting a new spell checker session.
         *
         * <p>Note: This is an internal protocol used by the system to establish spell checker sessions,
         * which is not guaranteed to be stable and is subject to change.</p>
         *
         * @param locale locale to be returned from
         *               {@link android.service.textservice.SpellCheckerService.Session#getLocale()}
         * @param listener IPC channel object to be used to implement
         *                 {@link android.service.textservice.SpellCheckerService.Session#onGetSuggestionsMultiple(TextInfo[], int, boolean)} and
         *                 {@link android.service.textservice.SpellCheckerService.Session#onGetSuggestions(TextInfo, int)}
         * @param bundle bundle to be returned from {@link android.service.textservice.SpellCheckerService.Session#getBundle()}
         * @param callback IPC channel to return the result to the caller in an asynchronous manner
         */
    @Override public void getISpellCheckerSession(java.lang.String locale, com.android.internal.textservice.ISpellCheckerSessionListener listener, android.os.Bundle bundle, com.android.internal.textservice.ISpellCheckerServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.textservice.ISpellCheckerService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.textservice.ISpellCheckerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.textservice.ISpellCheckerService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.textservice.ISpellCheckerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.textservice.ISpellCheckerService))) {
        return ((com.android.internal.textservice.ISpellCheckerService)iin);
      }
      return new com.android.internal.textservice.ISpellCheckerService.Stub.Proxy(obj);
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
        case TRANSACTION_getISpellCheckerSession:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.textservice.ISpellCheckerSessionListener _arg1;
          _arg1 = com.android.internal.textservice.ISpellCheckerSessionListener.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          com.android.internal.textservice.ISpellCheckerServiceCallback _arg3;
          _arg3 = com.android.internal.textservice.ISpellCheckerServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.getISpellCheckerSession(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.textservice.ISpellCheckerService
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
           * Called from the system when an application is requesting a new spell checker session.
           *
           * <p>Note: This is an internal protocol used by the system to establish spell checker sessions,
           * which is not guaranteed to be stable and is subject to change.</p>
           *
           * @param locale locale to be returned from
           *               {@link android.service.textservice.SpellCheckerService.Session#getLocale()}
           * @param listener IPC channel object to be used to implement
           *                 {@link android.service.textservice.SpellCheckerService.Session#onGetSuggestionsMultiple(TextInfo[], int, boolean)} and
           *                 {@link android.service.textservice.SpellCheckerService.Session#onGetSuggestions(TextInfo, int)}
           * @param bundle bundle to be returned from {@link android.service.textservice.SpellCheckerService.Session#getBundle()}
           * @param callback IPC channel to return the result to the caller in an asynchronous manner
           */
      @Override public void getISpellCheckerSession(java.lang.String locale, com.android.internal.textservice.ISpellCheckerSessionListener listener, android.os.Bundle bundle, com.android.internal.textservice.ISpellCheckerServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(locale);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          if ((bundle!=null)) {
            _data.writeInt(1);
            bundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getISpellCheckerSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getISpellCheckerSession(locale, listener, bundle, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.textservice.ISpellCheckerService sDefaultImpl;
    }
    static final int TRANSACTION_getISpellCheckerSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.textservice.ISpellCheckerService impl) {
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
    public static com.android.internal.textservice.ISpellCheckerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called from the system when an application is requesting a new spell checker session.
       *
       * <p>Note: This is an internal protocol used by the system to establish spell checker sessions,
       * which is not guaranteed to be stable and is subject to change.</p>
       *
       * @param locale locale to be returned from
       *               {@link android.service.textservice.SpellCheckerService.Session#getLocale()}
       * @param listener IPC channel object to be used to implement
       *                 {@link android.service.textservice.SpellCheckerService.Session#onGetSuggestionsMultiple(TextInfo[], int, boolean)} and
       *                 {@link android.service.textservice.SpellCheckerService.Session#onGetSuggestions(TextInfo, int)}
       * @param bundle bundle to be returned from {@link android.service.textservice.SpellCheckerService.Session#getBundle()}
       * @param callback IPC channel to return the result to the caller in an asynchronous manner
       */
  public void getISpellCheckerSession(java.lang.String locale, com.android.internal.textservice.ISpellCheckerSessionListener listener, android.os.Bundle bundle, com.android.internal.textservice.ISpellCheckerServiceCallback callback) throws android.os.RemoteException;
}
