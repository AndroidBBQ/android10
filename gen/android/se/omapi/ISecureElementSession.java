/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.se.omapi;
/** @hide */
public interface ISecureElementSession extends android.os.IInterface
{
  /** Default implementation for ISecureElementSession. */
  public static class Default implements android.se.omapi.ISecureElementSession
  {
    /**
         * Returns the ATR of the connected card or null if the ATR is not available
         */
    @Override public byte[] getAtr() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Close the connection with the Secure Element. This will close any
         * channels opened by this application with this Secure Element.
         */
    @Override public void close() throws android.os.RemoteException
    {
    }
    /**
         * Close any channel opened on this session.
         */
    @Override public void closeChannels() throws android.os.RemoteException
    {
    }
    /**
         * Tells if this session is closed.
         *
         * @return <code>true</code> if the session is closed, false otherwise.
         */
    @Override public boolean isClosed() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Opens a connection using the basic channel of the card in the
         * specified reader and returns a channel handle. Selects the specified
         * applet if aid != null.
         * Logical channels cannot be opened with this connection.
         * Use interface method openLogicalChannel() to open a logical channel.
         */
    @Override public android.se.omapi.ISecureElementChannel openBasicChannel(byte[] aid, byte p2, android.se.omapi.ISecureElementListener listener) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Opens a connection using the next free logical channel of the card in the
         * specified reader. Selects the specified applet.
         * Selection of other applets with this connection is not supported.
         */
    @Override public android.se.omapi.ISecureElementChannel openLogicalChannel(byte[] aid, byte p2, android.se.omapi.ISecureElementListener listener) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.se.omapi.ISecureElementSession
  {
    private static final java.lang.String DESCRIPTOR = "android.se.omapi.ISecureElementSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.se.omapi.ISecureElementSession interface,
     * generating a proxy if needed.
     */
    public static android.se.omapi.ISecureElementSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.se.omapi.ISecureElementSession))) {
        return ((android.se.omapi.ISecureElementSession)iin);
      }
      return new android.se.omapi.ISecureElementSession.Stub.Proxy(obj);
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
        case TRANSACTION_getAtr:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.getAtr();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeChannels:
        {
          data.enforceInterface(descriptor);
          this.closeChannels();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isClosed:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isClosed();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_openBasicChannel:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          byte _arg1;
          _arg1 = data.readByte();
          android.se.omapi.ISecureElementListener _arg2;
          _arg2 = android.se.omapi.ISecureElementListener.Stub.asInterface(data.readStrongBinder());
          android.se.omapi.ISecureElementChannel _result = this.openBasicChannel(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_openLogicalChannel:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          byte _arg1;
          _arg1 = data.readByte();
          android.se.omapi.ISecureElementListener _arg2;
          _arg2 = android.se.omapi.ISecureElementListener.Stub.asInterface(data.readStrongBinder());
          android.se.omapi.ISecureElementChannel _result = this.openLogicalChannel(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.se.omapi.ISecureElementSession
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
           * Returns the ATR of the connected card or null if the ATR is not available
           */
      @Override public byte[] getAtr() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAtr, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAtr();
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Close the connection with the Secure Element. This will close any
           * channels opened by this application with this Secure Element.
           */
      @Override public void close() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_close, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().close();
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
           * Close any channel opened on this session.
           */
      @Override public void closeChannels() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeChannels, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeChannels();
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
           * Tells if this session is closed.
           *
           * @return <code>true</code> if the session is closed, false otherwise.
           */
      @Override public boolean isClosed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isClosed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isClosed();
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
           * Opens a connection using the basic channel of the card in the
           * specified reader and returns a channel handle. Selects the specified
           * applet if aid != null.
           * Logical channels cannot be opened with this connection.
           * Use interface method openLogicalChannel() to open a logical channel.
           */
      @Override public android.se.omapi.ISecureElementChannel openBasicChannel(byte[] aid, byte p2, android.se.omapi.ISecureElementListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.se.omapi.ISecureElementChannel _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(aid);
          _data.writeByte(p2);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_openBasicChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openBasicChannel(aid, p2, listener);
          }
          _reply.readException();
          _result = android.se.omapi.ISecureElementChannel.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Opens a connection using the next free logical channel of the card in the
           * specified reader. Selects the specified applet.
           * Selection of other applets with this connection is not supported.
           */
      @Override public android.se.omapi.ISecureElementChannel openLogicalChannel(byte[] aid, byte p2, android.se.omapi.ISecureElementListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.se.omapi.ISecureElementChannel _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(aid);
          _data.writeByte(p2);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_openLogicalChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openLogicalChannel(aid, p2, listener);
          }
          _reply.readException();
          _result = android.se.omapi.ISecureElementChannel.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.se.omapi.ISecureElementSession sDefaultImpl;
    }
    static final int TRANSACTION_getAtr = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_closeChannels = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isClosed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_openBasicChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_openLogicalChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.se.omapi.ISecureElementSession impl) {
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
    public static android.se.omapi.ISecureElementSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns the ATR of the connected card or null if the ATR is not available
       */
  public byte[] getAtr() throws android.os.RemoteException;
  /**
       * Close the connection with the Secure Element. This will close any
       * channels opened by this application with this Secure Element.
       */
  public void close() throws android.os.RemoteException;
  /**
       * Close any channel opened on this session.
       */
  public void closeChannels() throws android.os.RemoteException;
  /**
       * Tells if this session is closed.
       *
       * @return <code>true</code> if the session is closed, false otherwise.
       */
  public boolean isClosed() throws android.os.RemoteException;
  /**
       * Opens a connection using the basic channel of the card in the
       * specified reader and returns a channel handle. Selects the specified
       * applet if aid != null.
       * Logical channels cannot be opened with this connection.
       * Use interface method openLogicalChannel() to open a logical channel.
       */
  public android.se.omapi.ISecureElementChannel openBasicChannel(byte[] aid, byte p2, android.se.omapi.ISecureElementListener listener) throws android.os.RemoteException;
  /**
       * Opens a connection using the next free logical channel of the card in the
       * specified reader. Selects the specified applet.
       * Selection of other applets with this connection is not supported.
       */
  public android.se.omapi.ISecureElementChannel openLogicalChannel(byte[] aid, byte p2, android.se.omapi.ISecureElementListener listener) throws android.os.RemoteException;
}
