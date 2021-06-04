/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.textservice;
/**
 * Interface to the text service manager.
 * @hide
 */
public interface ITextServicesManager extends android.os.IInterface
{
  /** Default implementation for ITextServicesManager. */
  public static class Default implements com.android.internal.textservice.ITextServicesManager
  {
    @Override public android.view.textservice.SpellCheckerInfo getCurrentSpellChecker(int userId, java.lang.String locale) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.view.textservice.SpellCheckerSubtype getCurrentSpellCheckerSubtype(int userId, boolean allowImplicitlySelectedSubtype) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void getSpellCheckerService(int userId, java.lang.String sciId, java.lang.String locale, com.android.internal.textservice.ITextServicesSessionListener tsListener, com.android.internal.textservice.ISpellCheckerSessionListener scListener, android.os.Bundle bundle) throws android.os.RemoteException
    {
    }
    @Override public void finishSpellCheckerService(int userId, com.android.internal.textservice.ISpellCheckerSessionListener listener) throws android.os.RemoteException
    {
    }
    @Override public boolean isSpellCheckerEnabled(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.view.textservice.SpellCheckerInfo[] getEnabledSpellCheckers(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.textservice.ITextServicesManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.textservice.ITextServicesManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.textservice.ITextServicesManager interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.textservice.ITextServicesManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.textservice.ITextServicesManager))) {
        return ((com.android.internal.textservice.ITextServicesManager)iin);
      }
      return new com.android.internal.textservice.ITextServicesManager.Stub.Proxy(obj);
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
        case TRANSACTION_getCurrentSpellChecker:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.view.textservice.SpellCheckerInfo _result = this.getCurrentSpellChecker(_arg0, _arg1);
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
        case TRANSACTION_getCurrentSpellCheckerSubtype:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.view.textservice.SpellCheckerSubtype _result = this.getCurrentSpellCheckerSubtype(_arg0, _arg1);
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
        case TRANSACTION_getSpellCheckerService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          com.android.internal.textservice.ITextServicesSessionListener _arg3;
          _arg3 = com.android.internal.textservice.ITextServicesSessionListener.Stub.asInterface(data.readStrongBinder());
          com.android.internal.textservice.ISpellCheckerSessionListener _arg4;
          _arg4 = com.android.internal.textservice.ISpellCheckerSessionListener.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.getSpellCheckerService(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_finishSpellCheckerService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.textservice.ISpellCheckerSessionListener _arg1;
          _arg1 = com.android.internal.textservice.ISpellCheckerSessionListener.Stub.asInterface(data.readStrongBinder());
          this.finishSpellCheckerService(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_isSpellCheckerEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isSpellCheckerEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getEnabledSpellCheckers:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.textservice.SpellCheckerInfo[] _result = this.getEnabledSpellCheckers(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.textservice.ITextServicesManager
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
      @Override public android.view.textservice.SpellCheckerInfo getCurrentSpellChecker(int userId, java.lang.String locale) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.textservice.SpellCheckerInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(locale);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentSpellChecker, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentSpellChecker(userId, locale);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.textservice.SpellCheckerInfo.CREATOR.createFromParcel(_reply);
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
      @Override public android.view.textservice.SpellCheckerSubtype getCurrentSpellCheckerSubtype(int userId, boolean allowImplicitlySelectedSubtype) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.textservice.SpellCheckerSubtype _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(((allowImplicitlySelectedSubtype)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentSpellCheckerSubtype, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentSpellCheckerSubtype(userId, allowImplicitlySelectedSubtype);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.textservice.SpellCheckerSubtype.CREATOR.createFromParcel(_reply);
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
      @Override public void getSpellCheckerService(int userId, java.lang.String sciId, java.lang.String locale, com.android.internal.textservice.ITextServicesSessionListener tsListener, com.android.internal.textservice.ISpellCheckerSessionListener scListener, android.os.Bundle bundle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(sciId);
          _data.writeString(locale);
          _data.writeStrongBinder((((tsListener!=null))?(tsListener.asBinder()):(null)));
          _data.writeStrongBinder((((scListener!=null))?(scListener.asBinder()):(null)));
          if ((bundle!=null)) {
            _data.writeInt(1);
            bundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSpellCheckerService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getSpellCheckerService(userId, sciId, locale, tsListener, scListener, bundle);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void finishSpellCheckerService(int userId, com.android.internal.textservice.ISpellCheckerSessionListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSpellCheckerService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSpellCheckerService(userId, listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public boolean isSpellCheckerEnabled(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSpellCheckerEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSpellCheckerEnabled(userId);
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
      @Override public android.view.textservice.SpellCheckerInfo[] getEnabledSpellCheckers(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.textservice.SpellCheckerInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnabledSpellCheckers, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnabledSpellCheckers(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.view.textservice.SpellCheckerInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.internal.textservice.ITextServicesManager sDefaultImpl;
    }
    static final int TRANSACTION_getCurrentSpellChecker = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getCurrentSpellCheckerSubtype = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getSpellCheckerService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_finishSpellCheckerService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isSpellCheckerEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getEnabledSpellCheckers = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(com.android.internal.textservice.ITextServicesManager impl) {
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
    public static com.android.internal.textservice.ITextServicesManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.view.textservice.SpellCheckerInfo getCurrentSpellChecker(int userId, java.lang.String locale) throws android.os.RemoteException;
  public android.view.textservice.SpellCheckerSubtype getCurrentSpellCheckerSubtype(int userId, boolean allowImplicitlySelectedSubtype) throws android.os.RemoteException;
  public void getSpellCheckerService(int userId, java.lang.String sciId, java.lang.String locale, com.android.internal.textservice.ITextServicesSessionListener tsListener, com.android.internal.textservice.ISpellCheckerSessionListener scListener, android.os.Bundle bundle) throws android.os.RemoteException;
  public void finishSpellCheckerService(int userId, com.android.internal.textservice.ISpellCheckerSessionListener listener) throws android.os.RemoteException;
  public boolean isSpellCheckerEnabled(int userId) throws android.os.RemoteException;
  public android.view.textservice.SpellCheckerInfo[] getEnabledSpellCheckers(int userId) throws android.os.RemoteException;
}
