/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.se.omapi;
/** @hide */
public interface ISecureElementReader extends android.os.IInterface
{
  /** Default implementation for ISecureElementReader. */
  public static class Default implements android.se.omapi.ISecureElementReader
  {
    /**
         * Returns true if a card is present in the specified reader.
         * Returns false if a card is not present in the specified reader.
         */
    @Override public boolean isSecureElementPresent() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Connects to a secure element in this reader. <br>
         * This method prepares (initialises) the Secure Element for communication
         * before the Session object is returned (e.g. powers the Secure Element by
         * ICC ON if its not already on). There might be multiple sessions opened at
         * the same time on the same reader. The system ensures the interleaving of
         * APDUs between the respective sessions.
         *
         * @return a Session object to be used to create Channels.
         */
    @Override public android.se.omapi.ISecureElementSession openSession() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Close all the sessions opened on this reader. All the channels opened by
         * all these sessions will be closed.
         */
    @Override public void closeSessions() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.se.omapi.ISecureElementReader
  {
    private static final java.lang.String DESCRIPTOR = "android.se.omapi.ISecureElementReader";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.se.omapi.ISecureElementReader interface,
     * generating a proxy if needed.
     */
    public static android.se.omapi.ISecureElementReader asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.se.omapi.ISecureElementReader))) {
        return ((android.se.omapi.ISecureElementReader)iin);
      }
      return new android.se.omapi.ISecureElementReader.Stub.Proxy(obj);
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
        case TRANSACTION_isSecureElementPresent:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSecureElementPresent();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_openSession:
        {
          data.enforceInterface(descriptor);
          android.se.omapi.ISecureElementSession _result = this.openSession();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_closeSessions:
        {
          data.enforceInterface(descriptor);
          this.closeSessions();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.se.omapi.ISecureElementReader
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
           * Returns true if a card is present in the specified reader.
           * Returns false if a card is not present in the specified reader.
           */
      @Override public boolean isSecureElementPresent() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSecureElementPresent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSecureElementPresent();
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
      /**
           * Connects to a secure element in this reader. <br>
           * This method prepares (initialises) the Secure Element for communication
           * before the Session object is returned (e.g. powers the Secure Element by
           * ICC ON if its not already on). There might be multiple sessions opened at
           * the same time on the same reader. The system ensures the interleaving of
           * APDUs between the respective sessions.
           *
           * @return a Session object to be used to create Channels.
           */
      @Override public android.se.omapi.ISecureElementSession openSession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.se.omapi.ISecureElementSession _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openSession();
          }
          _reply.readException();
          _result = android.se.omapi.ISecureElementSession.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Close all the sessions opened on this reader. All the channels opened by
           * all these sessions will be closed.
           */
      @Override public void closeSessions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSessions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSessions();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.se.omapi.ISecureElementReader sDefaultImpl;
    }
    static final int TRANSACTION_isSecureElementPresent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_openSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_closeSessions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.se.omapi.ISecureElementReader impl) {
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
    public static android.se.omapi.ISecureElementReader getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns true if a card is present in the specified reader.
       * Returns false if a card is not present in the specified reader.
       */
  public boolean isSecureElementPresent() throws android.os.RemoteException;
  /**
       * Connects to a secure element in this reader. <br>
       * This method prepares (initialises) the Secure Element for communication
       * before the Session object is returned (e.g. powers the Secure Element by
       * ICC ON if its not already on). There might be multiple sessions opened at
       * the same time on the same reader. The system ensures the interleaving of
       * APDUs between the respective sessions.
       *
       * @return a Session object to be used to create Channels.
       */
  public android.se.omapi.ISecureElementSession openSession() throws android.os.RemoteException;
  /**
       * Close all the sessions opened on this reader. All the channels opened by
       * all these sessions will be closed.
       */
  public void closeSessions() throws android.os.RemoteException;
}
