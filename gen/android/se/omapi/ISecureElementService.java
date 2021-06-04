/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.se.omapi;
/**
 * SecureElement service interface.
 * @hide
 */
public interface ISecureElementService extends android.os.IInterface
{
  /** Default implementation for ISecureElementService. */
  public static class Default implements android.se.omapi.ISecureElementService
  {
    /**
         * Returns the friendly names of available Secure Element readers.
         */
    @Override public java.lang.String[] getReaders() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns SecureElement Service reader object to the given name.
         */
    @Override public android.se.omapi.ISecureElementReader getReader(java.lang.String reader) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Checks if the application defined by the package name is allowed to
         * receive NFC transaction events for the defined AID.
         */
    @Override public boolean[] isNFCEventAllowed(java.lang.String reader, byte[] aid, java.lang.String[] packageNames) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.se.omapi.ISecureElementService
  {
    private static final java.lang.String DESCRIPTOR = "android.se.omapi.ISecureElementService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.se.omapi.ISecureElementService interface,
     * generating a proxy if needed.
     */
    public static android.se.omapi.ISecureElementService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.se.omapi.ISecureElementService))) {
        return ((android.se.omapi.ISecureElementService)iin);
      }
      return new android.se.omapi.ISecureElementService.Stub.Proxy(obj);
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
        case TRANSACTION_getReaders:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getReaders();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getReader:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.se.omapi.ISecureElementReader _result = this.getReader(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_isNFCEventAllowed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          boolean[] _result = this.isNFCEventAllowed(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeBooleanArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.se.omapi.ISecureElementService
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
           * Returns the friendly names of available Secure Element readers.
           */
      @Override public java.lang.String[] getReaders() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getReaders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getReaders();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Returns SecureElement Service reader object to the given name.
           */
      @Override public android.se.omapi.ISecureElementReader getReader(java.lang.String reader) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.se.omapi.ISecureElementReader _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(reader);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getReader, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getReader(reader);
          }
          _reply.readException();
          _result = android.se.omapi.ISecureElementReader.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Checks if the application defined by the package name is allowed to
           * receive NFC transaction events for the defined AID.
           */
      @Override public boolean[] isNFCEventAllowed(java.lang.String reader, byte[] aid, java.lang.String[] packageNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(reader);
          _data.writeByteArray(aid);
          _data.writeStringArray(packageNames);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNFCEventAllowed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNFCEventAllowed(reader, aid, packageNames);
          }
          _reply.readException();
          _result = _reply.createBooleanArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.se.omapi.ISecureElementService sDefaultImpl;
    }
    static final int TRANSACTION_getReaders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getReader = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isNFCEventAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.se.omapi.ISecureElementService impl) {
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
    public static android.se.omapi.ISecureElementService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns the friendly names of available Secure Element readers.
       */
  public java.lang.String[] getReaders() throws android.os.RemoteException;
  /**
       * Returns SecureElement Service reader object to the given name.
       */
  public android.se.omapi.ISecureElementReader getReader(java.lang.String reader) throws android.os.RemoteException;
  /**
       * Checks if the application defined by the package name is allowed to
       * receive NFC transaction events for the defined AID.
       */
  public boolean[] isNFCEventAllowed(java.lang.String reader, byte[] aid, java.lang.String[] packageNames) throws android.os.RemoteException;
}
