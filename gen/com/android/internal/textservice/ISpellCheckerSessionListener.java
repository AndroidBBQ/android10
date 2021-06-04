/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.textservice;
/**
 * @hide
 */
public interface ISpellCheckerSessionListener extends android.os.IInterface
{
  /** Default implementation for ISpellCheckerSessionListener. */
  public static class Default implements com.android.internal.textservice.ISpellCheckerSessionListener
  {
    @Override public void onGetSuggestions(android.view.textservice.SuggestionsInfo[] results) throws android.os.RemoteException
    {
    }
    @Override public void onGetSentenceSuggestions(android.view.textservice.SentenceSuggestionsInfo[] result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.textservice.ISpellCheckerSessionListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.textservice.ISpellCheckerSessionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.textservice.ISpellCheckerSessionListener interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.textservice.ISpellCheckerSessionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.textservice.ISpellCheckerSessionListener))) {
        return ((com.android.internal.textservice.ISpellCheckerSessionListener)iin);
      }
      return new com.android.internal.textservice.ISpellCheckerSessionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onGetSuggestions:
        {
          data.enforceInterface(descriptor);
          android.view.textservice.SuggestionsInfo[] _arg0;
          _arg0 = data.createTypedArray(android.view.textservice.SuggestionsInfo.CREATOR);
          this.onGetSuggestions(_arg0);
          return true;
        }
        case TRANSACTION_onGetSentenceSuggestions:
        {
          data.enforceInterface(descriptor);
          android.view.textservice.SentenceSuggestionsInfo[] _arg0;
          _arg0 = data.createTypedArray(android.view.textservice.SentenceSuggestionsInfo.CREATOR);
          this.onGetSentenceSuggestions(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.textservice.ISpellCheckerSessionListener
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
      @Override public void onGetSuggestions(android.view.textservice.SuggestionsInfo[] results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(results, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetSuggestions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetSuggestions(results);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGetSentenceSuggestions(android.view.textservice.SentenceSuggestionsInfo[] result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(result, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetSentenceSuggestions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetSentenceSuggestions(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.textservice.ISpellCheckerSessionListener sDefaultImpl;
    }
    static final int TRANSACTION_onGetSuggestions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGetSentenceSuggestions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.internal.textservice.ISpellCheckerSessionListener impl) {
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
    public static com.android.internal.textservice.ISpellCheckerSessionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onGetSuggestions(android.view.textservice.SuggestionsInfo[] results) throws android.os.RemoteException;
  public void onGetSentenceSuggestions(android.view.textservice.SentenceSuggestionsInfo[] result) throws android.os.RemoteException;
}
