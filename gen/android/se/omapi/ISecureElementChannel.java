/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.se.omapi;
/** @hide */
public interface ISecureElementChannel extends android.os.IInterface
{
  /** Default implementation for ISecureElementChannel. */
  public static class Default implements android.se.omapi.ISecureElementChannel
  {
    /**
         * Closes the specified connection and frees internal resources.
         * A logical channel will be closed.
         */
    @Override public void close() throws android.os.RemoteException
    {
    }
    /**
         * Tells if this channel is closed.
         *
         * @return <code>true</code> if the channel is closed,
         *         <code>false</code> otherwise.
         */
    @Override public boolean isClosed() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns a boolean telling if this channel is the basic channel.
         *
         * @return <code>true</code> if this channel is a basic channel.
         *         <code>false</code> if this channel is a logical channel.
         */
    @Override public boolean isBasicChannel() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the data as received from the application select command
         * inclusively the status word. The returned byte array contains the data
         * bytes in the following order:
         * [<first data byte>, ..., <last data byte>, <sw1>, <sw2>]
         */
    @Override public byte[] getSelectResponse() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Transmits the specified command APDU and returns the response APDU.
         * MANAGE channel commands are not supported.
         * Selection of applets is not supported in logical channels.
         */
    @Override public byte[] transmit(byte[] command) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Performs a selection of the next Applet on this channel that matches to
         * the partial AID specified in the openBasicChannel(byte[] aid) or
         * openLogicalChannel(byte[] aid) method. This mechanism can be used by a
         * device application to iterate through all Applets matching to the same
         * partial AID.
         * If selectNext() returns true a new Applet was successfully selected on
         * this channel.
         * If no further Applet exists with matches to the partial AID this method
         * returns false and the already selected Applet stays selected.
         *
         * @return <code>true</code> if new Applet was successfully selected.
         *         <code>false</code> if no further Applet exists which matches the
         *         partial AID.
         */
    @Override public boolean selectNext() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.se.omapi.ISecureElementChannel
  {
    private static final java.lang.String DESCRIPTOR = "android.se.omapi.ISecureElementChannel";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.se.omapi.ISecureElementChannel interface,
     * generating a proxy if needed.
     */
    public static android.se.omapi.ISecureElementChannel asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.se.omapi.ISecureElementChannel))) {
        return ((android.se.omapi.ISecureElementChannel)iin);
      }
      return new android.se.omapi.ISecureElementChannel.Stub.Proxy(obj);
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
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
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
        case TRANSACTION_isBasicChannel:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isBasicChannel();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSelectResponse:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.getSelectResponse();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_transmit:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          byte[] _result = this.transmit(_arg0);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_selectNext:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.selectNext();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.se.omapi.ISecureElementChannel
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
           * Closes the specified connection and frees internal resources.
           * A logical channel will be closed.
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
           * Tells if this channel is closed.
           *
           * @return <code>true</code> if the channel is closed,
           *         <code>false</code> otherwise.
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
           * Returns a boolean telling if this channel is the basic channel.
           *
           * @return <code>true</code> if this channel is a basic channel.
           *         <code>false</code> if this channel is a logical channel.
           */
      @Override public boolean isBasicChannel() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBasicChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBasicChannel();
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
           * Returns the data as received from the application select command
           * inclusively the status word. The returned byte array contains the data
           * bytes in the following order:
           * [<first data byte>, ..., <last data byte>, <sw1>, <sw2>]
           */
      @Override public byte[] getSelectResponse() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSelectResponse, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSelectResponse();
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
           * Transmits the specified command APDU and returns the response APDU.
           * MANAGE channel commands are not supported.
           * Selection of applets is not supported in logical channels.
           */
      @Override public byte[] transmit(byte[] command) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(command);
          boolean _status = mRemote.transact(Stub.TRANSACTION_transmit, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().transmit(command);
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
           * Performs a selection of the next Applet on this channel that matches to
           * the partial AID specified in the openBasicChannel(byte[] aid) or
           * openLogicalChannel(byte[] aid) method. This mechanism can be used by a
           * device application to iterate through all Applets matching to the same
           * partial AID.
           * If selectNext() returns true a new Applet was successfully selected on
           * this channel.
           * If no further Applet exists with matches to the partial AID this method
           * returns false and the already selected Applet stays selected.
           *
           * @return <code>true</code> if new Applet was successfully selected.
           *         <code>false</code> if no further Applet exists which matches the
           *         partial AID.
           */
      @Override public boolean selectNext() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_selectNext, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().selectNext();
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
      public static android.se.omapi.ISecureElementChannel sDefaultImpl;
    }
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isClosed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isBasicChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getSelectResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_transmit = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_selectNext = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.se.omapi.ISecureElementChannel impl) {
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
    public static android.se.omapi.ISecureElementChannel getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Closes the specified connection and frees internal resources.
       * A logical channel will be closed.
       */
  public void close() throws android.os.RemoteException;
  /**
       * Tells if this channel is closed.
       *
       * @return <code>true</code> if the channel is closed,
       *         <code>false</code> otherwise.
       */
  public boolean isClosed() throws android.os.RemoteException;
  /**
       * Returns a boolean telling if this channel is the basic channel.
       *
       * @return <code>true</code> if this channel is a basic channel.
       *         <code>false</code> if this channel is a logical channel.
       */
  public boolean isBasicChannel() throws android.os.RemoteException;
  /**
       * Returns the data as received from the application select command
       * inclusively the status word. The returned byte array contains the data
       * bytes in the following order:
       * [<first data byte>, ..., <last data byte>, <sw1>, <sw2>]
       */
  public byte[] getSelectResponse() throws android.os.RemoteException;
  /**
       * Transmits the specified command APDU and returns the response APDU.
       * MANAGE channel commands are not supported.
       * Selection of applets is not supported in logical channels.
       */
  public byte[] transmit(byte[] command) throws android.os.RemoteException;
  /**
       * Performs a selection of the next Applet on this channel that matches to
       * the partial AID specified in the openBasicChannel(byte[] aid) or
       * openLogicalChannel(byte[] aid) method. This mechanism can be used by a
       * device application to iterate through all Applets matching to the same
       * partial AID.
       * If selectNext() returns true a new Applet was successfully selected on
       * this channel.
       * If no further Applet exists with matches to the partial AID this method
       * returns false and the already selected Applet stays selected.
       *
       * @return <code>true</code> if new Applet was successfully selected.
       *         <code>false</code> if no further Applet exists which matches the
       *         partial AID.
       */
  public boolean selectNext() throws android.os.RemoteException;
}
