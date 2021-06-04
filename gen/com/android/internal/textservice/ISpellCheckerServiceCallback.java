/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.textservice;
/**
 * IPC channels from SpellCheckerService to TextServicesManagerService.
 * @hide
 */
public interface ISpellCheckerServiceCallback extends android.os.IInterface
{
  /** Default implementation for ISpellCheckerServiceCallback. */
  public static class Default implements com.android.internal.textservice.ISpellCheckerServiceCallback
  {
    // TODO: Currently SpellCheckerSession just ignores null newSession and continues waiting for
    // the next onSessionCreated with non-null newSession, which is supposed to never happen if
    // the system is working normally. We should at least free up resources in SpellCheckerSession.
    // Note: This method is called from non-system processes, in theory we cannot assume that
    // this method is always be called only once with non-null value.

    @Override public void onSessionCreated(com.android.internal.textservice.ISpellCheckerSession newSession) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.textservice.ISpellCheckerServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.textservice.ISpellCheckerServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.textservice.ISpellCheckerServiceCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.textservice.ISpellCheckerServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.textservice.ISpellCheckerServiceCallback))) {
        return ((com.android.internal.textservice.ISpellCheckerServiceCallback)iin);
      }
      return new com.android.internal.textservice.ISpellCheckerServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSessionCreated:
        {
          data.enforceInterface(descriptor);
          com.android.internal.textservice.ISpellCheckerSession _arg0;
          _arg0 = com.android.internal.textservice.ISpellCheckerSession.Stub.asInterface(data.readStrongBinder());
          this.onSessionCreated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.textservice.ISpellCheckerServiceCallback
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
      // TODO: Currently SpellCheckerSession just ignores null newSession and continues waiting for
      // the next onSessionCreated with non-null newSession, which is supposed to never happen if
      // the system is working normally. We should at least free up resources in SpellCheckerSession.
      // Note: This method is called from non-system processes, in theory we cannot assume that
      // this method is always be called only once with non-null value.

      @Override public void onSessionCreated(com.android.internal.textservice.ISpellCheckerSession newSession) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((newSession!=null))?(newSession.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSessionCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSessionCreated(newSession);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.textservice.ISpellCheckerServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSessionCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.textservice.ISpellCheckerServiceCallback impl) {
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
    public static com.android.internal.textservice.ISpellCheckerServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // TODO: Currently SpellCheckerSession just ignores null newSession and continues waiting for
  // the next onSessionCreated with non-null newSession, which is supposed to never happen if
  // the system is working normally. We should at least free up resources in SpellCheckerSession.
  // Note: This method is called from non-system processes, in theory we cannot assume that
  // this method is always be called only once with non-null value.

  public void onSessionCreated(com.android.internal.textservice.ISpellCheckerSession newSession) throws android.os.RemoteException;
}
