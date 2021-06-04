/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/** {@hide} */
public interface IUidObserver extends android.os.IInterface
{
  /** Default implementation for IUidObserver. */
  public static class Default implements android.app.IUidObserver
  {
    // WARNING: when these transactions are updated, check if they are any callers on the native
    // side. If so, make sure they are using the correct transaction ids and arguments.
    // If a transaction which will also be used on the native side is being inserted, add it to
    // below block of transactions.
    // Since these transactions are also called from native code, these must be kept in sync with
    // the ones in frameworks/native/include/binder/IActivityManager.h
    // =============== Beginning of transactions used on native side as well ======================
    /**
         * Report that there are no longer any processes running for a uid.
         */
    @Override public void onUidGone(int uid, boolean disabled) throws android.os.RemoteException
    {
    }
    /**
         * Report that a uid is now active (no longer idle).
         */
    @Override public void onUidActive(int uid) throws android.os.RemoteException
    {
    }
    /**
         * Report that a uid is idle -- it has either been running in the background for
         * a sufficient period of time, or all of its processes have gone away.
         */
    @Override public void onUidIdle(int uid, boolean disabled) throws android.os.RemoteException
    {
    }
    /**
         * General report of a state change of an uid.
         *
         * @param uid The uid for which the state change is being reported.
         * @param procState The updated process state for the uid.
         * @param procStateSeq The sequence no. associated with process state change of the uid,
         *                     see UidRecord.procStateSeq for details.
         */
    @Override public void onUidStateChanged(int uid, int procState, long procStateSeq) throws android.os.RemoteException
    {
    }
    // =============== End of transactions used on native side as well ============================
    /**
         * Report when the cached state of a uid has changed.
         * If true, a uid has become cached -- that is, it has some active processes that are
         * all in the cached state.  It should be doing as little as possible at this point.
         * If false, that a uid is no longer cached.  This will only be called after
         * onUidCached() has been reported true.  It will happen when either one of its actively
         * running processes is no longer cached, or it no longer has any actively running processes.
         */
    @Override public void onUidCachedChanged(int uid, boolean cached) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IUidObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IUidObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IUidObserver interface,
     * generating a proxy if needed.
     */
    public static android.app.IUidObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IUidObserver))) {
        return ((android.app.IUidObserver)iin);
      }
      return new android.app.IUidObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onUidGone:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onUidGone(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onUidActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onUidActive(_arg0);
          return true;
        }
        case TRANSACTION_onUidIdle:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onUidIdle(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onUidStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          this.onUidStateChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onUidCachedChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onUidCachedChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IUidObserver
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
      // WARNING: when these transactions are updated, check if they are any callers on the native
      // side. If so, make sure they are using the correct transaction ids and arguments.
      // If a transaction which will also be used on the native side is being inserted, add it to
      // below block of transactions.
      // Since these transactions are also called from native code, these must be kept in sync with
      // the ones in frameworks/native/include/binder/IActivityManager.h
      // =============== Beginning of transactions used on native side as well ======================
      /**
           * Report that there are no longer any processes running for a uid.
           */
      @Override public void onUidGone(int uid, boolean disabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((disabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidGone, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidGone(uid, disabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Report that a uid is now active (no longer idle).
           */
      @Override public void onUidActive(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidActive, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidActive(uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Report that a uid is idle -- it has either been running in the background for
           * a sufficient period of time, or all of its processes have gone away.
           */
      @Override public void onUidIdle(int uid, boolean disabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((disabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidIdle, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidIdle(uid, disabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * General report of a state change of an uid.
           *
           * @param uid The uid for which the state change is being reported.
           * @param procState The updated process state for the uid.
           * @param procStateSeq The sequence no. associated with process state change of the uid,
           *                     see UidRecord.procStateSeq for details.
           */
      @Override public void onUidStateChanged(int uid, int procState, long procStateSeq) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(procState);
          _data.writeLong(procStateSeq);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidStateChanged(uid, procState, procStateSeq);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // =============== End of transactions used on native side as well ============================
      /**
           * Report when the cached state of a uid has changed.
           * If true, a uid has become cached -- that is, it has some active processes that are
           * all in the cached state.  It should be doing as little as possible at this point.
           * If false, that a uid is no longer cached.  This will only be called after
           * onUidCached() has been reported true.  It will happen when either one of its actively
           * running processes is no longer cached, or it no longer has any actively running processes.
           */
      @Override public void onUidCachedChanged(int uid, boolean cached) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(((cached)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidCachedChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidCachedChanged(uid, cached);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IUidObserver sDefaultImpl;
    }
    static final int TRANSACTION_onUidGone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onUidActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onUidIdle = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onUidStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onUidCachedChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.app.IUidObserver impl) {
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
    public static android.app.IUidObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // WARNING: when these transactions are updated, check if they are any callers on the native
  // side. If so, make sure they are using the correct transaction ids and arguments.
  // If a transaction which will also be used on the native side is being inserted, add it to
  // below block of transactions.
  // Since these transactions are also called from native code, these must be kept in sync with
  // the ones in frameworks/native/include/binder/IActivityManager.h
  // =============== Beginning of transactions used on native side as well ======================
  /**
       * Report that there are no longer any processes running for a uid.
       */
  public void onUidGone(int uid, boolean disabled) throws android.os.RemoteException;
  /**
       * Report that a uid is now active (no longer idle).
       */
  public void onUidActive(int uid) throws android.os.RemoteException;
  /**
       * Report that a uid is idle -- it has either been running in the background for
       * a sufficient period of time, or all of its processes have gone away.
       */
  public void onUidIdle(int uid, boolean disabled) throws android.os.RemoteException;
  /**
       * General report of a state change of an uid.
       *
       * @param uid The uid for which the state change is being reported.
       * @param procState The updated process state for the uid.
       * @param procStateSeq The sequence no. associated with process state change of the uid,
       *                     see UidRecord.procStateSeq for details.
       */
  public void onUidStateChanged(int uid, int procState, long procStateSeq) throws android.os.RemoteException;
  // =============== End of transactions used on native side as well ============================
  /**
       * Report when the cached state of a uid has changed.
       * If true, a uid has become cached -- that is, it has some active processes that are
       * all in the cached state.  It should be doing as little as possible at this point.
       * If false, that a uid is no longer cached.  This will only be called after
       * onUidCached() has been reported true.  It will happen when either one of its actively
       * running processes is no longer cached, or it no longer has any actively running processes.
       */
  public void onUidCachedChanged(int uid, boolean cached) throws android.os.RemoteException;
}
