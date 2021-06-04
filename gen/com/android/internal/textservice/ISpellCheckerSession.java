/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.textservice;
/**
 * @hide
 */
public interface ISpellCheckerSession extends android.os.IInterface
{
  /** Default implementation for ISpellCheckerSession. */
  public static class Default implements com.android.internal.textservice.ISpellCheckerSession
  {
    @Override public void onGetSuggestionsMultiple(android.view.textservice.TextInfo[] textInfos, int suggestionsLimit, boolean multipleWords) throws android.os.RemoteException
    {
    }
    @Override public void onGetSentenceSuggestionsMultiple(android.view.textservice.TextInfo[] textInfos, int suggestionsLimit) throws android.os.RemoteException
    {
    }
    @Override public void onCancel() throws android.os.RemoteException
    {
    }
    @Override public void onClose() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.textservice.ISpellCheckerSession
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.textservice.ISpellCheckerSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.textservice.ISpellCheckerSession interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.textservice.ISpellCheckerSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.textservice.ISpellCheckerSession))) {
        return ((com.android.internal.textservice.ISpellCheckerSession)iin);
      }
      return new com.android.internal.textservice.ISpellCheckerSession.Stub.Proxy(obj);
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
        case TRANSACTION_onGetSuggestionsMultiple:
        {
          data.enforceInterface(descriptor);
          android.view.textservice.TextInfo[] _arg0;
          _arg0 = data.createTypedArray(android.view.textservice.TextInfo.CREATOR);
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onGetSuggestionsMultiple(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onGetSentenceSuggestionsMultiple:
        {
          data.enforceInterface(descriptor);
          android.view.textservice.TextInfo[] _arg0;
          _arg0 = data.createTypedArray(android.view.textservice.TextInfo.CREATOR);
          int _arg1;
          _arg1 = data.readInt();
          this.onGetSentenceSuggestionsMultiple(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onCancel:
        {
          data.enforceInterface(descriptor);
          this.onCancel();
          return true;
        }
        case TRANSACTION_onClose:
        {
          data.enforceInterface(descriptor);
          this.onClose();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.textservice.ISpellCheckerSession
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
      @Override public void onGetSuggestionsMultiple(android.view.textservice.TextInfo[] textInfos, int suggestionsLimit, boolean multipleWords) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(textInfos, 0);
          _data.writeInt(suggestionsLimit);
          _data.writeInt(((multipleWords)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetSuggestionsMultiple, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetSuggestionsMultiple(textInfos, suggestionsLimit, multipleWords);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGetSentenceSuggestionsMultiple(android.view.textservice.TextInfo[] textInfos, int suggestionsLimit) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(textInfos, 0);
          _data.writeInt(suggestionsLimit);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetSentenceSuggestionsMultiple, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetSentenceSuggestionsMultiple(textInfos, suggestionsLimit);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCancel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCancel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCancel();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onClose() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onClose, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onClose();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.textservice.ISpellCheckerSession sDefaultImpl;
    }
    static final int TRANSACTION_onGetSuggestionsMultiple = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGetSentenceSuggestionsMultiple = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onCancel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onClose = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.internal.textservice.ISpellCheckerSession impl) {
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
    public static com.android.internal.textservice.ISpellCheckerSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onGetSuggestionsMultiple(android.view.textservice.TextInfo[] textInfos, int suggestionsLimit, boolean multipleWords) throws android.os.RemoteException;
  public void onGetSentenceSuggestionsMultiple(android.view.textservice.TextInfo[] textInfos, int suggestionsLimit) throws android.os.RemoteException;
  public void onCancel() throws android.os.RemoteException;
  public void onClose() throws android.os.RemoteException;
}
