/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.textservice;
/**
 * (Per-session) IPC channels from TextServicesManagerService to spell checker client applications.
 * @hide
 */
public interface ITextServicesSessionListener extends android.os.IInterface
{
  /** Default implementation for ITextServicesSessionListener. */
  public static class Default implements com.android.internal.textservice.ITextServicesSessionListener
  {
    @Override public void onServiceConnected(com.android.internal.textservice.ISpellCheckerSession spellCheckerSession) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.textservice.ITextServicesSessionListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.textservice.ITextServicesSessionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.textservice.ITextServicesSessionListener interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.textservice.ITextServicesSessionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.textservice.ITextServicesSessionListener))) {
        return ((com.android.internal.textservice.ITextServicesSessionListener)iin);
      }
      return new com.android.internal.textservice.ITextServicesSessionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onServiceConnected:
        {
          data.enforceInterface(descriptor);
          com.android.internal.textservice.ISpellCheckerSession _arg0;
          _arg0 = com.android.internal.textservice.ISpellCheckerSession.Stub.asInterface(data.readStrongBinder());
          this.onServiceConnected(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.textservice.ITextServicesSessionListener
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
      @Override public void onServiceConnected(com.android.internal.textservice.ISpellCheckerSession spellCheckerSession) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((spellCheckerSession!=null))?(spellCheckerSession.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onServiceConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onServiceConnected(spellCheckerSession);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.textservice.ITextServicesSessionListener sDefaultImpl;
    }
    static final int TRANSACTION_onServiceConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.textservice.ITextServicesSessionListener impl) {
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
    public static com.android.internal.textservice.ITextServicesSessionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onServiceConnected(com.android.internal.textservice.ISpellCheckerSession spellCheckerSession) throws android.os.RemoteException;
}
